#pragma once

#include <Arduino.h>

#include "core/Input.h"

// Interfaz que cumple cada app. Para añadir una app nueva:
//   1. Crear su carpeta en src/apps/ con una clase que herede de App.
//   2. Registrarla en AppManager.cpp.
//   3. Si necesita web, añadir su sección en src/web/ (ver web/Web.h).
class App {
 public:
  virtual ~App() {}

  // Nombre de la app (para mensajes de depuración).
  virtual const char* name() const = 0;

  // Dibuja la app en la pantalla. El gestor ya ha limpiado el buffer.
  // La primera línea de píxeles (y = 0) la usa el indicador de app.
  virtual void draw() = 0;

  // true si la app tiene segundo nivel: al pulsar el dial se entra en ella
  // y recibe los eventos en handleInput() hasta que se pulsa BACK.
  virtual bool hasInteraction() const { return false; }

  // Se llaman al entrar en el segundo nivel y al volver al primero,
  // para que la app pueda reiniciar su selección.
  virtual void onEnter() {}
  virtual void onExit() {}

  // Eventos de entrada, solo cuando se está dentro de la app (segundo nivel).
  // Devuelve true si la app ha usado el evento. BACK solo sale al primer nivel
  // si la app no lo usa (por ejemplo, para cerrar una vista de detalle).
  virtual bool handleInput(InputEvent ev) {
    (void)ev;
    return false;
  }

};
