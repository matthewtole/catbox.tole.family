#include "panel-manager.h"
#include "api.h"
#include "config.h"

Panel panel_poop = {
    .id = "poop",
    .pin_ring = PIN_POOP_RING,
    .pin_button = PIN_POOP_BUTTON,
    .pin_indicator = PIN_POOP_INDICATOR,
    .duration = 8 * HOUR_SEC,
    .color = 0xff6600,
};

Panel panel_food = {
    .id = "food",
    .pin_ring = PIN_FOOD_RING,
    .pin_button = PIN_FOOD_BUTTON,
    .pin_indicator = PIN_FOOD_INDICATOR,
    .duration = 14 * DAY_HOURS * HOUR_SEC,
    .color = 0x339900,
};

Panel panel_water = {
    .id = "water",
    .pin_ring = PIN_WATER_RING,
    .pin_button = PIN_WATER_BUTTON,
    .pin_indicator = PIN_WATER_INDICATOR,
    .duration = 7 * DAY_HOURS * HOUR_SEC,
    .color = 0x33CCFF,
};

Panel *panels[NUM_PANELS] = {&panel_poop, &panel_food, &panel_water};

static void handle_poop_click() { panel_button_press(&panel_poop); }

static void handle_food_click() { panel_button_press(&panel_food); }

static void handle_water_click() { panel_button_press(&panel_water); }

void setup_panels() {
  for (uint8_t p = 0; p < NUM_PANELS; p += 1) {
    panel_setup(panels[p]);
  }

  panel_poop.button->attachClick(handle_poop_click);
  panel_food.button->attachClick(handle_food_click);
  panel_water.button->attachClick(handle_water_click);
}

void panels_loop() {
  for (uint8_t p = 0; p < NUM_PANELS; p += 1) {
    panel_loop(panels[p]);
    panels[p]->button->tick();
  }
}

void panels_send_data() {
  for (uint8_t p = 0; p < NUM_PANELS; p += 1) {
    if (panels[p]->pending_http) {
      send_data(panels[p]);
      break;
    }
  }
}