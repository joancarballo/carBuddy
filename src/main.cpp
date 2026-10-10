// carBuddy - Paso 3: calendario
//
// Estructura del proyecto:
//   src/core/      pantalla y entradas (dial y botones)
//   src/net/       Wi-Fi y servidor web
//   src/services/  hora (SNTP y huso horario) y calendario (descarga del iCal)
//   src/app/       interfaz de las apps y gestor
//   src/apps/      una carpeta por app

#include <Arduino.h>

#include "app/AppManager.h"
#include "core/Display.h"
#include "core/Input.h"
#include "net/Net.h"
#include "services/TimeService.h"
#include "services/calendar/CalendarService.h"

static uint32_t lastRender = 0;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("carBuddy - paso 3: calendario");

  Display::begin();
  Input::begin();

  Display::showMessage("Conectando WiFi...");
  Net::setPageBuilder(AppManager::webSections);
  Net::begin();

  TimeService::begin();
  CalendarService::begin();
  AppManager::begin();
}

void loop() {
  Net::loop();

  InputEvent ev;
  while (Input::poll(ev)) {
    AppManager::handleInput(ev);
  }

  if (millis() - lastRender >= 50) {
    lastRender = millis();
    AppManager::render();
  }
}
