#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_SSD1306.h>
#include <WiFiClient.h>

#include "Panel.h"

enum DisplayMode {
  DISPLAY_MODE_BOOT,
  DISPLAY_MODE_STATUS,
  DISPLAY_MODE_ERROR,
};

struct Display {
  DisplayMode mode;
  Adafruit_SSD1306 *display;
  char error_message[100];
  Panel **panels;
  tm *timeinfo;
};

Display *create_display(Panel *panels[NUM_PANELS], tm *timeinfo);
void display_setup(Display *self);
void display_loop(Display *self);
void display_set_mode(Display *self, DisplayMode mode);
void display_set_error(Display *self, const char *error_message);

#endif