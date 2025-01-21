#include "nvs_and_spiffs.h"

// --------------------------------------------------------------------------------------------------------------------
void init_ipsffs_memory(void)
{
	static const char *TAG_SPIFFS = "SPIFFS";
	ESP_LOGI(TAG_SPIFFS, "Initializing SPIFFS");

	esp_vfs_spiffs_conf_t conf = {
		.base_path = "/spiffs",
		.partition_label = NULL,
		.max_files = 5,
		.format_if_mount_failed = true
	};

	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			ESP_LOGE(TAG_SPIFFS, "Failed to mount or format filesystem");
		}
		else if (ret == ESP_ERR_NOT_FOUND)
		{
			ESP_LOGE(TAG_SPIFFS, "Failed to find SPIFFS partition");
		}
		else
		{
			ESP_LOGE(TAG_SPIFFS, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
		return;
	}

	size_t total = 0, used = 0;
	ret = esp_spiffs_info(conf.partition_label, &total, &used);
	if (ret != ESP_OK)
	{
		ESP_LOGE(TAG_SPIFFS, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
	}
	else
	{
		ESP_LOGI(TAG_SPIFFS, "Partition size: total: %d, used: %d", total, used);
	}

	// Open HTML file and print  FOR DEBUG
	FILE* f;
	ESP_LOGI(TAG_SPIFFS, "Opening file");
	f = fopen("/spiffs/index.html", "rb");
	if (f == NULL)
	{
		ESP_LOGE(TAG_SPIFFS, "Failed to open file for writing");
		return;
	}

	char str1[500];
	size_t n;
	n = fread(str1, 1, sizeof(str1), f);
	fclose(f);
	str1[n] = 0;

	ESP_LOGI(TAG_SPIFFS, "Read from file: \r\n%s", str1);
	ESP_LOGI(TAG_SPIFFS, "----------------------------------------------------\r\n");
}
 // --------------------------------------------------------------------------------------------------------------------
int read_write_data_into_NVS(const char *key, uint8_t write_or_read, int data) 
{
	static const char *TAG = "SAVE VALUE INTO NVS";
	static uint8_t init = 0;

	esp_err_t ret;
	nvs_handle_t my_handle;

	if(init == 0)
	{
		ESP_LOGI(TAG, "Init NVS memory...");
		ret = nvs_flash_init();

		if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
		{
			ESP_LOGI(TAG, "ERASE NVS memory...");
			ESP_ERROR_CHECK(nvs_flash_erase());
			ret = nvs_flash_init();
		}
		ESP_ERROR_CHECK(ret);
		init = 1;
	}

	if(write_or_read == 1)			// Write variable
	{
		ESP_LOGI(TAG, "Write into NVS...");
		ret = nvs_open("storage", NVS_READWRITE, &my_handle);
		if (ret == ESP_OK)
		{
			ret = nvs_set_i32(my_handle, key, data);
			if (ret == ESP_OK)
			{
				ret = nvs_commit(my_handle);
			}
			nvs_close(my_handle);
		}
		ESP_ERROR_CHECK(ret);
	}
	else if(write_or_read == 2)		// Read variable
	{
		ESP_LOGI(TAG, "Read from NVS...");
		int value = 0;
		ret = nvs_open("storage", NVS_READONLY, &my_handle);
		if (ret == ESP_OK)			// If number of name file was save before, get it.
		{
			ret = nvs_get_i32(my_handle, key, &value);
			nvs_close(my_handle);
			ESP_ERROR_CHECK(ret);
			return value;
		}
		else						// If name of file wasn't save before (Case: If all Flash memory was formated).
		{
			int data = 1;			// First name of file.
			ret = nvs_open("storage", NVS_READWRITE, &my_handle);
			if (ret == ESP_OK)
			{
				ret = nvs_set_i32(my_handle, key, data);
				if (ret == ESP_OK)
				{
					ret = nvs_commit(my_handle);
				}
				nvs_close(my_handle);
				ESP_ERROR_CHECK(ret);
			}
			return data;
		}
	}
	else
	{
		ESP_LOGE(TAG, "Wrong parametr");
	}

	return 0;
}
// --------------------------------------------------------------------------------------------------------------------
int get_file_name(char *name)
{
	static const char *TAG = "GET NEW FILE NAME: ";
	char name_of_file[15] = {0,};
	uint8_t write_file_name = 1;
	uint8_t read_file_name = 2;

	int value = read_write_data_into_NVS("name_of_file", read_file_name, 0);				
	ESP_LOGI(TAG, "Read last file name: %d ", value);

	value = value + 1;
	sprintf(name_of_file, "%d", value);
	ESP_LOGI(TAG, "Current file name: %s ", name_of_file);

	read_write_data_into_NVS("name_of_file", write_file_name, value);				

	sprintf(name ,"%s.txt" , name_of_file);

	ESP_LOGI(TAG, "FULL file name: %s ", name);

	return 0;
}
// --------------------------------------------------------------------------------------------------------------------