// Sección "Calendario" de la web: indicaciones para conseguir la URL iCal de
// Google, formulario para pegarla y vista previa de los eventos de hoy y mañana.

#include "apps/calendar/CalendarApp.h"

#include "net/Net.h"
#include "services/TimeService.h"
#include "services/calendar/CalendarService.h"

namespace {

const char* HOW_TO =
    "<ol>"
    "<li>Abre Google Calendar en el ordenador (calendar.google.com).</li>"
    "<li>Pulsa la rueda dentada y entra en <b>Configuración</b>.</li>"
    "<li>A la izquierda, en <b>Configuración de mis calendarios</b>, elige tu calendario.</li>"
    "<li>Baja hasta <b>Integrar el calendario</b>.</li>"
    "<li>Copia la <b>Dirección secreta en formato iCal</b> (acaba en <i>basic.ics</i>) y pégala aquí.</li>"
    "</ol>"
    "<p>No compartas esa dirección: quien la tenga puede ver tu calendario. "
    "Google puede tardar unas horas en reflejar los cambios en ella.</p>";

String clockText(time_t t) {
  struct tm tm;
  localtime_r(&t, &tm);
  char buf[8];
  snprintf(buf, sizeof(buf), "%02d:%02d", tm.tm_hour, tm.tm_min);
  return String(buf);
}

String statusText(const CalendarService::Status& st) {
  String text;
  switch (st.state) {
    case CalendarService::State::NoUrl:
      return "Todavía no hay ningún calendario.";
    case CalendarService::State::Waiting:
      text = "Esperando a tener Wi-Fi y hora para descargar.";
      break;
    case CalendarService::State::Downloading:
      text = "Descargando...";
      break;
    case CalendarService::State::Ok:
      return "Última descarga: " + clockText(st.lastOk) + " (" + String(st.eventCount) +
             " eventos entre hoy y mañana).";
    case CalendarService::State::Error:
      text = "Error al descargar: " + Net::htmlEscape(st.error) + ".";
      break;
  }
  if (st.lastOk != 0) {
    text += " Se muestran los datos de las " + clockText(st.lastOk) + ".";
  }
  return text;
}

// Lista de los eventos de hoy y de mañana, como en la pantalla: los que ya han
// terminado salen tachados y los de todo el día, sin horas.
String daysPreview() {
  struct tm t;
  if (!TimeService::now(t)) {
    return String();
  }
  const CivilTime::Minutes now = CivilTime::fromTm(t);
  const int64_t today = CivilTime::dayOf(now);
  std::vector<CalendarEvent> list = CalendarService::events();

  String html;
  for (int d = 0; d < 2; d++) {
    html += d == 0 ? "<p>Eventos de hoy:</p>" : "<p>Eventos de mañana:</p>";
    String items;
    for (size_t i = 0; i < list.size(); i++) {
      const CalendarEvent& e = list[i];
      if (e.day != today + d) {
        continue;
      }
      const bool past = e.end <= now;
      items += past ? "<li><s>" : "<li>";
      if (!e.allDay) {
        items += String(timeRangeText(e).c_str()) + " ";
      }
      items += Net::htmlEscape(e.title.c_str());
      items += past ? "</s></li>" : "</li>";
    }
    html += items.length() > 0 ? "<ul>" + items + "</ul>" : "<ul><li>Sin eventos.</li></ul>";
  }
  return html;
}

// Mientras descarga, la página pregunta el estado cada 2 s y se recarga al terminar.
const char* RELOAD_WHEN_DONE =
    "<script>setInterval(function(){fetch('/calendar/status')"
    ".then(function(r){return r.text()})"
    ".then(function(s){if(s=='ok'||s=='error'){location.reload();}});},2000);</script>";

}  // namespace

String CalendarApp::webSection() {
  const CalendarService::Status st = CalendarService::status();
  const bool hasUrl = CalendarService::hasUrl();

  String html;
  html.reserve(3072);
  html += "<h2 id=\"calendario\">Calendario</h2>";
  html += "<p>" + statusText(st) + "</p>";

  if (hasUrl && st.lastOk != 0) {
    html += daysPreview();
  }
  if (hasUrl) {
    html += "<form method=\"POST\" action=\"/calendar/refresh\"><button type=\"submit\">Actualizar ahora</button></form>";
  }

  // Las indicaciones se ven siempre si aún no hay URL; si ya la hay, plegadas.
  if (hasUrl) {
    html += "<details><summary>Cómo conseguir la URL en Google Calendar</summary>";
    html += HOW_TO;
    html += "</details>";
  } else {
    html += HOW_TO;
  }

  // La URL guardada no se muestra nunca: es secreta.
  html += "<form method=\"POST\" action=\"/calendar/url\"><p>";
  html += hasUrl ? "Ya hay una URL guardada. Para cambiarla, pega la nueva:" : "URL iCal privada:";
  html +=
      "<br><input name=\"url\" type=\"url\" size=\"50\" required "
      "placeholder=\"https://calendar.google.com/calendar/ical/.../basic.ics\"></p>"
      "<p><button type=\"submit\">Guardar y descargar</button></p></form>";

  if (st.state == CalendarService::State::Downloading || st.state == CalendarService::State::Waiting) {
    html += RELOAD_WHEN_DONE;
  }
  return html;
}

void CalendarApp::registerRoutes() {
  Net::server().on("/calendar/url", HTTP_POST, []() {
    WebServer& s = Net::server();
    String url = s.arg("url");
    url.trim();
    // Google a veces da la dirección como webcal://, que es lo mismo por HTTPS.
    if (url.startsWith("webcal://")) {
      url = "https://" + url.substring(9);
    }
    if (!url.startsWith("https://")) {
      s.send(400, "text/plain; charset=utf-8", "La URL tiene que empezar por https://");
      return;
    }
    CalendarService::setUrl(url);
    s.sendHeader("Location", "/#calendario");
    s.send(303, "text/plain", "");
  });

  Net::server().on("/calendar/refresh", HTTP_POST, []() {
    CalendarService::refreshNow();
    Net::server().sendHeader("Location", "/#calendario");
    Net::server().send(303, "text/plain", "");
  });

  // Estado para el script de la página: "nourl", "waiting", "downloading", "ok" o "error".
  Net::server().on("/calendar/status", HTTP_GET, []() {
    static const char* const NAMES[] = {"nourl", "waiting", "downloading", "ok", "error"};
    Net::server().send(200, "text/plain", NAMES[(int)CalendarService::status().state]);
  });
}
