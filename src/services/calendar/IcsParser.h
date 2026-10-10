#pragma once

#include <string>

#include "services/calendar/CalendarEvent.h"

// Lector de archivos iCal (.ics) que procesa los bytes según llegan, sin
// guardar el archivo entero: el calendario de Google puede ocupar varios MB.
// Por cada VEVENT completo llama a onEvent. No depende de Arduino.
class IcsParser {
 public:
  typedef void (*Callback)(const IcsEvent& ev, void* context);

  IcsParser(Callback onEvent, void* context);

  // Entrega el siguiente byte del archivo.
  void feed(char c);

  // Avisa de que el archivo ha terminado (procesa la última línea).
  void finish();

 private:
  // Una línea de contenido iCal puede llegar partida en varias ("folding"):
  // las siguientes empiezan por un espacio. Las partes se unen por bytes, antes
  // de interpretar el UTF-8, porque Google parte por bytes y puede dejar un
  // emoji a medias entre dos líneas.
  static const size_t MAX_LINE = 4096;

  void processLine();
  void handleProperty(const std::string& name, const std::string& params, const std::string& value);

  Callback onEvent_;
  void* context_;

  std::string line_;
  bool lineEnded_ = false;  // ya llegó el salto de línea; falta ver si la siguiente la continúa
  bool inEvent_ = false;
  int nestedDepth_ = 0;  // dentro de un VALARM u otro bloque dentro del VEVENT
  IcsEvent event_;
  int64_t durationMinutes_ = -1;  // DURATION, si el evento no trae DTEND
};

namespace Ics {

// Deshace los escapes del texto iCal: \n, \, \; y \\.
std::string unescapeText(const std::string& s);

// Convierte el HTML de las notas de Google en texto: <br> y </p> pasan a saltos
// de línea, el resto de etiquetas desaparece y se traducen &amp;, &nbsp;...
std::string htmlToText(const std::string& s);

// Corta un texto a maxBytes sin partir un carácter UTF-8.
std::string truncateUtf8(const std::string& s, size_t maxBytes);

// Lee una fecha u hora iCal: "20261011", "20261011T143000" (hora local o con TZID)
// o "20261011T123000Z" (UTC). Devuelve -1 si no es válida.
CivilTime::Minutes parseDateTime(const std::string& value, bool& isDate);

}  // namespace Ics
