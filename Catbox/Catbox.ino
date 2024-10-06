#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <TimeLib.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneButton.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "Panel.h"
#include "secrets.h"


const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
time_t lastWifiCheck = 0;
uint8_t wifi_status = WL_IDLE_STATUS;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 60 * 60 * -7, 60000);

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

// Adafruit_NeoPixel ring_poop(RING_PIXELS, PIN_POOP_RING, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring_food(RING_PIXELS, PIN_FOOD_RING, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel ring_water(RING_PIXELS, PIN_WATER_RING, NEO_GRB + NEO_KHZ800);

#define PIN_POOP_BUTTON 25
#define PIN_POOP_INDICATOR 26
OneButton btn_poop = OneButton(PIN_POOP_BUTTON, true, true);
static void handle_poop_click();
// bool poop_indicator = false;
// unsigned long poop_time = 0;
#define POOP_DURATION 1 * DAY_HOURS *HOUR_SEC
// char poop_label[6] = "00000";

#define PIN_FOOD_BUTTON 18
#define PIN_FOOD_INDICATOR 5
OneButton btn_food = OneButton(PIN_FOOD_BUTTON, true, true);
static void handle_food_click();
// bool food_indicator = false;
// unsigned long food_time = 0;
#define FOOD_DURATION 14 * DAY_HOURS *HOUR_SEC
// char food_label[6] = "00000";

#define PIN_WATER_BUTTON 32
#define PIN_WATER_INDICATOR 33
OneButton btn_water = OneButton(PIN_WATER_BUTTON, true, true);
static void handle_water_click();
// bool water_indicator = false;
// unsigned long water_time = 0;
#define WATER_DURATION 7 * DAY_HOURS *HOUR_SEC
// char water_label[6] = "00000";

Panel panel_poop = {
  .pin_ring = PIN_POOP_RING,
  .pin_button = PIN_POOP_BUTTON,
  .pin_indicator = PIN_POOP_INDICATOR,
  .last_pressed = 0,
  .duration = POOP_DURATION,
  .color = RED,
  .label = "00000",
  .icon = { 0x00, 0x00, 0x10, 0x18, 0x3c, 0x3c, 0x7e, 0x7e }
};

Panel panel_food = {
  .pin_ring = PIN_FOOD_RING,
  .pin_button = PIN_FOOD_BUTTON,
  .pin_indicator = PIN_FOOD_INDICATOR,
  .last_pressed = 0,
  .duration = FOOD_DURATION,
  .color = GREEN,
  .label = "00000",
  .icon = { 0x00, 0x6a, 0x6e, 0x64, 0x44, 0x44, 0x44, 0x44 }
};

Panel panel_water = {
  .pin_ring = PIN_WATER_RING,
  .pin_button = PIN_WATER_BUTTON,
  .pin_indicator = PIN_WATER_INDICATOR,
  .last_pressed = 0,
  .duration = WATER_DURATION,
  .color = BLUE,
  .label = "00000",
  .icon = { 0x08, 0x18, 0x3c, 0x3c, 0x7e, 0x7a, 0x3c, 0x18 }
};

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
  panel_draw_status(&panel_food, &display, &timeClient, 2, 12);
  panel_draw_status(&panel_poop, &display, &timeClient, 44, 12);
  panel_draw_status(&panel_water, &display, &timeClient, 86, 12);

  // BOTTOM ROW
  display.drawBitmap(2, 24, icon_time, 8, 8, 1);
  display.setCursor(12, 24);
  display.print(timeClient.getFormattedTime());

  display.display();
}

void draw_full_screen() {
  display.clearDisplay();
  display.drawRect(0, 0, 128, 32, SSD1306_WHITE);
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
  panel_loop(&panel_poop, &timeClient);
  panel_loop(&panel_food, &timeClient);
  panel_loop(&panel_water, &timeClient);
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
  timeClient.begin();
}



void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  draw_boot_screen();

  panel_create(&panel_poop);
  panel_create(&panel_food);
  panel_create(&panel_water);

  setup_wifi();
  // TODO: Get the data from the server

  btn_poop.attachClick(handle_poop_click);
  btn_food.attachClick(handle_food_click);
  btn_water.attachClick(handle_water_click);
}

void loop() {
  static uint16_t prev = millis();
  uint16_t _now = millis();
  if ((uint16_t)(_now - prev) >= 5) {
    check_wifi();
    draw_status_screen();
    prev = _now;
  }
  
  update_rings();

  btn_poop.tick();
  btn_food.tick();
  btn_water.tick();

  timeClient.update();
}

static void handle_poop_click() {
  panel_button_press(&panel_poop, &timeClient);
}

static void handle_food_click() {
  panel_button_press(&panel_food, &timeClient);
}

static void handle_water_click() {
  panel_button_press(&panel_water, &timeClient);
}
