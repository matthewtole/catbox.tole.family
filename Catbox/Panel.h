#ifndef PANEL_H
#define PANEL_H

#include <WS2812FX.h>
#include <NTPClient.h>
#include <Adafruit_SSD1306.h>

#define HOUR_SEC 1   // 60 * 60
#define DAY_HOURS 2  // 24

struct Panel {
  WS2812FX *ring;
  uint8_t pin_ring;
  uint8_t pin_button;
  uint8_t pin_indicator;
  unsigned long last_pressed;
  uint8_t duration;
  uint32_t color;
  char label[6];
  uint8_t icon[8];
};

void panel_create(Panel *panel);
void panel_button_press(Panel *panel, NTPClient *timeClient);
void panel_loop(Panel *panel, NTPClient *timeClient);
void panel_draw_status(Panel *panel, Adafruit_SSD1306 *display, NTPClient *timeClient, uint8_t x, uint8_t y);

#endif // PANEL_H