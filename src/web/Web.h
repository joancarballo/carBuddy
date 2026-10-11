#pragma once

#include <Arduino.h>
#include <WebServer.h>

// Web de configuración servida por la placa (http://carbuddy.local o la IP).
//
// Toda la web vive en esta carpeta:
//   Web.cpp             servidor, página principal y utilidades comunes
//   WifiSection.cpp     sección "Settings App": Wi-Fi
//   ClockSection.cpp    sección "Time App": hora y huso horario
//   CalendarSection.cpp sección "Calendario": calendarios iCal y vista previa
//
// Cada sección solo usa los servicios (Net, TimeService, CalendarService),
// nunca las apps de la pantalla. HTML sencillo con un estilo propio dentro de la
// página (Web.cpp), sin librerías ni recursos externos.
namespace Web {

// Registra las rutas de todas las secciones y arranca el servidor.
// Llamar después de Net::begin().
void begin();

// Atiende las peticiones. Llamar en cada vuelta del loop().
void loop();

// Servidor, para que las secciones registren sus rutas.
WebServer& server();

// Escapa &, <, > y " para poder meter un texto dentro del HTML.
String htmlEscape(const String& text);

// Página completa (con el estilo) con un solo bloque de contenido, para
// respuestas sueltas como la de guardar la Wi-Fi.
String page(const String& body);

// Responde a un formulario volviendo a la página principal, en la sección anchor.
void redirectHome(const char* anchor);

}  // namespace Web
