#pragma once

#include "app/App.h"

// Calendario (versión inicial): la fecha centrada arriba y líneas de texto de relleno.
class CalendarApp : public App {
 public:
  const char* name() const override { return "Calendario"; }
  void draw() override;
};
