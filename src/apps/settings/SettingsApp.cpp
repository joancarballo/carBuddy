#include "apps/settings/SettingsApp.h"

#include "core/Display.h"
#include "net/Net.h"

void SettingsApp::draw() {
  Display::gfx().setFont(u8g2_font_5x8_tf);

  // En modo configuración hace falta sitio para las instrucciones, así que
  // las líneas van más juntas.
  const bool showAp = Net::isApMode();
  const int lineHeight = showAp ? 9 : 13;
  int y = showAp ? 9 : 16;

  if (showAp) {
    String instructions = String("Conectate al wifi ") + Net::AP_SSID + " para configurar.";
    int lines = Display::textWrapped(0, y, Display::WIDTH, lineHeight, instructions);
    y += lines * lineHeight;
  }
  
  String wifi = Net::isConnected() ? Net::ssid() : String("No conectada");
  Display::text(0, y, Display::fit("Wifi: " + wifi, Display::WIDTH).c_str());
  y += lineHeight;

  String ip = Net::ip();
  Display::text(0, y, ("IP: " + (ip.length() > 0 ? ip : String("-"))).c_str());
  y += lineHeight;

  // Conectada se puede usar el nombre local; en modo configuración, la IP.
  String address = Net::isConnected() ? Net::hostName() : (ip.length() > 0 ? ip : String("-"));
  Display::text(0, y, ("URL/IP: " + address).c_str());
}

String SettingsApp::webSection() {
  return "<h2>Settings App</h2>" + Net::wifiSectionHtml();
}
