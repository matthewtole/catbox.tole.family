#include "api.h"
#include "timeinfo.h"

uint32_t http_delay = DEFAULT_HTTP_DELAY;
float http_delay_backoff = 1.5;
uint32_t http_delay_max = 5 * 60 * 1000;

void fetch_data(Panel **panels) {
  HTTPClient http;
  String url = String(String(SERVER_ROOT) + "/status");
  http.begin(url.c_str());
  http.GET();
  String payload = http.getString();
  http.end();
  JSONVar data = JSON.parse(payload);

  for (uint8_t p = 0; p < NUM_PANELS; p += 1) {
    panels[p]->last_pressed = timeinfo.tm_sec - int(data[panels[p]->id]);
    panel_update_light(panels[p]);
  }
}

void update_delay() {
  http_delay = min(http_delay_max, (uint32_t)(http_delay * http_delay_backoff));
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

    if (response_code == 200) {
      panel_set_pending(panel, false);
      http_delay = DEFAULT_HTTP_DELAY;
      http.end();
      return;
    }

    logger.log(MYLOG, ERROR, "HTTP failed, retry %d: %s", retry,
               http.errorToString(response_code).c_str());
    update_delay();
    delay(100); // Short delay between retries
  }
}