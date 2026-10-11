#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

#include <vector>

// Pantalla OLED de 128x64 y funciones de dibujo comunes a todas las apps.
namespace Display {

constexpr int WIDTH = 128;
constexpr int HEIGHT = 64;

void begin();

// Acceso directo a la librería de dibujo (U8g2).
U8G2& gfx();

// Texto en UTF-8 con acentos y emojis. Las coordenadas Y son la línea base del texto.
// Los emojis que no están en la tabla de core/Emoji.cpp se dibujan como un punto grande.
// Se usa el color de dibujo actual, así que también sirve para texto sobre fondo encendido.
void text(int x, int baselineY, const String& s);
void textCentered(int baselineY, const String& s);

// Ancho en píxeles de un texto con la fuente actual.
int textWidth(const String& s);

// Recorta un texto para que quepa en maxWidth píxeles con la fuente actual.
// Nunca corta un carácter por la mitad.
String fit(const String& s, int maxWidth);

// Parte un texto en líneas de como mucho maxWidth píxeles, por palabras.
// Respeta los saltos de línea del texto y parte las palabras demasiado largas.
std::vector<String> wrap(const String& s, int maxWidth);

// Escribe un texto partiéndolo en líneas por palabras.
// Devuelve cuántas líneas ha usado.
int textWrapped(int x, int baselineY, int maxWidth, int lineHeight, const String& s);

// Animación de espera: 5 puntos que se encienden por turnos, centrados en (centerX, centerY).
// Se mueve sola con el tiempo, así que basta con redibujarla a menudo.
void waitingDots(int centerX, int centerY);

// Mensaje de espera o de error centrado en la pantalla (o entre las filas top y
// bottom), con la fuente 6x10. Se parte en líneas por palabras y respeta los '\n'.
// Con animated, debajo van los puntos de espera; los errores van sin ellos.
void message(const String& s, bool animated, int top = 0, int bottom = HEIGHT);

// Borra la pantalla, escribe un mensaje centrado y lo envía (para el arranque,
// cuando todavía no se dibujan las apps).
void showMessage(const char* s, bool animated);

}  // namespace Display
