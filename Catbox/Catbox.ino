#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include <AceButton.h>
#include <TimeLib.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ELog.h>

using namespace ace_button;

Elog log;

#define DEBUG_DELAY
#define LOG_LEVEL DEBUG

#define LED_COUNT 3
#define NUM_PANELS 1

#define MODE_ON 0
#define MODE_OFF 1

#define ID_FOOD 0
#define ID_POOP 1
#define ID_WATER 2

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
  Adafruit_NeoPixel* leds;
  // Button manager
  AceButton* button;
};

#ifdef DEBUG_DELAY
#define DELAY_FOOD 5
#else
#define DELAY_FOOD 24 * 14 * 60 * 60
#endif

struct Panel panels[] = { { 0, 2, 0, Adafruit_NeoPixel::Color(0, 100, 0), MODE_ON, 0, DELAY_FOOD, NULL, NULL } };

Adafruit_NeoPixel ledStatus(1, 22, NEO_GRB + NEO_KHZ800);

const char* ssid = "";
const char* password = "";
String serverName = "";

AsyncWebServer server(80);

void changeStatusColor(uint32_t color) {
  ledStatus.clear();
  ledStatus.fill(color);
  ledStatus.show();
}

void setup() {
  Serial.begin(115200);
  log.addSerialLogging(Serial, "CatBox", LOG_LEVEL);

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
  WiFi.mode(WIFI_STA);
  log.log(DEBUG,  "Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    log.log(DEBUG, "...");
    delay(500);
  }
  log.log(INFO, "Connected to WiFi network with IP Address: %s" + WiFi.localIP().toString());
  changeStatusColor(Adafruit_NeoPixel::Color(0, 180, 0));

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  AsyncElegantOTA.begin(&server);
  server.begin();
}

void setupPanel(Panel* panel) {
  panel->leds = new Adafruit_NeoPixel(LED_COUNT, panel->pinLeds, NEO_GRB + NEO_KHZ800);
  panel->button = new AceButton(panel->pinButton, HIGH, panel->id);

  panel->leds->begin();
  panel->leds->setBrightness(100);

  pinMode(panel->pinButton, INPUT);
  ButtonConfig* buttonConfig = panel->button->getButtonConfig();
  buttonConfig->setEventHandler(handleEvent);
  buttonConfig->setFeature(ButtonConfig::kFeatureClick);
  buttonConfig->setFeature(ButtonConfig::kFeatureDoubleClick);
  buttonConfig->setDoubleClickDelay(600);
}

void loopPanel(Panel* panel) {
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
  int res = http.GET();
  if (res > 0) {
    log.log(DEBUG, "HTTP Response code: %d", res);
    String payload = http.getString();
    log.log(DEBUG, payload);
  } else {
    log.log(ERROR, "Error code: %d", res);
  }
  http.end();
  return res;
}

void handleClick(uint8_t id) {
  log.log(DEBUG, "Click: %d", id);
  
  Panel* panel = getPanel(id);
  panel->mode = MODE_OFF;
  panel->lastPress = now();

  httpRequest(serverName);
}

void handleDoubleClick(uint8_t id) {
  log.log(DEBUG, "Double Click: %d", id);
}

Panel* getPanel(uint8_t id) {
  return &panels[id];
}

