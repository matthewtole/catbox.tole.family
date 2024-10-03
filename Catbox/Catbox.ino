#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>
#include <TimeLib.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "secrets.h"

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
time_t lastWifiCheck = 0;

static const uint8_t logo[72] = {
  0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0x07, 0x83, 
    0xc0, 0x07, 0xc7, 0xe0, 0x07, 0xe7, 0xc0, 0x07, 
    0xe7, 0xe0, 0x07, 0xe7, 0xc0, 0x23, 0xc7, 0xc4, 
    0xf1, 0xc3, 0x8f, 0xf8, 0x00, 0x1f, 0xf8, 0x10, 
    0x1f, 0x78, 0x7e, 0x1e, 0x79, 0xff, 0x9e, 0x11, 
    0xff, 0xc8, 0x03, 0xff, 0xc0, 0x07, 0xff, 0xe0, 
    0x0f, 0xff, 0xf0, 0x0f, 0xff, 0xf0, 0x1f, 0xff, 
    0xf8, 0x1f, 0xff, 0xf8, 0x1f, 0xff, 0xf8, 0x1f, 
    0xc5, 0xf0, 0x04, 0x00, 0x20, 0x00, 0x00, 0x00
};
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define PIN        5

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 8 * 3

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
AsyncWebServer server(80);

void setup() {
  Serial.begin(9600);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

   pixels.begin();
   pixels.setBrightness(20);

  draw_boot_screen();
  delay(2000);

  setupWifi();
  // TODO: Get the data from the server
}

void draw_boot_screen() {
  display.clearDisplay();
  display.drawBitmap(4, 4, logo, 24, 24, 1);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(36, 12);
  display.println(F("CATBOX v3.0.0"));
  display.display();
}

void draw_status_screen() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(WiFi.localIP());

  display.setCursor(0, 12);
  display.println("Uptime: 00:00:43:00");

  display.display();
}

void loop() {
  static uint16_t prev = millis();
  // DO NOT USE delay(5) to do this.
  // The (uint16_t) cast is required on 32-bit processors, harmless on 8-bit.
  uint16_t _now = millis();
  if ((uint16_t)(_now - prev) >= 5) {

    if (now() >= lastWifiCheck + 60) {
      lastWifiCheck = now();
      if (WiFi.status() != WL_CONNECTED) {
        // status = STATUS_OFFLINE;
      }
    }

    // updateStatusLed();

    prev = _now;
  }

  draw_status_screen(); 

  pixels.clear();
   for(int i=0; i<8; i++) { 
    pixels.setPixelColor(i, pixels.Color(200, 0, 0));
   }

   for(int i=0; i<8; i++) { 
    pixels.setPixelColor(8 + i, pixels.Color(200, 0, 0));
   }

   for(int i=0; i<8; i++) { 
    pixels.setPixelColor(16 + i, pixels.Color(0, 0, 200));
   }
    

    pixels.show();   // Send the updated pixel colors to the hardware.
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

  // status = STATUS_OK;

  // Setup the HTTP server to handle OTA updates
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "CatBox v3");
  });

  AsyncElegantOTA.begin(&server);
  server.begin();
}
