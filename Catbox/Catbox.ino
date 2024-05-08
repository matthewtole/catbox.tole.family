#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include <AceButton.h>
#include <TimeLib.h>
#include <HTTPClient.h>
#include <WiFi.h>

using namespace ace_button;

#define DEBUG_DELAY
#define SERIAL_ENABLED

#define LED_COUNT 3
#define NUM_PANELS 1

#define MODE_ON 0
#define MODE_OFF 1

#define ID_FOOD 0

struct Panel {
  uint8_t id;
  uint8_t pinLeds;
  uint8_t pinButton;
  uint32_t color;
  uint8_t mode;
  time_t lastPress;
  uint32_t delay;
};

#ifdef DEBUG_DELAY
#define DELAY_FOOD 5
#else
#define DELAY_FOOD 24 * 14 * 60 * 60
#endif

struct Panel panels[] = { { 0, 2, 0, Adafruit_NeoPixel::Color(0, 100, 0), MODE_ON, 0, DELAY_FOOD } };

Adafruit_NeoPixel ledsFood(LED_COUNT, 2, NEO_GRB + NEO_KHZ800);
AceButton buttonFood(panels[0].pinButton, HIGH, ID_FOOD);

Adafruit_NeoPixel ledStatus(1, 22, NEO_GRB + NEO_KHZ800);

const char* ssid = "";
const char* password = "";
String serverName = "";

void changeStatusColor(uint32_t color) {
  ledStatus.clear();
  ledStatus.fill(color);
  ledStatus.show();
}

void setup() {
  ledStatus.begin();
  changeStatusColor(Adafruit_NeoPixel::Color(180, 90, 0));
  for (uint8_t p = 0; p < NUM_PANELS; p = p + 1) {
    setupPanel(&panels[p]);
  }
  setupWifi();  
}

void loop() {
  for (uint8_t p = 0; p < NUM_PANELS; p = p + 1) {
    loopPanel(&panels[p]);
  }
}

void setupWifi() {
  WiFi.begin(ssid, password);
  #ifdef SERIAL_ENABLED
  Serial.println("Connecting");
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    #ifdef SERIAL_ENABLED
    Serial.print(".");
    #endif
  }
  #ifdef SERIAL_ENABLED
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  #endif
  changeStatusColor(Adafruit_NeoPixel::Color(0, 180, 0));
}

void setupPanel(Panel* panel) {
  Adafruit_NeoPixel* leds = getLeds(panel->id);
  AceButton* button = getButton(panel->id);

  leds->begin();
  leds->setBrightness(100);

  pinMode(panel->pinButton, INPUT);
  ButtonConfig* buttonConfig = button->getButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setDoubleClickDelay(600);
}

void loopPanel(Panel* panel) {
  Adafruit_NeoPixel* leds = getLeds(panel->id);
  AceButton* button = getButton(panel->id);

  leds->clear();
  if (panel->mode == MODE_ON) {
    leds->fill(panel->color);
  }
  leds->show();

  button->check();

  if (now() >= panel->lastPress + panel->delay) {
    panel->mode = MODE_ON;
  }
}

void handleEvent(AceButton* button, uint8_t eventType, uint8_t buttonState) {
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
    changeStatusColor(Adafruit_NeoPixel::Color(180, 0, 0));
    return -1000;
  }
  HTTPClient http;
  http.begin(url.c_str());
  int httpResponseCode = http.GET();
  #ifdef SERIAL_ENABLED
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    Serial.println(payload);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  #endif
  http.end();
  return httpResponseCode;
}



void handleClick(uint8_t id) {
#ifdef SERIAL_ENABLED
  Serial.print("Click: ");
  Serial.println(id);
#endif
  Panel* panel = getPanel(id);
  panel->mode = MODE_OFF;
  panel->lastPress = now();

  httpRequest(serverName);
}

void handleDoubleClick(uint8_t id) {
#ifdef SERIAL_ENABLED
  Serial.print("Double Click: ");
  Serial.println(id);
#endif
}

Panel* getPanel(uint8_t id) {
  return &panels[id];
}

Adafruit_NeoPixel* getLeds(uint8_t id) {
  switch (id) {
    case ID_FOOD:
      return &ledsFood;
      break;
    default:
      return NULL;
  }
}

AceButton* getButton(uint8_t id) {
  switch (id) {
    case ID_FOOD:
      return &buttonFood;
      break;
    default:
      return NULL;
  }
}
