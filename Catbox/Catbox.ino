#include "config.h"

#include <Arduino.h>
#include <PrettyOTA.h>

#include "api.h"
#include "display.h"
#include "panel-manager.h"
#include "panel.h"
#include "timeinfo.h"
#include "wifi-manager.h"

Display *display;
AsyncWebServer server(80);
PrettyOTA OTAUpdates;

void setup() {
  Serial.begin(115200);
  display = create_display(panels);

  display_setup(display);
  setup_panels();
  setup_wifi();
  fetch_data(panels);

  BaseType_t taskCreated =
      xTaskCreate(background_task, "background_task", 8192, NULL, 2, NULL);

  if (taskCreated != pdPASS) {
    Serial.println("Failed to create background task");
    delay(1000);
    ESP.restart();
  }

  OTAUpdates.Begin(&server);
  OTAUpdates.SetHardwareID(PRODUCT_ID);
  OTAUpdates.SetAppVersion(PRODUCT_VERSION);
  PRETTY_OTA_SET_CURRENT_BUILD_TIME_AND_DATE();
  server.begin();

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