#include "config.h"

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <Wire.h>

#include "Display.h"
#include "Panel.h"
#include "api.h"
#include "catbox_network.h"
#include "panels.h"

// Generated using this online tool
// https://jrabausch.github.io/lcd-image/web/

static const uint8_t logo[72] = {
    0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0x07, 0x83, 0xc0, 0x07, 0xc7, 0xe0,
    0x07, 0xe7, 0xc0, 0x07, 0xe7, 0xe0, 0x07, 0xe7, 0xc0, 0x23, 0xc7, 0xc4,
    0xf1, 0xc3, 0x8f, 0xf8, 0x00, 0x1f, 0xf8, 0x10, 0x1f, 0x78, 0x7e, 0x1e,
    0x79, 0xff, 0x9e, 0x11, 0xff, 0xc8, 0x03, 0xff, 0xc0, 0x07, 0xff, 0xe0,
    0x0f, 0xff, 0xf0, 0x0f, 0xff, 0xf0, 0x1f, 0xff, 0xf8, 0x1f, 0xff, 0xf8,
    0x1f, 0xff, 0xf8, 0x1f, 0xc5, 0xf0, 0x04, 0x00, 0x20, 0x00, 0x00, 0x00};

struct tm timeinfo;
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;

Display *display;

void setup() {
  Serial.begin(115200);
  logger.registerSerial(MYLOG, DEBUG, "tst");
  display = create_display(panels, &timeinfo);
  display_setup(display);

  setup_panels(&timeinfo);

  setup_wifi();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  fetch_data(panels, &timeinfo);

  BaseType_t taskCreated =
      xTaskCreate(background_task, "background_task", 8192, NULL, 2, NULL);

  if (taskCreated != pdPASS) {
    logger.log(MYLOG, ERROR, "Failed to create background task");
  }

  display_set_mode(display, DISPLAY_MODE_STATUS);
}

bool should_update_display() {
  static unsigned long prev = millis();
  unsigned long _now = millis();
  if ((_now - prev) >= 100) {
    prev = _now;
    return true;
  }
  return false;
}

void loop() {
  getLocalTime(&timeinfo);
  panels_loop();

  if (should_update_display()) {
    check_wifi();
    display_loop(display);
  }
}

void background_task(void *parameter) {
  while (true) {
    panels_send_data();
    vTaskDelay(http_delay * portTICK_PERIOD_MS);
  }
}