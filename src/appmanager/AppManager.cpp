#include "appmanager/AppManager.h"

#include "appmanager/App.h"
#include "apps/calendar/CalendarApp.h"
#include "apps/clock/ClockApp.h"
#include "apps/settings/SettingsApp.h"
#include "core/Display.h"
#include "net/Net.h"

namespace {

constexpr int MAX_APPS = 8;
constexpr uint32_t INDICATOR_MS = 3000;
constexpr uint32_t LEVEL_LINE_MS = 10000;  // fila 0 encendida al estar dentro de una app
constexpr int RECONNECT_BAR_WIDTH = 16;    // barra que recorre la fila 0 sin Wi-Fi
constexpr uint32_t RECONNECT_STEP_MS = 10; // 1 píxel cada 10 ms

ClockApp clockApp;
CalendarApp calendarApp;
SettingsApp settingsApp;

App* apps[MAX_APPS];
int appCount = 0;
int current = 0;
bool inApp = false;           // true en el segundo nivel
uint32_t lastRotationMs = 0;  // última vez que se giró el dial
uint32_t levelLineMs = 0;     // al entrar en el segundo nivel o girar dentro de él
bool wasApMode = false;       // para saber cuándo se crea la red de configuración

// Si la placa acaba de crear la red de configuración (por ejemplo, tras 2 minutos
// sin Wi-Fi), se muestra Configuración, que explica cómo conectarse a ella.
void showSettingsOnApMode() {
  const bool apMode = Net::isApMode();
  if (apMode && !wasApMode) {
    if (inApp) {
      apps[current]->onExit();
      inApp = false;
    }
    for (int i = 0; i < appCount; i++) {
      if (apps[i] == &settingsApp) {
        current = i;
      }
    }
    lastRotationMs = millis();  // que se vea el indicador
    Serial.printf("App: %s (modo configuracion)\n", apps[current]->name());
  }
  wasApMode = apMode;
}

// Sin Wi-Fi (los 2 minutos antes de crear la red de configuración): una barra
// recorre la fila 0 de izquierda a derecha y vuelta, para que se vea que la placa
// está intentando reconectar.
void drawReconnectBar() {
  const int travel = Display::WIDTH - RECONNECT_BAR_WIDTH;
  const int t = (millis() / RECONNECT_STEP_MS) % (2 * travel);
  const int x = t < travel ? t : 2 * travel - t;
  Display::gfx().drawHLine(x, 0, RECONNECT_BAR_WIDTH);
}

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

  current = 0;
  inApp = false;
  wasApMode = Net::isApMode();  // al arrancar el orden de las apps ya lo tiene en cuenta
  lastRotationMs = millis();  // el indicador se ve los primeros 3 s
  Serial.printf("App inicial: %s\n", apps[current]->name());
}

void handleInput(InputEvent ev) {
  App* app = apps[current];

  // Segundo nivel: los eventos van a la app. BACK sale de ella si la app no lo usa.
  if (inApp) {
    if (ev == InputEvent::Left || ev == InputEvent::Right) {
      levelLineMs = millis();
    }
    bool used = app->handleInput(ev);
    if (ev == InputEvent::Back && !used) {
      inApp = false;
      app->onExit();
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
        levelLineMs = millis();
        app->onEnter();
      }
      break;
    default:
      break;  // el resto de botones no hacen nada en el primer nivel
  }
}

void render() {
  showSettingsOnApMode();

  U8G2& g = Display::gfx();
  g.clearBuffer();
  apps[current]->draw();
  g.setDrawColor(1);
  if (Net::isReconnecting()) {
    drawReconnectBar();  // tiene prioridad sobre el indicador y la línea del nivel 2
  } else if (inApp) {
    // Dentro de una app, la fila 0 entera encendida indica el segundo nivel.
    if (millis() - levelLineMs < LEVEL_LINE_MS) {
      g.drawHLine(0, 0, Display::WIDTH);
    }
  } else if (millis() - lastRotationMs < INDICATOR_MS) {
    drawIndicator();
  }
  g.sendBuffer();
}

}  // namespace AppManager
