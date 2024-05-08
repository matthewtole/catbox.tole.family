#include "Arduino.h"
#include <Adafruit_NeoPixel.h>
#include <AceButton.h>
#include <TimeLib.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <ESPmDNS.h>

#include "secrets.h"
#include "panel.h"

using namespace ace_button;

#define NUM_PANELS 3

#define STATUS_BOOTING 0
#define STATUS_OFFLINE 1
#define STATUS_OK 2

Adafruit_NeoPixel ledStatus(1, 22, NEO_GRB + NEO_KHZ800);
uint8_t status = STATUS_BOOTING;

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
String serverName = "";
time_t lastWifiCheck = 0;

AsyncWebServer server(80);

void setup()
{
  Serial.begin(9600);

  ledStatus.begin();
  changeStatusColor(Adafruit_NeoPixel::Color(180, 90, 0));
  for (uint8_t p = 0; p < NUM_PANELS; p = p + 1)
  {
    setupPanel(&panels[p]);
  }
  setupWifi();
  // TODO: Get the data from the server
}

void loop()
{
  for (uint8_t p = 0; p < NUM_PANELS; p = p + 1)
  {
    loopPanel(&panels[p]);
  }

  if (now() >= lastWifiCheck + 60)
  {
    lastWifiCheck = now();
    if (WiFi.status() != WL_CONNECTED)
    {
      status = STATUS_OFFLINE;
    }
  }

  updateStatusLed();
}

void updateStatusLed()
{
  switch (status)
  {
  case STATUS_BOOTING:
    ledStatus.setPixelColor(0, Adafruit_NeoPixel::Color(180, 90, 0));
    break;
  case STATUS_OFFLINE:
    ledStatus.setPixelColor(0, Adafruit_NeoPixel::Color(180, 0, 0));
    break;
  case STATUS_OK:
    ledStatus.setPixelColor(0, Adafruit_NeoPixel::Color(0, 180, 0));
    break;
  }
  ledStatus.show();
}

void setupWifi()
{
  // Connect to the WiFi network
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Register the devices name with mDNS
  if (!MDNS.begin("catbox"))
  {
    Serial.println("Error setting up MDNS responder!");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.print("mDNS responder started at ");
  Serial.println("catbox.local");

  status = STATUS_OK;

  // Setup the HTTP server to handle OTA updates
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "CatBox v3"); });

  AsyncElegantOTA.begin(&server);
  server.begin();
}
