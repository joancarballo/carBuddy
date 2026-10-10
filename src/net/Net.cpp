#include "net/Net.h"

#include <ESPmDNS.h>
#include <Preferences.h>
#include <WiFi.h>

namespace {

const uint32_t CONNECT_TIMEOUT_MS = 15000;

WebServer webServer(80);
Preferences prefs;
bool apMode = false;
String savedSsid;
bool restartPending = false;
uint32_t restartAt = 0;
String (*pageBuilder)() = nullptr;

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
  if (savedSsid.length() > 0) {
    html += "<p><a href=\"/\">Cancelar</a></p>";
  }
  return html;
}

void handleRoot() {
  String html = pageStart();
  if (pageBuilder != nullptr) {
    html += pageBuilder();
  }
  html += pageEnd();
  webServer.send(200, "text/html", html);
}

void handleEdit() {
  String html = pageStart();
  html += "<h2>Settings App</h2>";
  html += formHtml();
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

void begin() {
  prefs.begin("wifi", true);
  savedSsid = prefs.getString("ssid", "");
  String password = prefs.getString("pass", "");
  prefs.end();

  bool connected = false;
  if (savedSsid.length() > 0) {
    Serial.printf("Conectando a la wifi \"%s\"...\n", savedSsid.c_str());
    WiFi.setHostname(MDNS_NAME);
    WiFi.mode(WIFI_STA);
    WiFi.begin(savedSsid.c_str(), password.c_str());
    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < CONNECT_TIMEOUT_MS) {
      delay(250);
    }
    connected = (WiFi.status() == WL_CONNECTED);
  } else {
    Serial.println("No hay ninguna wifi guardada.");
  }

  if (connected) {
    apMode = false;
    WiFi.setAutoReconnect(true);
    Serial.printf("Conectado. IP: %s\n", WiFi.localIP().toString().c_str());
    if (MDNS.begin(MDNS_NAME)) {
      MDNS.addService("http", "tcp", 80);
      Serial.printf("Tambien disponible en http://%s.local\n", MDNS_NAME);
    }
  } else {
    apMode = true;
    WiFi.disconnect(true);
    delay(100);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID);
    Serial.printf("Modo configuracion: red \"%s\", IP %s\n", AP_SSID,
                  WiFi.softAPIP().toString().c_str());
  }

  webServer.on("/", HTTP_GET, handleRoot);
  webServer.on("/edit", HTTP_GET, handleEdit);
  webServer.on("/save", HTTP_POST, handleSave);
  webServer.onNotFound(handleNotFound);
  webServer.begin();
}

void loop() {
  webServer.handleClient();
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
    html += "<p>Sin conexi&oacute;n con esta wifi.</p>";
    html += "<p>IP de configuraci&oacute;n: " + WiFi.softAPIP().toString() + "</p>";
  } else {
    html += "<p>Desconectada. La placa est&aacute; intentando reconectar.</p>";
  }
  html += "<form method=\"GET\" action=\"/edit\"><button type=\"submit\">Editar</button></form>";
  return html;
}

}  // namespace Net
