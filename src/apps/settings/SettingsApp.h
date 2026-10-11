#pragma once

#include "appmanager/App.h"

// Configuración: muestra el estado de la Wi-Fi y cómo acceder a la web.
// Su sección de la web ("Settings App", Wi-Fi) está en web/WifiSection.cpp.
class SettingsApp : public App {
 public:
  const char* name() const override { return "Configuracion"; }
  void draw() override;
};
