#include "services/TimeZones.h"

#include <string.h>

// Ordenados por hora UTC de invierno. El texto indica si la zona cambia de hora.
const TimeZoneInfo TIME_ZONES[] = {
    {"Pacific/Pago_Pago", "UTC-11:00 · Pago Pago, Midway (sin cambio horario)", "<-11>11"},
    {"Pacific/Honolulu", "UTC-10:00 · Honolulu, Papeete (sin cambio horario)", "HST10"},
    {"America/Anchorage", "UTC-09:00 · Anchorage (con cambio horario)", "AKST9AKDT,M3.2.0,M11.1.0"},
    {"America/Los_Angeles", "UTC-08:00 · Los Ángeles, Vancouver, Tijuana (con cambio horario)", "PST8PDT,M3.2.0,M11.1.0"},
    {"America/Denver", "UTC-07:00 · Denver, Calgary, Salt Lake City (con cambio horario)", "MST7MDT,M3.2.0,M11.1.0"},
    {"America/Phoenix", "UTC-07:00 · Phoenix, Hermosillo (sin cambio horario)", "MST7"},
    {"America/Chicago", "UTC-06:00 · Chicago, Dallas, Winnipeg (con cambio horario)", "CST6CDT,M3.2.0,M11.1.0"},
    {"America/Mexico_City", "UTC-06:00 · Ciudad de México, Guatemala, San José (sin cambio horario)", "CST6"},
    {"America/New_York", "UTC-05:00 · Nueva York, Toronto, Miami (con cambio horario)", "EST5EDT,M3.2.0,M11.1.0"},
    {"America/Bogota", "UTC-05:00 · Bogotá, Lima, Quito, Panamá (sin cambio horario)", "<-05>5"},
    {"America/Halifax", "UTC-04:00 · Halifax, Bermudas (con cambio horario)", "AST4ADT,M3.2.0,M11.1.0"},
    {"America/Caracas", "UTC-04:00 · Caracas, La Paz, San Juan (sin cambio horario)", "<-04>4"},
    {"America/Sao_Paulo", "UTC-03:00 · Buenos Aires, São Paulo, Montevideo (sin cambio horario)", "<-03>3"},
    {"Atlantic/Reykjavik", "UTC+00:00 · Reikiavik, Accra, Dakar (sin cambio horario)", "GMT0"},
    {"Europe/London", "UTC+00:00 · Londres, Dublín, Lisboa, Canarias (con cambio horario)", "GMT0BST,M3.5.0/1,M10.5.0"},
    {"Europe/Madrid", "UTC+01:00 · Madrid, París, Berlín, Roma, Ámsterdam (con cambio horario)", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"Africa/Lagos", "UTC+01:00 · Lagos, Argel, Túnez (sin cambio horario)", "<+01>-1"},
    {"Europe/Athens", "UTC+02:00 · Atenas, Helsinki, Kiev, Bucarest (con cambio horario)", "EET-2EEST,M3.5.0/3,M10.5.0/4"},
    {"Africa/Johannesburg", "UTC+02:00 · Johannesburgo, Harare, Kigali (sin cambio horario)", "SAST-2"},
    {"Europe/Istanbul", "UTC+03:00 · Estambul, Moscú, Riad, Nairobi (sin cambio horario)", "<+03>-3"},
    {"Asia/Dubai", "UTC+04:00 · Dubái, Bakú, Tiflis (sin cambio horario)", "<+04>-4"},
    {"Asia/Karachi", "UTC+05:00 · Karachi, Taskent, Maldivas (sin cambio horario)", "PKT-5"},
    {"Asia/Kolkata", "UTC+05:30 · Nueva Delhi, Bombay, Colombo (sin cambio horario)", "IST-5:30"},
    {"Asia/Dhaka", "UTC+06:00 · Daca, Timbu (sin cambio horario)", "<+06>-6"},
    {"Asia/Bangkok", "UTC+07:00 · Bangkok, Hanói, Yakarta (sin cambio horario)", "<+07>-7"},
    {"Asia/Shanghai", "UTC+08:00 · Pekín, Singapur, Hong Kong, Manila, Perth (sin cambio horario)", "CST-8"},
    {"Asia/Tokyo", "UTC+09:00 · Tokio, Seúl (sin cambio horario)", "JST-9"},
    {"Australia/Brisbane", "UTC+10:00 · Brisbane, Port Moresby, Vladivostok (sin cambio horario)", "AEST-10"},
    {"Australia/Sydney", "UTC+10:00 · Sídney, Melbourne, Canberra (con cambio horario)", "AEST-10AEDT,M10.1.0,M4.1.0/3"},
    {"Pacific/Auckland", "UTC+12:00 · Auckland, Wellington (con cambio horario)", "NZST-12NZDT,M9.5.0,M4.1.0/3"},
};

const size_t TIME_ZONE_COUNT = sizeof(TIME_ZONES) / sizeof(TIME_ZONES[0]);

const TimeZoneInfo* findTimeZone(const char* id) {
  for (size_t i = 0; i < TIME_ZONE_COUNT; i++) {
    if (strcmp(TIME_ZONES[i].id, id) == 0) {
      return &TIME_ZONES[i];
    }
  }
  return nullptr;
}
