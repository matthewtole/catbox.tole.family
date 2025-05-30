#include "Arduino.h"

#include "catbox_network.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino_JSON.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <Elog.h>
#include <HTTPClient.h>
#include <OneButton.h>
#include <WiFi.h>
#include <Wire.h>

#include "esp_core_dump.h"
#include "esp_spi_flash.h"
#include "esp_system.h"

#include "Display.h"
#include "Panel.h"
#include "constants.h"

// Generated using this online tool
// https://jrabausch.github.io/lcd-image/web/

static const uint8_t logo[72] = {
    0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0x07, 0x83, 0xc0, 0x07, 0xc7, 0xe0,
    0x07, 0xe7, 0xc0, 0x07, 0xe7, 0xe0, 0x07, 0xe7, 0xc0, 0x23, 0xc7, 0xc4,
    0xf1, 0xc3, 0x8f, 0xf8, 0x00, 0x1f, 0xf8, 0x10, 0x1f, 0x78, 0x7e, 0x1e,
    0x79, 0xff, 0x9e, 0x11, 0xff, 0xc8, 0x03, 0xff, 0xc0, 0x07, 0xff, 0xe0,
    0x0f, 0xff, 0xf0, 0x0f, 0xff, 0xf0, 0x1f, 0xff, 0xf8, 0x1f, 0xff, 0xf8,
    0x1f, 0xff, 0xf8, 0x1f, 0xc5, 0xf0, 0x04, 0x00, 0x20, 0x00, 0x00, 0x00};

#define PIN_POOP_RING 15
#define PIN_POOP_BUTTON 25
#define PIN_POOP_INDICATOR 26

#define PIN_FOOD_RING 2
#define PIN_FOOD_BUTTON 18
#define PIN_FOOD_INDICATOR 5

#define PIN_WATER_RING 4
#define PIN_WATER_BUTTON 32
#define PIN_WATER_INDICATOR 33

#define RING_PIXELS 8

static void handle_poop_click();
static void handle_poop_double_click();
static void handle_food_click();
static void handle_food_double_click();
static void handle_water_click();
static void handle_water_double_click();
void draw_boot_screen();
void draw_status_screen();

struct tm timeinfo;
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

Panel panel_poop = {
    .id = "poop",
    .pin_ring = PIN_POOP_RING,
    .pin_button = PIN_POOP_BUTTON,
    .pin_indicator = PIN_POOP_INDICATOR,
    .duration = 8 * HOUR_SEC,
    .color = 0xff6600,
};

Panel panel_food = {
    .id = "food",
    .pin_ring = PIN_FOOD_RING,
    .pin_button = PIN_FOOD_BUTTON,
    .pin_indicator = PIN_FOOD_INDICATOR,
    .duration = 14 * DAY_HOURS * HOUR_SEC,
    .color = 0x339900,
};

Panel panel_water = {
    .id = "water",
    .pin_ring = PIN_WATER_RING,
    .pin_button = PIN_WATER_BUTTON,
    .pin_indicator = PIN_WATER_INDICATOR,
    .duration = 7 * DAY_HOURS * HOUR_SEC,
    .color = 0x33CCFF,
};

#define NUM_PANELS 3
Panel *panels[NUM_PANELS] = {&panel_poop, &panel_food, &panel_water};

Display *display;

void setup() {
  Serial.begin(115200);
  logger.registerSerial(MYLOG, DEBUG, "tst");
  display = create_display(panels, &timeinfo);
  display_setup(display);

  for (uint8_t p = 0; p < NUM_PANELS; p += 1) {
    panel_setup(panels[p], &timeinfo);
  }

  setup_wifi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  fetch_data(panels, &timeinfo);

  panel_poop.button->attachClick(handle_poop_click);
  panel_food.button->attachClick(handle_food_click);
  panel_water.button->attachClick(handle_water_click);

  BaseType_t taskCreated =
      xTaskCreate(background_task, "background_task", 8192, NULL, 2, NULL);

  if (taskCreated != pdPASS) {
    logger.log(MYLOG, ERROR, "Failed to create background task");
  }

  display_set_mode(display, DISPLAY_MODE_STATUS);
}

void loop() {
  static unsigned long prev = millis();
  unsigned long _now = millis();
  if ((_now - prev) >= 100) {
    check_wifi();
    display_loop(display);
    prev = _now;
  }

  for (uint8_t p = 0; p < NUM_PANELS; p += 1) {
    panel_loop(panels[p]);
    panels[p]->button->tick();
  }

  getLocalTime(&timeinfo);
}

void background_task(void *parameter) {
  while (true) {
    for (uint8_t p = 0; p < NUM_PANELS; p += 1) {
      if (panels[p]->pending_http) {
        logger.log(MYLOG, DEBUG, "%s has a pending HTTP request",
                   panels[p]->id);
        send_data(panels[p]);
        break;
      }
    }
    vTaskDelay(http_delay * portTICK_PERIOD_MS);
  }
}

static void handle_poop_click() { panel_button_press(&panel_poop); }

static void handle_food_click() { panel_button_press(&panel_food); }

static void handle_water_click() { panel_button_press(&panel_water); }
