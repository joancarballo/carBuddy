#pragma once

#include <stddef.h>

// Husos horarios disponibles en el desplegable de la web.
struct TimeZoneInfo {
  const char* id;     // identificador estable que se guarda en la memoria
  const char* label;  // texto que se muestra en el desplegable
  const char* posix;  // regla horaria (incluye el cambio de verano/invierno)
};

extern const TimeZoneInfo TIME_ZONES[];
extern const size_t TIME_ZONE_COUNT;

// Devuelve nullptr si el identificador no existe.
const TimeZoneInfo* findTimeZone(const char* id);
