#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

// System includes first
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <Wire.h>

#include "config.h"

extern uint8_t status;

// Function declarations
void check_wifi();
void setup_wifi();
String get_wifi_ssid();
String get_ip_address();

#endif