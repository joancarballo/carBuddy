#include "apps/clock/ClockApp.h"

#include "core/Display.h"
#include "services/TimeService.h"

void ClockApp::draw() {
  U8G2& g = Display::gfx();
  struct tm t;

  if (!TimeService::now(t)) {
    Display::message("Sin hora\nEsperando a internet", true);
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
