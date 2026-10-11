#include "apps/calendar/CalendarApp.h"

#include "core/Display.h"
#include "net/Net.h"
#include "services/TimeService.h"
#include "services/calendar/CalendarService.h"

using CivilTime::Minutes;

namespace {

// Medidas de la lista (la fila 0 es del indicador de app).
const int LIST_TOP = 12;      // fila superior de la primera línea de la lista
const int ROW_HEIGHT = 10;
const int VISIBLE_ROWS = 5;
const int CONTENT_WIDTH = 122;  // a la derecha queda la columna de las flechas
const int ARROW_X = 125;        // centro de las flechas

const char* const SHORT_DAYS[] = {"Dom", "Lun", "Mar", "Mié", "Jue", "Vie", "Sáb"};

// Cabecera (y = 1..8) y 5 filas de 10 píxeles: 12..21, 22..31, 32..41, 42..51 y 52..61.
static_assert(LIST_TOP + VISIBLE_ROWS * ROW_HEIGHT <= Display::HEIGHT, "las 5 filas tienen que caber");

// Mensaje que se muestra mientras todavía no hay ningún evento descargado.
void addStatusNotices(std::vector<String>& out) {
  const CalendarService::Status st = CalendarService::status();
  switch (st.state) {
    case CalendarService::State::NoUrl:
      out.push_back("Sin calendario.");
      out.push_back("Pega la URL iCal en la web:");
      out.push_back(Net::hostName());
      break;
    case CalendarService::State::Waiting:
      out.push_back("Esperando Wi-Fi y hora...");
      break;
    case CalendarService::State::Downloading:
    case CalendarService::State::Ok:
      out.push_back("Descargando...");
      break;
    case CalendarService::State::Error:
      out.push_back("Error al descargar:");
      out.push_back(st.error);
      break;
  }
}

}  // namespace

// ---------------------------------------------------------------------------
// Datos y filas
// ---------------------------------------------------------------------------

void CalendarApp::refreshData(int64_t today) {
  const uint32_t v = CalendarService::version();
  const bool newData = (v != dataVersion_);

  // En el primer nivel se reconstruye siempre (así los avisos de estado están
  // al día). Dentro de la app solo si llegan datos nuevos o cambia el día.
  if (inside_ && !newData && today == rowsDay_) {
    return;
  }

  // Evento seleccionado, para volver a seleccionarlo en la lista nueva.
  const bool keep = isSelectable(selected_);
  std::string keepTitle;
  Minutes keepStart = 0;
  if (keep) {
    keepTitle = events_[rows_[selected_].eventIndex].title;
    keepStart = events_[rows_[selected_].eventIndex].start;
  }

  if (newData) {
    events_ = CalendarService::events();
    dataVersion_ = v;
  }
  buildRows(today);

  selected_ = -1;
  for (size_t i = 0; keep && i < rows_.size(); i++) {
    if (rows_[i].type == RowType::Event && events_[rows_[i].eventIndex].title == keepTitle &&
        events_[rows_[i].eventIndex].start == keepStart) {
      selected_ = i;
      keepSelectionVisible();
      break;
    }
  }
}

void CalendarApp::buildRows(int64_t today) {
  rows_.clear();
  rowsDay_ = today;

  if (events_.empty() && CalendarService::status().lastOk == 0) {
    std::vector<String> notices;
    addStatusNotices(notices);
    for (size_t i = 0; i < notices.size(); i++) {
      rows_.push_back(Row{RowType::Notice, -1, notices[i]});
    }
    return;
  }

  // Hoy, el separador "Mañana" y mañana.
  for (int d = 0; d < 2; d++) {
    if (d == 1) {
      rows_.push_back(Row{RowType::Separator, -1, "Mañana"});
    }
    const size_t before = rows_.size();
    for (size_t i = 0; i < events_.size(); i++) {
      if (events_[i].day == today + d) {
        rows_.push_back(Row{RowType::Event, (int)i, String()});
      }
    }
    if (rows_.size() == before) {
      rows_.push_back(Row{RowType::Notice, -1, d == 0 ? "Hoy no hay eventos" : "Sin eventos"});
    }
  }
}

// Fila con la que empieza la lista: el primer evento de hoy que no ha terminado.
// Los de todo el día van arriba del todo; solo se esconden (quedan encima, con la
// flecha ▲) si ya hay eventos de hoy terminados que tampoco caben.
int CalendarApp::firstUpcomingRow(Minutes now) const {
  bool anyPast = false;
  for (size_t i = 0; i < rows_.size(); i++) {
    if (rows_[i].type != RowType::Event) {
      if (rows_[i].type == RowType::Separator) {
        return anyPast ? (int)i : 0;  // hoy ya ha terminado todo
      }
      continue;
    }
    const CalendarEvent& e = events_[rows_[i].eventIndex];
    if (e.allDay) {
      continue;
    }
    if (e.end > now) {
      return anyPast ? (int)i : 0;
    }
    anyPast = true;
  }
  return 0;
}

bool CalendarApp::isSelectable(int row) const {
  return row >= 0 && row < (int)rows_.size() && rows_[row].type == RowType::Event;
}

void CalendarApp::moveSelection(int step) {
  const int count = rows_.size();

  // Primer giro: se selecciona el primer evento visible.
  if (selected_ < 0) {
    for (int i = top_; i < count; i++) {
      if (isSelectable(i)) {
        selected_ = i;
        break;
      }
    }
    keepSelectionVisible();
    return;
  }

  // Siguiente evento en ese sentido, saltando el separador y los avisos.
  for (int i = selected_ + step; i >= 0 && i < count; i += step) {
    if (isSelectable(i)) {
      selected_ = i;
      break;
    }
  }
  keepSelectionVisible();
}

void CalendarApp::keepSelectionVisible() {
  if (selected_ < 0) {
    return;
  }
  if (selected_ < top_) {
    top_ = selected_;
  }
  if (selected_ >= top_ + VISIBLE_ROWS) {
    top_ = selected_ - VISIBLE_ROWS + 1;
  }
  // Si encima hay un separador o un aviso, se enseña también.
  if (top_ > 0 && !isSelectable(top_ - 1) && selected_ - (top_ - 1) < VISIBLE_ROWS) {
    top_--;
  }
  // Al llegar al último evento, se enseñan los avisos que haya debajo.
  bool moreBelow = false;
  for (int i = selected_ + 1; i < (int)rows_.size(); i++) {
    moreBelow = moreBelow || isSelectable(i);
  }
  if (!moreBelow) {
    const int lastTop = (int)rows_.size() - VISIBLE_ROWS;
    if (lastTop > top_ && lastTop <= selected_) {
      top_ = lastTop;
    }
  }
}

// ---------------------------------------------------------------------------
// Dibujo
// ---------------------------------------------------------------------------

void CalendarApp::drawHeader(const struct tm& t) {
  char header[32];
  snprintf(header, sizeof(header), "%s %02d/%02d/%02d  %02d:%02d", SHORT_DAYS[t.tm_wday], t.tm_mday,
           t.tm_mon + 1, (t.tm_year + 1900) % 100, t.tm_hour, t.tm_min);
  Display::textCentered(8, header);
}

void CalendarApp::drawRow(int row, int topY, Minutes now) {
  U8G2& g = Display::gfx();
  const Row& r = rows_[row];
  const int baseline = topY + 8;
  const int middleY = topY + 5;

  if (r.type == RowType::Separator) {
    // ─── Mañana ───
    const int w = Display::textWidth(r.text);
    const int x = (CONTENT_WIDTH - w) / 2;
    g.drawHLine(0, middleY, x - 3);
    g.drawHLine(x + w + 3, middleY, CONTENT_WIDTH - (x + w + 3));
    Display::text(x, baseline, r.text);
    return;
  }

  if (r.type == RowType::Notice) {
    Display::text(1, baseline, Display::fit(r.text, CONTENT_WIDTH - 1));
    return;
  }

  const CalendarEvent& e = events_[r.eventIndex];

  // El evento seleccionado lleva el fondo encendido y el texto en negro.
  const bool isSelected = (row == selected_);
  if (isSelected) {
    g.drawBox(0, topY, CONTENT_WIDTH, ROW_HEIGHT);
    g.setDrawColor(0);
  }

  // Los eventos de todo el día solo llevan el título, que ocupa toda la fila.
  if (e.allDay) {
    Display::text(1, baseline, Display::fit(e.title.c_str(), CONTENT_WIDTH - 2));
  } else {
    const String times = timeRangeText(e).c_str();
    const int timesX = CONTENT_WIDTH - 1 - Display::textWidth(times);
    Display::text(1, baseline, Display::fit(e.title.c_str(), timesX - 4));
    Display::text(timesX, baseline, times);
  }

  // Tachado si ya ha terminado.
  if (e.end <= now) {
    g.drawHLine(1, middleY, CONTENT_WIDTH - 2);
  }

  g.setDrawColor(1);
}

void CalendarApp::drawArrows(int topY, int bottomY, bool up, bool down) {
  U8G2& g = Display::gfx();
  if (up) {
    g.drawTriangle(ARROW_X, topY, ARROW_X - 2, topY + 2, ARROW_X + 2, topY + 2);
  }
  if (down) {
    g.drawTriangle(ARROW_X - 2, bottomY - 2, ARROW_X + 2, bottomY - 2, ARROW_X, bottomY);
  }
}

void CalendarApp::draw() {
  U8G2& g = Display::gfx();
  struct tm t;

  if (!TimeService::now(t)) {
    g.setFont(u8g2_font_6x10_tf);
    Display::textCentered(28, "Sin hora");
    Display::textCentered(42, "Esperando a internet");
    return;
  }

  const Minutes now = CivilTime::fromTm(t);
  const int64_t today = CivilTime::dayOf(now);
  refreshData(today);

  g.setFont(u8g2_font_5x8_tf);

  if (detailOpen_) {
    if (millis() - lastInputMs_ < DETAIL_TIMEOUT_MS) {
      drawDetail();
      return;
    }
    detailOpen_ = false;  // 30 s sin tocar nada: vuelve a la lista
  }

  // Sin evento seleccionado, la lista se coloca sola en el siguiente evento.
  if (selected_ < 0) {
    top_ = firstUpcomingRow(now);
    const int lastTop = (int)rows_.size() - VISIBLE_ROWS;
    if (top_ > lastTop) {
      top_ = lastTop > 0 ? lastTop : 0;
    }
  }

  drawHeader(t);
  for (int i = 0; i < VISIBLE_ROWS && top_ + i < (int)rows_.size(); i++) {
    drawRow(top_ + i, LIST_TOP + i * ROW_HEIGHT, now);
  }
  drawArrows(LIST_TOP, LIST_TOP + VISIBLE_ROWS * ROW_HEIGHT - 2, top_ > 0,
             top_ + VISIBLE_ROWS < (int)rows_.size());
}

// ---------------------------------------------------------------------------
// Controles
// ---------------------------------------------------------------------------

void CalendarApp::onEnter() {
  inside_ = true;
  selected_ = -1;
  detailOpen_ = false;
  lastInputMs_ = millis();
}

void CalendarApp::onExit() {
  inside_ = false;
  selected_ = -1;
  detailOpen_ = false;
}

bool CalendarApp::handleInput(InputEvent ev) {
  lastInputMs_ = millis();

  if (detailOpen_) {
    switch (ev) {
      case InputEvent::Left: scrollDetail(-1); break;
      case InputEvent::Right: scrollDetail(1); break;
      case InputEvent::Back: detailOpen_ = false; break;
      default: break;
    }
    return true;
  }

  switch (ev) {
    case InputEvent::Left:
      moveSelection(-1);
      return true;
    case InputEvent::Right:
      moveSelection(1);
      return true;
    case InputEvent::DialPress: {
      struct tm t;
      if (isSelectable(selected_) && TimeService::now(t)) {
        openDetail(events_[rows_[selected_].eventIndex], CivilTime::dayOf(CivilTime::fromTm(t)));
      }
      return true;
    }
    case InputEvent::Back:
      return false;  // sale al primer nivel
    default:
      return true;
  }
}
