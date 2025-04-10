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

// Структура для зберігання часу від GPS
typedef struct {
    uint8_t hour;      
    uint8_t minute;    
    uint8_t second;    
    uint16_t thousand; 
    uint8_t day;   
    uint8_t month; 
    uint16_t year; 
} dysplay_time_date_gps_t;

// Структура для зберігання координат і GPS-даних
typedef struct {
    float latitude;
    float longitude;
    float speed;
    dysplay_time_date_gps_t time_date;    
    uint8_t sats_in_view;
} dysplay_gps_data_t;

// Структура для RTC годинника
typedef struct {
    uint8_t hours; 
    uint8_t minutes; 
    uint8_t seconds;
    uint8_t day;
    uint8_t date; 
    uint8_t month;
    uint8_t year;
} dysplay_rtc_t;

// Структури для виводу на OLED

typedef struct{
	uint8_t battery;
} dysplay_butery_t;

typedef struct {
    char wifi_mode[10];
    char wifi_ip[50];
} dysplay_wifi_t;

typedef struct{
	char log_status[10];
    float latitude;
    float longitude;
    float speed;
    uint8_t sats_in_view;
    uint8_t hours; 
    uint8_t minutes; 
    uint8_t seconds;
    uint8_t day;
    uint8_t date; 
    uint8_t month;
    uint8_t year;  
    uint8_t print_on_oled_status;
} dysplay_gps_log_status_t;

// ------------------------------------------------------------------------------------------


void task_log_data_into_file(void *ignore);
void increment_counter_task(void *pvParameter);
void send_sms_message_plus_battery_level(char *message);
void task_get_gps_data_one_time(void* ignode);
void restart_all_esp32(void);


#endif // MAIN_H