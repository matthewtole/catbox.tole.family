#ifndef API_H
#define API_H

#include <Arduino_JSON.h>
#include <HTTPClient.h>

#include "config.h"
#include "panel.h"
#include "wifi-manager.h"

extern uint32_t http_delay;

void fetch_data(Panel **panels);
void send_data(Panel *panel);

#endif