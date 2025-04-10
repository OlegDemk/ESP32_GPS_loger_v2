#include "wifi.h"

#define MAX_RETRY_COUNT 3

extern QueueHandle_t dysplay_wifi_mode_and_ip_queue;

httpd_handle_t ap_server_handle = NULL;
httpd_handle_t sta_server_handle = NULL;

int retry_count = 0;

extern TaskHandle_t increment_counter_task_handler;


// -----------------------------------------------------------------------------------------------------
void stop_webserver(httpd_handle_t *server_handle)
{
    if(*server_handle)
    {
        httpd_stop(*server_handle);
        *server_handle = NULL;
    }
}
// -----------------------------------------------------------------------------------------------------
void save_wifi_config(const char *ssid, const char *password)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if(err == ESP_OK)
    {
        nvs_set_str(my_handle, "wifi_ssid", ssid);
        nvs_set_str(my_handle, "wifi_password", password);
        nvs_commit(my_handle);
        nvs_close(my_handle);
        ESP_LOGI("NVS","WiFi settings saved");
    }
    else
    {
        ESP_LOGE("NVS","ERROR");
    }
}
// -----------------------------------------------------------------------------------------------------
void get_saved_wifi_config(char *ssid, char *password)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &my_handle);
    if(err == ESP_OK)
    {
        size_t ssid_len = 32, passwors_len = 32;
        nvs_get_str(my_handle, "wifi_ssid", ssid, &ssid_len);
        nvs_get_str(my_handle, "wifi_password", password, &passwors_len);
        nvs_close(my_handle);
    }
    else
    {
        ESP_LOGE("NVS","ERROR READ");
    }
}
// -----------------------------------------------------------------------------------------------------
void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) 
{
    static const char *TAG = "EVENT HANDLER";

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    } 
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) 
    {
        if (retry_count < MAX_RETRY_COUNT) 
        {
            ESP_LOGI(TAG, "WiFi disconnected, retrying... (Attempt %d/%d)", retry_count + 1, MAX_RETRY_COUNT);
            esp_wifi_connect();
            retry_count++;
        } 
        else 
        {
            stop_webserver(&sta_server_handle);

            // Видалити задачу, якщо вона була створина до цього
            // if(increment_counter_task_handler != NULL)
            // {
            //     vTaskDelete(increment_counter_task_handler);
            //     increment_counter_task_handler = NULL;
            // }
            
            retry_count = 0;  // Скидаємо лічильник для майбутніх підключень

            ESP_LOGI(TAG, "Max retry limit reached, switching to AP mode. <<<<<<<<<<<<<<<<<<<<<<<<<<<<");
            esp_wifi_stop();
            start_wifi_ap();
            //start_webserver_ap();
            
        }
    } 
    else if(event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) 
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

        // Print how to connect to STA server
        // char ip_buff[60] = {0};
        // snprintf(ip_buff, sizeof(ip_buff),  IPSTR "/counter", IP2STR(&event->ip_info.ip));
        // ESP_LOGI(TAG, "Type into url: %s", ip_buff);

        retry_count = 0;  // Скидаємо лічильник при успішному підключенні

        // Вивести дані на OLED
        dysplay_wifi_t dysplay_wifi;
        strcpy(dysplay_wifi.wifi_mode, "STA");          // Запис режиму
        snprintf(dysplay_wifi.wifi_ip, sizeof(dysplay_wifi.wifi_ip), IPSTR, IP2STR(&event->ip_info.ip));    // Запис IP
        xQueueSend(dysplay_wifi_mode_and_ip_queue, &dysplay_wifi, pdMS_TO_TICKS(100));   // передати для екрнана

        ESP_LOGI(TAG, "START HTTP SERVER");                                                 // реагує в браузері на: http://192.168.0.137/counter <<<<<<<<<<<<<<<<<<<<<<<<,  
        //start_webserver_sta();                          // Запуск HTTP-сервера 
        NEW_start_webserver_sta();
        // xTaskCreate(&increment_counter_task, "increment_counter_task", 4096, NULL, 5, &increment_counter_task_handler);
    }
}
// -----------------------------------------------------------------------------------------------------
/*
	@brief
    	Run STA WiFi mode
        If SSID and PASSWORD correct - connect to routrer, after that use another device, for example, PC connect to ESP32 use:
          "http://192.168.0.137/counter" and get counter value into brouser. (See unic IP into comport).
        If SSID and PASSWORD incorect -  turn off STA mode and run AP mode and run AP http server, and send simple http page "wifi_config_form".
        Connect to ESP32 use "http://192.168.4.1/config" (It PI has to be static), after connect, into brouser will be simple html page. Write
        new SSID and PASSWORD. Press "Submit" button, the new SSID and PASSWORD will be save into NVS memory and reser ESP32.
*/
void wifi_start(void)
{
    // Get ssid and password from memmory.
	char ssid[32] = {0};
	char password[32] = {0};
	get_saved_wifi_config(ssid, password);

	// If no any ssid and password.
	if(strlen(ssid) > 0)
	{
		// if settings was saved connect to annown WiFi network
		ESP_LOGI("WIFI","Connect to WiFi...");
		wifi_init_sta(ssid, password);
	}
	else
	{
		// If no annown WiFi data start AP   (Connect to "ESP32_Config" Acaes Point use "http://192.168.4.1/config" and set new SSID and PASSWORD)
		ESP_LOGI("WIFI","MAKE Asses Popnt...");
		start_wifi_ap();
		start_webserver_ap();
	}
}
// -----------------------------------------------------------------------------------------------------
void wifi_stop(void)
{
    ESP_LOGI("WiFi", "Stopping Wi-Fi and cleaning up...");

    // 1. Зупинити всі можливі вебсервери
    stop_webserver(&ap_server_handle);
    stop_webserver(&sta_server_handle);

    // // 2. Видалити задачу, яка була створена у STA режимі
    // if(increment_counter_task_handler != NULL)
    // {
    //     vTaskDelete(increment_counter_task_handler);
    //     increment_counter_task_handler = NULL;
    // }

    // 3. Зупинити саму WiFi підсистему
    ESP_ERROR_CHECK(esp_wifi_stop());
    ESP_ERROR_CHECK(esp_wifi_deinit());

    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<   BUG Тут перезагрузка

    // 4. Видалити WiFi інтерфейси
   // esp_netif_destroy_default_wifi();   

   

    // Видалити event loop, якщо створений
    esp_err_t err = esp_event_loop_delete_default();
    if (err == ESP_OK)
    {
        ESP_LOGI("WIFI", "Default event loop deleted successfully");
    }
    else if (err == ESP_ERR_INVALID_STATE)
    {
        ESP_LOGW("WIFI", "Event loop was not created or already deleted");
    }
    else
    {
        ESP_LOGE("WIFI", "Failed to delete event loop: %s", esp_err_to_name(err));
    }


    //esp_netif_destroy(esp_netif_t *esp_netif);
   

    // 4. Очистити інформацію на екрані
    dysplay_wifi_t dysplay_wifi;
    strcpy(dysplay_wifi.wifi_mode, "OFF");
    strcpy(dysplay_wifi.wifi_ip, "WiFi OFF");

    // 5. Вивести це на OLED через чергу
    xQueueSend(dysplay_wifi_mode_and_ip_queue, &dysplay_wifi, pdMS_TO_TICKS(100));

    ESP_LOGI("WiFi", "Wi-Fi stopped successfully.");
}
// -----------------------------------------------------------------------------------------------------

