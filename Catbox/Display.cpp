#include <Elog.h>

#include "./fonts/PicoPixel.h"

#include "display.h"
#include "wifi-manager.h"

static const unsigned char PROGMEM logo[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x81, 0xc0, 0x07, 0xc3, 0xc0,
    0x07, 0xc7, 0xc0, 0x07, 0xc7, 0xc0, 0x03, 0xc7, 0xc0, 0x03, 0xc7, 0x80,
    0x61, 0x82, 0x8e, 0xf0, 0x00, 0x0f, 0x78, 0x00, 0x1e, 0x78, 0x7c, 0x1e,
    0x30, 0xff, 0x1c, 0x01, 0xff, 0x80, 0x03, 0xff, 0xc0, 0x07, 0xff, 0xe0,
    0x07, 0xff, 0xe0, 0x0f, 0xff, 0xf0, 0x0f, 0xff, 0xf0, 0x1f, 0xff, 0xf8,
    0x1f, 0xff, 0xf8, 0x0f, 0x41, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static const unsigned char PROGMEM icon_alert[] = {
    0x08, 0x00, 0x1c, 0x00, 0x14, 0x00, 0x36, 0x00,
    0x36, 0x00, 0x7f, 0x00, 0x77, 0x00, 0xff, 0x80};

void display_draw_boot(Display *display);
void display_draw_status(Display *display);
void display_draw_error(Display *display);

int get_hours(tm *timeinfo, unsigned long time);
void _draw_text(Display *self, const char *text, uint8_t x, uint8_t y,
                const GFXfont *font, uint8_t color);

#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define LOGO_WIDTH 24
#define LOGO_HEIGHT 24

#define WHITE SSD1306_WHITE
#define BLACK SSD1306_BLACK

#define CENTER_X (DISPLAY_WIDTH / 2)
#define CENTER_Y (DISPLAY_HEIGHT / 2)

#define MYLOG 0

Display *create_display(Panel *panels[NUM_PANELS], tm *timeinfo) {
  Display *display = new Display();
  display->mode = DISPLAY_MODE_BOOT;
  strcpy(display->error_message, "");
  display->display =
      new Adafruit_SSD1306(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);
  display->panels = panels;
  display->timeinfo = timeinfo;
  return display;
}

void display_setup(Display *self) {
  if (!self->display->begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    logger.log(MYLOG, ERROR, "SSD1306 allocation failed");
  }
  display_loop(self);
}

void display_loop(Display *self) {
  switch (self->mode) {
  case DISPLAY_MODE_BOOT:
    display_draw_boot(self);
    break;
  case DISPLAY_MODE_STATUS:
    display_draw_status(self);
    break;
  case DISPLAY_MODE_ERROR:
    display_draw_error(self);
    break;
  }
}

void display_set_mode(Display *display, DisplayMode mode) {
  display->mode = mode;
}

void display_set_error(Display *display, const char *error_message) {
  strcpy(display->error_message, error_message);
}

void display_draw_boot(Display *self) {
  self->display->clearDisplay();

  self->display->drawBitmap(CENTER_X - LOGO_WIDTH / 2, 6, logo, LOGO_WIDTH,
                            LOGO_HEIGHT, WHITE);

  // TODO: Make this dynamic
  // TODO: Automate this center alignment work
  self->display->setTextColor(BLACK);
  self->display->setCursor(26, 33);
  self->display->print("CATBOX v4.0.0");

  // TODO: Make this actually reflect something
  self->display->drawRect(8, 48, 112, 8, 1);
  self->display->fillRect(10, 50, 64, 4, 1);

  self->display->display();
}

void _draw_status_panel(Display *self, Panel *panel, uint8_t x, uint8_t y,
                        bool align_right) {
  _draw_text(self, String(panel->label).c_str(), x, y, &Picopixel, WHITE);
  sprintf(panel->label, "%05d",
          min(99999, get_hours(panel->timeinfo, panel->last_pressed)));
  _draw_text(self, panel->label, x, y + 3, NULL, WHITE);
}

void display_draw_status(Display *self) {
  self->display->clearDisplay();

  // HEADER
  self->display->fillRect(0, 0, DISPLAY_WIDTH, 11, WHITE);
  _draw_text(self, "CATBOX", 4, 2, NULL, BLACK);
  _draw_text(self, "00:00:00", 97, 7, &Picopixel, BLACK);

  // PANELS
  _draw_status_panel(self, self->panels[0], 4, 20, false);
  _draw_status_panel(self, self->panels[1], 60, 20, true);
  _draw_status_panel(self, self->panels[2], 4, 39, false);

  // CLOCK
  static char clock_text[10];
  strftime(clock_text, sizeof(clock_text), "%H:%M:%S", self->timeinfo);
  _draw_text(self, "CLOCK", 60, 39, &Picopixel, WHITE);
  _draw_text(self, clock_text, 60, 42, NULL, WHITE);

  //   FOOTER
  self->display->drawLine(0, 52, 128, 52, WHITE);
  _draw_text(self, get_wifi_ssid().c_str(), 4, 60, NULL, BLACK);
  _draw_text(self, get_ip_address().c_str(), 86, 60, NULL, BLACK);

  self->display->display();
}

#define ERROR_MARGIN 4
#define ERROR_RADIUS 4

void display_draw_error(Display *self) {
  self->display->clearDisplay();

  self->display->drawRoundRect(
      ERROR_MARGIN, ERROR_MARGIN, DISPLAY_WIDTH - ERROR_MARGIN * 2,
      DISPLAY_HEIGHT - ERROR_MARGIN * 2, ERROR_RADIUS, 1);
  self->display->fillRoundRect(ERROR_MARGIN + 2, ERROR_MARGIN + 2,
                               DISPLAY_WIDTH - ERROR_MARGIN * 4, 11,
                               ERROR_RADIUS - 1, 1);

  self->display->setTextColor(BLACK);
  self->display->setCursor(50, 8);
  self->display->print("ERROR");

  // Layer 4
  self->display->setTextWrap(true);
  self->display->setTextColor(BLACK);
  self->display->setCursor(7, 21);
  self->display->print(self->error_message);

  self->display->drawBitmap(9, 7, icon_alert, 9, 8, BLACK);
  self->display->drawBitmap(DISPLAY_WIDTH - 9 - 9, 7, icon_alert, 9, 8, BLACK);

  self->display->display();
}

void _draw_text(Display *self, const char *text, uint8_t x, uint8_t y,
                const GFXfont *font, uint8_t color) {
  self->display->setFont(font);
  self->display->setCursor(x, y);
  self->display->setTextColor(color);
  self->display->print(text);
}

int get_hours(tm *timeinfo, unsigned long time) {
  return (int)((timeinfo->tm_sec - time) / (HOUR_SEC));
}
