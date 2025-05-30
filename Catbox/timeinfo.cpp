#include "timeinfo.h"
#include "config.h"

const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 0;
const int daylightOffset_sec = 3600;
struct tm timeinfo;

void setup_timeinfo() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void timeinfo_loop() { getLocalTime(&timeinfo); }

int get_hours(unsigned long time) {
  return (int)((timeinfo.tm_sec - time) / (HOUR_SEC));
}
