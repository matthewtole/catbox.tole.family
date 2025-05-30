#ifndef PANEL_MANAGER_H
#define PANEL_MANAGER_H

#include "config.h"
#include "panel.h"

#define PIN_POOP_RING 15
#define PIN_POOP_BUTTON 25
#define PIN_POOP_INDICATOR 26

#define PIN_FOOD_RING 2
#define PIN_FOOD_BUTTON 18
#define PIN_FOOD_INDICATOR 5

#define PIN_WATER_RING 4
#define PIN_WATER_BUTTON 32
#define PIN_WATER_INDICATOR 33

#define RING_PIXELS 8

static void handle_poop_click();
static void handle_food_click();
static void handle_water_click();

extern Panel *panels[NUM_PANELS];

void setup_panels();
void panels_loop();
void panels_send_data();

#endif