#pragma once

#include <Arduino.h>
#include <time.h>

// Hora y fecha locales, sincronizadas por internet (SNTP), con huso horario configurable.
namespace TimeService {

// Carga el huso horario guardado (por defecto Madrid) y arranca la sincronización.
// Llamar después de Net::begin().
void begin();

// Rellena out con la hora local. Devuelve false si todavía no se ha sincronizado.
bool now(struct tm& out);

// "HH:MM" en formato 24 h, o "--:--" si no hay hora.
String timeText();

// "Sábado 10/10/26", o "Sin hora" si no hay hora.
String dateText();

// Identificador del huso horario activo (por ejemplo "Europe/Madrid").
const char* zoneId();

// Guarda y aplica un huso horario. Devuelve false si el identificador no existe.
bool setZone(const String& id);

}  // namespace TimeService
