#pragma once

#include <Arduino.h>

#include "core/Input.h"

// Gestiona la lista de apps, cuál está activa y los dos niveles de control:
//   Nivel 1: girar el dial cambia de app; pulsar el dial entra en la app si tiene interacción.
//   Nivel 2: la app recibe los eventos; BACK vuelve al nivel 1.
namespace AppManager {

// Define el orden de las apps y registra sus rutas web.
// Llamar después de Net::begin().
void begin();

void handleInput(InputEvent ev);

// Dibuja la app activa y el indicador, y lo envía a la pantalla.
void render();

// Secciones web de todas las apps, en HTML.
String webSections();

}  // namespace AppManager
