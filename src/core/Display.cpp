#include "core/Display.h"

#include "pins.h"

// Pantalla SH1106 de 128x64 por I2C.
// Si la imagen sale desplazada un par de píxeles o con una columna de ruido,
// prueba con la línea alternativa (controlador SSD1306).
static U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, PIN_OLED_SCL, PIN_OLED_SDA);
// static U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE, PIN_OLED_SCL, PIN_OLED_SDA);

namespace Display {

void begin() {
  u8g2.setBusClock(400000);
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tf);
}

U8G2& gfx() {
  return u8g2;
}

void text(int x, int baselineY, const char* s) {
  u8g2.drawUTF8(x, baselineY, s);
}

void textCentered(int baselineY, const char* s) {
  int width = u8g2.getUTF8Width(s);
  u8g2.drawUTF8((WIDTH - width) / 2, baselineY, s);
}

String fit(const String& s, int maxWidth) {
  String out = s;
  while (out.length() > 0 && u8g2.getUTF8Width(out.c_str()) > maxWidth) {
    out.remove(out.length() - 1);
  }
  return out;
}

int textWrapped(int x, int baselineY, int maxWidth, int lineHeight, const String& s) {
  int lines = 0;
  String line;
  int start = 0;
  const int len = s.length();

  while (start < len) {
    int end = s.indexOf(' ', start);
    if (end < 0) {
      end = len;
    }
    String word = s.substring(start, end);
    String candidate = line.length() > 0 ? line + " " + word : word;

    if (line.length() > 0 && u8g2.getUTF8Width(candidate.c_str()) > maxWidth) {
      u8g2.drawUTF8(x, baselineY + lines * lineHeight, line.c_str());
      lines++;
      line = word;
    } else {
      line = candidate;
    }
    start = end + 1;
  }

  if (line.length() > 0) {
    u8g2.drawUTF8(x, baselineY + lines * lineHeight, line.c_str());
    lines++;
  }
  return lines;
}

void showMessage(const char* message) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawUTF8(0, 10, "carBuddy");
  u8g2.drawUTF8(0, 30, message);
  u8g2.sendBuffer();
}

}  // namespace Display
