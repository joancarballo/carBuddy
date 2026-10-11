#pragma once

#include <Arduino.h>

// Conexión Wi-Fi. La web de configuración está en web/.
//
// Si hay una Wi-Fi guardada y se puede conectar, la placa funciona como cliente.
// Si no, crea su propia red abierta (AP_SSID) para poder configurarla.
namespace Net {

// Red que crea la placa cuando no puede conectarse a ninguna (solo para configurar).
constexpr const char* AP_SSID = "carBuddy-setup";

// Nombre en la red local: http://carbuddy.local
constexpr const char* MDNS_NAME = "carbuddy";

// Función que se llama a menudo mientras se espera a la Wi-Fi en begin(),
// para poder animar la pantalla (begin() no vuelve hasta conectar o rendirse).
void setWaitCallback(void (*callback)());

// Conecta a la Wi-Fi guardada (hasta 15 s) o crea la red de configuración.
void begin();

// Vigila la Wi-Fi. Llamar en cada vuelta del loop().
// Si la Wi-Fi guardada no está conectada durante 2 minutos, crea la red de
// configuración; mientras existe, reintenta la Wi-Fi cada 30 s (si nadie está
// conectado a esa red) y la apaga en cuanto vuelve.
void loop();

// true si la placa está creando su propia red de configuración.
bool isApMode();

// true si está conectada como cliente a una Wi-Fi.
bool isConnected();

// true si se ha perdido la Wi-Fi guardada y la placa está intentando recuperarla
// durante los 2 minutos previos a crear la red de configuración.
bool isReconnecting();

// Nombre de la Wi-Fi guardada ("" si no hay ninguna).
String ssid();

// IP de la placa: la de la Wi-Fi, o la de su propia red en modo configuración.
// Devuelve "" si no tiene.
String ip();

// Nombre en la red local, "carbuddy.local".
String hostName();

// Guarda una Wi-Fi nueva y reinicia la placa al cabo de un momento para conectarse a ella.
void saveWifi(const String& ssid, const String& pass);

}  // namespace Net
