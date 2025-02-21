#pragma once

#include "../../main.h"

void start_wifi_ap(void);
httpd_handle_t start_webserver_ap(void);
static esp_err_t config_get_handler_ap(httpd_req_t *req);
static esp_err_t config_post_handler_ap(httpd_req_t *req);


