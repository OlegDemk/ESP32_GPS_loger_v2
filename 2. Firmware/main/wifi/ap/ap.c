#include "ap.h"

extern httpd_handle_t ap_server_handle;

extern QueueHandle_t dysplay_wifi_mode_and_ip_queue;

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

// HTML-код форми для налаштування Wi-Fi
const char *wifi_config_form = "<!DOCTYPE html><html><head><title>WiFi Config</title></head>"
                               "<body><h2>Enter WiFi Credentials</h2>"
                               "<form action=\"/config\" method=\"post\">"
                               "SSID:<br><input type=\"text\" name=\"ssid\"><br>"
                               "Password:<br><input type=\"password\" name=\"password\"><br><br>"
                               "<input type=\"submit\" value=\"Submit\"></form></body></html>";



 //-----------------------------------------------------------------------------------------------------
// Обробник POST-запиту для збереження конфігурації Wi-Fi
static esp_err_t config_post_handler_ap(httpd_req_t *req)
{
    // "ssid=xxx&password=xxxxxxxxxx"
    ESP_LOGI("POST HANDLER","Sawe new ssid and password");

    char content[100] = {0};
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0)
    {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    content[recv_size] = '\0';

    // Parse ssid and password
    ESP_LOGI("POST HANDLER","Content: %s", content);                        // Розпарсити цей рядок "ssid=007&password=demkiv999666"  з нього дані
    char ssid[32] = {0}, password[32] = {0};

    // Знайти початок "ssid="" і "password=""
    char *ssid_ptr = strstr(content, "ssid=");
    char *password_ptr = strstr(content, "password=");

    if(ssid_ptr && password_ptr)             // if fouund
    {
        // Example string "ssid=XXXX&password=XXXXXXXXXXX"
        // Зсунути знячення початку строки на довжину "ssid=" і "password=". 
        ssid_ptr += strlen("ssid=");
        password_ptr += strlen("password=");

        // Витягнути ssid використовуючи позицію '&'
        strncpy(ssid, ssid_ptr, password_ptr - ssid_ptr - strlen("&password="));
        ssid[password_ptr - ssid_ptr - strlen("&password=")] = '\0';

        // скопіювати знячерря password до кінця
        strncpy(password, password_ptr, sizeof(password) - 1);

        ESP_LOGI("POST HANDLER","NEW ssid: %s, NEW password: %s", ssid, password);

        save_wifi_config(ssid, password);

        httpd_resp_send(req, "Configuration saved, restart device", HTTPD_RESP_USE_STRLEN);

        vTaskDelay(100 / portTICK_PERIOD_MS);  // Коротка затримка перед перезавантаженням
        esp_restart();
    }
    else
    {
        ESP_LOGE("POST HANDLER","PARSING ERROR");
    }
    return ESP_OK;
}
// -----------------------------------------------------------------------------------------------------
// Обробник GET-запиту для відображення HTML-форми
static esp_err_t config_get_handler_ap(httpd_req_t *req)
{
    ESP_LOGI("GET HANDLER","Send html page");

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, wifi_config_form, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
// -----------------------------------------------------------------------------------------------------
httpd_handle_t start_webserver_ap(void)
{
    stop_webserver(&ap_server_handle);          // Stop ap_server if it was run before

    ESP_LOGI("WAP","start web server");

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    //#define CONFIG_HTTPD_MAX_REQ_HDR_LEN 1024 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< SDKConfig->HTTP Server->Max HTTP Request Header Length = 1024

    if (httpd_start(&ap_server_handle, &config) == ESP_OK)
    {
        httpd_uri_t config_post_uri = {
            .uri       = "/config",
            .method    = HTTP_POST,
            .handler   = config_post_handler_ap,
            .user_ctx  = NULL
    };

    httpd_uri_t config_get_uri = {
            .uri       = "/config",
            .method    = HTTP_GET,
            .handler   = config_get_handler_ap,
            .user_ctx  = NULL
    };

        httpd_register_uri_handler(ap_server_handle, &config_post_uri);
        httpd_register_uri_handler(ap_server_handle, &config_get_uri);
    }
    return ap_server_handle;
}
// -----------------------------------------------------------------------------------------------------
void start_wifi_ap(void)
{
     // Створюємо інтерфейс Wi-Fi для AP
     esp_netif_create_default_wifi_ap();
 
     // Налаштування параметрів точки доступу
     wifi_config_t wifi_config = {
         .ap = {
             .ssid = "ESP32_Config",
             .ssid_len = strlen("ESP32_Config"),
             .channel = 1, 
             .password = "1234567890",
             .max_connection = 4,
             .authmode = WIFI_AUTH_WPA_WPA2_PSK
         },
     };
 
     // Якщо пароль пустий, використовуємо відкриту мережу
     if (strlen((char*)wifi_config.ap.password) == 0) {
         wifi_config.ap.authmode = WIFI_AUTH_OPEN;
     }
 
     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));  // Встановлюємо режим AP
     ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config)); // Налаштовуємо Wi-Fi
     ESP_ERROR_CHECK(esp_wifi_start());  // Запускаємо Wi-Fi
 
    
    ESP_LOGI("WiFi AP", "AP started. Connect to %s, and visit http://192.168.4.1/config to set Wi-Fi credentials.", wifi_config.ap.ssid);

    // Вивести дані на OLED
    dysplay_wifi_t dysplay_wifi;
    strcpy(dysplay_wifi.wifi_mode, "AP");          // Запис режиму
    strcpy(dysplay_wifi.wifi_ip, "AP: ESP32_Confighttp://192.168.4.1/config");     // Запис IP  
    xQueueSend(dysplay_wifi_mode_and_ip_queue, &dysplay_wifi, pdMS_TO_TICKS(100));   // передати для екрнана

    vTaskDelay(1000 / portTICK_PERIOD_MS); // Затримка перед запуском HTTP-сервера

    start_webserver_ap();               // Запустити вебсервер після успішного старту AP
}
// -----------------------------------------------------------------------------------------------------
