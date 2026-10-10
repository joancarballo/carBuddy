#include "apps/clock/ClockApp.h"

#include <string.h>

#include "core/Display.h"
#include "net/Net.h"
#include "services/TimeService.h"
#include "services/TimeZones.h"

void ClockApp::draw() {
  U8G2& g = Display::gfx();
  struct tm t;

  if (!TimeService::now(t)) {
    g.setFont(u8g2_font_6x10_tf);
    Display::textCentered(28, "Sin hora");
    Display::textCentered(42, "Esperando a internet");
    return;
  }

  // Hora en grande. Los dos puntos se dibujan a mano para no depender de la fuente.
  g.setFont(u8g2_font_logisoso32_tn);
  const int digitsWidth = g.getStrWidth("88");
  const int colonWidth = 10;
  const int gap = 4;
  const int totalWidth = digitsWidth * 2 + colonWidth + gap * 2;
  const int x = (Display::WIDTH - totalWidth) / 2;
  const int baseline = 42;

  char hh[3];
  char mm[3];
  snprintf(hh, sizeof(hh), "%02d", t.tm_hour);
  snprintf(mm, sizeof(mm), "%02d", t.tm_min);

  g.drawStr(x, baseline, hh);
  const int colonX = x + digitsWidth + gap + (colonWidth - 4) / 2;
  g.drawBox(colonX, baseline - 24, 4, 4);
  g.drawBox(colonX, baseline - 12, 4, 4);
  g.drawStr(x + digitsWidth + gap * 2 + colonWidth, baseline, mm);

  // Fecha debajo
  g.setFont(u8g2_font_6x10_tf);
  Display::textCentered(60, TimeService::dateText().c_str());
}

String ClockApp::webSection() {
  String html;
  html.reserve(4096);

  html += "<h2>Time App</h2>";
  html += "<p>Hora: <b id=\"t\">" + TimeService::timeText() + "</b><br>";
  html += "Fecha: <b id=\"d\">" + TimeService::dateText() + "</b></p>";

  // Al elegir un huso horario se guarda automáticamente (envía el formulario).
  html +=
      "<form method=\"POST\" action=\"/time/tz\">"
      "<p>Huso horario:<br>"
      "<select name=\"tz\" onchange=\"this.form.submit()\">";
  for (size_t i = 0; i < TIME_ZONE_COUNT; i++) {
    html += "<option value=\"";
    html += TIME_ZONES[i].id;
    html += "\"";
    if (strcmp(TIME_ZONES[i].id, TimeService::zoneId()) == 0) {
      html += " selected";
    }
    html += ">";
    html += TIME_ZONES[i].label;
    html += "</option>";
  }
  html += "</select></p><noscript><button type=\"submit\">Guardar</button></noscript></form>";

  // Actualiza la hora y la fecha de la página cada 5 segundos.
  html +=
      "<script>setInterval(function(){fetch('/time/now')"
      ".then(function(r){return r.text()})"
      ".then(function(s){var p=s.split('|');"
      "document.getElementById('t').textContent=p[0];"
      "document.getElementById('d').textContent=p[1];});},5000);</script>";

  return html;
}

void ClockApp::registerRoutes() {
  Net::server().on("/time/tz", HTTP_POST, []() {
    WebServer& s = Net::server();
    TimeService::setZone(s.arg("tz"));
    s.sendHeader("Location", "/");
    s.send(303, "text/plain", "");
  });

  Net::server().on("/time/now", HTTP_GET, []() {
    String body = TimeService::timeText() + "|" + TimeService::dateText();
    Net::server().send(200, "text/plain; charset=utf-8", body);
  });
}
