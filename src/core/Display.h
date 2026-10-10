#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

// Pantalla OLED de 128x64 y funciones de dibujo comunes a todas las apps.
namespace Display {

constexpr int WIDTH = 128;
constexpr int HEIGHT = 64;

void begin();

// Acceso directo a la librería de dibujo (U8g2).
U8G2& gfx();

// Texto en UTF-8 (admite acentos). Las coordenadas Y son la línea base del texto.
void text(int x, int baselineY, const char* s);
void textCentered(int baselineY, const char* s);

// Recorta un texto para que quepa en maxWidth píxeles con la fuente actual.
String fit(const String& s, int maxWidth);

// Escribe un texto partiéndolo en líneas por palabras.
// Devuelve cuántas líneas ha usado.
int textWrapped(int x, int baselineY, int maxWidth, int lineHeight, const String& s);

// Pantalla sencilla de arranque con un mensaje.
void showMessage(const char* message);

}  // namespace Display
