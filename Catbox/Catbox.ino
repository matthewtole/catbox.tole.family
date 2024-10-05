#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>
// #include <AsyncElegantOTA.h>
#include <ESPmDNS.h>
#include <TimeLib.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneButton.h>

#include "secrets.h"

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
time_t lastWifiCheck = 0;
uint8_t wifi_status = WL_IDLE_STATUS;
// AsyncWebServer server(80);

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

static const uint8_t icon_poop[8] = { 0x00, 0x00, 0x10, 0x18, 0x3c, 0x3c, 0x7e, 0x7e };
static const uint8_t icon_food[8] = { 0x00, 0x6a, 0x6e, 0x64, 0x44, 0x44, 0x44, 0x44 };
static const uint8_t icon_water[8] = { 0x08, 0x18, 0x3c, 0x3c, 0x7e, 0x7a, 0x3c, 0x18 };
static const uint8_t icon_online[8] = { 0x00, 0x3c, 0x42, 0x99, 0x24, 0x42, 0x18, 0x00 };
static const uint8_t icon_offline[8] = { 0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00 };
static const uint8_t icon_time[8] = { 0x3c, 0x42, 0x91, 0x91, 0x8d, 0x81, 0x42, 0x3c };

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


#define PIN_POOP_RING 15
#define PIN_FOOD_RING 2
#define PIN_WATER_RING 4

#define RING_PIXELS 8

Adafruit_NeoPixel ring_poop(RING_PIXELS, PIN_POOP_RING, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring_food(RING_PIXELS, PIN_FOOD_RING, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring_water(RING_PIXELS, PIN_WATER_RING, NEO_GRB + NEO_KHZ800);

#define PIN_POOP_BUTTON 35
#define PIN_POOP_INDICATOR 27
OneButton btn_poop = OneButton(PIN_POOP_BUTTON, true, true);
static void handle_poop_click();
bool poop_indicator = false;

#define PIN_FOOD_BUTTON 32
#define PIN_FOOD_INDICATOR 26
OneButton btn_food = OneButton(PIN_FOOD_BUTTON, true, true);
static void handle_food_click();
bool food_indicator = false;

#define PIN_WATER_BUTTON 33
#define PIN_WATER_INDICATOR 25
OneButton btn_water = OneButton(PIN_WATER_BUTTON, true, true);
static void handle_water_click();
bool water_indicator = false;

void draw_boot_screen() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.drawBitmap(4, 4, logo, 24, 24, 1);

  display.setCursor(36, 12);
  display.println(F("CATBOX v3.0.0"));

  display.display();
}

void draw_status_screen() {
  display.clearDisplay();

  // Setup the display
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // TOP ROW
  display.drawBitmap(2, 2, icon_online, 8, 8, 1);
  display.setCursor(12, 2);
  display.println(WiFi.localIP());

  // MIDDLE ROW
  display.drawBitmap(2, 12, icon_food, 8, 8, 1);
  display.setCursor(12, 14);
  display.print("00000");

  display.drawBitmap(44, 12, icon_poop, 8, 8, 1);
  display.setCursor(54, 14);
  display.print("00000");

  display.drawBitmap(86, 12, icon_water, 8, 8, 1);
  display.setCursor(96, 14);
  display.print("00000");

  // BOTTOM ROW
  display.drawBitmap(2, 24, icon_time, 8, 8, 1);
  display.setCursor(12, 24);
  display.print("00:00:00");

  display.display();
}


void check_wifi() {
  if (now() >= lastWifiCheck + 60) {
    lastWifiCheck = now();
    wifi_status = WiFi.status();
  }
}

void update_ring(Adafruit_NeoPixel *ring, uint32_t color) {
  for (int i = 0; i < RING_PIXELS; i++) {
    ring->setPixelColor(i, color);
  }
  ring->show();
}

void update_rings() {
  update_ring(&ring_poop, Adafruit_NeoPixel::Color(230, 81, 0));
  update_ring(&ring_food, Adafruit_NeoPixel::Color(27, 94, 32));
  update_ring(&ring_water, Adafruit_NeoPixel::Color(79, 195, 247));
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

  // Setup the HTTP server to handle OTA updates
  // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   request->send(200, "text/plain", "CatBox v3");
  // });

  // AsyncElegantOTA.begin(&server);
  // server.begin();
}



void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  draw_boot_screen();

  ring_poop.begin();
  ring_poop.setBrightness(255);
  update_ring(&ring_poop, ring_poop.Color(255, 255, 255));

  ring_food.begin();
  ring_food.setBrightness(255);
  update_ring(&ring_food, ring_food.Color(255, 255, 255));

  ring_water.begin();
  ring_water.setBrightness(255);
  update_ring(&ring_water, ring_water.Color(255, 255, 255));

  setup_wifi();
  // TODO: Get the data from the server

  btn_poop.attachClick(handle_poop_click);
  pinMode(PIN_POOP_INDICATOR, OUTPUT);

  btn_food.attachClick(handle_food_click);
  pinMode(PIN_FOOD_INDICATOR, OUTPUT);

  btn_water.attachClick(handle_water_click);
  pinMode(PIN_WATER_INDICATOR, OUTPUT);
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

  btn_poop.tick();
  digitalWrite(PIN_POOP_INDICATOR, poop_indicator ? HIGH : LOW);

  btn_food.tick();
  digitalWrite(PIN_FOOD_INDICATOR, food_indicator ? HIGH : LOW);

  btn_water.tick();
  digitalWrite(PIN_WATER_INDICATOR, water_indicator ? HIGH : LOW);
}

static void handle_poop_click() {
  poop_indicator = !poop_indicator;
  Serial.println("Poop");
}

static void handle_food_click() {
  food_indicator = !food_indicator;
  Serial.println("Food");
}

static void handle_water_click() {
  water_indicator = !water_indicator;
  Serial.println("Water");
}
