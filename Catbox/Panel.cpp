#include <HTTPClient.h>

#include "panel.h"
#include "timeinfo.h"

#define RING_PIXELS 8

void panel_setup(Panel *panel) {
  panel->last_pressed = 0;
  panel->ring_next_update = 0;
  panel->pending_http = false;

  panel->button = new OneButton(panel->pin_button, true, true);

  panel->ring =
      new Adafruit_NeoPixel(RING_PIXELS, panel->pin_ring, NEO_GRB + NEO_KHZ800);
  panel->ring->begin();
  panel->ring->setBrightness(200);
  panel->ring->fill(panel->ring->Color(255, 255, 255));
  panel->ring->show();

  pinMode(panel->pin_indicator, OUTPUT);
  digitalWrite(panel->pin_indicator, HIGH);
}

void panel_set_pending(Panel *panel, bool pending) {
  panel->pending_http = pending;
  digitalWrite(panel->pin_indicator, pending ? LOW : HIGH);
}

void panel_button_press(Panel *panel) {
  panel_set_pending(panel, true);
  panel->last_pressed = mktime(&timeinfo);
}

void panel_update_light(Panel *panel) {
  panel->ring->clear();
  if ((timeinfo.tm_sec - panel->last_pressed) >= panel->duration) {
    panel->ring->fill(panel->color);
  } else {
    panel->ring->fill(panel->ring->Color(0, 0, 0));
  }
  panel->ring->show();
}

void panel_loop(Panel *panel) { panel_update_light(panel); }
