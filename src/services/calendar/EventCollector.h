#pragma once

#include <stdint.h>

#include <utility>
#include <vector>

#include "services/calendar/CalendarEvent.h"

// Recibe los VEVENT del archivo uno a uno y se queda solo con lo que cae en la
// ventana de días que se muestra (hoy y mañana). Al terminar devuelve la lista
// lista para la pantalla: un elemento por evento y día, ordenada.
// No depende de Arduino.
class EventCollector {
 public:
  // Máximo de elementos en la lista, para no agotar la memoria.
  static const size_t MAX_EVENTS = 40;

  // Ventana: dayCount días a partir de firstDay (días desde el 1/1/1970).
  EventCollector(int64_t firstDay, int dayCount);

  void add(const IcsEvent& ev);

  // Aplica las repeticiones modificadas o canceladas y devuelve la lista ordenada.
  std::vector<CalendarEvent> finish();

  // Reglas de repetición que no se entienden (el evento sale solo en su
  // primera fecha). Se guardan las primeras para el diagnóstico.
  const std::vector<std::string>& unsupportedRules() const { return unsupportedRules_; }

  // true si se llegó a MAX_EVENTS y se dejaron eventos fuera.
  bool full() const { return full_; }

  // Para usar como callback de IcsParser.
  static void addCallback(const IcsEvent& ev, void* collector);

 private:
  struct Occurrence {
    uint32_t uidHash;
    CivilTime::Minutes originalStart;
    bool fromRule;  // generada por una RRULE (puede haber sido modificada)
    CalendarEvent event;
  };

  void addOccurrence(const IcsEvent& ev, CivilTime::Minutes start, bool fromRule, const std::string& repeat);

  int64_t firstDay_;
  int dayCount_;
  std::vector<Occurrence> occurrences_;
  // Repeticiones que tienen su propio VEVENT con RECURRENCE-ID (movidas,
  // cambiadas o canceladas): la versión generada por la RRULE se descarta.
  std::vector<std::pair<uint32_t, CivilTime::Minutes> > overrides_;
  std::vector<std::string> unsupportedRules_;
  bool full_ = false;
};
