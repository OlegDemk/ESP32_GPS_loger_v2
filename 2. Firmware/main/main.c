#include "main.h"

// Task handlers
TaskHandle_t task_resurse_monitor_handlr;
TaskHandle_t task_led_blink_handler;
TaskHandle_t task_battery_data_handle;
TaskHandle_t task_get_gps_data_one_time_handler;
TaskHandle_t task_gsm_handler;
TaskHandle_t task_log_data_into_file_handlr;
TaskHandle_t task_bme280_handlr;

// Queues
QueueHandle_t gps_data_log_queue = NULL;

#define QUEUE_LENGHT_BME280 1
QueueHandle_t bme280_queue = NULL;

#define QUEUE_LENGHT_GPS 1
QueueHandle_t GPS_queue = NULL;

extern QueueHandle_t i2c_queue;

#define QUEUE_LENGHT_BATTERY 1
QueueHandle_t battery_queue = NULL;

// ------------------------------------------------------------------------------------------
void task_bme280(void *ignore)
{
	static const char *TAG_BME280 = "BME280";
	bme280_thp_t bme280_thp_queue;
	bme280_data_t bme280_data;

	while(1)
	{
		bme280_test_read_thp(&bme280_data);  // Передати вказівник на структуру в функцію

		ESP_LOGI(TAG_BME280, "%.2f degC / %.3f hPa / %.3f %%", bme280_data.temperature, bme280_data.pressure, bme280_data.humidity);		

		bme280_thp_queue.temperature = bme280_data.temperature;
		bme280_thp_queue.humidity = bme280_data.humidity;
		bme280_thp_queue.preassure = bme280_data.pressure;

		BaseType_t result = xQueueSend(bme280_queue, (void*)&bme280_thp_queue, pdMS_TO_TICKS(100));
		if(result == pdPASS)
		{
			ESP_LOGI(TAG_BME280, "Send data: T:%.1f, H:%.1f, P:%.1f", bme280_thp_queue.temperature, bme280_thp_queue.humidity, bme280_thp_queue.preassure);
		}
		else if(result == errQUEUE_FULL)
		{
			ESP_LOGE(TAG_BME280, "The queue is full");		 
		}
		else
		{
			ESP_LOGE(TAG_BME280, "Failed send BME280 data");
		}
		//ESP_LOGI(TAG_BME280, "%.2f degC / %.3f hPa / %.3f %%", t, p, h);

		vTaskDelay(1000/portTICK_PERIOD_MS);
	}
}
// ------------------------------------------------------------------------------------------
void restart_all_esp32(void)
{
	turn_off_gps_module();
	#if SMS_FITBACK == ON
		send_sms_message_plus_battery_level("Restarting...");
	#endif
	
	esp_restart();
}
// ----------------------------------------------------------------------------------------------
void send_sms_message_plus_battery_level(char *message)
{
	//	Add battery level to sms
	char buff_sms_str[35] = {0};
	uint8_t battery_level = raad_barrety_level();

	snprintf(buff_sms_str, sizeof(buff_sms_str), "%s Bat: %d%%", message, battery_level);
	send_sms(CONFIG_MY_MOBILE_NUMBER, buff_sms_str);
}
// ----------------------------------------------------------------------------------------------
bool init_gps_status_flag = false;
bool gps_log_working_flag = false;

void task_log_data_into_file(void *ignore)
{
	static const char *LOG_TAG = "LOG GPS DATA";
	
	uint8_t log_data_save_period = 5;				// Period of lging data into Micro CD.
	int gps_data_incoming_counter = 1;			
	int gps_point_counter = 0;						// Point counter, for save into file.
	bool init = true;								// For create new file(first time) or add new GPS data to old file.
	
	gps_data_gps_t gps_data;
	BaseType_t qStatus = false;
	
	const char* base_path = "/data";
	
	// Get name for new file (using flash memory)
	char name[20] = {0,};
	get_file_name(&name);						   // некоректно вертає назву файлу <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	
	if(init_gps_status_flag == false)														// If GPS module was OFF
	{
		turn_on_gps();
		init_gps_status_flag = true;
	}
	
	#if SMS_FITBACK == ON	
		send_sms_message_plus_battery_level("GPS log Start...");							//	Add battery level to sms
	#endif
	
	while(1)
	{
		qStatus = xQueueReceive(gps_data_log_queue, &gps_data, 1000/portTICK_PERIOD_MS);	// Receive GPS dsta from GPS
		if(qStatus == pdPASS)
		{
			show_gps_data(&gps_data);
			// gps_signal_led_indication(&gps_data);    Зробити індикацію з використанням PCA !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			send_data_to_client(&gps_data);													

			if(gps_data_incoming_counter == log_data_save_period)							// SaveGPS data periodically
			{
				if(init == true)															//	If save data first time
				{
					log_data1(base_path, name, &gps_data);
					init = false;
				}
				else
				{
					log_data2(base_path, name, &gps_data, gps_point_counter++);
				}
			}
			gps_data_incoming_counter++;
			if(gps_data_incoming_counter > log_data_save_period)
			{
				gps_data_incoming_counter = 0;
			}
		}
		vTaskDelay(500/portTICK_PERIOD_MS);
	}
}
// ----------------------------------------------------------------------------------------------
void task_get_gps_data_one_time(void* ignode)
{
	static const char *GSM_TAG = "GPS";
	
	gps_data_gps_t gps_data;
	BaseType_t qStatus = false;
	int status = 0;
	
	#if SMS_FITBACK == ON
		send_sms_message_plus_battery_level("One point GPS START...");
	#endif
	
	ESP_LOGI(GSM_TAG, "GET ONE SHOT GPS DATA...");
	
	vTaskDelay(10000 / portTICK_PERIOD_MS);													// It delay need betvean two SMS
	
	if(init_gps_status_flag == false)														// If GPS module in OFF state
	{
		turn_on_gps(); 
		init_gps_status_flag = true;
	}
	
	while(1)
	{
		qStatus = xQueueReceive(gps_data_log_queue, &gps_data, 1000/portTICK_PERIOD_MS);	// Receive GPS dsta from GPS
		if(qStatus == pdPASS)
		{
			// gps_signal_led_indication(&gps_data);     Зробити індикацію з використанням PCA !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			show_gps_data(&gps_data);
			
			int status = check_gps_data_valid(&gps_data); 
			if(status == 1)																	// if GPS data valid 
			{
				ESP_LOGI(GSM_TAG, "GSM data valid ");
				char buff[50] = {0,};
				
				sprintf(buff, "%05f, %05f", gps_data.latitude, gps_data.longitude);
				//send sms with GPS data
				#if SMS_FITBACK == ON
					send_sms_message_plus_battery_level(buff);
				#endif
		
				vTaskDelay(1000 / portTICK_PERIOD_MS);

				// If GPS_log mode is in OFF mode, then GPS can be turn OFF
				if(gps_log_working_flag == false)
				{
					ESP_LOGI(GSM_TAG, "TURN OFF GPS MODULE");   
					turn_off_gps();
					init_gps_status_flag = false;
				}
				gpio_set_level(CONFIG_GREEN_GPIO, 0);
				
				ESP_LOGI(GSM_TAG, "Delete one shot GPS log task");
				vTaskDelete(NULL);	
			}
		}
	}
}
// ------------------------------------------------------------------------------------------------------------------
void task_battery_data(void *ignore)
{
	const static char *TAG_BATTERY = "BATTERY TASK";

	battery_t batterty;
	batterty.battery_level = 0;

	while(1)
	{
		batterty.battery_level = raad_barrety_level();  

		// передача даних для http сервера
		BaseType_t result = xQueueSend(battery_queue, (void*)&batterty, pdMS_TO_TICKS(100));
		if(result == pdPASS)
		{
			ESP_LOGI(TAG_BATTERY, "Send battery data: %d", batterty.battery_level);
		}
		else if(result == errQUEUE_FULL)
		{
			ESP_LOGE(TAG_BATTERY, "The queue is full");		
		}
		else
		{
			ESP_LOGE(TAG_BATTERY, "Failed send battery data");
		}
		
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
// ------------------------------------------------------------------------------------------------------------------
void task_blink(void *ignore)				// For debug
{
	while(1)
	{
		set_led(GREEN_LED, LED_ON);
		vTaskDelay(50 / portTICK_PERIOD_MS);
		set_led(GREEN_LED, LED_OFF);
		vTaskDelay(100 / portTICK_PERIOD_MS);
		set_led(GREEN_LED, LED_ON);
		vTaskDelay(50 / portTICK_PERIOD_MS);
		set_led(GREEN_LED, LED_OFF);
		vTaskDelay(1750 / portTICK_PERIOD_MS);
	}
}
// ------------------------------------------------------------------------------------------------------------------
void task_resurse_monitor(void *ignore)
{
	while(1)
	{
        char status[1024] = {0,};

		// Shows: name of tasks, status, priority, free stack size and task number on each tasks
		vTaskList(status);
		ESP_LOGI("\033[1;36mTaskList", "\n%s\033[1;36m", status);

        // Shows: spend time cheduller on each tasks
        memset(status, 0, sizeof(status));
        vTaskGetRunTimeStats(status);
        ESP_LOGI("\033[1;36mRunTimeStats", "\n%s\033[1;36m", status);

		vTaskDelay(15000/portTICK_PERIOD_MS);		
	}
}
// ---------------------------------------------------------------------------------------------------------------------------------------
TaskHandle_t increment_counter_task_handler;
int counter = 0; 

void increment_counter_task(void *pvParameter)
{
    while(1) 
    {
        counter++;
        ESP_LOGI("COUNTER TASK", "Counter: %d", counter);
        vTaskDelay(pdMS_TO_TICKS(1000));  
    }
}
// ---------------------------------------------------------------------------------------------------------------------------------------
void init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();
    if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}
// ---------------------------------------------------------------------------------------------------------------------------------------
void task_gsm(void *ignore)
{
	static const char *GSM_TAG = "GSM";
	int buf_size = 1024;

	ESP_LOGI(GSM_TAG ,"Primary mobile number: %s", CONFIG_MY_MOBILE_NUMBER);
	
	vTaskDelay(1000 / portTICK_PERIOD_MS);

	gsm(ON);																					// turn on GSM module by defoult
	init_uart_for_gsm();
	vTaskDelay(12000 / portTICK_PERIOD_MS);														// Delay for load GSM module
	
	init_sim800l();

	while(1)
	{
		uint8_t data[buf_size];
		
		int length = uart_read_bytes(UART_NUM_1, data, buf_size, 1000/portTICK_PERIOD_MS);			// Waiting data from GSM module
		if(length > 0)
		{
			data[length] = '\0';
			ESP_LOGI(GSM_TAG, "New message from GSM: %s", data);
			
			if(strstr((char*)data, "RING") != NULL)													// If incoming call detected
			{
				ESP_LOGI(GSM_TAG, "Incoming call");
				
				// Define incoming phone namber. If it my number answer call
				send_at_command("AT+CLCC\r\n");					
				vTaskDelay(1000 / portTICK_PERIOD_MS);
				
				uint8_t response[buf_size];
                read_gsm_response(response, buf_size);
				
				if (strstr((char*)response, CONFIG_MY_MOBILE_NUMBER) != NULL)
                {
                    ESP_LOGI(GSM_TAG, "Authorized number calling: %s", CONFIG_MY_MOBILE_NUMBER);
                    answer_call(); 																	// Pick up the phone
                }
                else
                {
                    ESP_LOGI(GSM_TAG, "Unauthorized number calling. Ignoring call.");
                    send_at_command("ATH\r\n");  													// Hung up the phone
                }	
			}
			else if(strstr((char*)data, "+CMT") != NULL)											// If incoming SMS detected
			{
				ESP_LOGI(GSM_TAG, "SMS received, checking the content...");
				
				// Check if it's from the authorized number
    			if (strstr((char*)data, CONFIG_MY_MOBILE_NUMBER) != NULL)
    			{
					process_sms((char*)data);
				}
				else
				{
					ESP_LOGI(GSM_TAG, "SMS from unauthorized number. Ignoring.");
				}
			}
		}
	}
}
// ------------------------------------------------------------------------------------------


void app_main(void)
{
	GPS_queue = xQueueCreate(QUEUE_LENGHT_GPS, sizeof(gps_data_gps_t)); 
	if(GPS_queue == NULL)
	{
		ESP_LOGE("CREATE QUEUE" ,"Failed create GPS_queue queue");
		return;
	}

	bme280_queue = xQueueCreate(QUEUE_LENGHT_BME280, sizeof(bme280_thp_t)); 
	if(bme280_queue == NULL)
	{
		ESP_LOGE("CREATE QUEUE" ,"Failed create bme280 queue");
		return;
	}

	battery_queue = xQueueCreate(QUEUE_LENGHT_BATTERY, sizeof(battery_t)); 
	if(battery_queue == NULL)
	{
		ESP_LOGE("CREATE QUEUE" ,"Failed create GPS_queue queue");
		return;
	}

	// I2C  //////////////////////////////////////////////////////////
	bme280_data_t bme280_data; 

	i2c_queue = xQueueCreate(I2C_QUEUE_LENGTH, I2C_QUEUE_ITEM_SIZE);
	if(i2c_queue == NULL)
	{
		ESP_LOGE("CREATE QUEUE" ,"Failed create I2C queue");
		return;
	}
	i2c_master_init();
	xTaskCreate(i2c_manager_task, "I2C_Manager", 4096, NULL, configMAX_PRIORITIES-1, NULL);
	i2c_staner();     

	/////////////////////////////////////////////////////////////////
	vTaskDelay(10 / portTICK_PERIOD_MS);
	oled_print_text("HELLO", 0, 5);				
	bme280_test_read_thp(&bme280_data);			// T, H and P
	test_rtc_ds3231m();							// Time
	test_pca9536d();							// LEDs and button
	init_nvs();
	///////////////////////////////////////////////////////////////////


	const char* base_path = "/data";
    ESP_ERROR_CHECK(example_mount_storage(base_path));			

	vTaskDelay(100 / portTICK_PERIOD_MS);


	wifi_start();

	vTaskDelay(1000 / portTICK_PERIOD_MS);
	
    xTaskCreate(task_resurse_monitor, "task_resurse_monitor", 4096, NULL, configMAX_PRIORITIES - 3, &task_resurse_monitor_handlr);
	xTaskCreate(task_blink, "task_blink", 1024, NULL, configMAX_PRIORITIES - 3, &task_led_blink_handler);
	xTaskCreate(task_battery_data, "task_battery_data", 4096, NULL, configMAX_PRIORITIES - 2, &task_battery_data_handle);
	xTaskCreate(task_bme280, "task_bme280", 4096, NULL, configMAX_PRIORITIES - 2, &task_bme280_handlr);

	xTaskCreate(task_gsm, "task_gsm", 4096, NULL, configMAX_PRIORITIES - 1, &task_gsm_handler);


	


	









	/* ЗАГАЛЬНИЙ АЛГОРИТМ
	1. Ініціаціалізація.
		1. OLED
		2. BME280 (відображати T, H, P на екрані постійно)
		3. RTC (відображати годину і дату на екрані постійно)
		4. PCA9536 (показати статус на екрані) (Мигнути всіма ледами)
		5. Micro SD card (показати статус на екрані)
		6. Battery (показувати заряд на екрані постійно)
		7. GSM module (воказати статус готовності на екрані) (показувати рівень сигналу на екрані постійно)
		8. NVS

	2. Чекати на дію(команду)	
		1. Команда: Прийом SMS (Включити/виключити логування, вхідний звінок, взнати миттєву геолокацію)
		2. Команда: Прийом SMS (Запустити/Виключити WIFi)
			1. Якщо збережені в память SSOD і password коректні:
				Підключитися до мережі у вигляді STA.
				Запустити HTTP server.
			2. Якщо підключитись до мережі не вдалось.
				Запустити AP.
				Запустити HTTP server для вводу нового SSOD і password.
		2. Коротке утримання кнопки > 1 секунда (Включити Wifi) 
		3. Довге утримання кнопки > 5 секунди (Виключити Wifi)
		а як запустити процес логування кнопкою?????????
	*/
	
	

	//init_http_server_new();

	
	


/*
	Вигляд дисплею
	1.	Рядок час і дата з внутрішнього RTC (його налаштувати через сторінну або через GPS), і заряд батереї.
	2.  Статус роботи, Час роботи (дні, години, хвилини, секунди)
	3. 	Вивести IP сервера ESP32, до якого можна підключитися.
	4.	
	5. 

	CURRENT:
		1. Вивести дату і час на 0 рядок дисплею
		2. 

		4. Переробити на новий веб сервер.
		1. Запускати HTTP server з SMS командою.

	BUGS:


	TASKS:
		4. Зробити реалізацію кнопки старту і стопу запису GPS даних. (кнопка має знати стан GPS логера, чи він включений чи ні) Додати кнопки LogON logOFF і point
		3. Добавити можливість налаштування підключення до вибраного AP.
		3. При скачуванні файла додеється знак "_" перед назвою файла.
		7. Логування даних в консоль. Порифакторити всі LOG_TAGs 
		10. Рефакторити код в main.c, позабирати лишні функції в різні папки і файли
		2. Пофіксити Config файл.
		1. Оптимізувати енергоспоживання дівайсу
		5. Замінити task_blink з задачі на наймер

	DONE:
		2. Некоректно відображаються GPS діні в таблиці на сторінці.
		1. Відсилати реальні GSM а не фейкові.
		3. Під час команди logON дані не записуються в файл --------- SPI FLASH WRITE DATA: File can't create log file: writing data  -------- E (588183) log_data2 FUNCTION: : File can't be write !	(повторно монтую карту памяті, в фіункції логування, якщо вона не змонтована корректно)
		6. Зробити реальну передачу стану заряду акумулятора. (потім реалізувати її як функцію)
		8. Не відображати системні папки і файли в таблиці HTTP сервера.
		1. ПРи включеному сервері рандомно віирубається логування даних в файл. При цьому що файл займає в районі 50000 байт. 
		9. Додавати у відповідь на будь яку командою SMS стан заряду дівайсу. 
		1. Додавати до кожної точки геолокації Саму геолокацію, швидкість і час, які взяти з даних GPS.	
		1. ПРи включеному сервері рандомно віирубається логування даних в файл.	При цьому що файл займає в районі 50000 байт. 
		1 .БАГ нормально не всі ЛОГИ логує в один файл і кожного разу дописує в нього а не створює новий !₴!!!!
			E (163603) sdmmc_cmd: sdmmc_read_sectors_dma: sdmmc_send_cmd returned 0x108
			E (163603) diskio_sdmmc: sdmmc_read_blocks failed (0x108)
		2. Записувати мій номер в конфізі.
		2. Залити проект з VS Code і новою версією плати на ГітХАБ
		3. Зробити інтерфейс запису і стирання даних на дисплей

*/
}
// ------------------------------------------------------------------------------------------------------------------