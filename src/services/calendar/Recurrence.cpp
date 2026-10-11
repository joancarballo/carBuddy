#include "services/calendar/Recurrence.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>

#include "services/calendar/IcsParser.h"

using CivilTime::Minutes;

namespace {

// Límite de periodos que se recorren, por si una regla rara no terminara nunca.
const int MAX_PERIODS = 20000;

const char* const WEEKDAY_CODES[] = {"SU", "MO", "TU", "WE", "TH", "FR", "SA"};
const char* const WEEKDAY_LETTERS[] = {"D", "L", "M", "X", "J", "V", "S"};
const char* const WEEKDAY_NAMES[] = {"domingo", "lunes", "martes", "miércoles", "jueves", "viernes", "sábado"};

int weekdayFromCode(const std::string& code) {
  for (int i = 0; i < 7; i++) {
    if (code == WEEKDAY_CODES[i]) {
      return i;
    }
  }
  return -1;
}

// Separa "a,b,c" en partes.
std::vector<std::string> split(const std::string& s, char sep) {
  std::vector<std::string> parts;
  size_t start = 0;
  while (start <= s.size()) {
    size_t end = s.find(sep, start);
    if (end == std::string::npos) {
      end = s.size();
    }
    parts.push_back(s.substr(start, end - start));
    start = end + 1;
  }
  return parts;
}

// Lunes de la semana de un día (las semanas empiezan en lunes, como en Google).
int64_t weekStart(int64_t day) {
  return day - (CivilTime::weekday(day) + 6) % 7;
}

int64_t monthIndex(int64_t day) {
  int y, m, d;
  CivilTime::civilFromDays(day, y, m, d);
  return (int64_t)y * 12 + (m - 1);
}

bool contains(const std::vector<int>& v, int x) {
  return std::find(v.begin(), v.end(), x) != v.end();
}

// Días de un mes que cumplen BYMONTHDAY y BYDAY. Si no hay ninguno de los dos,
// el mismo número de día que el primer evento (si el mes lo tiene).
void daysInMonth(int y, int m, int defaultDay, const Recurrence::Rule& rule, std::vector<int64_t>& out) {
  const int length = CivilTime::daysInMonth(y, m);
  const int64_t first = CivilTime::daysFromCivil(y, m, 1);

  if (!rule.byMonthDay.empty()) {
    for (size_t i = 0; i < rule.byMonthDay.size(); i++) {
      int md = rule.byMonthDay[i];
      if (md < 0) {
        md = length + md + 1;
      }
      if (md < 1 || md > length) {
        continue;
      }
      const int64_t day = first + md - 1;
      // Con BYDAY además de BYMONTHDAY, el día tiene que cumplir los dos.
      bool weekdayOk = rule.byDay.empty();
      for (size_t j = 0; j < rule.byDay.size(); j++) {
        weekdayOk = weekdayOk || rule.byDay[j].weekday == CivilTime::weekday(day);
      }
      if (weekdayOk) {
        out.push_back(day);
      }
    }
    return;
  }

  if (!rule.byDay.empty()) {
    for (size_t i = 0; i < rule.byDay.size(); i++) {
      const Recurrence::ByDay& bd = rule.byDay[i];
      // Primer día del mes que cae en ese día de la semana.
      const int64_t firstMatch = first + (bd.weekday - CivilTime::weekday(first) + 7) % 7;
      if (bd.ordinal == 0) {
        for (int64_t d = firstMatch; d < first + length; d += 7) {
          out.push_back(d);
        }
      } else if (bd.ordinal > 0) {
        const int64_t d = firstMatch + 7 * (bd.ordinal - 1);
        if (d < first + length) {
          out.push_back(d);
        }
      } else {
        const int64_t last = first + length - 1;
        const int64_t lastMatch = last - (CivilTime::weekday(last) - bd.weekday + 7) % 7;
        const int64_t d = lastMatch + 7 * (bd.ordinal + 1);
        if (d >= first) {
          out.push_back(d);
        }
      }
    }
    return;
  }

  if (defaultDay <= length) {
    out.push_back(first + defaultDay - 1);
  }
}

// Días candidatos del periodo número p (un día, una semana, un mes o un año).
void periodDays(const Recurrence::Rule& rule, int64_t firstDay, int64_t p, std::vector<int64_t>& out) {
  int y0, m0, d0;
  CivilTime::civilFromDays(firstDay, y0, m0, d0);
  const int64_t step = p * rule.interval;

  switch (rule.freq) {
    case Recurrence::Freq::Daily: {
      const int64_t day = firstDay + step;
      int y, m, d;
      CivilTime::civilFromDays(day, y, m, d);
      bool ok = rule.byMonth.empty() || contains(rule.byMonth, m);
      ok = ok && (rule.byMonthDay.empty() || contains(rule.byMonthDay, d));
      if (ok && !rule.byDay.empty()) {
        ok = false;
        for (size_t i = 0; i < rule.byDay.size(); i++) {
          ok = ok || rule.byDay[i].weekday == CivilTime::weekday(day);
        }
      }
      if (ok) {
        out.push_back(day);
      }
      break;
    }
    case Recurrence::Freq::Weekly: {
      const int64_t monday = weekStart(firstDay) + 7 * step;
      if (rule.byDay.empty()) {
        out.push_back(monday + (CivilTime::weekday(firstDay) + 6) % 7);
      }
      for (size_t i = 0; i < rule.byDay.size(); i++) {
        out.push_back(monday + (rule.byDay[i].weekday + 6) % 7);
      }
      break;
    }
    case Recurrence::Freq::Monthly: {
      const int64_t month = (int64_t)y0 * 12 + (m0 - 1) + step;
      const int y = (int)(month / 12);
      const int m = (int)(month % 12) + 1;
      if (rule.byMonth.empty() || contains(rule.byMonth, m)) {
        daysInMonth(y, m, d0, rule, out);
      }
      break;
    }
    case Recurrence::Freq::Yearly: {
      const int y = y0 + (int)step;
      if (rule.byMonth.empty()) {
        daysInMonth(y, m0, d0, rule, out);
      }
      for (size_t i = 0; i < rule.byMonth.size(); i++) {
        daysInMonth(y, rule.byMonth[i], d0, rule, out);
      }
      break;
    }
    case Recurrence::Freq::None:
      break;
  }
  std::sort(out.begin(), out.end());
  out.erase(std::unique(out.begin(), out.end()), out.end());
}

// Número del periodo que contiene un día (para saltar directamente cerca de la
// ventana sin recorrer años de repeticiones).
int64_t periodOf(const Recurrence::Rule& rule, int64_t firstDay, int64_t day) {
  int64_t diff = 0;
  switch (rule.freq) {
    case Recurrence::Freq::Daily: diff = day - firstDay; break;
    case Recurrence::Freq::Weekly: diff = (weekStart(day) - weekStart(firstDay)) / 7; break;
    case Recurrence::Freq::Monthly: diff = monthIndex(day) - monthIndex(firstDay); break;
    case Recurrence::Freq::Yearly: diff = (monthIndex(day) / 12) - (monthIndex(firstDay) / 12); break;
    case Recurrence::Freq::None: break;
  }
  return diff / rule.interval;
}

bool isExcluded(const IcsEvent& ev, Minutes start) {
  for (size_t i = 0; i < ev.exdates.size(); i++) {
    // En los eventos de todo el día basta con que coincida el día.
    if (ev.exdates[i] == start ||
        (ev.allDay && CivilTime::dayOf(ev.exdates[i]) == CivilTime::dayOf(start))) {
      return true;
    }
  }
  return false;
}

std::string ordinalName(int n) {
  switch (n) {
    case 1: return "primer";
    case 2: return "segundo";
    case 3: return "tercer";
    case 4: return "cuarto";
    case 5: return "quinto";
    case -1: return "último";
    case -2: return "penúltimo";
    default: return std::to_string(n) + "º";
  }
}

}  // namespace

namespace Recurrence {

bool parse(const std::string& rrule, Rule& out) {
  out = Rule();
  std::vector<std::string> parts = split(rrule, ';');
  for (size_t i = 0; i < parts.size(); i++) {
    const size_t eq = parts[i].find('=');
    if (eq == std::string::npos) {
      continue;
    }
    const std::string key = parts[i].substr(0, eq);
    const std::string value = parts[i].substr(eq + 1);

    if (key == "FREQ") {
      if (value == "DAILY") {
        out.freq = Freq::Daily;
      } else if (value == "WEEKLY") {
        out.freq = Freq::Weekly;
      } else if (value == "MONTHLY") {
        out.freq = Freq::Monthly;
      } else if (value == "YEARLY") {
        out.freq = Freq::Yearly;
      } else {
        return false;  // cada hora, cada minuto...
      }
    } else if (key == "INTERVAL") {
      out.interval = atoi(value.c_str());
      if (out.interval < 1) {
        out.interval = 1;
      }
    } else if (key == "COUNT") {
      out.count = atoi(value.c_str());
    } else if (key == "UNTIL") {
      bool isDate = false;
      out.until = Ics::parseDateTime(value, isDate);
      if (isDate && out.until >= 0) {
        out.until += CivilTime::MINUTES_PER_DAY - 1;  // incluye todo ese día
      }
    } else if (key == "BYDAY") {
      std::vector<std::string> days = split(value, ',');
      for (size_t j = 0; j < days.size(); j++) {
        const std::string& d = days[j];
        if (d.size() < 2) {
          return false;
        }
        const int weekday = weekdayFromCode(d.substr(d.size() - 2));
        if (weekday < 0) {
          return false;
        }
        ByDay bd;
        bd.weekday = weekday;
        bd.ordinal = d.size() > 2 ? atoi(d.substr(0, d.size() - 2).c_str()) : 0;
        out.byDay.push_back(bd);
      }
    } else if (key == "BYMONTHDAY") {
      std::vector<std::string> days = split(value, ',');
      for (size_t j = 0; j < days.size(); j++) {
        out.byMonthDay.push_back(atoi(days[j].c_str()));
      }
    } else if (key == "BYMONTH") {
      std::vector<std::string> months = split(value, ',');
      for (size_t j = 0; j < months.size(); j++) {
        out.byMonth.push_back(atoi(months[j].c_str()));
      }
    } else if (key == "WKST") {
      // Google usa lunes o domingo; solo cambia algo en semanas alternas. Se ignora.
    } else {
      return false;  // BYSETPOS, BYWEEKNO, BYYEARDAY...: no se admiten
    }
  }
  return out.freq != Freq::None;
}

void occurrences(const IcsEvent& ev, const Rule& rule, Minutes from, Minutes to, std::vector<Minutes>& starts) {
  const Minutes duration = ev.end - ev.start;
  const int timeOfDay = CivilTime::minuteOfDay(ev.start);
  const int64_t firstDay = CivilTime::dayOf(ev.start);
  const int64_t lastDay = CivilTime::dayOf(to - 1);

  // Con COUNT hay que contar desde el principio; sin él se puede saltar cerca de la ventana.
  int64_t p = 0;
  if (rule.count == 0) {
    const int64_t earliest = CivilTime::dayOf(from - duration);
    p = std::max<int64_t>(0, periodOf(rule, firstDay, earliest) - 1);
  }

  int generated = 0;
  std::vector<int64_t> days;
  for (int n = 0; n < MAX_PERIODS; n++, p++) {
    days.clear();
    periodDays(rule, firstDay, p, days);
    for (size_t i = 0; i < days.size(); i++) {
      const int64_t day = days[i];
      if (day < firstDay) {
        continue;
      }
      const Minutes start = day * CivilTime::MINUTES_PER_DAY + timeOfDay;
      if (rule.until >= 0 && start > rule.until) {
        return;
      }
      generated++;
      if (rule.count > 0 && generated > rule.count) {
        return;
      }
      if (day > lastDay) {
        return;
      }
      const bool overlaps = duration > 0 ? (start < to && start + duration > from) : (start >= from && start < to);
      if (overlaps && !isExcluded(ev, start)) {
        starts.push_back(start);
      }
    }
  }
}

std::string describe(const Rule& rule) {
  char buf[64];
  std::string text;
  const bool one = rule.interval == 1;

  switch (rule.freq) {
    case Freq::Daily:
      text = one ? "Cada día" : "Cada " + std::to_string(rule.interval) + " días";
      break;
    case Freq::Weekly:
      text = one ? "Cada semana" : "Cada " + std::to_string(rule.interval) + " semanas";
      if (!rule.byDay.empty()) {
        // En orden de lunes a domingo.
        text += ":";
        for (int i = 1; i <= 7; i++) {
          for (size_t j = 0; j < rule.byDay.size(); j++) {
            if (rule.byDay[j].weekday == i % 7) {
              text += " ";
              text += WEEKDAY_LETTERS[i % 7];
              text += ",";
              break;
            }
          }
        }
        text.pop_back();  // la última coma
      }
      break;
    case Freq::Monthly:
      text = one ? "Cada mes" : "Cada " + std::to_string(rule.interval) + " meses";
      if (!rule.byDay.empty() && rule.byDay[0].ordinal != 0) {
        text += ", el " + ordinalName(rule.byDay[0].ordinal) + " " + WEEKDAY_NAMES[rule.byDay[0].weekday];
      } else if (!rule.byMonthDay.empty()) {
        text += rule.byMonthDay[0] == -1 ? ", el último día" : ", el día " + std::to_string(rule.byMonthDay[0]);
      }
      break;
    case Freq::Yearly:
      text = one ? "Cada año" : "Cada " + std::to_string(rule.interval) + " años";
      break;
    case Freq::None:
      break;
  }

  if (rule.until >= 0) {
    int y, m, d;
    CivilTime::civilFromDays(CivilTime::dayOf(rule.until), y, m, d);
    snprintf(buf, sizeof(buf), ", hasta %02d/%02d/%02d", d, m, y % 100);
    text += buf;
  } else if (rule.count > 0) {
    text += ", " + std::to_string(rule.count) + " veces";
  }
  return text;
}

}  // namespace Recurrence
