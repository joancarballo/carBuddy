#include "net/Net.h"

#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>

namespace {

const uint32_t CONNECT_TIMEOUT_MS = 15000;      // espera al arrancar antes de crear la red de configuración
const uint32_t AP_FALLBACK_MS = 2UL * 60 * 1000;  // sin Wi-Fi durante 2 min: se vuelve a crear
const uint32_t RETRY_MS = 30000;                  // en modo configuración, reintento de la Wi-Fi guardada

WebServer webServer(80);
Preferences prefs;
bool apMode = false;
bool mdnsStarted = false;
String savedSsid;
String savedPass;
uint32_t lastConnectedMs = 0;  // última vez que se vio la Wi-Fi conectada
uint32_t lastRetryMs = 0;
bool restartPending = false;
uint32_t restartAt = 0;
String (*pageBuilder)() = nullptr;
void (*waitCallback)() = nullptr;

String pageStart() {
  return "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
         "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
         "<title>carBuddy</title></head><body><h1>carBuddy</h1>";
}

String pageEnd() {
  return "</body></html>";
}

// Formulario para introducir la wifi. El nombre de la red guardada aparece
// rellenado; la contraseña nunca se muestra.
String formHtml() {
  String html =
      "<form method=\"POST\" action=\"/save\">"
      "<p>Nombre de la wifi (SSID):<br>"
      "<input name=\"ssid\" maxlength=\"32\" required value=\"";
  html += Net::htmlEscape(savedSsid);
  html +=
      "\"></p>"
      "<p>Contrase&ntilde;a:<br><input name=\"pass\" type=\"password\" maxlength=\"63\"></p>"
      "<p><button type=\"submit\">Guardar y reiniciar</button></p>"
      "</form>";
  return html;
}

// mDNS (carbuddy.local) solo funciona conectado a una Wi-Fi; se arranca una vez.
void startMdns() {
  if (mdnsStarted) {
    return;
  }
  if (MDNS.begin(Net::MDNS_NAME)) {
    MDNS.addService("http", "tcp", 80);
    mdnsStarted = true;
    Serial.printf("Tambien disponible en http://%s.local\n", Net::MDNS_NAME);
  }
}

// Crea la red abierta de configuración. Si hay una Wi-Fi guardada, la placa
// sigue intentando conectarse a ella a la vez (modo AP + estación).
void startConfigAp() {
  apMode = true;
  if (savedSsid.length() > 0) {
    // Los reintentos los hace loop(), solo cuando nadie usa la red de
    // configuración: cada intento cambia de canal y cortaría al teléfono.
    WiFi.setAutoReconnect(false);
    WiFi.mode(WIFI_AP_STA);
  } else {
    WiFi.mode(WIFI_AP);
  }
  WiFi.softAP(Net::AP_SSID);
  lastRetryMs = millis();
  Serial.printf("Modo configuracion: red \"%s\", IP %s\n", Net::AP_SSID, WiFi.softAPIP().toString().c_str());
}

// La Wi-Fi guardada ha vuelto: se apaga la red de configuración.
void stopConfigAp() {
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  apMode = false;
  Serial.printf("Wifi recuperada. IP: %s\n", WiFi.localIP().toString().c_str());
  startMdns();
}

// Vigila la Wi-Fi: sin conexión durante AP_FALLBACK_MS crea la red de
// configuración; en modo configuración reintenta y la apaga al conectar.
void watchWifi() {
  if (savedSsid.length() == 0) {
    return;  // sin Wi-Fi guardada solo existe la red de configuración
  }
  const bool connected = WiFi.status() == WL_CONNECTED;

  if (!apMode) {
    if (connected) {
      lastConnectedMs = millis();
    } else if (millis() - lastConnectedMs >= AP_FALLBACK_MS) {
      Serial.println("Sin wifi durante 2 minutos.");
      startConfigAp();
    }
    return;
  }

  if (connected) {
    stopConfigAp();
    lastConnectedMs = millis();
  } else if (WiFi.softAPgetStationNum() == 0 && millis() - lastRetryMs >= RETRY_MS) {
    lastRetryMs = millis();
    WiFi.begin(savedSsid.c_str(), savedPass.c_str());
  }
}

void handleRoot() {
  String html = pageStart();
  if (pageBuilder != nullptr) {
    html += pageBuilder();
  }
  html += pageEnd();
  webServer.send(200, "text/html", html);
}

void handleSave() {
  String ssid = webServer.arg("ssid");
  String pass = webServer.arg("pass");
  ssid.trim();

  if (ssid.length() == 0) {
    webServer.send(400, "text/plain", "Falta el nombre de la wifi");
    return;
  }

  prefs.begin("wifi", false);
  prefs.putString("ssid", ssid);
  prefs.putString("pass", pass);
  prefs.end();

  Serial.printf("Wifi guardada: \"%s\". Reiniciando...\n", ssid.c_str());
  webServer.send(200, "text/html",
                 "<!DOCTYPE html><html><head><meta charset=\"utf-8\"></head>"
                 "<body><p>Guardado. La placa se reinicia y se conecta a la nueva wifi.</p></body></html>");

  restartPending = true;
  restartAt = millis() + 1500;
}

void handleNotFound() {
  webServer.send(404, "text/plain", "No encontrado");
}

}  // namespace

namespace Net {

void setWaitCallback(void (*callback)()) {
  waitCallback = callback;
}

void begin() {
  prefs.begin("wifi", true);
  savedSsid = prefs.getString("ssid", "");
  savedPass = prefs.getString("pass", "");
  prefs.end();

  bool connected = false;
  if (savedSsid.length() > 0) {
    Serial.printf("Conectando a la wifi \"%s\"...\n", savedSsid.c_str());
    WiFi.setHostname(MDNS_NAME);
    WiFi.mode(WIFI_STA);
    WiFi.begin(savedSsid.c_str(), savedPass.c_str());
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < CONNECT_TIMEOUT_MS) {
      if (waitCallback != nullptr) {
        waitCallback();
      }
      delay(50);
    }
    connected = (WiFi.status() == WL_CONNECTED);
  } else {
    Serial.println("No hay ninguna wifi guardada.");
  }

  if (connected) {
    apMode = false;
    lastConnectedMs = millis();
    WiFi.setAutoReconnect(true);
    Serial.printf("Conectado. IP: %s\n", WiFi.localIP().toString().c_str());
    startMdns();
  } else {
    startConfigAp();
  }

  webServer.on("/", HTTP_GET, handleRoot);
  webServer.on("/save", HTTP_POST, handleSave);
  webServer.onNotFound(handleNotFound);
  webServer.begin();
}

void loop() {
  webServer.handleClient();
  watchWifi();
  if (restartPending && (int32_t)(millis() - restartAt) >= 0) {
    ESP.restart();
  }
}

WebServer& server() {
  return webServer;
}

void setPageBuilder(String (*builder)()) {
  pageBuilder = builder;
}

bool isApMode() {
  return apMode;
}

bool isConnected() {
  return !apMode && WiFi.status() == WL_CONNECTED;
}

String ssid() {
  return savedSsid;
}

String ip() {
  if (apMode) {
    return WiFi.softAPIP().toString();
  }
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.localIP().toString();
  }
  return "";
}

String hostName() {
  return String(MDNS_NAME) + ".local";
}

String htmlEscape(const String& text) {
  String out;
  out.reserve(text.length() + 8);
  for (size_t i = 0; i < text.length(); i++) {
    char c = text[i];
    switch (c) {
      case '&': out += "&amp;"; break;
      case '<': out += "&lt;"; break;
      case '>': out += "&gt;"; break;
      case '"': out += "&quot;"; break;
      default: out += c;
    }
  }
  return out;
}

String wifiSectionHtml() {
  String html;
  if (savedSsid.length() == 0) {
    // Todavía no hay nada guardado: se muestra el formulario directamente.
    html += "<p>No hay ninguna wifi guardada.</p>";
    html += formHtml();
    return html;
  }

  html += "<p>Wifi: " + htmlEscape(savedSsid) + "</p>";
  if (isConnected()) {
    html += "<p>IP: " + WiFi.localIP().toString() + "</p>";
    html += "<p>Nombre: " + hostName() + "</p>";
  } else if (apMode) {
    html += "<p>Sin conexi&oacute;n con esta wifi. La placa lo vuelve a intentar cada 30 s.</p>";
    html += "<p>IP de configuraci&oacute;n: " + WiFi.softAPIP().toString() + "</p>";
  } else {
    html += "<p>Desconectada. La placa est&aacute; intentando reconectar; si no lo consigue en 2 minutos, "
            "crea la red " + String(AP_SSID) + " para configurarla.</p>";
  }
  // El formulario se despliega en la misma página.
  html += "<details><summary>Editar</summary>";
  html += formHtml();
  html += "</details>";
  return html;
}

}  // namespace Net
