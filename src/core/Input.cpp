#include "core/Input.h"

#include "pins.h"

namespace {

// Decodificación del dial por cuadratura. Cada transición válida suma o resta 1;
// un clic completo del dial son 4 transiciones.
const int8_t ENC_TABLE[16] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};
volatile uint8_t encState = 0;
volatile int32_t encTicks = 0;
int32_t consumedTicks = 0;

void IRAM_ATTR onEncoderChange() {
  encState = ((encState << 2) | (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B)) & 0x0F;
  encTicks += ENC_TABLE[encState];
}

struct Button {
  uint8_t pin;
  InputEvent event;
  bool down;
};

Button buttons[] = {
    {PIN_ENC_PUSH, InputEvent::DialPress, false},
    {PIN_BTN_CONFIRM, InputEvent::Confirm, false},
    {PIN_BTN_BACK, InputEvent::Back, false},
};

}  // namespace

namespace Input {

void begin() {
  pinMode(PIN_ENC_A, INPUT_PULLUP);
  pinMode(PIN_ENC_B, INPUT_PULLUP);
  for (Button& b : buttons) {
    pinMode(b.pin, INPUT_PULLUP);
  }

  encState = (digitalRead(PIN_ENC_A) << 1) | digitalRead(PIN_ENC_B);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_A), onEncoderChange, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_B), onEncoderChange, CHANGE);
}

bool poll(InputEvent& ev) {
  // Giro del dial: un evento por cada clic completo (4 transiciones).
  // Si el sentido es el contrario al que esperas, intercambia PIN_ENC_A y PIN_ENC_B en pins.h.
  int32_t diff = encTicks - consumedTicks;
  if (diff >= 4) {
    consumedTicks += 4;
    ev = InputEvent::Right;
    return true;
  }
  if (diff <= -4) {
    consumedTicks -= 4;
    ev = InputEvent::Left;
    return true;
  }

  // Botones: un evento en el momento de pulsar.
  for (Button& b : buttons) {
    bool down = (digitalRead(b.pin) == LOW);
    if (down && !b.down) {
      b.down = true;
      ev = b.event;
      return true;
    }
    if (!down) {
      b.down = false;
    }
  }
  return false;
}

}  // namespace Input
