#pragma once

#include <stdint.h>
#include <time.h>

// Fechas y horas "de reloj de pared" (hora local), sin husos horarios.
//
// Todo el calendario trabaja en minutos locales desde el 1/1/1970: así un evento
// a las 10:00 sigue a las 10:00 aunque cambie la hora de verano, y las repeticiones
// ("cada martes") se calculan con días del calendario, no con segundos.
// No depende de Arduino, para poder probarlo en el ordenador.
namespace CivilTime {

typedef int64_t Minutes;  // minutos locales desde el 1/1/1970 00:00

const Minutes MINUTES_PER_DAY = 24 * 60;

// Días desde el 1/1/1970 para una fecha (algoritmo de Howard Hinnant).
inline int64_t daysFromCivil(int y, int m, int d) {
  y -= m <= 2;
  const int64_t era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = (unsigned)(y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + (int64_t)doe - 719468;
}

// Fecha a partir de los días desde el 1/1/1970.
inline void civilFromDays(int64_t z, int& y, int& m, int& d) {
  z += 719468;
  const int64_t era = (z >= 0 ? z : z - 146096) / 146097;
  const unsigned doe = (unsigned)(z - era * 146097);
  const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const unsigned mp = (5 * doy + 2) / 153;
  d = (int)(doy - (153 * mp + 2) / 5 + 1);
  m = (int)(mp < 10 ? mp + 3 : mp - 9);
  y = (int)(yoe + era * 400 + (m <= 2));
}

// Día de la semana: 0 = domingo ... 6 = sábado.
inline int weekday(int64_t days) {
  return (int)(days >= -4 ? (days + 4) % 7 : (days + 5) % 7 + 6);
}

inline int daysInMonth(int y, int m) {
  static const int DAYS[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  const bool leap = (y % 4 == 0 && y % 100 != 0) || y % 400 == 0;
  return (m == 2 && leap) ? 29 : DAYS[m - 1];
}

inline int64_t dayOf(Minutes t) {
  return t >= 0 ? t / MINUTES_PER_DAY : (t - MINUTES_PER_DAY + 1) / MINUTES_PER_DAY;
}

inline int minuteOfDay(Minutes t) {
  return (int)(t - dayOf(t) * MINUTES_PER_DAY);
}

inline Minutes make(int y, int mo, int d, int h, int mi) {
  return daysFromCivil(y, mo, d) * MINUTES_PER_DAY + h * 60 + mi;
}

// Hora local (struct tm) a minutos locales.
inline Minutes fromTm(const struct tm& t) {
  return make(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min);
}

// Instante UTC a minutos locales, con el huso horario activo de la placa.
inline Minutes fromUtc(time_t utc) {
  struct tm t;
  localtime_r(&utc, &t);
  return fromTm(t);
}

}  // namespace CivilTime
