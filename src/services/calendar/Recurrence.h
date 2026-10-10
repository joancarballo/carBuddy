#pragma once

#include <string>
#include <vector>

#include "services/calendar/CalendarEvent.h"

// Repeticiones de eventos (la línea RRULE de iCal), con el subconjunto que usa
// Google Calendar: FREQ=DAILY/WEEKLY/MONTHLY/YEARLY con INTERVAL, BYDAY
// (también "2MO" o "-1FR"), BYMONTHDAY, BYMONTH, UNTIL y COUNT.
// No depende de Arduino.
namespace Recurrence {

enum class Freq { None, Daily, Weekly, Monthly, Yearly };

struct ByDay {
  int weekday;  // 0 = domingo ... 6 = sábado
  int ordinal;  // 0 = todos; 2 = el segundo del mes; -1 = el último
};

struct Rule {
  Freq freq = Freq::None;
  int interval = 1;
  int count = 0;                    // 0 = sin límite de veces
  CivilTime::Minutes until = -1;    // -1 = sin fecha final
  std::vector<ByDay> byDay;
  std::vector<int> byMonthDay;      // negativos: contando desde el final del mes
  std::vector<int> byMonth;         // 1..12
};

// Interpreta una regla. Devuelve false si usa algo que no se admite: entonces
// el evento se muestra solo en su primera fecha.
bool parse(const std::string& rrule, Rule& out);

// Añade a starts el inicio de cada repetición del evento que se solapa con
// [from, to). Ya descarta las fechas de EXDATE.
void occurrences(const IcsEvent& ev, const Rule& rule, CivilTime::Minutes from, CivilTime::Minutes to,
                 std::vector<CivilTime::Minutes>& starts);

// Texto para la pantalla, por ejemplo "Cada semana: L, X" o "Cada mes, el último viernes".
std::string describe(const Rule& rule);

}  // namespace Recurrence
