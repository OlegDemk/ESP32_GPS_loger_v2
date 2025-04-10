/*
 * main_gps.c
 *
 *  Created on: Dec 25, 2023
 *      Author: odemki
 */



#include "main_gps.h"


#define TIME_ZONE (+2)   //Beijing Time
#define YEAR_BASE (2000) //date in GPS starts from 2000

extern QueueHandle_t gps_data_log_queue;
extern QueueHandle_t GPS_queue;

extern bool gps_log_working_flag;
extern bool init_gps_status_flag;

extern QueueHandle_t dysplay_gps_data_queue;

extern TaskHandle_t task_log_data_into_file_handlr;
extern TaskHandle_t task_get_gps_data_one_time_handler;

 
/////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief GPS Event Handler
 *
 * @param event_handler_arg handler specific arguments
 * @param event_base event base, here is fixed to ESP_NMEA_EVENT
 * @param event_id event id
 * @param event_data event specific arguments
 */
static void gps_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
	static const char *TAG = "GPS: ";
	gps_data_gps_t gps_data;
    gps_t *gps = NULL;

    switch (event_id) {
    case GPS_UPDATE:
        gps = (gps_t *)event_data;
        /* print information parsed from GPS statements */
//        ESP_LOGI(TAG, "%d/%d/%d %d:%d:%d => \r\n"
//                 "\t\t\t\t\t\tlatitude   = %.05f°N\r\n"
//                 "\t\t\t\t\t\tlongitude = %.05f°E\r\n"
//                 "\t\t\t\t\t\taltitude   = %.02fm\r\n"
//                 "\t\t\t\t\t\tspeed      = %fm/s",
//                 gps->date.year + YEAR_BASE, gps->date.month, gps->date.day,
//                 gps->tim.hour + TIME_ZONE, gps->tim.minute, gps->tim.second,
//				 gps->latitude, gps->longitude, gps->altitude, gps->speed);

    	// Fill in queue
        gps_data.latitude = gps->latitude;
        gps_data.longitude = gps->longitude;
        gps_data.altitude = gps->altitude;
        gps_data.speed = gps->speed;
        gps_data.sats_in_view = gps->sats_in_view;

        gps_data.time.hour = gps->tim.hour+TIME_ZONE;
        gps_data.time.minute = gps->tim.minute;
        gps_data.time.second = gps->tim.second;

        gps_data.date.year = gps->date.year + YEAR_BASE;
        gps_data.date.month = gps->date.month;
        gps_data.date.day = gps->date.day;

        // xQueueSendToBack(gps_data_html_queue, &qLogGPSData_t, 0);
        xQueueSendToBack(gps_data_log_queue, &gps_data, 0);
//        xQueueSendToBack(gps_data_json_queue, &qLogGPSData_t, 0);


        break;
    case GPS_UNKNOWN:
        /* print unknown statements */
        ESP_LOGW(TAG, "Unknown statement:%s", (char *)event_data);
        break;
    default:
        break;
    }
}
// --------------------------------------------------------------------------------------------------

nmea_parser_handle_t nmea_hdl = NULL;
//nmea_parser_config_t config = NULL;

void turn_on_gps(void)
{
	init_on_off_gps_gpio();
	turn_on_gps_module();
	nmea_parser_config_t config = NMEA_PARSER_CONFIG_DEFAULT();	    /* NMEA parser configuration */
	nmea_hdl = nmea_parser_init(&config);		/* init NMEA parser library */
	nmea_parser_add_handler(nmea_hdl, gps_event_handler, NULL);		/* register event handler for NMEA parser library */
}
// --------------------------------------------------------------------------------------------------
void turn_off_gps(void)
{
	init_on_off_gps_gpio();
	turn_off_gps_module();
	nmea_parser_remove_handler(nmea_hdl, gps_event_handler);		/* unregister event handler */
	nmea_parser_deinit(nmea_hdl);									//    /* deinit NMEA parser library */
}
// --------------------------------------------------------------------------------------------------

#define GPIO_POWER_GPS (1ULL<<CONFIG_POWER_GPS_GPIO)

void init_on_off_gps_gpio(void)
{
	gpio_config_t io_conf = {};		// structure for configure GPIOs
	// Configure output PIN
	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_OUTPUT;
	io_conf.pin_bit_mask = GPIO_POWER_GPS;
	io_conf.pull_down_en = 0;
	io_conf.pull_up_en = 0;
	gpio_config(&io_conf);
}
// --------------------------------------------------------------------------------------------------
void turn_on_gps_module(void)
{
	gpio_set_level(CONFIG_POWER_GPS_GPIO, 0);
}
// --------------------------------------------------------------------------------------------------
void turn_off_gps_module(void)
{
	gpio_set_level(CONFIG_POWER_GPS_GPIO, 1);
}
// --------------------------------------------------------------------------------------------------
int check_gps_data_valid(gps_data_gps_t *gps_data)
{
	static const char *TAG_LOG = "GPS: ";
	static bool status = 0;
	
	if(gps_data->latitude == 0 )
	{	
		return 0;
	}
	else
	{
		return 1;
	}
	status = !status;
}
// --------------------------------------------------------------------------------------------------
void show_gps_data(gps_data_gps_t *gps_data)
{
	static const char *TAG_LOG = "GPS: ";

	ESP_LOGI(TAG_LOG, "---------- GPS data ----------");
	ESP_LOGI(TAG_LOG, "latitude: %.05f°N", gps_data->latitude);
	ESP_LOGI(TAG_LOG, "longitude: %.05f°E", gps_data->longitude);
	ESP_LOGI(TAG_LOG, "altitude: %.02fm", gps_data->altitude);
	ESP_LOGI(TAG_LOG, "speed: %.02fm", gps_data->speed);

	ESP_LOGI(TAG_LOG, "second: %d", gps_data->time.second);
	ESP_LOGI(TAG_LOG, "minute: %d", gps_data->time.minute);
	ESP_LOGI(TAG_LOG, "hour: %d", gps_data->time.hour);

	ESP_LOGI(TAG_LOG, "day: %d", gps_data->date.day);
	ESP_LOGI(TAG_LOG, "month: %d", gps_data->date.month);
	ESP_LOGI(TAG_LOG, "year: %d", gps_data->date.year);
	ESP_LOGI(TAG_LOG, "sats -in view: %d", gps_data->sats_in_view);
	ESP_LOGI(TAG_LOG, "---------------------------------");
}
// -----------------------------------------------------------------------------------------------------
void send_data_to_client(gps_data_gps_t *gps_data)	
{
	static const char *LOG_TAG = "SEND GPS DATA TO HTTPS";
	BaseType_t qStatus = false;

	gps_data_gps_t gps_data_for_http;
	// Copy one structure to another
	memcpy(&gps_data_for_http, gps_data, sizeof(gps_data_gps_t));

	// Send data to show in HTTP server
	BaseType_t result = xQueueSend(GPS_queue, (void*)&gps_data_for_http, pdMS_TO_TICKS(100));
	if(result == pdPASS)
	{
		ESP_LOGI(LOG_TAG, "Send data");
	}
	else if(result == errQUEUE_FULL)
	{
		ESP_LOGE(LOG_TAG, "The queue is full");		
	}
	else
	{
		ESP_LOGE(LOG_TAG, "Failed send GPS data");
	}
}
// -----------------------------------------------------------------------------------------------------
void gps_log_on(void)
{
	ESP_LOGI("SMS command", "LOG ON, command from SMS");
	
	if(gps_log_working_flag == false)
	{
		gps_log_working_flag = true;
	
		xTaskCreate(task_log_data_into_file, "task_log_data_into_file", 4096, NULL, configMAX_PRIORITIES - 1, &task_log_data_into_file_handlr);
		gps_data_log_queue = xQueueCreate(5, sizeof(gps_data_gps_t));
	}
	else
	{
		#if SMS_FITBACK == ON
			send_sms_message_plus_battery_level("GPS log was started!");
		#endif	
	}
}
// -----------------------------------------------------------------------------------------------------
void gps_log_off(void)   
{
	ESP_LOGI("SMS command", "LOG OFF, command from SMS");
	
	if(gps_log_working_flag == true)
	{
		#if SMS_FITBACK == ON
			send_sms_message_plus_battery_level("GPS log STOP");
		#endif
	
		// Delete log task
		if(task_log_data_into_file_handlr != NULL)
		{
			vTaskDelete(task_log_data_into_file_handlr);
			task_log_data_into_file_handlr = NULL;
		}	
		// Delete queue
		if(gps_data_log_queue != NULL)
		{
			vQueueDelete(gps_data_log_queue);
			gps_data_log_queue = NULL;
		}
		turn_off_gps();
		gpio_set_level(CONFIG_GREEN_GPIO, 0);
		init_gps_status_flag = false;
		gps_log_working_flag = false;

		// For delete data from OLED
		dysplay_gps_log_status_t dysplay_gps_log_status_data;
		dysplay_gps_log_status_data.print_on_oled_status = 1;
		xQueueSend(dysplay_gps_data_queue, (void*)&dysplay_gps_log_status_data, pdMS_TO_TICKS(100));
	}
	else
	{
		#if SMS_FITBACK == ON
			send_sms_message_plus_battery_level("GPS log was stop!");
		#endif
	}

	
}
// -----------------------------------------------------------------------------------------------------
void send_one_point_gps_data(void) 
{
	ESP_LOGI("SMS command", "Send one point GPS data, command from SMS");
	
	xTaskCreate(task_get_gps_data_one_time, "task_get_gps_data_one_time", 4096, NULL, configMAX_PRIORITIES - 1, &task_get_gps_data_one_time_handler);
	gps_data_log_queue = xQueueCreate(5, sizeof(gps_data_gps_t));
}
// -----------------------------------------------------------------------------------------------------