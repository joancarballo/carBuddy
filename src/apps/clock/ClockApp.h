#pragma once

#include "app/App.h"

// Reloj: hora en grande (HH:MM, 24 h) y debajo la fecha.
// En la web: sección "Time App" con la hora y el huso horario.
class ClockApp : public App {
 public:
  const char* name() const override { return "Reloj"; }
  void draw() override;
  String webSection() override;
  void registerRoutes() override;
};
