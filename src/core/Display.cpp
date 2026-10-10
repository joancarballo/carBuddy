#include "core/Display.h"

#include "core/Emoji.h"
#include "pins.h"

// Pantalla SH1106 de 128x64 por I2C.
// Si la imagen sale desplazada un par de píxeles o con una columna de ruido,
// prueba con la línea alternativa (controlador SSD1306).
static U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, PIN_OLED_SCL, PIN_OLED_SDA);
// static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, PIN_OLED_SCL, PIN_OLED_SDA);

namespace {

const int DOT_RADIUS = 3;  // punto que sustituye a los emojis desconocidos
const int DOT_WIDTH = DOT_RADIUS * 2 + 1;

// Lee un carácter UTF-8 y avanza p. Los bytes no válidos se devuelven como '?'.
uint32_t nextCodepoint(const char*& p) {
  const uint8_t c = (uint8_t)*p++;
  int extra;
  uint32_t cp;
  if (c < 0x80) {
    return c;
  } else if ((c & 0xE0) == 0xC0) {
    cp = c & 0x1F;
    extra = 1;
  } else if ((c & 0xF0) == 0xE0) {
    cp = c & 0x0F;
    extra = 2;
  } else if ((c & 0xF8) == 0xF0) {
    cp = c & 0x07;
    extra = 3;
  } else {
    return '?';
  }
  for (int i = 0; i < extra; i++) {
    if (((uint8_t)*p & 0xC0) != 0x80) {
      return '?';  // carácter incompleto: no se avanza más allá del final
    }
    cp = (cp << 6) | ((uint8_t)*p++ & 0x3F);
  }
  return cp;
}

bool isRegionalIndicator(uint32_t cp) {
  return cp >= 0x1F1E6 && cp <= 0x1F1FF;
}

// Caracteres invisibles que acompañan a los emojis: selectores de variación,
// tonos de piel, etiquetas (banderas regionales) y el marco de las teclas (1️⃣).
bool isInvisible(uint32_t cp) {
  return (cp >= 0xFE00 && cp <= 0xFE0F) || (cp >= 0x1F3FB && cp <= 0x1F3FF) ||
         (cp >= 0xE0020 && cp <= 0xE007F) || cp == 0x20E3 || cp == 0x200B || cp == 0x200C;
}

// Signos tipográficos que la fuente no tiene, cambiados por otros parecidos.
const char* replacement(uint32_t cp) {
  switch (cp) {
    case 0x2018:
    case 0x2019:
    case 0x201A: return "'";
    case 0x201C:
    case 0x201D:
    case 0x201E: return "\"";
    case 0x2013:
    case 0x2014: return "-";
    case 0x2026: return "...";
    case 0x2022: return "\xC2\xB7";  // punto medio
    case 0x20AC: return "EUR";
    case 0x00A0: return " ";
    case '\t':
    case '\n':
    case '\r': return " ";
    default: return nullptr;
  }
}

bool fontHasGlyph(uint32_t cp) {
  return cp <= 0xFFFF && u8g2_IsGlyph(u8g2.getU8g2(), (uint16_t)cp);
}

// Recorre el texto, lo dibuja si draw es true y devuelve su ancho.
// El texto normal se acumula en tramos que se dibujan con la fuente; cada emoji
// se dibuja aparte con su dibujo de core/Emoji.cpp, o con un punto si no lo tiene.
int walk(int x, int baselineY, const String& s, bool draw) {
  int width = 0;
  String run;

  // Dibuja el tramo de texto acumulado y deja 1 píxel de separación.
  auto flushRun = [&]() {
    if (run.length() == 0) {
      return;
    }
    if (draw) {
      u8g2.drawUTF8(x + width, baselineY, run.c_str());
    }
    width += u8g2.getUTF8Width(run.c_str()) + 1;
    run = "";
  };

  auto drawArt = [&](const Emoji::Art* art) {
    flushRun();
    if (draw) {
      Emoji::draw(x + width, baselineY - Emoji::HEIGHT + 1, art);
    }
    width += Emoji::width(art) + 1;
  };

  auto drawDot = [&]() {
    flushRun();
    if (draw) {
      u8g2.drawDisc(x + width + DOT_RADIUS, baselineY - DOT_RADIUS, DOT_RADIUS);
    }
    width += DOT_WIDTH + 1;
  };

  const char* p = s.c_str();
  while (*p != '\0') {
    const char* start = p;
    const uint32_t cp = nextCodepoint(p);

    if (isInvisible(cp)) {
      continue;
    }

    // Unión de emojis (👨‍👩‍👧): solo se dibuja el primero, así que se salta el siguiente.
    if (cp == 0x200D) {
      if (*p != '\0') {
        nextCodepoint(p);
      }
      continue;
    }

    // Banderas: dos letras regionales seguidas forman una bandera.
    if (isRegionalIndicator(cp)) {
      const char* peek = p;
      const uint32_t second = (*peek != '\0') ? nextCodepoint(peek) : 0;
      if (isRegionalIndicator(second)) {
        p = peek;
        const Emoji::Art* flag = Emoji::findFlag(cp, second);
        if (flag != nullptr) {
          drawArt(flag);
        } else {
          drawDot();
        }
      } else {
        drawDot();
      }
      continue;
    }

    if (cp >= 0x2000) {
      const Emoji::Art* art = Emoji::find(cp);
      if (art != nullptr) {
        drawArt(art);
        continue;
      }
    }

    const char* rep = replacement(cp);
    if (rep != nullptr) {
      run += rep;
    } else if (fontHasGlyph(cp)) {
      run += s.substring(start - s.c_str(), p - s.c_str());
    } else {
      drawDot();
    }
  }
  flushRun();

  // El último elemento no necesita separación a su derecha.
  return width > 0 ? width - 1 : 0;
}

// Quita el último carácter UTF-8 completo.
void removeLastChar(String& s) {
  int i = s.length() - 1;
  while (i > 0 && ((uint8_t)s[i] & 0xC0) == 0x80) {
    i--;
  }
  s.remove(i);
}

// Parte una palabra demasiado larga en trozos que quepan en maxWidth.
// El último trozo también se añade; quien llama lo recupera para seguir la línea.
void splitLongWord(const String& word, int maxWidth, std::vector<String>& lines) {
  String rest = word;
  while (rest.length() > 0) {
    String piece = Display::fit(rest, maxWidth);
    if (piece.length() == 0) {
      // Ni un carácter cabe: se pone uno igualmente para no quedarse en bucle.
      const char* p = rest.c_str();
      nextCodepoint(p);
      piece = rest.substring(0, p - rest.c_str());
    }
    rest = rest.substring(piece.length());
    lines.push_back(piece);
  }
}

// Parte un párrafo (sin saltos de línea) en líneas, por palabras.
void wrapParagraph(const String& paragraph, int maxWidth, std::vector<String>& lines) {
  String line;
  int start = 0;
  const int len = paragraph.length();

  while (start <= len) {
    int end = paragraph.indexOf(' ', start);
    if (end < 0) {
      end = len;
    }
    String word = paragraph.substring(start, end);
    start = end + 1;
    if (word.length() == 0) {
      continue;
    }

    String candidate = line.length() > 0 ? line + " " + word : word;
    if (walk(0, 0, candidate, false) <= maxWidth) {
      line = candidate;
      continue;
    }

    if (line.length() > 0) {
      lines.push_back(line);
      line = "";
    }
    if (walk(0, 0, word, false) <= maxWidth) {
      line = word;
    } else {
      splitLongWord(word, maxWidth, lines);
      line = lines.back();
      lines.pop_back();
    }
  }

  if (line.length() > 0) {
    lines.push_back(line);
  }
}

}  // namespace

namespace Display {

void begin() {
  u8g2.setBusClock(400000);
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);
}

U8G2& gfx() {
  return u8g2;
}

void text(int x, int baselineY, const String& s) {
  walk(x, baselineY, s, true);
}

void textCentered(int baselineY, const String& s) {
  walk((WIDTH - textWidth(s)) / 2, baselineY, s, true);
}

int textWidth(const String& s) {
  return walk(0, 0, s, false);
}

String fit(const String& s, int maxWidth) {
  String out = s;
  while (out.length() > 0 && textWidth(out) > maxWidth) {
    removeLastChar(out);
  }
  return out;
}

std::vector<String> wrap(const String& s, int maxWidth) {
  std::vector<String> lines;
  int start = 0;
  const int len = s.length();
  bool lastWasBlank = true;  // evita líneas vacías al principio y repetidas

  while (start <= len) {
    int end = s.indexOf('\n', start);
    if (end < 0) {
      end = len;
    }
    String paragraph = s.substring(start, end);
    paragraph.trim();
    start = end + 1;

    if (paragraph.length() == 0) {
      if (!lastWasBlank) {
        lines.push_back("");
        lastWasBlank = true;
      }
      continue;
    }
    wrapParagraph(paragraph, maxWidth, lines);
    lastWasBlank = false;
  }

  while (!lines.empty() && lines.back().length() == 0) {
    lines.pop_back();
  }
  return lines;
}

int textWrapped(int x, int baselineY, int maxWidth, int lineHeight, const String& s) {
  std::vector<String> lines = wrap(s, maxWidth);
  for (size_t i = 0; i < lines.size(); i++) {
    text(x, baselineY + i * lineHeight, lines[i]);
  }
  return lines.size();
}

void showMessage(const char* message) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawUTF8(0, 10, "carBuddy");
  u8g2.drawUTF8(0, 30, message);
  u8g2.sendBuffer();
}

}  // namespace Display
