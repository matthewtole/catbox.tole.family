#ifndef CATBOX_NETWORK_H
#define CATBOX_NETWORK_H

// System includes first
#include <Arduino.h>
#include <Arduino_JSON.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <Wire.h>

// ESP32 specific includes
#include "esp_core_dump.h"
#include "esp_spi_flash.h"
#include "esp_system.h"

// Local includes last
#include "Panel.h"
#include "constants.h"

// Forward declarations to avoid circular dependencies
struct Display;
struct Panel;

// Constants
#define DEFAULT_HTTP_DELAY 500

// External declarations
extern uint32_t http_delay;

// Function declarations
void check_wifi();
void setup_wifi();
void fetch_data(Panel **panels, tm *timeinfo);
void send_data(Panel *panel);

#endif