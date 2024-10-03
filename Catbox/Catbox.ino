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
uint8_t wifi_status = WL_IDLE_STATUS;
AsyncWebServer server(80);

// Generated using this online tool
// https://jrabausch.github.io/lcd-image/web/

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

static const uint8_t icon_poop[8] = {0x00,0x00,0x10,0x18,0x3c,0x3c,0x7e,0x7e};
static const uint8_t icon_food[8] = {0x00,0x6a,0x6e,0x64,0x44,0x44,0x44,0x44};
static const uint8_t icon_water[8] = {0x08,0x18,0x3c,0x3c,0x7e,0x7a,0x3c,0x18};
static const uint8_t icon_online[8] = {0x00,0x3c,0x42,0x99,0x24,0x42,0x18,0x00};
static const uint8_t icon_offline[8] = {0x00,0x18,0x18,0x18,0x18,0x00,0x18,0x00};
static const uint8_t icon_time[8] = {0x3c,0x42,0x91,0x91,0x8d,0x81,0x42,0x3c};

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);




#define PIN_POOP_RING   5
#define PIN_FOOD_RING   5
#define PIN_WATER_RING  5

#define RING_PIXELS 8

Adafruit_NeoPixel poop_ring(RING_PIXELS, PIN_POOP_RING, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel food_ring(RING_PIXELS, PIN_FOOD_RING, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel water_ring(RING_PIXELS, PIN_WATER_RING, NEO_GRB + NEO_KHZ800);



void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  draw_boot_screen();

   poop_ring.begin();
   poop_ring.setBrightness(100);
   update_ring(&poop_ring, poop_ring.Color(255, 255, 255));
   
   food_ring.begin();
   food_ring.setBrightness(100);
   update_ring(&food_ring, poop_ring.Color(255, 255, 255));

   water_ring.begin();
   water_ring.setBrightness(100);
   update_ring(&water_ring, poop_ring.Color(255, 255, 255));

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
  // Setup the display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // TOP ROW
  display.drawBitmap(2, 2, wifi_status == WL_CONNECTED  ? icon_online : icon_no_internet, 8, 8, 1);
  display.setCursor(12, 4);
  display.println(WiFi.localIP());

  // MIDDLE ROW
  display.drawBitmap(2, 12, icon_poop, 8, 8, 1);
  display.setCursor(12, 12);
  display.print("00000");
  
  display.drawBitmap(44, 12, icon_water, 8, 8, 1);
  display.setCursor(54, 12);
  display.print("00000");
  
  display.drawBitmap(86, 12, icon_water, 8, 8, 1);
  display.setCursor(96, 12);
  display.print("00000");

  // BOTTOM ROW
  // TODO: Add the time since last update

  display.display();
}

void loop() {
  static uint16_t prev = millis();
  uint16_t _now = millis();
  if ((uint16_t)(_now - prev) >= 5) {
    check_wifi();
    draw_status_screen(); 
    update_rings();
    prev = _now;
  }
}

void check_wifi() {
  if (now() >= lastWifiCheck + 60) {
    lastWifiCheck = now();
    wifi_status = WiFi.status();
  }
}

void update_rings() {
  update_ring(&ring_poop, ring_poop.Color(255, 0, 0));
  update_ring(&ring_food, ring_food.Color(0, 255, 0));
  update_ring(&ring_water, ring_water.Color(0, 0, 255));
}

void update_ring(Adafruit_NeoPixel *ring, uint32_t color) {
  for (int i = 0; i < RING_PIXELS; i++) {
    ring->setPixelColor(i, color);
  }
  ring->show();

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

  wifi_status = WiFi.status();

  // Setup the HTTP server to handle OTA updates
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "CatBox v3");
  });

  AsyncElegantOTA.begin(&server);
  server.begin();
}
