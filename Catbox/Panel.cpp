#include "Panel.h"

#define RING_PIXELS 8


void panel_create(Panel *panel) {
  panel->ring = new WS2812FX(RING_PIXELS, panel->pin_ring, NEO_GRB + NEO_KHZ800);
  panel->ring->init();
  panel->ring->setBrightness(255);
  panel->ring->setColor(255, 255, 255);
  panel->ring->setMode(FX_MODE_STATIC);
  panel->ring->start();

  pinMode(panel->pin_indicator, OUTPUT);
  digitalWrite(panel->pin_indicator, HIGH);
}

void panel_button_press(Panel *panel, NTPClient *timeClient) {
  panel->last_pressed = timeClient->getEpochTime();
  panel->ring->setColor(BLACK);
  panel->ring->service();
}

void panel_loop(Panel *panel, NTPClient *timeClient) {
  if (panel->ring->getColor() != panel->color) {
    if ((timeClient->getEpochTime() - panel->last_pressed) >= panel->duration) {
      panel->ring->setColor(panel->color);
    }
  }
  panel->ring->service();
}

int get_hours(NTPClient *timeClient, unsigned long time) {
  return (int)((timeClient->getEpochTime() - time) / HOUR_SEC);
}

void panel_draw_status(Panel *panel, Adafruit_SSD1306 *display, NTPClient *timeClient, uint8_t x, uint8_t y) {
  sprintf(panel->label, "%05d", min(99999, get_hours(timeClient, panel->last_pressed)));
  display->drawBitmap(x, y, panel->icon, 8, 8, 1);
  display->setCursor(x + 10, y + 2);
  display->print(panel->label);
}