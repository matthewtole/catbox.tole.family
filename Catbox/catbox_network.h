#ifndef CATBOX_NETWORK_H
#define CATBOX_NETWORK_H

// System includes first
#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <Wire.h>

// Local includes last
#include "Panel.h"
#include "constants.h"

// Forward declarations to avoid circular dependencies
struct Display;
struct Panel;

// External declarations
extern uint8_t status;

// Function declarations
void check_wifi();
void setup_wifi();
String get_wifi_ssid();
String get_ip_address();

#endif