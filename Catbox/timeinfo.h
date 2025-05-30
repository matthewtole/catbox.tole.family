#ifndef TIMEINFO_H
#define TIMEINFO_H

#include <Arduino.h>

extern struct tm timeinfo;

void setup_timeinfo();
void timeinfo_loop();
int get_hours(unsigned long time);

#endif
