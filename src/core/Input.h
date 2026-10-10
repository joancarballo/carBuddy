#pragma once

#include <Arduino.h>

// Entradas físicas convertidas en eventos lógicos.
enum class InputEvent : uint8_t {
  None,
  Left,       // un clic del dial en un sentido
  Right,      // un clic del dial en el otro sentido
  DialPress,  // pulsación del dial
  Confirm,    // botón CONFIRM
  Back,       // botón BACK
};

namespace Input {

void begin();

// Devuelve true y rellena ev si hay un evento pendiente.
// Llamar repetidamente hasta que devuelva false.
bool poll(InputEvent& ev);

}  // namespace Input
