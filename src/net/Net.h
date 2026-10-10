#pragma once

#include <Arduino.h>
#include <WebServer.h>

// Conexión Wi-Fi y servidor web.
//
// Si hay una Wi-Fi guardada y se puede conectar, la placa funciona como cliente.
// Si no, crea su propia red abierta (AP_SSID) para poder configurarla.
namespace Net {

// Red que crea la placa cuando no puede conectarse a ninguna (solo para configurar).
constexpr const char* AP_SSID = "carBuddy-setup";

// Nombre en la red local: http://carbuddy.local
constexpr const char* MDNS_NAME = "carbuddy";

// Conecta a la Wi-Fi guardada (hasta 15 s) o crea la red de configuración.
// Arranca el servidor web.
void begin();

// Atiende las peticiones web. Llamar en cada vuelta del loop().
void loop();

// Servidor web, para que cada app registre sus propias rutas.
WebServer& server();

// Función que devuelve el contenido (HTML) de la página principal.
void setPageBuilder(String (*builder)());

// true si la placa está creando su propia red de configuración.
bool isApMode();

// true si está conectada como cliente a una Wi-Fi.
bool isConnected();

// Nombre de la Wi-Fi guardada ("" si no hay ninguna).
String ssid();

// IP de la placa: la de la Wi-Fi, o la de su propia red en modo configuración.
// Devuelve "" si no tiene.
String ip();

// Nombre en la red local, "carbuddy.local".
String hostName();

// Escapa &, <, > y " para poder meter un texto dentro del HTML.
String htmlEscape(const String& text);

// Estado de la Wi-Fi y botón para editarla, en HTML.
String wifiSectionHtml();

}  // namespace Net
