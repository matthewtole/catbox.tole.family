#include <Arduino.h>

#include "config.h"
#include "timeinfo.h"
#include "wifi-manager.h"

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
unsigned long last_wifi_check = 0;
uint8_t wifi_status = WL_IDLE_STATUS;

#define WIFI_CHECK_INTERVAL 60 * 1000
#define WIFI_RECONNECT_INTERVAL 10 * 1000

void check_wifi() {
  if (mktime(&timeinfo) >= last_wifi_check + WIFI_CHECK_INTERVAL) {
    last_wifi_check = mktime(&timeinfo);
    wifi_status = WiFi.status();

    if (wifi_status != WL_CONNECTED) {
      Serial.println("WiFi disconnected, attempting reconnect");
      WiFi.disconnect();
      delay(WIFI_RECONNECT_INTERVAL);
      WiFi.begin(ssid, password);
    }
  }
}

void setup_wifi() {
  // Connect to the WiFi network
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Register the devices name with mDNS
  if (!MDNS.begin("catbox")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started at catbox.local");

  wifi_status = WiFi.status();
}

String get_wifi_ssid() { return WiFi.SSID(); }
String get_ip_address() { return WiFi.localIP().toString(); }