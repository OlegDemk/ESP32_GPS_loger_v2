/*
 * udp.h
 *
 *  Created on: May 18, 2023
 *      Author: odemki
 */

#ifndef MAIN_WIFI_HTTP_H_
#define MAIN_WIFI_HTTP_H_

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include <esp_http_server.h>

#include "sys/param.h"
#include "esp_vfs.h"

#include "esp_err.h"
#include "stddef.h"

#include "driver/gpio.h"
#include "soc/gpio_reg.h"

#include "cJSON.h"

#include "../main.h"

#include "../gsm/gsm_sim800l.h"
#include "../memory/nvs_and_spiffs.h"

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)
#define SCRATCH_BUFSIZE 8192

struct file_server_data
{
	char base_path[ESP_VFS_PATH_MAX + 1];	// Base path of files storage
	char scratch[SCRATCH_BUFSIZE];			// Scratch buffer of temporary during file transfer
};

void init_http_server(void);
httpd_handle_t start_webserver(void);
void stop_webserver(httpd_handle_t server);




#endif /* MAIN_WIFI_HTTP_H_ */
