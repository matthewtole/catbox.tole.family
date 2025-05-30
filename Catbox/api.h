#ifndef API_H
#define API_H

#include <Arduino_JSON.h>
#include <Elog.h>
#include <HTTPClient.h>

#include "Panel.h"
#include "catbox_network.h"
#include "config.h"

extern uint32_t http_delay;

void fetch_data(Panel **panels, tm *timeinfo);
void send_data(Panel *panel);

#endif