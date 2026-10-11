// Detalle de un evento del calendario: título, día y horas, calendario (si hay
// varios), lugar, repetición y notas, en líneas que se recorren girando el dial.

#include "apps/calendar/CalendarApp.h"

#include "core/Display.h"
#include "services/calendar/CalendarService.h"

namespace {

const int FIRST_BASELINE = 8;  // la fila 0 es la línea del segundo nivel
const int LINE_HEIGHT = 9;
const int VISIBLE_LINES = 7;
const int TEXT_WIDTH = 122;  // a la derecha quedan las flechas
const int ARROWS_TOP = 2;
const int ARROWS_BOTTOM = 62;

void addWrapped(std::vector<String>& lines, const String& text) {
  std::vector<String> wrapped = Display::wrap(text, TEXT_WIDTH);
  lines.insert(lines.end(), wrapped.begin(), wrapped.end());
}

String whenText(const CalendarEvent& ev, int64_t today) {
  String day = ev.day == today ? "Hoy" : (ev.day == today + 1 ? "Mañana" : "Otro día");
  if (ev.allDay) {
    return day + ", todo el día";
  }
  const int s = CivilTime::minuteOfDay(ev.start);
  const int e = CivilTime::minuteOfDay(ev.end);
  char buf[24];
  snprintf(buf, sizeof(buf), ", %02d:%02d - %02d:%02d", s / 60, s % 60, e / 60, e % 60);
  return day + buf;
}

}  // namespace

void CalendarApp::openDetail(const CalendarEvent& ev, int64_t today) {
  // Las líneas se parten con la misma fuente con la que se dibujan.
  Display::gfx().setFont(u8g2_font_5x8_tf);

  detailLines_.clear();
  addWrapped(detailLines_, ev.title.c_str());
  addWrapped(detailLines_, whenText(ev, today));
  // Con varios calendarios se indica de cuál viene.
  if (CalendarService::calendars().size() > 1 && !ev.calendar.empty()) {
    addWrapped(detailLines_, String("Calendario: ") + ev.calendar.c_str());
  }
  if (!ev.location.empty()) {
    addWrapped(detailLines_, String("Lugar: ") + ev.location.c_str());
  }
  if (!ev.repeat.empty()) {
    addWrapped(detailLines_, String("Repite: ") + ev.repeat.c_str());
  }
  if (!ev.notes.empty()) {
    detailLines_.push_back("");
    addWrapped(detailLines_, ev.notes.c_str());
  }

  titleLines_ = Display::wrap(ev.title.c_str(), TEXT_WIDTH).size();
  detailTop_ = 0;
  detailOpen_ = true;
}

void CalendarApp::scrollDetail(int step) {
  const int maxTop = (int)detailLines_.size() - VISIBLE_LINES;
  detailTop_ += step;
  if (detailTop_ > maxTop) {
    detailTop_ = maxTop;
  }
  if (detailTop_ < 0) {
    detailTop_ = 0;
  }
}

void CalendarApp::drawDetail() {
  U8G2& g = Display::gfx();
  for (int i = 0; i < VISIBLE_LINES && detailTop_ + i < (int)detailLines_.size(); i++) {
    const int line = detailTop_ + i;
    const int baseline = FIRST_BASELINE + i * LINE_HEIGHT;
    Display::text(0, baseline, detailLines_[line]);
    // El título va subrayado para distinguirlo del resto.
    if (line < titleLines_) {
      g.drawHLine(0, baseline + 1, Display::textWidth(detailLines_[line]));
    }
  }
  drawArrows(ARROWS_TOP, ARROWS_BOTTOM, detailTop_ > 0,
             detailTop_ + VISIBLE_LINES < (int)detailLines_.size());
}
