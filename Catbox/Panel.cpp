#include "Panel.h"

#include <HTTPClient.h>

#define RING_PIXELS 8

HTTPClient http;

void panel_setup(Panel *panel, NTPClient *time_client) {
  panel->time_client = time_client;
  panel->last_pressed = 0;
  panel->ring_next_update = 0;

  panel->ring = new WS2812FX(RING_PIXELS, panel->pin_ring, NEO_GRB + NEO_KHZ800);
  panel->ring->init();
  panel->ring->setBrightness(200);
  panel->ring->setSpeed(500);
  panel->ring->setColor(255, 255, 255);
  panel->ring->setMode(FX_MODE_STATIC);
  panel->ring->start();
  panel->ring->service();

  pinMode(panel->pin_indicator, OUTPUT);
  digitalWrite(panel->pin_indicator, LOW);
}

void panel_post_data_async(void *parameter) {
  Panel *panel = (Panel *)parameter;
  panel_post_data(panel);
  vTaskDelete(NULL);
}

void panel_post_data(Panel *panel) {
  String url = String(String(SERVER_ROOT) + "/button?id=" + String(panel->id));
  http.begin(url.c_str());
  int response_code = http.GET();
  if (response_code > 0) {
    String response = http.getString();
    Serial.println(response_code);
    Serial.println(response);
    digitalWrite(panel->pin_indicator, LOW);
  } else {
    Serial.print("Error on HTTP request: ");
    Serial.println(response_code);
  }
  http.end();
}

void panel_button_press(Panel *panel) {
  digitalWrite(panel->pin_indicator, HIGH);

  panel->last_pressed = panel->time_client->getEpochTime();
  panel_update_light(panel);

  xTaskCreate(panel_post_data_async, "panel_post_data", 4096, (void *)panel, 1, NULL);
}

void panel_update_light(Panel *panel) {
  if ((panel->time_client->getEpochTime() - panel->last_pressed) >= panel->duration) {
    // panel->ring->setMode(FX_MODE_FADE);
    panel->ring->setColor(panel->color);
  } else {
    panel->ring->setColor(BLACK);
  }
  panel->ring->service();
}

void panel_loop(Panel *panel) {
  if (panel->time_client->getEpochTime() >= panel->ring_next_update) {
    panel_update_light(panel);
    panel->ring_next_update = panel->time_client->getEpochTime() + RING_UPDATE_INTERVAL;
  }
  panel->ring->service();
}

int get_hours(NTPClient *timeClient, unsigned long time) {
  return (int)((timeClient->getEpochTime() - time) / (HOUR_SEC));
}

void panel_draw_status(Panel *panel, Adafruit_SSD1306 *display, uint8_t x, uint8_t y) {
  sprintf(panel->label, "%05d", min(99999, get_hours(panel->time_client, panel->last_pressed)));
  display->drawBitmap(x, y, panel->icon, 8, 8, 1);
  display->setCursor(x + 10, y + 2);
  display->print(panel->label);
}