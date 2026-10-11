#include "services/calendar/CalendarService.h"

#include <HTTPClient.h>
#include <NetworkClientSecure.h>
#include <Preferences.h>
#include <WiFi.h>

#include "certs.h"
#include "services/TimeService.h"
#include "services/calendar/EventCollector.h"
#include "services/calendar/IcsParser.h"

namespace {

const uint32_t REFRESH_MS = 15UL * 60 * 1000;   // descarga normal cada 15 min
const uint32_t RETRY_MS = 2UL * 60 * 1000;      // tras un error, reintento a los 2 min
const uint32_t WAKE_MS = 30UL * 1000;           // la tarea mira si toca descargar cada 30 s
const uint32_t READ_TIMEOUT_MS = 20000;         // sin recibir datos durante este tiempo: error
const int DAYS_SHOWN = 2;                       // hoy y mañana

// Todo lo que se comparte entre la tarea de descarga y el resto del programa
// está protegido por este mutex.
SemaphoreHandle_t mutex = nullptr;
TaskHandle_t taskHandle = nullptr;

String url;
std::vector<CalendarEvent> eventList;
uint32_t listVersion = 0;
CalendarService::Status currentStatus = {CalendarService::State::NoUrl, "", 0, 0};

// Solo los usa la tarea de descarga.
volatile bool requested = false;  // se escribe desde la web (otro núcleo)
bool lastFailed = false;
uint32_t lastAttemptMs = 0;
int64_t lastDay = -1;  // día de la última descarga correcta

void lock() {
  xSemaphoreTake(mutex, portMAX_DELAY);
}

void unlock() {
  xSemaphoreGive(mutex);
}

void setState(CalendarService::State state, const String& error = String()) {
  lock();
  currentStatus.state = state;
  currentStatus.error = error;
  unlock();
}

// Resumen de la descarga en el monitor serie, para comprobar que no falta nada:
// tamaño, eventos leídos, cada evento de hoy y mañana y las repeticiones no admitidas.
void logResult(size_t bytes, const IcsParser& parser, const EventCollector& collector,
               const std::vector<CalendarEvent>& list) {
  Serial.printf("Calendario: %u bytes, %d eventos en el archivo, %u hoy y mañana\n", (unsigned)bytes,
                parser.eventsRead(), (unsigned)list.size());
  for (size_t i = 0; i < list.size(); i++) {
    int y, m, d;
    CivilTime::civilFromDays(list[i].day, y, m, d);
    Serial.printf("  %02d/%02d %s %s\n", d, m, timeRangeText(list[i]).c_str(), list[i].title.c_str());
  }
  const std::vector<std::string>& rules = collector.unsupportedRules();
  for (size_t i = 0; i < rules.size(); i++) {
    Serial.printf("  Repetición no admitida (solo se usa la primera fecha): %s\n", rules[i].c_str());
  }
  if (collector.full()) {
    Serial.printf("  Lista llena: se han quedado eventos fuera (máximo %u)\n", (unsigned)EventCollector::MAX_EVENTS);
  }
}

// Descarga el archivo y lo procesa según llega. Devuelve false y rellena error si falla.
bool download(const String& address, int64_t today, std::vector<CalendarEvent>& out, String& error) {
  NetworkClientSecure client;
  client.setCACert(GOOGLE_ROOT_CAS);

  HTTPClient http;
  http.useHTTP10(true);  // sin "chunked": así el cuerpo se lee tal cual llega
  http.setTimeout(READ_TIMEOUT_MS);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  if (!http.begin(client, address)) {
    error = "URL no válida";
    return false;
  }

  const int code = http.GET();
  if (code != HTTP_CODE_OK) {
    error = code < 0 ? HTTPClient::errorToString(code) : "respuesta HTTP " + String(code);
    http.end();
    return false;
  }

  EventCollector collector(today, DAYS_SHOWN);
  IcsParser parser(EventCollector::addCallback, &collector);

  NetworkClient* stream = http.getStreamPtr();
  int remaining = http.getSize();  // -1 si el servidor no dice el tamaño
  uint8_t buffer[1024];
  size_t total = 0;
  uint32_t lastData = millis();

  while (remaining != 0) {
    const size_t available = stream->available();
    if (available > 0) {
      const int n = stream->read(buffer, available < sizeof(buffer) ? available : sizeof(buffer));
      for (int i = 0; i < n; i++) {
        parser.feed((char)buffer[i]);
      }
      total += n;
      if (remaining > 0) {
        remaining -= n;
      }
      lastData = millis();
    } else if (!stream->connected()) {
      break;
    } else if (millis() - lastData > READ_TIMEOUT_MS) {
      error = "se cortó la descarga";
      http.end();
      return false;
    } else {
      vTaskDelay(pdMS_TO_TICKS(2));
    }
  }
  http.end();

  parser.finish();
  out = collector.finish();
  logResult(total, parser, collector, out);
  return true;
}

// Decide si toca descargar: hay URL, Wi-Fi y hora, y se ha pedido, ha pasado
// el tiempo de refresco o ha cambiado el día.
bool shouldDownload(int64_t today, String& address) {
  lock();
  address = url;
  unlock();

  if (address.length() == 0) {
    setState(CalendarService::State::NoUrl);
    return false;
  }
  struct tm now;
  if (WiFi.status() != WL_CONNECTED || !TimeService::now(now)) {
    setState(CalendarService::State::Waiting);
    return false;
  }
  if (requested || lastAttemptMs == 0 || today != lastDay) {
    return true;
  }
  return millis() - lastAttemptMs >= (lastFailed ? RETRY_MS : REFRESH_MS);
}

void task(void*) {
  for (;;) {
    ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(WAKE_MS));

    struct tm now;
    const int64_t today = TimeService::now(now) ? CivilTime::dayOf(CivilTime::fromTm(now)) : -1;
    String address;
    if (!shouldDownload(today, address)) {
      continue;
    }

    requested = false;
    lastAttemptMs = millis();
    setState(CalendarService::State::Downloading);

    std::vector<CalendarEvent> list;
    String error;
    const bool ok = download(address, today, list, error);
    lastFailed = !ok;

    lock();
    // Si mientras tanto se cambió la URL, este resultado ya no sirve.
    const bool stale = (address != url);
    if (ok && !stale) {
      eventList.swap(list);
      listVersion++;
      currentStatus.lastOk = time(nullptr);
      currentStatus.eventCount = eventList.size();
      lastDay = today;
    }
    currentStatus.state = ok ? CalendarService::State::Ok : CalendarService::State::Error;
    currentStatus.error = ok ? String() : error;
    unlock();

    if (!ok) {
      Serial.printf("Calendario: error al descargar (%s)\n", error.c_str());
    }
  }
}

}  // namespace

namespace CalendarService {

void begin() {
  mutex = xSemaphoreCreateMutex();

  Preferences prefs;
  prefs.begin("cal", true);
  url = prefs.getString("url", "");
  prefs.end();
  currentStatus.state = url.length() > 0 ? State::Waiting : State::NoUrl;

  // Núcleo 0: el loop() de Arduino (pantalla y web) va en el núcleo 1.
  // La pila es grande porque la conexión HTTPS la necesita.
  xTaskCreatePinnedToCore(task, "calendar", 16384, nullptr, 1, &taskHandle, 0);
}

bool hasUrl() {
  lock();
  const bool has = url.length() > 0;
  unlock();
  return has;
}

void setUrl(const String& newUrl) {
  Preferences prefs;
  prefs.begin("cal", false);
  prefs.putString("url", newUrl);
  prefs.end();

  lock();
  url = newUrl;
  eventList.clear();
  listVersion++;
  currentStatus.lastOk = 0;
  currentStatus.eventCount = 0;
  currentStatus.state = newUrl.length() > 0 ? State::Waiting : State::NoUrl;
  currentStatus.error = "";
  unlock();

  Serial.println("Calendario: URL guardada");
  refreshNow();
}

void refreshNow() {
  requested = true;
  // Se marca ya como "descargando" para que la web espere al resultado.
  lock();
  if (currentStatus.state == State::Ok || currentStatus.state == State::Error) {
    currentStatus.state = State::Downloading;
  }
  unlock();
  if (taskHandle != nullptr) {
    xTaskNotifyGive(taskHandle);
  }
}

Status status() {
  lock();
  Status s = currentStatus;
  unlock();
  return s;
}

std::vector<CalendarEvent> events() {
  lock();
  std::vector<CalendarEvent> copy = eventList;
  unlock();
  return copy;
}

uint32_t version() {
  lock();
  const uint32_t v = listVersion;
  unlock();
  return v;
}

}  // namespace CalendarService
