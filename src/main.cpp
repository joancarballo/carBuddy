// carBuddy - Paso 3: calendario
//
// Estructura del proyecto:
//   src/core/      pantalla y entradas (dial y botones)
//   src/net/       Wi-Fi (conexión y red de configuración)
//   src/web/       web de configuración (una sección por tema)
//   src/services/  hora (SNTP y huso horario) y calendario (descarga del iCal)
//   src/appmanager/ interfaz de las apps y gestor (orden, app activa, niveles)
//   src/apps/      una carpeta por app

#include <Arduino.h>

#include "appmanager/AppManager.h"
#include "core/Display.h"
#include "core/Input.h"
#include "net/Net.h"
#include "services/TimeService.h"
#include "services/calendar/CalendarService.h"
#include "web/Web.h"

static uint32_t lastRender = 0;

// Pantalla de arranque mientras se conecta a la Wi-Fi, con la animación de espera.
static void showConnecting() {
  Display::showMessage("Conectando Wi-Fi", true);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("carBuddy - paso 3: calendario");

  Display::begin();
  Input::begin();

  showConnecting();
  Net::setWaitCallback(showConnecting);
  Net::begin();

  TimeService::begin();
  CalendarService::begin();
  AppManager::begin();
  Web::begin();
}

void loop() {
  Net::loop();
  Web::loop();

  InputEvent ev;
  while (Input::poll(ev)) {
    AppManager::handleInput(ev);
  }

  if (millis() - lastRender >= 50) {
    lastRender = millis();
    AppManager::render();
  }
}
