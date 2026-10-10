#pragma once

#include <Arduino.h>

#include <vector>

#include "services/calendar/CalendarEvent.h"

// Descarga del calendario de Google (URL iCal privada) y lista de eventos de
// hoy y mañana.
//
// La descarga va en una tarea aparte (núcleo 0) para que la pantalla no se
// congele mientras tanto: al arrancar, cada 15 minutos, al cambiar de día y al
// guardar una URL nueva. La URL se guarda en Preferences (espacio "cal") y nunca
// se escribe en el monitor serie ni se devuelve a la web.
namespace CalendarService {

enum class State {
  NoUrl,        // todavía no se ha introducido la URL
  Waiting,      // esperando a tener Wi-Fi y hora
  Downloading,
  Ok,
  Error,
};

struct Status {
  State state;
  String error;       // motivo del último error (solo con State::Error)
  time_t lastOk;      // hora de la última descarga correcta (0 = ninguna)
  size_t eventCount;  // elementos de la lista de hoy y mañana
};

// Lee la URL guardada y arranca la tarea de descarga. Llamar después de TimeService::begin().
void begin();

bool hasUrl();

// Guarda una URL nueva, borra los eventos del calendario anterior y la descarga ya.
void setUrl(const String& url);

// Pide una descarga lo antes posible.
void refreshNow();

Status status();

// Copia de la lista de eventos, ordenada por día y hora.
std::vector<CalendarEvent> events();

// Cambia cada vez que llega una lista nueva, para saber si hay que volver a pedirla.
uint32_t version();

}  // namespace CalendarService
