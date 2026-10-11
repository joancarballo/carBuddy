// Sección "Calendario" de la web: listado de calendarios (cambiar nombre,
// borrar y añadir otro en una ventana con las indicaciones de Google) y tabla
// con los eventos de hoy y mañana.

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

// Tabla de los eventos de un día, como en la pantalla: los que ya han terminado
// salen tachados. Con varios calendarios hay una columna con el de cada evento.
String dayTable(const std::vector<CalendarEvent>& list, int64_t day, CivilTime::Minutes now, bool showCalendar) {
  String html = "<table><tr><th>Hora</th><th>Evento</th>";
  if (showCalendar) {
    html += "<th>Calendario</th>";
  }
  html += "</tr>";

  int count = 0;
  for (size_t i = 0; i < list.size(); i++) {
    const CalendarEvent& e = list[i];
    if (e.day != day) {
      continue;
    }
    html += e.end <= now ? "<tr class=\"past\">" : "<tr>";
    html += "<td>" + String(timeRangeText(e).c_str()) + "</td>";
    html += "<td>" + Web::htmlEscape(e.title.c_str()) + "</td>";
    if (showCalendar) {
      html += "<td>" + Web::htmlEscape(e.calendar.c_str()) + "</td>";
    }
    html += "</tr>";
    count++;
  }
  if (count == 0) {
    html += "<tr><td colspan=\"3\" class=\"muted\">Sin eventos</td></tr>";
  }
  return html + "</table>";
}

// Vista previa de hoy y mañana. El botón "Actualizar ahora" va junto al título de hoy.
String daysPreview(bool showCalendar) {
  struct tm t;
  if (!TimeService::now(t)) {
    return String();
  }
  const CivilTime::Minutes now = CivilTime::fromTm(t);
  const int64_t today = CivilTime::dayOf(now);
  const std::vector<CalendarEvent> list = CalendarService::events();

  String html =
      "<div class=\"row\"><h3>Eventos de hoy</h3>"
      "<form method=\"POST\" action=\"/calendar/refresh\"><button type=\"submit\" class=\"small\">Actualizar ahora</button></form>"
      "</div>";
  html += dayTable(list, today, now, showCalendar);
  html += "<div class=\"row\"><h3>Eventos de mañana</h3></div>";
  html += dayTable(list, today + 1, now, showCalendar);
  return html;
}

// Mientras descarga, la página pregunta el estado cada 2 s y se recarga al terminar.
const char* RELOAD_WHEN_DONE =
    "<script>setInterval(function(){fetch('/calendar/status')"
    ".then(function(r){return r.text()})"
    ".then(function(s){if(s=='ok'||s=='error'){location.reload();}});},2000);</script>";

// Cambiar el nombre: el navegador pregunta el nombre nuevo (con el actual ya
// escrito) y se envía el formulario; si se cancela o se deja vacío, no se envía.
const char* RENAME_SCRIPT =
    "<script>function rn(f){var e=f.elements['name'];"
    "var v=prompt('Nuevo nombre del calendario:',e.value);"
    "if(!v||!v.trim()){return false;}e.value=v.trim();return true;}</script>";

// Ventana para añadir un calendario: formulario, indicaciones, Guardar y Cancelar.
// Las URL guardadas no se muestran nunca: son secretas.
String addDialog() {
  return String(
             "<dialog id=\"add-cal\"><h2>Añadir un calendario</h2>"
             "<form method=\"POST\" action=\"/calendar/add\">"
             "<p>Nombre (opcional):<br><input name=\"name\" maxlength=\"24\" placeholder=\"Familia\"></p>"
             "<p>URL iCal privada:<br><input name=\"url\" type=\"url\" required "
             "placeholder=\"https://calendar.google.com/calendar/ical/.../basic.ics\"></p>"
             "<h3>Cómo conseguir la URL en Google Calendar</h3>") +
         HOW_TO +
         "<div class=\"actions\">"
         "<button type=\"button\" class=\"ghost\" onclick=\"this.form.reset();this.closest('dialog').close()\">Cancelar</button>"
         "<button type=\"submit\">Guardar</button>"
         "</div></form></dialog>";
}

// Botón que abre la ventana para añadir un calendario.
String addButton(bool first) {
  return String("<button type=\"button\" class=\"small\" onclick=\"document.getElementById('add-cal').showModal()\">") +
         (first ? "+ Añadir un calendario" : "+ Añadir otro calendario") + "</button>";
}

// Listado de calendarios: nombre y estado, y en la misma línea los botones
// pequeños para cambiar el nombre y borrarlo. Al final, el de añadir otro.
String calendarList(const std::vector<CalendarService::CalendarInfo>& cals) {
  String html = "<ul class=\"cals\">";
  for (size_t i = 0; i < cals.size(); i++) {
    const String name = Web::htmlEscape(cals[i].name);
    const String index = "<input type=\"hidden\" name=\"i\" value=\"" + String(i) + "\">";

    html += "<li class=\"cal\"><span class=\"grow\"><b>" + name + "</b> · ";
    if (cals[i].error.length() > 0) {
      html += "error (" + Web::htmlEscape(cals[i].error) + ")";
    } else {
      html += String(cals[i].eventCount) + " eventos hoy y mañana";
    }
    html += "</span>";
    html += "<form method=\"POST\" action=\"/calendar/rename\" onsubmit=\"return rn(this)\">" + index +
            "<input type=\"hidden\" name=\"name\" value=\"" + name + "\">"
            "<button type=\"submit\" class=\"small\">Editar nombre</button></form>";
    html += "<form method=\"POST\" action=\"/calendar/delete\" data-n=\"" + name +
            "\" onsubmit=\"return confirm('¿Borrar el calendario '+this.dataset.n+'?')\">" + index +
            "<button type=\"submit\" class=\"small danger\">Borrar calendario</button></form></li>";
  }

  html += "<li class=\"cal\">";
  if ((int)cals.size() < CalendarService::MAX_CALENDARS) {
    html += addButton(cals.empty());
  } else {
    html += "<span class=\"muted\">Máximo de " + String(CalendarService::MAX_CALENDARS) +
            " calendarios. Borra uno para añadir otro.</span>";
  }
  html += "</li></ul>";
  return html;
}

}  // namespace

namespace CalendarSection {

String html() {
  const CalendarService::Status st = CalendarService::status();
  const std::vector<CalendarService::CalendarInfo> cals = CalendarService::calendars();

  String html;
  html.reserve(6144);
  html += "<h2 id=\"calendario\">Calendario</h2>";
  html += "<p>" + statusText(st) + "</p>";
  html += calendarList(cals);
  if (!cals.empty() && st.lastOk != 0) {
    html += daysPreview(cals.size() > 1);
  }
  html += addDialog();
  html += RENAME_SCRIPT;

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
