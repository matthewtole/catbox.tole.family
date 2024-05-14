#include <WiFi.h>
#include "Arduino.h"
#include "Panel.h"
#include <TimeLib.h>
#include <HTTPClient.h>

String serverName = "https://catbox.tole.family/.netlify/functions/button";

struct Panel panels[] = {
  { .id = ID_FOOD, .pinLeds = 2, .pinButton = 4, .color = Adafruit_NeoPixel::Color(27, 94, 32), .delay = DELAY_FOOD },
  { .id = ID_WATER, .pinLeds = 14, .pinButton = 12, .color = Adafruit_NeoPixel::Color(79, 195, 247), .delay = DELAY_WATER },
  { .id = ID_POOP, .pinLeds = 5, .pinButton = 18, .color = Adafruit_NeoPixel::Color(230, 81, 0), .delay = DELAY_POOP },
};

void handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState);
void handleClick(uint8_t id);
void handleDoubleClick(uint8_t id);
Panel *getPanel(uint8_t id);
int httpRequest(String url);

void setupPanel(Panel *panel, uint8_t brightness) {
  panel->mode = MODE_ON;
  panel->lastPress = 0;
  panel->leds = new Adafruit_NeoPixel(LED_COUNT, panel->pinLeds, NEO_GRB + NEO_KHZ800);
  panel->leds->begin();
  panel->leds->setBrightness(brightness);

  pinMode(panel->pinButton, INPUT_PULLUP);
  panel->button = new AceButton(panel->pinButton, HIGH, panel->id);
  ButtonConfig *buttonConfig = panel->button->getButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setClickDelay(600);
}

void loopPanel(Panel *panel) {
  panel->leds->clear();
  if (panel->mode == MODE_ON) {
    panel->leds->fill(panel->color);
  }
  panel->leds->show();

  panel->button->check();

  if (now() >= panel->lastPress + panel->delay) {
    panel->mode = MODE_ON;
  }
}

void handleEvent(AceButton *button, uint8_t eventType, uint8_t buttonState) {
  Serial.printf("Button clicked: \nid = %d\n eventType = %d\n state=%d\n", button->getId(), eventType, buttonState);
  switch (eventType) {
    case AceButton::kEventClicked:
      handleClick(button->getId());
      break;
  }
}

int httpRequest(String url) {
  if (WiFi.status() != WL_CONNECTED) {
    return -1000;
  }
  HTTPClient http;
  http.begin(url.c_str());
  Serial.println(url);
  int res = http.GET();
  if (res > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(res);
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.print("Error code: ");
    Serial.println(res);
  }
  http.end();
  return res;
}

void handleClick(uint8_t id) {
  Serial.print("Click: ");
  Serial.println(id);

  Panel *panel = getPanel(id);
  if (panel == NULL) {
    Serial.printf("Failed to get the panel for ID %d", id);
    return;
  }
  panel->mode = MODE_OFF;
  panel->lastPress = now();
  loopPanel(panel);

  String url = serverName + "?id=" + String(id);
  httpRequest(url);
}

void handleDoubleClick(uint8_t id) {
  Serial.print("Click: ");
  Serial.println(id);
}

Panel *getPanel(uint8_t id) {
  for (uint8_t p = 0; p < NUM_PANELS; p += 1) {
    if (panels[p].id == id) {
      return &panels[p];
    }
  }
  return NULL;
}
