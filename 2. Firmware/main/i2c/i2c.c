#include "i2c.h"

QueueHandle_t i2c_queue;

#define DEBUG OFF

typedef struct{
	uint8_t addres;
	const char *name;
}i2c_device_t;

i2c_device_t i2c_devices[] = {
	{0x77, "BME280 T, H ant P sensor"},
	{0x68, "DC3231 Real-Time Clock"},
	{0x3C, "SSD1306 OLED"},
	{0x41, "PCA9536D LEDs and button"},
};
const int i2c_device_connt = sizeof(i2c_devices) / sizeof(i2c_devices[0]);


// -----------------------------------------------------------------------------------------------------
void i2c_master_init(void)
{
	esp_err_t err;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM ,&conf); 
    err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
	if(err != ESP_OK)
	{
		ESP_LOGE("I2C init", "I2C DOESN't initialized");
	}
	else
	{
		ESP_LOGI("I2C init", "I2C initialized");
	}
}
// -----------------------------------------------------------------------------------------------------
esp_err_t i2c_request(uint8_t device_address, uint8_t register_address, uint8_t *data, size_t length, bool is_read, bool is_scaner_request)
{
	if (is_read && (data == NULL))
	{
    	ESP_LOGE("i2c_request", "Invalid data pointer for read");
    	return ESP_FAIL;
	}
	/*
		Для запису в регістр передається просто змінна, для читання передається вказівник на змінну 
		куда буде записане значення читання з регістра 
	*/

	i2c_request_t request = {
		.device_address = device_address,
		.register_address = register_address,
		.data = data,							 
		.length = length,
		.is_read = is_read,
		.scaner_request = is_scaner_request,
		.done_signal = xSemaphoreCreateBinary()
	};

	#if DEBUG == ON
		if(request.is_read == true) {ESP_LOGI("i2c_request", "READ REGISTER <<<"); }
		else {ESP_LOGI("i2c_request", "WRITE REGISTER <<<");	}

		ESP_LOGI("i2c_request", "request.device_address: %d", request.device_address);   
		ESP_LOGI("i2c_request", "request.register_address: %d", request.register_address); 
		if(request.length > 1)
		{
			ESP_LOGI("i2c_request", "Struct start...");
			for(int i = 0; i < request.length; i++)
			{
				ESP_LOGI("i2c_request", "request.data[%d]:  %d", i ,request.data[i]);	
			}
			ESP_LOGI("i2c_request", "Struct sstop");
			vTaskDelay(100/ portTICK_PERIOD_MS); 
		}
		else
		{
			ESP_LOGI("i2c_request", "request.data: %d", *request.data);
		}
		ESP_LOGI("i2c_request", "request.lenght: %d", request.length); 
		ESP_LOGI("i2c_request", "request.is_read: %d", request.is_read); 
	#endif

	if(xQueueSend(i2c_queue, &request, portMAX_DELAY) == pdPASS)
	{
		xSemaphoreTake(request.done_signal, portMAX_DELAY);
		vSemaphoreDelete(request.done_signal); 
		return ESP_OK;
	}
	else
	{
		return ESP_FAIL;
	}
}
// -----------------------------------------------------------------------------------------------------
void i2c_manager_task(void *pvParameters)
{
	i2c_request_t request;

	while(1)
	{
		if(xQueueReceive(i2c_queue, &request, portMAX_DELAY) == pdPASS)
		{
			#if DEBUG == ON
				ESP_LOGI("i2c_manager_task", "Processing I2C request...");
				ESP_LOGI("i2c_manager_task", "STRUCT START ------------------------------------------------------------");
				ESP_LOGI("i2c_manager_task", "STRUCT: device_address: %d", request.device_address);
				ESP_LOGI("i2c_manager_task", "STRUCT: register_address: %d", request.register_address);
				if(request.length > 1)
				{
					ESP_LOGI("i2c_request", "Struct start...");
					for(int i = 0; i < request.length; i++)
					{
						ESP_LOGI("i2c_request", "request.data[%d]:  %d", i ,request.data[i]);	
					}
					ESP_LOGI("i2c_request", "Struct sstop");
					vTaskDelay(100/ portTICK_PERIOD_MS); 
				}
				else
				{
					ESP_LOGI("i2c_manager_task", "STRUCT: request.data: %d", *request.data);
				}
				ESP_LOGI("i2c_manager_task", "STRUCT: request.length: %d", request.length);
				ESP_LOGI("i2c_manager_task", "STRUCT: request.is_read: %d", request.is_read);
				ESP_LOGI("i2c_manager_task", "STRUCT STOP ------------------------------------------------------------");
			#endif

			esp_err_t err;

			if(request.is_read == true)
			{
				#if DEBUG == ON
					ESP_LOGI("i2c_manager_task", "Read register");
				#endif
				i2c_cmd_handle_t cmd = i2c_cmd_link_create();
				
				i2c_master_start(cmd);
				i2c_master_write_byte(cmd, (request.device_address << 1) | I2C_MASTER_WRITE, true);
				i2c_master_write_byte(cmd, request.register_address, true);
				i2c_master_start(cmd);
				i2c_master_write_byte(cmd, (request.device_address << 1) | I2C_MASTER_READ, true);
				i2c_master_read(cmd, request.data, request.length, I2C_MASTER_LAST_NACK);
				i2c_master_stop(cmd);
			
				err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
				i2c_cmd_link_delete(cmd);
				
				if ((err != ESP_OK) && !request.scaner_request)			// Dont print error if It scaner i2c bus
				{
					ESP_LOGE("I2C_REQUEST", "Error in I2C operation: %s", esp_err_to_name(err));
				}
				#if DEBUG == ON
					ESP_LOGI("READ REGISTER DEBUG ", "Register: %d, Data:%d", request.register_address, *request.data);
				#endif
			}
			else
			{
				#if DEBUG == ON
					ESP_LOGI("i2c_manager_task", "Write register");
					ESP_LOGI("WRITE REGISTER DEBUG ", "Register: %d, Data%d", request.register_address, *request.data);
				#endif

				i2c_cmd_handle_t cmd = i2c_cmd_link_create();
			
				// Формуємо команду для запису
				i2c_master_start(cmd);
				i2c_master_write_byte(cmd, (request.device_address << 1) | I2C_MASTER_WRITE, true);
				i2c_master_write_byte(cmd, request.register_address, true);
				i2c_master_write(cmd, request.data, request.length, true);
				i2c_master_stop(cmd);
				
				err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_PERIOD_MS);
				i2c_cmd_link_delete(cmd);
				
				if (err != ESP_OK) 
				{
					ESP_LOGE("I2C_REQUEST", "Error in I2C operation: %s", esp_err_to_name(err));
				}
			}

			xSemaphoreGive(request.done_signal);
		}
	}
}
// -----------------------------------------------------------------------------------------------------
void i2c_staner(void) 
{
	static const char *TAG_I2C = "I2C_ADDR_SCAN";
	ESP_LOGI(TAG_I2C, "====================  I2C scaner start ====================");

	for(int addr = 1; addr < 127; addr++)
	{
		uint8_t dummy_data = 0;
		esp_err_t err = i2c_request(addr, 0x00, &dummy_data, 1, true, true);
	
		if(err == ESP_OK)
		{
			char *device_name = "Unknown device";
			// ESP_LOGI(TAG_I2C, "Device found at 0x%02X", addr);		// Show all found addersses
			for(int i = 0; i < i2c_device_connt; i++)
			{
				if(i2c_devices[i].addres == addr)
				{
					device_name = i2c_devices[i].name;
					ESP_LOGI(TAG_I2C, "I2C device found.  Adderss: 0x%02X, 		name: %s", addr, device_name);
				}
			}
		}
	}
	ESP_LOGI(TAG_I2C, "====================  I2C scaner finish  ====================");
}
// -----------------------------------------------------------------------------------------------------











