
#pragma once

#include "../main.h"


void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void stop_webserver(httpd_handle_t *server_handle);

void save_wifi_config(const char *ssid, const char *password);
void get_saved_wifi_config(char *ssid, char *password);
void wifi_start(void);