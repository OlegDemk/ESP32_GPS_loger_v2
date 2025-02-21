#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_log.h"
#include "string.h"
#include "stdint.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"
// ------------------------------------------------------------------------------------------------------------------
#include "driver/i2c.h"
#include "i2c/i2c.h"
#include "i2c/bme280/bme280.h"
#include "i2c/bme280/main_bme280.h"
#include "i2c/dc3231/dc3231.h"
#include "i2c/pca9536d/pca9536d.h"
#include "i2c/oled_ssd1306/ssd1306.h"
#include "i2c/oled_ssd1306/font.h"
// ------------------------------------------------------------------------------------------------------------------
#include "driver/gpio.h"
// ------------------------------------------------------------------------------------------------------------------
#include "esp_heap_caps.h"
// ------------------------------------------------------------------------------------------------------------------
#include "nvs_flash.h"
#include "nvs.h"
// ------------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------------------------
#include "esp_spiffs.h"
#include "spiffs_config.h"
#include "esp_netif.h"
#include "esp_event.h"
// ------------------------------------------------------------------------------------------------------------------
#include "driver/adc.h"
#include "esp_adc_cal.h"
// ------------------------------------------------------------------------------------------------------------------
#include "gps_data.h"
#include "driver/uart.h"
#include "gps/main_gps.h"
#include "gps/nmea_parser.h"
// ------------------------------------------------------------------------------------------------------------------
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_http_server.h"

#include "WiFi/sta/sta.h"
#include "WiFi/ap/ap.h"
#include "WiFi/wifi.h"
// ------------------------------------------------------------------------------------------------------------------
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "buttery/buttery.h"
// ------------------------------------------------------------------------------------------------------------------
#include "gsm/gsm_sim800l.h"
// ------------------------------------------------------------------------------------------------------------------
#include "memory/nvs_and_spiffs.h"
// ------------------------------------------------------------------------------------------------------------------
#include "microsd/mount.h"

#define ON 1
#define OFF 0

#define SMS_FITBACK OFF

// ------------------------------------------------------------------------------------------
typedef struct {
	float temperature;
	float humidity;
	float preassure;
} bme280_thp_t;
// ------------------------------------------------------------------------------------------
typedef struct{
	uint8_t battery_level;
} battery_t;
// ------------------------------------------------------------------------------------------


void task_log_data_into_file(void *ignore);
void increment_counter_task(void *pvParameter);
void send_sms_message_plus_battery_level(char *message);
void task_get_gps_data_one_time(void* ignode);
void restart_all_esp32(void);

#endif // MAIN_H