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
      new WS2812FX(RING_PIXELS, panel->pin_ring, NEO_GRB + NEO_KHZ800);
  panel->ring->init();
  panel->ring->setBrightness(200);
  panel->ring->setSpeed(500);
  panel->ring->setColor(255, 255, 255);
  panel->ring->setMode(FX_MODE_STATIC);
  panel->ring->start();
  panel->ring->service();

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
  panel_update_light(panel);
}

void panel_update_light(Panel *panel) {
  if ((timeinfo.tm_sec - panel->last_pressed) >= panel->duration) {
    panel->ring->setColor(panel->color);
  } else {
    panel->ring->setColor(BLACK);
  }
  panel->ring->service();
}

void panel_loop(Panel *panel) {
  if (timeinfo.tm_sec >= panel->ring_next_update) {
    panel_update_light(panel);
    panel->ring_next_update = timeinfo.tm_sec + RING_UPDATE_INTERVAL;
  }
  panel->ring->service();
}
