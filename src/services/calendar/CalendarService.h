#pragma once

#include <Arduino.h>

#include <vector>

#include "services/calendar/CalendarEvent.h"

// Descarga de los calendarios de Google (URL iCal privada de cada uno) y lista
// conjunta de eventos de hoy y mañana.
//
// Se pueden guardar hasta MAX_CALENDARS calendarios, cada uno con un nombre.
// La descarga va en una tarea aparte (núcleo 0) para que la pantalla no se
// congele mientras tanto: al arrancar, cada 15 minutos, al cambiar de día y al
// añadir un calendario. Si un calendario falla, se siguen mostrando sus últimos
// eventos. Las URL se guardan en Preferences (espacio "cal") y nunca se escriben
// en el monitor serie ni se devuelven a la web.
namespace CalendarService {

const int MAX_CALENDARS = 5;

enum class State {
  NoUrl,        // todavía no hay ningún calendario
  Waiting,      // esperando a tener Wi-Fi y hora
  Downloading,
  Ok,
  Error,        // algún calendario no se pudo descargar
};

struct Status {
  State state;
  String error;       // motivo del primer error, con el nombre del calendario
  time_t lastOk;      // hora de la última descarga correcta (0 = ninguna)
  size_t eventCount;  // elementos de la lista de hoy y mañana
};

// Datos de un calendario para la web (sin la URL, que es secreta).
struct CalendarInfo {
  String name;
  String error;  // vacío si la última descarga fue bien
  size_t eventCount;
};

// Lee los calendarios guardados y arranca la tarea de descarga.
// Llamar después de TimeService::begin().
void begin();

bool hasCalendars();

// Calendarios guardados, en orden.
std::vector<CalendarInfo> calendars();

// Añade un calendario y lo descarga ya. Si el nombre está vacío se usa
// "Calendario N". Devuelve false si ya hay MAX_CALENDARS.
bool addCalendar(const String& name, const String& url);

// Cambia el nombre del calendario de esa posición.
void renameCalendar(int index, const String& name);

// Quita el calendario de esa posición y sus eventos.
void removeCalendar(int index);

// Pide una descarga lo antes posible.
void refreshNow();

Status status();

// Copia de la lista de eventos de todos los calendarios, ordenada por día y hora.
std::vector<CalendarEvent> events();

// Cambia cada vez que cambia la lista, para saber si hay que volver a pedirla.
uint32_t version();

}  // namespace CalendarService
