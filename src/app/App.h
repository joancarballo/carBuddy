#pragma once

#include <Arduino.h>

#include "core/Input.h"

// Interfaz que cumple cada app. Para añadir una app nueva:
//   1. Crear su carpeta en src/apps/ con una clase que herede de App.
//   2. Registrarla en AppManager.cpp.
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

  // Eventos de entrada, solo cuando se está dentro de la app (segundo nivel).
  virtual void handleInput(InputEvent ev) { (void)ev; }

  // Sección de la web para esta app, en HTML. Vacío si no tiene.
  virtual String webSection() { return String(); }

  // Rutas web propias de la app (se llama una vez al arrancar).
  virtual void registerRoutes() {}
};
