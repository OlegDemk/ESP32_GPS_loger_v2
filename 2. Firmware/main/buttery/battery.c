#include "buttery.h"

#define ADC_PIN ADC1_CHANNEL_7	         // Pin 35
#define DEFAULT_VREF 1100

#define VOLTAGE_DIVIDER_R_1 36           // 36 kOm          Top resistor
#define VOLTAGE_DIVIDER_R_2 100          // 100 kOm         Bottom resistor
#define VOLTAGE_DRIVER_FACTOR (VOLTAGE_DIVIDER_R_1+VOLTAGE_DIVIDER_R_2)/VOLTAGE_DIVIDER_R_2
static esp_adc_cal_characteristics_t *adc_chars;

// ------------------------------------------------------------------------------------------------------------
void init_adc(void)
{
	// Налаштування ширини і роздільної здатності ADC
    adc1_config_width(ADC_WIDTH_BIT_12);  						 			// Роздільна здатність 12 біт
    adc1_config_channel_atten(ADC_PIN, ADC_ATTEN_DB_11);  					// Атенюація для діапазону 0-3.3 В

    // Характеристики калибровки
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
}
// ------------------------------------------------------------------------------------------------------------
uint8_t raad_barrety_level(void)
{
	float battery_voltage = 0;

	// Init ADC only onne time
	static bool init = false;
	if(init == false)
	{
		init_adc();
		init = true;
	}

    if (adc_chars == NULL)
    {
        ESP_LOGE("ADC", "ADC calibration characteristics not initialized!");
        return -1;  															// Повертає помилку, якщо не ініціалізовано
    }

	int raw_value = adc1_get_raw(ADC_PIN);										// Зчитування даних із ADC
    uint32_t voltage = esp_adc_cal_raw_to_voltage(raw_value, adc_chars);		// Перетворення в мілівольти

	// Перетворення в вольти
    battery_voltage = (voltage / 1000.0) * VOLTAGE_DRIVER_FACTOR;                          // повертає значення в Вольти

	// Розрахунок відсотків заряду
	uint8_t battery_level = (((battery_voltage - 2.7)/(4.2 - 2.7)))*100;
	if(battery_level > 100)
	{
		battery_level = 100;
	}

	ESP_LOGI("ADC", "Battery voltage: %0.1f, Battery level: %d ", battery_voltage, battery_level);

	return battery_level;
}
// ------------------------------------------------------------------------------------------------------------