#include "apps/calendar/CalendarApp.h"

#include "core/Display.h"
#include "services/TimeService.h"

void CalendarApp::draw() {
  U8G2& g = Display::gfx();

  // Fecha centrada en la parte superior
  g.setFont(u8g2_font_6x10_tf);
  Display::textCentered(10, TimeService::dateText().c_str());

  // Tantas líneas de texto como caben debajo
  g.setFont(u8g2_font_5x8_tf);
  const int firstBaseline = 20;
  const int lineHeight = 8;
  int n = 1;
  for (int y = firstBaseline; y < Display::HEIGHT; y += lineHeight, n++) {
    char line[32];
    snprintf(line, sizeof(line), "Linea de texto %d", n);
    Display::text(0, y, line);
  }
}
