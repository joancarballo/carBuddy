#pragma once

#include <stdint.h>

// Emojis dibujados a mano para la pantalla monocroma.
//
// La librería U8g2 no puede dibujar emojis: solo admite caracteres hasta U+FFFF
// y sus fuentes de emojis son demasiado grandes. Por eso cada emoji es un dibujo
// de 8 filas escrito con '#' (píxel encendido) y '.' (apagado).
namespace Emoji {

constexpr int HEIGHT = 8;

struct Art {
  const char* rows[HEIGHT];  // todas las filas tienen el mismo ancho
};

// Dibujo de un emoji por su punto de código, o nullptr si no está en la tabla.
const Art* find(uint32_t codepoint);

// Dibujo de una bandera a partir de sus dos letras regionales (por ejemplo,
// U+1F1EC U+1F1E7 para el Reino Unido), o nullptr si no está en la tabla.
const Art* findFlag(uint32_t first, uint32_t second);

// Ancho en píxeles de un dibujo.
int width(const Art* art);

// Dibuja con el color actual de la pantalla. topY es la fila superior del dibujo.
// Los píxeles apagados no se tocan, así que también se ve sobre un fondo encendido.
void draw(int x, int topY, const Art* art);

}  // namespace Emoji
