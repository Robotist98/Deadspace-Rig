#pragma once

// Copy this file to config.h and fill in your settings before flashing.
// ESP8266 supports 2.4 GHz Wi-Fi networks.
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// Optional fixed IP. Leave empty to let the router assign an address (DHCP).
// Choose an unused address reserved for the ESP or outside the DHCP pool.
#define WIFI_STATIC_IP ""
// Example network values: change these to match your router when using a fixed IP.
#define WIFI_GATEWAY "192.168.0.1"
#define WIFI_SUBNET "255.255.255.0"
// Empty uses the gateway as the DNS server.
#define WIFI_DNS ""

// OTA uploads automatically use this password from your local config.h.
// OTA stays disabled while this is empty.
#define OTA_PASSWORD ""
#define OTA_HOSTNAME "deadspace-leds"
