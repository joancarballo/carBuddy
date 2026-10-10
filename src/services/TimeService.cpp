#include "services/TimeService.h"

#include <Preferences.h>
#include <stdlib.h>

#include "services/TimeZones.h"

namespace {

const char* DEFAULT_ZONE = "Europe/Madrid";
const char* NTP_SERVER_1 = "pool.ntp.org";
const char* NTP_SERVER_2 = "time.cloudflare.com";

// Cualquier hora anterior a esta (noviembre de 2023) significa que aún no se ha sincronizado.
const time_t MIN_VALID_TIME = 1700000000;

const char* DAY_NAMES[] = {"Domingo", "Lunes", "Martes", "Miércoles", "Jueves", "Viernes", "Sábado"};

Preferences prefs;
String currentZoneId;

}  // namespace

namespace TimeService {

void begin() {
  prefs.begin("time", true);
  String id = prefs.getString("tz", DEFAULT_ZONE);
  prefs.end();

  const TimeZoneInfo* zone = findTimeZone(id.c_str());
  if (zone == nullptr) {
    zone = findTimeZone(DEFAULT_ZONE);
  }
  currentZoneId = zone->id;
  configTzTime(zone->posix, NTP_SERVER_1, NTP_SERVER_2);
}

bool now(struct tm& out) {
  time_t t = time(nullptr);
  if (t < MIN_VALID_TIME) {
    return false;
  }
  localtime_r(&t, &out);
  return true;
}

String timeText() {
  struct tm t;
  if (!now(t)) {
    return "--:--";
  }
  char buf[8];
  snprintf(buf, sizeof(buf), "%02d:%02d", t.tm_hour, t.tm_min);
  return String(buf);
}

String dateText() {
  struct tm t;
  if (!now(t)) {
    return "Sin hora";
  }
  char buf[40];
  snprintf(buf, sizeof(buf), "%s %02d/%02d/%02d", DAY_NAMES[t.tm_wday], t.tm_mday, t.tm_mon + 1,
           (t.tm_year + 1900) % 100);
  return String(buf);
}

const char* zoneId() {
  return currentZoneId.c_str();
}

bool setZone(const String& id) {
  const TimeZoneInfo* zone = findTimeZone(id.c_str());
  if (zone == nullptr) {
    return false;
  }
  prefs.begin("time", false);
  prefs.putString("tz", zone->id);
  prefs.end();

  currentZoneId = zone->id;
  setenv("TZ", zone->posix, 1);
  tzset();
  return true;
}

}  // namespace TimeService
