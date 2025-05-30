#include "config.h"

#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <Wire.h>

#include "api.h"
#include "display.h"
#include "panel-manager.h"
#include "panel.h"
#include "timeinfo.h"
#include "wifi-manager.h"

Display *display;

void setup() {
  Serial.begin(115200);
  logger.registerSerial(MYLOG, DEBUG, "tst");
  display = create_display(panels);

  display_setup(display);
  setup_panels();
  setup_wifi();
  fetch_data(panels);

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
  timeinfo_loop();
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