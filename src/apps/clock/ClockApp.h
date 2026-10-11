#pragma once

#include "appmanager/App.h"

// Reloj: hora en grande (HH:MM, 24 h) y debajo la fecha.
// Su sección de la web ("Time App") está en web/ClockSection.cpp.
class ClockApp : public App {
 public:
  const char* name() const override { return "Reloj"; }
  void draw() override;
};
