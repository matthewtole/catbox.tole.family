#include "Arduino.h"

#include <HTTPClient.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <OneButton.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Thread.h>
#include <Arduino_JSON.h>
#include <Elog.h>

#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_core_dump.h"

#include "Panel.h"
#include "secrets.h"

#define MYLOG 0

#define DEFAULT_HTTP_DELAY 500

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
time_t lastWifiCheck = 0;
uint8_t wifi_status = WL_IDLE_STATUS;
uint32_t http_delay = DEFAULT_HTTP_DELAY;
float http_delay_backoff = 1.5;
uint32_t http_delay_max = 5 * 60 * 1000;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 60 * 60 * -7, 60000);

// Generated using this online tool
// https://jrabausch.github.io/lcd-image/web/

static const uint8_t logo[72] = {
  0x00, 0x00, 0x00, 0x01, 0x00, 0x80, 0x07, 0x83,
  0xc0, 0x07, 0xc7, 0xe0, 0x07, 0xe7, 0xc0, 0x07,
  0xe7, 0xe0, 0x07, 0xe7, 0xc0, 0x23, 0xc7, 0xc4,
  0xf1, 0xc3, 0x8f, 0xf8, 0x00, 0x1f, 0xf8, 0x10,
  0x1f, 0x78, 0x7e, 0x1e, 0x79, 0xff, 0x9e, 0x11,
  0xff, 0xc8, 0x03, 0xff, 0xc0, 0x07, 0xff, 0xe0,
  0x0f, 0xff, 0xf0, 0x0f, 0xff, 0xf0, 0x1f, 0xff,
  0xf8, 0x1f, 0xff, 0xf8, 0x1f, 0xff, 0xf8, 0x1f,
  0xc5, 0xf0, 0x04, 0x00, 0x20, 0x00, 0x00, 0x00
};

static const uint8_t icon_online[8] = { 0x00, 0x3c, 0x42, 0x99, 0x24, 0x42, 0x18, 0x00 };
static const uint8_t icon_offline[8] = { 0x00, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00 };
static const uint8_t icon_time[8] = { 0x3c, 0x42, 0x91, 0x91, 0x8d, 0x81, 0x42, 0x3c };

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

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
static void handle_poop_double_click();
static void handle_food_click();
static void handle_food_double_click();
static void handle_water_click();
static void handle_water_double_click();
void draw_boot_screen();
void draw_status_screen();
void check_wifi();
void setup_wifi();
void fetch_data();
void send_data(Panel *panel);

Panel panel_poop = {
  .id = "poop",
  .pin_ring = PIN_POOP_RING,
  .pin_button = PIN_POOP_BUTTON,
  .pin_indicator = PIN_POOP_INDICATOR,
  .duration = 1 * DAY_HOURS * HOUR_SEC,
  .snooze_duration = 12 * HOUR_SEC,
  .color = 0xff6600,
  .icon = { 0x00, 0x00, 0x10, 0x18, 0x3c, 0x3c, 0x7e, 0x7e },
  .x_offset = 44,

};

Panel panel_food = {
  .id = "food",
  .pin_ring = PIN_FOOD_RING,
  .pin_button = PIN_FOOD_BUTTON,
  .pin_indicator = PIN_FOOD_INDICATOR,
  .duration = 14 * DAY_HOURS * HOUR_SEC,
  .snooze_duration = 4 * DAY_HOURS * HOUR_SEC,
  .color = 0x339900,
  .icon = { 0x00, 0x6a, 0x6e, 0x64, 0x44, 0x44, 0x44, 0x44 },
  .x_offset = 2,
};

Panel panel_water = {
  .id = "water",
  .pin_ring = PIN_WATER_RING,
  .pin_button = PIN_WATER_BUTTON,
  .pin_indicator = PIN_WATER_INDICATOR,
  .duration = 7 * DAY_HOURS * HOUR_SEC,
  .snooze_duration = 2 * DAY_HOURS * HOUR_SEC,
  .color = 0x33CCFF,
  .icon = { 0x08, 0x18, 0x3c, 0x3c, 0x7e, 0x7a, 0x3c, 0x18 },
  .x_offset = 86,
};

#define NUM_PANELS 3
Panel *panels[NUM_PANELS] = { &panel_poop, &panel_food, &panel_water };

void setup() {
  Serial.begin(115200);
  logger.registerSerial(MYLOG, DEBUG, "tst");

  esp_core_dump_init();
  esp_core_dump_config_t config;
  config.core_dump_type = ESP_CORE_DUMP_FLASH;
  esp_core_dump_init(&config);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    logger.log(MYLOG, ERROR, "SSD1306 allocation failed");
    for (;;)
      ;
  }
  draw_boot_screen();

  for (uint8_t p = 0; p < NUM_PANELS; p += 1) {
    panel_setup(panels[p], &timeClient);
  }

  setup_wifi();
  timeClient.update();
  // fetch_data();

  panel_poop.button->attachClick(handle_poop_click);
  // btn_poop.attachDoubleClick(handle_poop_double_click);
  panel_food.button->attachClick(handle_food_click);
  // btn_food.attachDoubleClick(handle_food_double_click);
  panel_water.button->attachClick(handle_water_click);
  // btn_water.attachDoubleClick(handle_water_double_click);

  BaseType_t taskCreated = xTaskCreate(
    background_task,
    "background_task",
    8192,  // Increased from 4096
    NULL,
    2,
    NULL
  );
  
  if (taskCreated != pdPASS) {
    logger.log(MYLOG, ERROR, "Failed to create background task");
  }
}

void loop() {
  static unsigned long prev = millis();
  unsigned long _now = millis();
  if ((_now - prev) >= 100) {
    check_wifi();
    draw_status_screen();
    prev = _now;
  }

  for (uint8_t p = 0; p < NUM_PANELS; p += 1) {
    panel_loop(panels[p]);
    panels[p]->button->tick();
  }

  timeClient.update();
}

void background_task(void *parameter) {
  while (true) {
    for (uint8_t p = 0; p < NUM_PANELS; p += 1) {
      if (panels[p]->pending_http) {
        logger.log(MYLOG, DEBUG, "%s has a pending HTTP request", panels[p]->id);
        send_data(panels[p]);
        break;
      }
    }
    vTaskDelay(http_delay * portTICK_PERIOD_MS);
  }
}

void send_data(Panel *panel) {
  if (WiFi.status() != WL_CONNECTED) {
    logger.log(MYLOG, ERROR, "Not connected");
    return;
  }
  
  HTTPClient http;
  http.setTimeout(10000);  // 10 second timeout
  
  String url = String(String(SERVER_ROOT) + "/button?id=" + String(panel->id));
  logger.log(MYLOG, DEBUG, "send_data_task: %s", url.c_str());
  
  bool success = false;
  for (int retry = 0; retry < 3 && !success; retry++) {
    http.begin(url.c_str());
    int response_code = http.GET();
    
    if (response_code > 0) {
      String response = http.getString();
      logger.log(MYLOG, DEBUG, response.c_str());
      digitalWrite(panel->pin_indicator, HIGH);
      panel->pending_http = false;
      http_delay = DEFAULT_HTTP_DELAY;
      success = true;
    } else {
      logger.log(MYLOG, ERROR, "HTTP failed, retry %d: %s", 
                retry, http.errorToString(response_code).c_str());
      http_delay = min(http_delay_max, (uint32_t)(http_delay * http_delay_backoff));
      delay(100);  // Short delay between retries
    }
    http.end();
  }
}


void draw_boot_screen() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.drawBitmap(4, 4, logo, 24, 24, 1);

  display.setCursor(36, 12);
  display.println(F("CATBOX v3.0.1"));

  display.display();
}

void draw_status_screen() {
  display.clearDisplay();

  // Setup the display
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // TOP ROW
  display.drawBitmap(2, 2, icon_online, 8, 8, 1);
  display.setCursor(12, 2);
  display.println(WiFi.localIP());

  // MIDDLE ROW
  for (uint8_t p = 0; p < NUM_PANELS; p += 1) {
    panel_draw_status(panels[p], &display, panels[p]->x_offset, 12);
  }

  // BOTTOM ROW
  display.drawBitmap(2, 24, icon_time, 8, 8, 1);
  display.setCursor(12, 24);
  display.print(timeClient.getFormattedTime());

  display.display();
}

void check_wifi() {
  if (now() >= lastWifiCheck + 60) {
    lastWifiCheck = now();
    wifi_status = WiFi.status();
    
    if (wifi_status != WL_CONNECTED) {
      logger.log(MYLOG, ERROR, "WiFi disconnected, attempting reconnect");
      WiFi.disconnect();
      delay(1000);
      WiFi.begin(ssid, password);
    }
  }
}

void setup_wifi() {
  // Connect to the WiFi network
  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  Serial.print("Connecting...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Register the devices name with mDNS
  if (!MDNS.begin("catbox")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started at catbox.local");

  wifi_status = WiFi.status();
  timeClient.begin();
}

static void handle_poop_click() {
  panel_button_press(&panel_poop);
}

static void handle_poop_double_click() {
  panel_snooze(&panel_poop);
}

static void handle_food_click() {
  panel_button_press(&panel_food);
}

static void handle_food_double_click() {
  panel_snooze(&panel_food);
}

static void handle_water_click() {
  panel_button_press(&panel_water);
}

static void handle_water_double_click() {
  panel_snooze(&panel_water);
}


void fetch_data() {
  HTTPClient http;
  String url = String(String(SERVER_ROOT) + "/data");
  http.begin(url.c_str());
  http.GET();
  String payload = http.getString();
  http.end();
  JSONVar data = JSON.parse(payload);

  for (uint8_t p = 0; p < NUM_PANELS; p += 1) {
    panels[p]->last_pressed = timeClient.getEpochTime() - int(data[panels[p]->id]["lastPressed"]);
    panel_update_light(panels[p]);
  }
}
