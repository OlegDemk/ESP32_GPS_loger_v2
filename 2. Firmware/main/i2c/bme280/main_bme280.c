
#include "main_bme280.h"

// ---------------------------------------------------------------------------------------
void BME280_delay_msek(u32 msek)
{
	vTaskDelay(msek/portTICK_PERIOD_MS);
}
// ---------------------------------------------------------------------------------------
s8 bme280_i2c_manager_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt) 
{
    for (int i = 0; i < cnt; i++) 
	{
        esp_err_t err = i2c_request(dev_addr, reg_addr + i, &reg_data[i], 1, false, false);
        if (err != ESP_OK) 
		{
            return -1; 
        }
    }
    return 0; 
}
// ---------------------------------------------------------------------------------------
s8 bme280_i2c_manager_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt) 
{
    esp_err_t err = i2c_request(dev_addr, reg_addr, reg_data, cnt, true, false );
    if (err != ESP_OK) 
	{
        return -1; 
    }
    return 0; 
}
// ---------------------------------------------------------------------------------------
void bme280_test_read_thp(bme280_data_t *data)
{
	static const char *TAG_BME280 = "BME280";
	if(data == NULL)
	{
		ESP_LOGE(TAG_BME280, "NULL pointer received !");
	}

	ESP_LOGI(TAG_BME280, "FUNCTION");

	static s32 com_rslt;
	s32 v_uncomp_pressure_s32;
	s32 v_uncomp_temperature_s32;
	s32 v_uncomp_humidity_s32;

	static bool init_status = false;
	static struct bme280_t bme280;

	if(init_status == false)
	{
		ESP_LOGI(TAG_BME280, "Start init");

		bme280.bus_write = bme280_i2c_manager_write;  
		bme280.bus_read = bme280_i2c_manager_read;
		bme280.dev_addr = BME280_I2C_ADDRESS2;
		bme280.delay_msec = BME280_delay_msek;
		com_rslt = bme280_init(&bme280);

		if(com_rslt != SUCCESS)
		{
			ESP_LOGE(TAG_BME280, "Init error. code: %d", com_rslt);
			// vTaskDelay(1000 / portTICK_PERIOD_MS);
			// continue;
		}

		// Setings 
		com_rslt += bme280_set_oversamp_pressure(BME280_OVERSAMP_16X);
		com_rslt += bme280_set_oversamp_temperature(BME280_OVERSAMP_2X);
		com_rslt += bme280_set_oversamp_humidity(BME280_OVERSAMP_1X);
		com_rslt += bme280_set_standby_durn(BME280_STANDBY_TIME_1_MS);
		com_rslt += bme280_set_filter(BME280_FILTER_COEFF_16);
		com_rslt += bme280_set_power_mode(BME280_NORMAL_MODE);

		if(com_rslt != SUCCESS)
		{
			ESP_LOGE(TAG_BME280, "Failed configure BME280. code: %d", com_rslt);
		}
		else
		{
			init_status = true;
		}
	}

	if (com_rslt == SUCCESS)
	{
		com_rslt = bme280_read_uncomp_pressure_temperature_humidity(  
			&v_uncomp_pressure_s32, &v_uncomp_temperature_s32, &v_uncomp_humidity_s32);

		if (com_rslt == SUCCESS)
		{
			// double temperature = bme280_compensate_temperature_double(v_uncomp_temperature_s32);
			// double pressure = bme280_compensate_pressure_double(v_uncomp_pressure_s32)/100;
			// double humidity = bme280_compensate_humidity_double(v_uncomp_humidity_s32);
			// ESP_LOGI(TAG_BME280, "%.2f degC / %.3f hPa / %.3f %%", temperature, pressure, humidity);

			data->temperature = bme280_compensate_temperature_double(v_uncomp_temperature_s32);
			data->pressure = bme280_compensate_pressure_double(v_uncomp_pressure_s32)/100;
			data->humidity = bme280_compensate_humidity_double(v_uncomp_humidity_s32);
			ESP_LOGI(TAG_BME280, "%.2f degC / %.3f hPa / %.3f %%", data->temperature, data->pressure, data->humidity);
		}
		else
		{
			ESP_LOGE(TAG_BME280, "measure error. code: %d", com_rslt);
		}	
	}
	else
	{
		ESP_LOGE(TAG_BME280, "init or setting error. code: %d", com_rslt);
	}
}
// ---------------------------------------------------------------------------------------