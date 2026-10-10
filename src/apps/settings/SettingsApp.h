#pragma once

#include "app/App.h"

// Configuración: muestra el estado de la Wi-Fi y cómo acceder a la web.
// En la web: sección "Settings App" para editar la Wi-Fi.
class SettingsApp : public App {
 public:
  const char* name() const override { return "Configuracion"; }
  void draw() override;
  String webSection() override;
};
