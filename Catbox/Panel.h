#ifndef PANEL_H
#define PANEL_H

#include <Adafruit_NeoPixel.h>
#include <AceButton.h>

using namespace ace_button;

// If defined, all of the panels will use very short delays
#define DEBUG_DELAY

#ifdef DEBUG_DELAY
#define DELAY_FOOD 10
#define DELAY_POOP 2
#define DELAY_WATER 5
#else
#define DELAY_FOOD 60 * 60 * 24 * 14 // 14 days
#define DELAY_POOP 60 * 60 * 24      // 24 hours
#define DELAY_WATER 60 * 60 * 24 * 7 // 2 days
#endif

#define ID_FOOD 0
#define ID_POOP 1
#define ID_WATER 2

struct Panel panels[] = {
    {ID_FOOD, 2, 5, Adafruit_NeoPixel::Color(0, 100, 0), MODE_ON, 0, DELAY_FOOD, NULL, NULL},
    {ID_POOP, 3, 6, Adafruit_NeoPixel::Color(100, 0, 0), MODE_ON, 0, DELAY_POOP, NULL, NULL},
    {ID_WATER, 4, 7, Adafruit_NeoPixel::Color(0, 0, 100), MODE_ON, 0, DELAY_WATER, NULL, NULL},
};

#define MODE_ON 0
#define MODE_OFF 1

// Each panel has an LED strip length 3
#define LED_COUNT 3

struct Panel
{
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

void setupPanel(Panel *panel);
void loopPanel(Panel *panel);

#endif