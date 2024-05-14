#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>
#include <TimeLib.h>

#include "secrets.h"
#include "Panel.h"

#define STATUS_BOOTING 0
#define STATUS_OFFLINE 1
#define STATUS_OK 2

Adafruit_NeoPixel ledStatus(1, 26, NEO_GRB + NEO_KHZ800);
uint8_t status = STATUS_BOOTING;

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
time_t lastWifiCheck = 0;

AsyncWebServer server(80);

void setup() {
  Serial.begin(9600);

  ledStatus.begin();
  ledStatus.setBrightness(20);
  status = STATUS_BOOTING;
  updateStatusLed();
  for (uint8_t p = 0; p < NUM_PANELS; p = p + 1) {
    setupPanel(&panels[p], 150);
  }
  setupWifi();
  // TODO: Get the data from the server
}


void loop() {
  static uint16_t prev = millis();
  // DO NOT USE delay(5) to do this.
  // The (uint16_t) cast is required on 32-bit processors, harmless on 8-bit.
  uint16_t _now = millis();
  if ((uint16_t)(_now - prev) >= 5) {
    for (uint8_t p = 0; p < NUM_PANELS; p = p + 1) {
      loopPanel(&panels[p]);
    }

    if (now() >= lastWifiCheck + 60) {
      lastWifiCheck = now();
      if (WiFi.status() != WL_CONNECTED) {
        status = STATUS_OFFLINE;
      }
    }

    updateStatusLed();


    prev = _now;
  }
}

void updateStatusLed() {
  ledStatus.clear();
  switch (status) {
    case STATUS_BOOTING:
      ledStatus.setPixelColor(0, Adafruit_NeoPixel::Color(180, 90, 0));
      break;
    case STATUS_OFFLINE:
      ledStatus.setPixelColor(0, Adafruit_NeoPixel::Color(180, 0, 0));
      break;
    case STATUS_OK:
      ledStatus.setPixelColor(0, Adafruit_NeoPixel::Color(0, 180, 0));
      break;
  }
  ledStatus.show();
}

void setupWifi() {
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

  status = STATUS_OK;

  // Setup the HTTP server to handle OTA updates
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "CatBox v3");
  });

  AsyncElegantOTA.begin(&server);
  server.begin();
}
