#include "catbox_network.h"

#include <Elog.h>

const char *ssid = WIFI_SSID;
const char *password = WIFI_PASSWORD;
time_t lastWifiCheck = 0;
uint8_t wifi_status = WL_IDLE_STATUS;
uint32_t http_delay = DEFAULT_HTTP_DELAY;
float http_delay_backoff = 1.5;
uint32_t http_delay_max = 5 * 60 * 1000;

void fetch_data(Panel **panels, tm *timeinfo) {
  HTTPClient http;
  String url = String(String(SERVER_ROOT) + "/status");
  http.begin(url.c_str());
  http.GET();
  String payload = http.getString();
  http.end();
  JSONVar data = JSON.parse(payload);

  for (uint8_t p = 0; p < NUM_PANELS; p += 1) {
    panels[p]->last_pressed = timeinfo->tm_sec - int(data[panels[p]->id]);
    panel_update_light(panels[p]);
  }
}

void send_data(Panel *panel) {
  if (WiFi.status() != WL_CONNECTED) {
    logger.log(MYLOG, ERROR, "Not connected");
    return;
  }

  HTTPClient http;
  http.setTimeout(10000); // 10 second timeout

  String url = String(String(SERVER_ROOT) + "/update?id=" + String(panel->id));
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
      logger.log(MYLOG, ERROR, "HTTP failed, retry %d: %s", retry,
                 http.errorToString(response_code).c_str());
      http_delay =
          min(http_delay_max, (uint32_t)(http_delay * http_delay_backoff));
      delay(100); // Short delay between retries
    }
    http.end();
  }
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
}