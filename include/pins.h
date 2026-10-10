#pragma once

// Asignación de pines de la ESP32-S3 N16R8.
// Si cambias el cableado, este es el único sitio que hay que tocar.

// Pantalla OLED (I2C)
constexpr int PIN_OLED_SDA = 8;
constexpr int PIN_OLED_SCL = 9;

// Dial rotativo (EC11) y su pulsador
constexpr int PIN_ENC_A = 4;     // TRA
constexpr int PIN_ENC_B = 5;     // TRB
constexpr int PIN_ENC_PUSH = 6;  // PSH

// Botones
constexpr int PIN_BTN_BACK = 7;      // BAK (KEY0)
constexpr int PIN_BTN_CONFIRM = 15;  // CON (KEY1)
