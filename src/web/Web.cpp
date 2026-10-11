#include "web/Web.h"

#include "net/Net.h"
#include "web/Sections.h"

namespace {

WebServer webServer(80);

String pageStart() {
  return "<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
         "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
         "<title>carBuddy</title></head><body><h1>carBuddy</h1>";
}

String pageEnd() {
  return "</body></html>";
}

// Página principal: las secciones en el mismo orden que las apps en la pantalla.
// Sin Wi-Fi, la de la Wi-Fi va la primera, que es la que hace falta.
void handleRoot() {
  String html = pageStart();
  if (!Net::isConnected()) {
    html += WifiSection::html() + "<hr>";
  }
  html += ClockSection::html() + "<hr>";
  html += CalendarSection::html() + "<hr>";
  if (Net::isConnected()) {
    html += WifiSection::html() + "<hr>";
  }
  html += pageEnd();
  webServer.send(200, "text/html", html);
}

void handleNotFound() {
  webServer.send(404, "text/plain", "No encontrado");
}

}  // namespace

namespace Web {

void begin() {
  webServer.on("/", HTTP_GET, handleRoot);
  WifiSection::registerRoutes();
  ClockSection::registerRoutes();
  CalendarSection::registerRoutes();
  webServer.onNotFound(handleNotFound);
  webServer.begin();
}

void loop() {
  webServer.handleClient();
}

WebServer& server() {
  return webServer;
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

void redirectHome(const char* anchor) {
  webServer.sendHeader("Location", String("/#") + anchor);
  webServer.send(303, "text/plain", "");
}

}  // namespace Web
