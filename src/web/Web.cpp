#include "web/Web.h"

#include "net/Net.h"
#include "web/Sections.h"

namespace {

WebServer webServer(80);

// Estilo de la web: claro, tonos pastel y verde menta (el Peppermint 16-6329 TCX,
// #699E6D, aclarado a #A9DCB4), con formas redondeadas. Va dentro de la página
// porque la placa la sirve sin conexión a internet.
const char* STYLE =
    "<style>"
    ":root{--bg:#f6faf7;--card:#fff;--ink:#2f3d33;--muted:#7a8a7e;--line:#dcebe0;"
    "--mint:#a9dcb4;--mint-soft:#e4f4e8;--mint-ink:#2e5a3a;--rose:#f8dede;--rose-ink:#8a3d3d}"
    "*{box-sizing:border-box}"
    "body{margin:0;padding:24px 16px;background:var(--bg);color:var(--ink);"
    "font:16px/1.5 system-ui,-apple-system,'Segoe UI',sans-serif}"
    "main{max-width:640px;margin:0 auto}"
    "h1{margin:0 0 20px;font-size:1.6rem;color:var(--mint-ink)}"
    "h1::before{content:'';display:inline-block;width:.7em;height:.7em;margin-right:.4em;"
    "border-radius:50%;background:var(--mint)}"
    "section{margin-bottom:16px;padding:20px;background:var(--card);border:1px solid var(--line);"
    "border-radius:20px;box-shadow:0 2px 10px rgba(60,100,70,.06)}"
    "h2{margin:0 0 12px;font-size:1.15rem;color:var(--mint-ink)}"
    "p{margin:8px 0}form{margin:8px 0}ul,ol{padding-left:22px}li{margin:6px 0}"
    "input,select{width:100%;padding:10px 12px;font:inherit;color:inherit;background:#fbfdfb;"
    "border:1px solid var(--line);border-radius:12px}"
    "input:focus,select:focus{outline:2px solid var(--mint);border-color:var(--mint)}"
    "button{padding:9px 18px;font:inherit;font-weight:600;color:var(--mint-ink);background:var(--mint);"
    "border:0;border-radius:999px;cursor:pointer}"
    "button:hover{filter:brightness(.95)}"
    "button.danger{color:var(--rose-ink);background:var(--rose)}"
    "details{margin:12px 0}"
    "summary{display:inline-block;padding:6px 14px;font-weight:600;color:var(--mint-ink);"
    "background:var(--mint-soft);border-radius:999px;cursor:pointer}"
    "details[open]>summary{margin-bottom:10px}"
    "s{color:var(--muted)}a{color:var(--mint-ink)}"
    "</style>";

String pageStart() {
  return String("<!DOCTYPE html><html><head><meta charset=\"utf-8\">"
                "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
                "<title>carBuddy</title>") +
         STYLE + "</head><body><main><h1>carBuddy</h1>";
}

String pageEnd() {
  return "</main></body></html>";
}

// Cada sección va en su propia tarjeta.
String card(const String& html) {
  return "<section>" + html + "</section>";
}

// Página principal: las secciones en el mismo orden que las apps en la pantalla.
// Sin Wi-Fi, la de la Wi-Fi va la primera, que es la que hace falta.
void handleRoot() {
  String html = pageStart();
  if (!Net::isConnected()) {
    html += card(WifiSection::html());
  }
  html += card(ClockSection::html());
  html += card(CalendarSection::html());
  if (Net::isConnected()) {
    html += card(WifiSection::html());
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

String page(const String& body) {
  return pageStart() + card(body) + pageEnd();
}

void redirectHome(const char* anchor) {
  webServer.sendHeader("Location", String("/#") + anchor);
  webServer.send(303, "text/plain", "");
}

}  // namespace Web
