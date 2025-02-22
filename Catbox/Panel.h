#ifndef PANEL_H
#define PANEL_H

#include <WS2812FX.h>
#include <NTPClient.h>
#include <Adafruit_SSD1306.h>
#include <OneButton.h>

#define HOUR_SEC 60 * 60
#define DAY_HOURS 24
#define RING_UPDATE_INTERVAL 60

struct Panel {
  char id[6];
  WS2812FX *ring;
  uint8_t pin_ring;
  uint8_t pin_button;
  uint8_t pin_indicator;
  unsigned long last_pressed;
  unsigned long duration;
  uint32_t color;
  char label[6];
  uint8_t icon[8];
  NTPClient *time_client;
  unsigned long ring_next_update;
  bool pending_http;
  uint8_t x_offset;
  OneButton *button;
};

void panel_setup(Panel *panel, NTPClient *time_client);
void panel_button_press(Panel *panel);
void panel_loop(Panel *panel);
void panel_draw_status(Panel *panel, Adafruit_SSD1306 *display, uint8_t x, uint8_t y);
void panel_post_data(Panel *panel);
void panel_update_light(Panel *panel);

#endif  // PANEL_H