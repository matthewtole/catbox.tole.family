#include <WiFi.h>
#include "Arduino.h"
#include "Panel.h"
#include <TimeLib.h>
#include <HTTPClient.h>

String serverName = "http://eo66nndw5h9g95v.m.pipedream.net";

struct Panel panels[] = {
  { .id = ID_FOOD, .pinLeds = 2, .pinButton = 4, .color = Adafruit_NeoPixel::Color(0, 100, 0), .delay = DELAY_FOOD },
  { .id = ID_WATER, .pinLeds = 14, .pinButton = 12, .color = Adafruit_NeoPixel::Color(0, 0, 100), .delay = DELAY_WATER },
  { .id = ID_POOP, .pinLeds = 5, .pinButton = 18, .color = Adafruit_NeoPixel::Color(100, 0, 0), .delay = DELAY_POOP },
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
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setDoubleClickDelay(600);
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
  switch (eventType) {
    case AceButton::kEventClicked:
      handleClick(button->getId());
      break;
    case AceButton::kEventDoubleClicked:
      handleDoubleClick(button->getId());
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
  panel->mode = MODE_OFF;
  panel->lastPress = now(); 

  httpRequest(serverName);
}

void handleDoubleClick(uint8_t id) {
  Serial.print("Click: ");
  Serial.println(id);
}

Panel *getPanel(uint8_t id) {
  return &panels[id];
}
