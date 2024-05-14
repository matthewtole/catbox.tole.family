#ifndef PANEL_H
#define PANEL_H

#include <Adafruit_NeoPixel.h>
#include <AceButton.h>

using namespace ace_button;

// If defined, all of the panels will use very short delays
#define DEBUG_DELAY

#ifdef DEBUG_DELAY
#define DELAY_FOOD 20
#define DELAY_POOP 20
#define DELAY_WATER 20
#else
#define DELAY_FOOD 60 * 60 * 24 * 14  // 14 days
#define DELAY_POOP 60 * 60 * 24       // 24 hours
#define DELAY_WATER 60 * 60 * 24 * 7  // 2 days
#endif

#define MODE_ON 0
#define MODE_OFF 1

#define ID_FOOD 0
#define ID_POOP 1
#define ID_WATER 2

#define NUM_PANELS 3
extern struct Panel panels[];

// Each panel has an 2 LED strips of length 3
#define LED_COUNT 6

struct Panel {
  // Panel ID
  uint8_t id;
  // GPIO pin for the LEDs
  uint8_t pinLeds;
  // GPIO pin for the button
  uint8_t pinButton;
  // Color of the LEDs
  uint32_t color;
  // Current mode of the panel
  uint8_t mode;
  // Last time the button was pressed
  time_t lastPress;
  // Delay in seconds before the panel goes back to MODE_ON
  uint32_t delay;
  // LED manager
  Adafruit_NeoPixel *leds;
  // Button manager
  AceButton *button;
};

void setupPanel(Panel *panel, uint8_t brightness);
void loopPanel(Panel *panel);

#endif