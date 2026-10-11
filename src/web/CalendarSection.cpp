// Sección "Calendario" de la web: calendarios guardados, indicaciones para
// conseguir la URL iCal de Google, formulario para añadir otro y vista previa
// de los eventos de hoy y mañana.

#include "services/TimeService.h"
#include "services/calendar/CalendarService.h"
#include "web/Sections.h"
#include "web/Web.h"

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
      text = "Error al descargar: " + Web::htmlEscape(st.error) + ".";
      break;
  }
  if (st.lastOk != 0) {
    text += " Se muestran los datos de las " + clockText(st.lastOk) + ".";
  }
  return text;
}

// Lista de los eventos de hoy y de mañana, como en la pantalla: los que ya han
// terminado salen tachados y los de todo el día, sin horas. Con varios
// calendarios, cada evento lleva entre paréntesis el suyo.
String daysPreview(bool showCalendar) {
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
      items += Web::htmlEscape(e.title.c_str());
      if (showCalendar) {
        items += " (" + Web::htmlEscape(e.calendar.c_str()) + ")";
      }
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

// Lista de calendarios guardados, con su estado, un desplegable para cambiar
// el nombre y un botón para quitarlos.
String calendarList(const std::vector<CalendarService::CalendarInfo>& cals) {
  String html = "<p>Calendarios:</p><ul>";
  for (size_t i = 0; i < cals.size(); i++) {
    const String name = Web::htmlEscape(cals[i].name);
    const String index = "<input type=\"hidden\" name=\"i\" value=\"" + String(i) + "\">";

    html += "<li><b>" + name + "</b>: ";
    if (cals[i].error.length() > 0) {
      html += "error (" + Web::htmlEscape(cals[i].error) + ")";
    } else {
      html += String(cals[i].eventCount) + " eventos hoy y mañana";
    }
    html += "<details><summary>Editar nombre</summary>"
            "<form method=\"POST\" action=\"/calendar/rename\">" + index +
            "<input name=\"name\" maxlength=\"24\" required value=\"" + name + "\"> "
            "<button type=\"submit\">Guardar</button></form></details>";
    html += "<form method=\"POST\" action=\"/calendar/delete\" onsubmit=\"return confirm('¿Quitar " + name +
            "?')\">" + index + "<button type=\"submit\" class=\"danger\">Quitar</button></form></li>";
  }
  html += "</ul>";
  return html;
}

}  // namespace

namespace CalendarSection {

String html() {
  const CalendarService::Status st = CalendarService::status();
  const std::vector<CalendarService::CalendarInfo> cals = CalendarService::calendars();
  const bool hasCalendars = !cals.empty();

  String html;
  html.reserve(4096);
  html += "<h2 id=\"calendario\">Calendario</h2>";
  html += "<p>" + statusText(st) + "</p>";

  if (hasCalendars) {
    html += calendarList(cals);
    if (st.lastOk != 0) {
      html += daysPreview(cals.size() > 1);
    }
    html += "<form method=\"POST\" action=\"/calendar/refresh\"><button type=\"submit\">Actualizar ahora</button></form>";
  }

  // Desplegable para añadir un calendario: formulario e indicaciones. Sin
  // ningún calendario va abierto. Las URL guardadas no se muestran nunca: son secretas.
  if ((int)cals.size() < CalendarService::MAX_CALENDARS) {
    html += hasCalendars ? "<details><summary>Añadir otro calendario</summary>"
                         : "<details open><summary>Añadir un calendario</summary>";
    html +=
        "<form method=\"POST\" action=\"/calendar/add\">"
        "<p>Nombre (opcional):<br><input name=\"name\" maxlength=\"24\" placeholder=\"Familia\"></p>"
        "<p>URL iCal privada:<br><input name=\"url\" type=\"url\" size=\"50\" required "
        "placeholder=\"https://calendar.google.com/calendar/ical/.../basic.ics\"></p>"
        "<p><button type=\"submit\">Añadir y descargar</button></p></form>";
    html += "<p>Cómo conseguir la URL en Google Calendar:</p>";
    html += HOW_TO;
    html += "</details>";
  } else {
    html += "<p>Ya hay " + String(CalendarService::MAX_CALENDARS) +
            " calendarios, el máximo. Quita uno para añadir otro.</p>";
  }

  if (st.state == CalendarService::State::Downloading || st.state == CalendarService::State::Waiting) {
    html += RELOAD_WHEN_DONE;
  }
  return html;
}

void registerRoutes() {
  Web::server().on("/calendar/add", HTTP_POST, []() {
    WebServer& s = Web::server();
    String name = s.arg("name");
    String url = s.arg("url");
    name.trim();
    url.trim();
    // Google a veces da la dirección como webcal://, que es lo mismo por HTTPS.
    if (url.startsWith("webcal://")) {
      url = "https://" + url.substring(9);
    }
    if (!url.startsWith("https://")) {
      s.send(400, "text/plain; charset=utf-8", "La URL tiene que empezar por https://");
      return;
    }
    if (!CalendarService::addCalendar(name.substring(0, 24), url)) {
      s.send(400, "text/plain; charset=utf-8", "Ya está el máximo de calendarios");
      return;
    }
    Web::redirectHome("calendario");
  });

  Web::server().on("/calendar/rename", HTTP_POST, []() {
    String name = Web::server().arg("name");
    name.trim();
    if (name.length() > 0) {
      CalendarService::renameCalendar(Web::server().arg("i").toInt(), name.substring(0, 24));
    }
    Web::redirectHome("calendario");
  });

  Web::server().on("/calendar/delete", HTTP_POST, []() {
    CalendarService::removeCalendar(Web::server().arg("i").toInt());
    Web::redirectHome("calendario");
  });

  Web::server().on("/calendar/refresh", HTTP_POST, []() {
    CalendarService::refreshNow();
    Web::redirectHome("calendario");
  });

  // Estado para el script de la página: "nourl", "waiting", "downloading", "ok" o "error".
  Web::server().on("/calendar/status", HTTP_GET, []() {
    static const char* const NAMES[] = {"nourl", "waiting", "downloading", "ok", "error"};
    Web::server().send(200, "text/plain", NAMES[(int)CalendarService::status().state]);
  });
}

}  // namespace CalendarSection
