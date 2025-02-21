#pragma once

#include "../../main.h"

#include <esp_http_server.h>

#include "sys/param.h"
#include "esp_vfs.h"

#include "esp_err.h"
#include "stddef.h"

#include "driver/gpio.h"
#include "soc/gpio_reg.h"

#include "cJSON.h"

#include "../../gsm/gsm_sim800l.h"
#include "../../memory/nvs_and_spiffs.h"

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)
#define SCRATCH_BUFSIZE 8192

struct file_server_data
{
	char base_path[ESP_VFS_PATH_MAX + 1];	// Base path of files storage
	char scratch[SCRATCH_BUFSIZE];			// Scratch buffer of temporary during file transfer
};

httpd_handle_t start_webserver_sta(void);
esp_err_t get_counter_handler_sta(httpd_req_t *req);
void wifi_init_sta(const char *ssid, const char *password);

httpd_handle_t NEW_start_webserver_sta(void);
// void stop_webserver(httpd_handle_t server);

