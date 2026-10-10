# carBuddy

Dispositivo de escritorio basado en una **ESP32-S3** con pantalla OLED, dial y botones. Un único firmware con varias "apps" (reloj, calendario compatible con iCal o Google Calendar,...) que se configura desde una pequeña web servida por la propia placa. Todo corre en la ESP32-S3, sin servidor ni programa externo.

> **Estado:** en desarrollo. El hardware está montado y el firmware está en su fase inicial.

## Apps previstas

- **Reloj:** hora sincronizada por internet.
- **Calendario:** eventos de hoy y de las próximas horas, a partir de la URL iCal de Google Calendar, con vista de mañana.
- **Configuración:** servidor HTTP con una web protegida con contraseña para ajustar la Wi-Fi y las opciones de cada app.

## Hardware

- ESP32-S3 N16R8 (16 MB de flash y 8 MB de PSRAM).
- Módulo OLED de 1,3" (SH1106 o SH1116, interfaz I2C) con encoder rotativo EC11 y dos botones.

### Conexiones

| Pin del módulo | Función             | GPIO de la ESP32-S3 |
| -------------- | ------------------- | ------------------- |
| CON            | Botón CONFIRM       | 15                  |
| SDA            | Datos I2C           | 8                   |
| SCL            | Reloj I2C           | 9                   |
| PSH            | Pulsador del dial   | 6                   |
| TRA            | Dial, fase A        | 4                   |
| TRB            | Dial, fase B        | 5                   |
| BAK            | Botón BACK          | 7                   |
| GND            | Masa                | GND                 |
| VCC            | Alimentación (3,3 V)| 3V3                 |

El módulo ya incluye las resistencias pull-up del I2C, del dial y de los botones. Hay que alimentarlo solo a 3,3 V.

## Controles

La interfaz tiene dos niveles:

| Entrada            | En la pantalla principal | Dentro de una app                         |
| ------------------ | ------------------------ | ----------------------------------------- |
| Girar el dial      | Cambia de app            | Recorre los elementos de la app           |
| Pulsar el dial     | Entra en la app          | Interactúa con el elemento seleccionado   |
| Botón CONFIRM      | -                        | Interacción secundaria                    |
| Botón BACK         | -                        | Vuelve al nivel de apps                   |

## Compilar y cargar

Requisitos: [Visual Studio Code](https://code.visualstudio.com/) con la extensión [PlatformIO](https://platformio.org/).

1. Clona el repositorio y ábrelo en VS Code.
2. Conecta la placa por el puerto USB marcado como UART.
3. Compila y carga desde la barra de PlatformIO (Build y Upload).

La configuración de la placa está en `platformio.ini`.

## Hoja de ruta

- [ ] Prueba de hardware (pantalla, dial y botones)
- [ ] Servidor HTTP de configuración
- [ ] App de reloj
- [ ] App de calendario
- [ ] Otras apps (Claude Buddy...)

## Seguridad

Las credenciales (contraseña de la Wi-Fi, contraseña de la web de configuración, URL privada del calendario) no se escriben en el código ni se suben al repositorio: se introducen desde la web de configuración y se guardan en la memoria de la placa.