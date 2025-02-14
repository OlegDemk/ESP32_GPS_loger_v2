
#include "pca9536d.h"

#define DEBUG OFF

#define TAG "PCA9536D_DRIVER"

// -----------------------------------------------------------------------------------------------------
esp_err_t pca9536d_write_register(uint8_t reg, uint8_t value)
{
    // Передаєм адресу
    return i2c_request(PCA9535D_I2C_ADDRESS, reg, &value, 1, false, false);	    // false = write
}
// -----------------------------------------------------------------------------------------------------
esp_err_t pca9536d_read_register(uint8_t reg, uint8_t *value)          
{
    // Передаєм вказівник
    return i2c_request(PCA9535D_I2C_ADDRESS ,reg, value, 1, true, false);	        // true = read
}
// -----------------------------------------------------------------------------------------------------
esp_err_t pca9536d_set_port_direction(uint8_t direction)
{
    return pca9536d_write_register(PCA9535D_CONFIG, direction);          // direction: 1 - input, 0 - output
}
// -----------------------------------------------------------------------------------------------------
esp_err_t pca9536d_write_output(uint8_t value)
{
    return pca9536d_write_register(PCA9535D_OUTPUT_PORT, value);
}
// -----------------------------------------------------------------------------------------------------
esp_err_t pca9536d_read_input(uint8_t *value)
{
    return pca9536d_read_register(PCA9535D_INPUT_PORT, value);
}
// -----------------------------------------------------------------------------------------------------
esp_err_t pca9536d_read_config(uint8_t *value) 
{
    return pca9536d_read_register(PCA9535D_CONFIG, value);
}
// -----------------------------------------------------------------------------------------------------
void set_led(uint8_t LED, uint8_t state)
{
    // First init
    static bool init_status = false;
    if(init_status == false)
    {
        // Init putput and input PINs
        ESP_ERROR_CHECK(pca9536d_set_port_direction(0xF1));         // Init P0 as Input and P1, P2, P3 as output.  
        init_status = true;

        // Turn off all LEDs by default
        ESP_ERROR_CHECK(pca9536d_write_output(0xF1));
    }
    // Read all LEDs state (for create bit mask)
    uint8_t read_data = 0;
    ESP_ERROR_CHECK(pca9536d_read_register(PCA9535D_OUTPUT_PORT, &read_data));
    #if DEBUG == ON 
        ESP_LOGI(TAG, "PCA9535D_CONFIG: %d  <<<<<", read_data);
    #endif

    // Make LEDs mask
    uint8_t current_output = read_data;
    if(state == LED_ON)
    {
        current_output |= (1 << (LED + 1));            // Set bite 1
        #if DEBUG == ON
            ESP_LOGI(TAG, "LED %d ON, new_output: %d", LED, current_output);
        #endif
    }
    else
    {
        current_output &= ~(1 << (LED + 1));            // Set bite 0
        #if DEBUG == ON
            ESP_LOGI(TAG, "LED %d OFF, new_output: %d", LED, current_output);
        #endif
    }

    ESP_ERROR_CHECK(pca9536d_write_output(current_output));
}
// -----------------------------------------------------------------------------------------------------
uint8_t read_key(void)   
{   
    static bool init_status = false;
    uint8_t button_state = 0;
    bool button_status = 0;

    if(init_status == false)
    {
        #if DEBUG == ON
            ESP_LOGI(TAG, "INIT READ KEY...");
        #endif
        ESP_ERROR_CHECK(pca9536d_set_port_direction(0xF1));         // Init P0 as Input and P1, P2, P3 as output.
        init_status = true;
    }

    #if DEBUG == ON
        ESP_LOGI(TAG, "READ KEY...");
    #endif

    ESP_ERROR_CHECK(pca9536d_read_input(&button_state));
    #if DEBUG == ON
        ESP_LOGI(TAG, "Raw input register: 0x%02X", button_state);
    #endif

    if((button_state & 0x01))               // detect zero bit 0xXXXXXXX1 (button bit)
    {
        return false;
    } 
    else
    {
        return true;
    }
}
// -----------------------------------------------------------------------------------------------------
void test_pca9536d(void)
{
    ESP_LOGI(TAG, "RED");
    set_led(RED_LED, LED_ON);    
    vTaskDelay(200/portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "GREEN");
    set_led(GREEN_LED, LED_ON);
    vTaskDelay(200/portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "BLUE");
    set_led(BLUE_LED, LED_ON);
    vTaskDelay(200/portTICK_PERIOD_MS);

    set_led(RED_LED, LED_OFF);
    set_led(GREEN_LED, LED_OFF);
    set_led(BLUE_LED, LED_OFF);
    vTaskDelay(100/portTICK_PERIOD_MS);

    // Read button state
	bool button_state = read_key(); 
    if(button_state == 0)
    {
        ESP_LOGI(TAG, "OFF");
    }
    else
    {
        ESP_LOGI(TAG, "ON");
    }
}
// -----------------------------------------------------------------------------------------------------
void make_led_blink(int led, int delay, int count_repeats)
{
    if(led >= 0 && led <= 2)
    {
        for(int i = 0; i < count_repeats; i++)
        {
            set_led(led, LED_ON);
            vTaskDelay(delay/portTICK_PERIOD_MS);
            set_led(led, LED_OFF);
            vTaskDelay(delay/portTICK_PERIOD_MS);
        }   
    }
    else
    {
        ESP_LOGE(TAG, "Wrong LED parametr");
    }
}
// -----------------------------------------------------------------------------------------------------

