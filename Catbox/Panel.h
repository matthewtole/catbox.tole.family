#ifndef PANEL_H
#define PANEL_H

#include <Adafruit_NeoPixel.h>
#include <OneButton.h>

#define DAY_HOURS 24
#define RING_UPDATE_INTERVAL 60

#define NUM_PANELS 3

struct Panel {
  char id[6];
  Adafruit_NeoPixel *ring;
  uint8_t pin_ring;
  uint8_t pin_button;
  uint8_t pin_indicator;
  unsigned long last_pressed;
  unsigned long duration;
  uint32_t color;
  char label[6];
  unsigned long ring_next_update;
  bool pending_http;
  OneButton *button;
};

void panel_setup(Panel *panel);
void panel_button_press(Panel *panel);
void panel_loop(Panel *panel);
void panel_post_data(Panel *panel);
void panel_update_light(Panel *panel);
void panel_set_pending(Panel *panel, bool pending);

#endif // PANEL_H