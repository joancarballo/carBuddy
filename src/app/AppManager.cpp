#include "app/AppManager.h"

#include "app/App.h"
#include "apps/calendar/CalendarApp.h"
#include "apps/clock/ClockApp.h"
#include "apps/settings/SettingsApp.h"
#include "core/Display.h"
#include "net/Net.h"

namespace {

constexpr int MAX_APPS = 8;
constexpr uint32_t INDICATOR_MS = 3000;

ClockApp clockApp;
CalendarApp calendarApp;
SettingsApp settingsApp;

App* apps[MAX_APPS];
int appCount = 0;
int current = 0;
bool inApp = false;           // true en el segundo nivel
uint32_t lastRotationMs = 0;  // última vez que se giró el dial

// Primera línea de píxeles dividida en tantos bloques como apps; se ilumina el de la app actual.
void drawIndicator() {
  const int x0 = current * Display::WIDTH / appCount;
  const int x1 = (current + 1) * Display::WIDTH / appCount;
  Display::gfx().drawHLine(x0, 0, x1 - x0);
}

}  // namespace

namespace AppManager {

void begin() {
  // Configuración va siempre la última, salvo que no haya una wifi bien
  // configurada: entonces va la primera para ver cómo configurarla.
  if (Net::isConnected()) {
    apps[0] = &clockApp;
    apps[1] = &calendarApp;
    apps[2] = &settingsApp;
  } else {
    apps[0] = &settingsApp;
    apps[1] = &clockApp;
    apps[2] = &calendarApp;
  }
  appCount = 3;

  for (int i = 0; i < appCount; i++) {
    apps[i]->registerRoutes();
  }

  current = 0;
  inApp = false;
  lastRotationMs = millis();  // el indicador se ve los primeros 3 s
  Serial.printf("App inicial: %s\n", apps[current]->name());
}

void handleInput(InputEvent ev) {
  App* app = apps[current];

  // Segundo nivel: los eventos van a la app, salvo BACK que sale de ella.
  if (inApp) {
    if (ev == InputEvent::Back) {
      inApp = false;
    } else {
      app->handleInput(ev);
    }
    return;
  }

  // Primer nivel
  switch (ev) {
    case InputEvent::Right:
      current = (current + 1) % appCount;
      lastRotationMs = millis();
      Serial.printf("App: %s\n", apps[current]->name());
      break;
    case InputEvent::Left:
      current = (current + appCount - 1) % appCount;
      lastRotationMs = millis();
      Serial.printf("App: %s\n", apps[current]->name());
      break;
    case InputEvent::DialPress:
      if (app->hasInteraction()) {
        inApp = true;
      }
      break;
    default:
      break;  // el resto de botones no hacen nada en el primer nivel
  }
}

void render() {
  U8G2& g = Display::gfx();
  g.clearBuffer();
  apps[current]->draw();
  if (millis() - lastRotationMs < INDICATOR_MS) {
    drawIndicator();
  }
  g.sendBuffer();
}

String webSections() {
  String html;
  for (int i = 0; i < appCount; i++) {
    String section = apps[i]->webSection();
    if (section.length() > 0) {
      html += section;
      html += "<hr>";
    }
  }
  return html;
}

}  // namespace AppManager
