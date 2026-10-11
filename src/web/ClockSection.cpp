// Sección "Time App" de la web: hora y fecha de la placa (se actualizan solas)
// y desplegable de husos horarios, que se guarda al elegir uno.

#include <string.h>

#include "services/TimeService.h"
#include "services/TimeZones.h"
#include "web/Sections.h"
#include "web/Web.h"

namespace ClockSection {

String html() {
  String html;
  html.reserve(4096);

  html += "<h2 id=\"hora\">Time App</h2>";
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

void registerRoutes() {
  Web::server().on("/time/tz", HTTP_POST, []() {
    TimeService::setZone(Web::server().arg("tz"));
    Web::redirectHome("hora");
  });

  Web::server().on("/time/now", HTTP_GET, []() {
    String body = TimeService::timeText() + "|" + TimeService::dateText();
    Web::server().send(200, "text/plain; charset=utf-8", body);
  });
}

}  // namespace ClockSection
