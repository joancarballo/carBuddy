// carBuddy - Paso 0: prueba de hardware
//
// Comprueba que la pantalla OLED, el dial y los tres pulsadores responden.
// Muestra el estado en la pantalla y por el monitor serie (115200 baudios).

#include <Arduino.h>
#include <U8g2lib.h>
#include <Wire.h>
#include "pins.h"

// Pantalla SH1106 de 128x64 por I2C.
// Si la imagen sale desplazada un par de píxeles o con una columna de ruido,
// prueba con la línea alternativa (controlador SSD1306).
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE, PIN_OLED_SCL, PIN_OLED_SDA);
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0, U8X8_PIN_NONE, PIN_OLED_SCL, PIN_OLED_SDA);

// Decodificación del dial por cuadratura. Cada transición válida suma o resta 1;
// un paso completo del dial (un "clic") son 4 transiciones.
static const int8_t ENC_TABLE[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
volatile uint8_t encState = 0;
volatile int32_t encTicks = 0;

void IRAM_ATTR onEncoderChange() {
  encState = ((encState << 2) | (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B)) & 0x0F;
  encTicks += ENC_TABLE[encState];
}

struct Button {
  const char* name;
  uint8_t pin;
  bool pressed;
};

Button buttons[] = {
    {"PUSH", PIN_ENC_PUSH, false},
    {"CONFIRM", PIN_BTN_CONFIRM, false},
    {"BACK", PIN_BTN_BACK, false},
};

int32_t lastTicks = 0;
const char* lastDirection = "-";
uint32_t lastDraw = 0;

void scanI2C() {
  Serial.println("Escaneando el bus I2C...");
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  dispositivo en 0x%02X\n", addr);
      found++;
    }
  }
  if (found == 0) {
    Serial.println("  no se ha encontrado ningun dispositivo");
  }
}

void draw(int32_t ticks) {
  char line[32];

  display.clearBuffer();
  display.setFont(u8g2_font_6x10_tf);

  display.drawStr(0, 10, "carBuddy - test HW");

  snprintf(line, sizeof(line), "Clics: %ld", (long)(ticks / 4));
  display.drawStr(0, 24, line);

  snprintf(line, sizeof(line), "Ticks: %ld", (long)ticks);
  display.drawStr(0, 36, line);

  snprintf(line, sizeof(line), "Giro: %s", lastDirection);
  display.drawStr(0, 48, line);

  snprintf(line, sizeof(line), "PSH:%c CON:%c BAK:%c",
    buttons[0].pressed ? 'X' : '-',
    buttons[1].pressed ? 'X' : '-',
    buttons[2].pressed ? 'X' : '-');
  display.drawStr(0, 60, line);

  display.sendBuffer();
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println();
  Serial.println("carBuddy - paso 0: prueba de hardware");

  display.setBusClock(400000);
  display.begin();
  scanI2C();

  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  for (Button& b : buttons) {
    pinMode(b.pin, INPUT_PULLUP);
  }

  encState = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), onEncoderChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_B), onEncoderChange, CHANGE);

  draw(0);
}

void loop() {
  int32_t ticks = encTicks;
  if (ticks != lastTicks) {
    lastDirection = (ticks > lastTicks) ? "sentido +" : "sentido -";
    Serial.printf("Dial: ticks=%ld clics=%ld (%s)\n", (long)ticks, (long)(ticks / 4), lastDirection);
    lastTicks = ticks;
  }

  for (Button& b : buttons) {
    bool pressed = (digitalRead(b.pin) == LOW);
    if (pressed != b.pressed) {
      b.pressed = pressed;
      Serial.printf("Boton %s: %s\n", b.name, pressed ? "PULSADO" : "soltado");
    }
  }

  if (millis() - lastDraw >= 50) {
    lastDraw = millis();
    draw(ticks);
  }
}
