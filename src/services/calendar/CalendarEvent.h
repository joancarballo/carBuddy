#pragma once

#include <stdint.h>
#include <stdio.h>

#include <string>
#include <vector>

#include "services/calendar/CivilTime.h"

// Un VEVENT tal como viene del archivo iCal, ya limpio de escapes y HTML.
// Las horas están en minutos locales (ver CivilTime.h); -1 si no hay.
struct IcsEvent {
  std::string uid;
  std::string summary;
  std::string location;
  std::string description;
  std::string rrule;  // regla de repetición sin interpretar, por ejemplo "FREQ=WEEKLY;BYDAY=MO"
  bool cancelled = false;
  bool allDay = false;
  CivilTime::Minutes start = -1;
  CivilTime::Minutes end = -1;
  CivilTime::Minutes recurrenceId = -1;  // si modifica una repetición de otro evento
  std::vector<CivilTime::Minutes> exdates;
};

// Un evento listo para mostrar: una aparición concreta en un día concreto.
// Un evento que dura varios días aparece una vez por cada día.
struct CalendarEvent {
  std::string title;
  std::string location;
  std::string notes;
  std::string repeat;  // "Cada semana: L, X" o vacío
  bool allDay = false;
  CivilTime::Minutes start = 0;
  CivilTime::Minutes end = 0;
  int64_t day = 0;  // día (desde el 1/1/1970) en cuya lista aparece
};

// "10:00-11:30", o "Todo el día".
inline std::string timeRangeText(const CalendarEvent& e) {
  if (e.allDay) {
    return "Todo el día";
  }
  const int s = CivilTime::minuteOfDay(e.start);
  const int en = CivilTime::minuteOfDay(e.end);
  char buf[16];
  snprintf(buf, sizeof(buf), "%02d:%02d-%02d:%02d", s / 60, s % 60, en / 60, en % 60);
  return buf;
}
