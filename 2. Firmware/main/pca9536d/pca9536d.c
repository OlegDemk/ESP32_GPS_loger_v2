#include "pca9536d/pca9536d.h"


typedef struct {
	uint8_t device_address;
	uint8_t register_address;
	uint8_t  *data;
	size_t length;
	bool is_read;  						// true = read, false = write
	SemaphoreHandle_t done_signal;		// Signal of end 
}i2c_request_t;


// -----------------------------------------------------------------------------------------------------
void i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_DISABLE,
        .scl_pullup_en = GPIO_PULLUP_DISABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    i2c_param_config(I2C_MASTER_NUM ,&conf); 
    i2c_driver_install(I2C_MASTER_NUM, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    ESP_LOGI(TAG, "I2C initialized");
}
// -----------------------------------------------------------------------------------------------------
esp_err_t pca9536d_write_register(uint8_t reg, uint8_t value)
{
    ESP_LOGI("BOTTON", "pca9536d_set_port_direction  reg %d,  direction: %d ????????????????????????????????", reg, value);
    // Передаєм адресу
    return i2c_request(PCA9535D_I2C_ADDRESS, reg, &value, 1, false);	    // false = write
}
// -----------------------------------------------------------------------------------------------------
esp_err_t pca9536d_read_register(uint8_t reg, uint8_t *value)          
{
    // Передаєм вказівник
    return i2c_request(PCA9535D_I2C_ADDRESS ,reg, value, 1, true);	        // true = read
}
// -----------------------------------------------------------------------------------------------------
esp_err_t pca9536d_set_port_direction(uint8_t direction)
{
    ESP_LOGI("BOTTON", "pca9536d_set_port_direction   direction: %d ????????????????????????????????", direction);
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

esp_err_t pca9536d_read_config(uint8_t *value) ///////////////////////My
{
    return pca9536d_read_register(PCA9535D_CONFIG, value);
}


// -----------------------------------------------------------------------------------------------------
void set_led(uint8_t LED, uint8_t state)
{
    static bool init_status = false;
    if(init_status == false)
    {
        ESP_ERROR_CHECK(pca9536d_set_port_direction(0x01));         // Init P0 as Input and P1, P2, P3 as output.  
        init_status = true;

        // ESP_ERROR_CHECK(pca9536d_set_port_direction(0x0F));         // Init P0 as Input and P1, P2, P3 as output.
        // init_status = true;
    }
    
    uint8_t led_off[] = {0xF0, 0xF0, 0xF0};
    uint8_t led_on[] = {0xF8, 0xF4, 0xF2};

    if(LED < sizeof(led_off))
    {
        uint8_t output = (state == LED_ON) ? led_on[LED] : led_off[LED];
        ESP_ERROR_CHECK(pca9536d_write_output(output));
    }
    else
    {
        ESP_LOGE("set_led", "Invalid LED specified");
    }

    uint8_t fuckking_button = 0;
    ESP_ERROR_CHECK(pca9536d_read_input(&fuckking_button));

    ESP_LOGI("BOTTON", "Raw input register:  %d ==============================================================================", fuckking_button);
}
// -----------------------------------------------------------------------------------------------------
uint8_t read_key(void)   
{   
    static bool init_status = false;
    uint8_t button_state = 0;
    bool button_status = 0;
    if(init_status == false)
    {
        ESP_ERROR_CHECK(pca9536d_set_port_direction(0x01));         // Init P0 as Input and P1, P2, P3 as output.
        init_status = true;
    }

    uint8_t test_read_config = 0;
    ESP_ERROR_CHECK(pca9536d_read_config(&test_read_config));
    ESP_LOGI("BOTTON", "pca9536d_read_configg: %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!", test_read_config);

    ESP_ERROR_CHECK(pca9536d_read_input(&button_state));

    ESP_LOGI("BOTTON", "Raw input register: 0x%02X", button_state);


    //////////////////////////////////////////////////////////////////////////
    // uint8_t reg_0x00 = 0;
    // ESP_ERROR_CHECK(pca9536d_read_register(PCA9535D_INPUT_PORT, &reg_0x00));
    // ESP_LOGI("BOTTON", "PCA9535D_INPUT_PORT: %d<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<", reg_0x00);

    // ESP_ERROR_CHECK(pca9536d_read_register(PCA9535D_OUTPUT_PORT, &reg_0x00));
    // ESP_LOGI("BOTTON", "PCA9535D_OUTPUT_PORT: %d<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<", reg_0x00);

    // ESP_ERROR_CHECK(pca9536d_read_register(PCA9535D_POLARITY_PORT, &reg_0x00));
    // ESP_LOGI("BOTTON", "PCA9535D_POLARITY_PORT: %d<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<", reg_0x00);

    // ESP_ERROR_CHECK(pca9536d_read_register(PCA9535D_CONFIG, &reg_0x00));
    // ESP_LOGI("BOTTON", "PCA9535D_CONFIG: %d<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<", reg_0x00);

    /////////////////////////////////////////////////////////////////////

    if((button_state & 0x01))    // detect zero bit 0xXXXXXXX1 (button bit)
    {
        //ESP_LOGI("BOTTON", "OFF");
        button_status = false;
    } 
    else
    {
        //ESP_LOGI("BOTTON", "ON");
        button_status = true;
    }

    // if(128 & (button_state << 7))    // detect zero bit 0xXXXXXXX1 (button bit)
    // {
    //     //ESP_LOGI("BOTTON", "OFF");
    //     button_status = false;
    // } 
    // else
    // {
    //     //ESP_LOGI("BOTTON", "ON");
    //     button_status = true;
    // }

    //////////////////////////////////////////////////////////////////////////////////
    // read all registers

    // uint8_t reg_0x00 = 0;
    // uint8_t reg_0x01 = 0;
    // uint8_t reg_0x02 = 0;
    // uint8_t reg_0x03 = 0;

    // pca9536d_read_register(PCA9535D_INPUT_PORT, &reg_0x00);
    // pca9536d_read_register(PCA9535D_OUTPUT_PORT, &reg_0x01);
    // pca9536d_read_register(PCA9535D_POLARITY_PORT, &reg_0x02);
    // pca9536d_read_register(PCA9535D_CONFIG, &reg_0x03); 

    // // i2c_request(PCA9535D_I2C_ADDRESS ,PCA9535D_INPUT_PORT, &reg_0x00, 1, true);
    // // i2c_request(PCA9535D_I2C_ADDRESS ,PCA9535D_OUTPUT_PORT, &reg_0x01, 1, true);
    // // i2c_request(PCA9535D_I2C_ADDRESS ,PCA9535D_POLARITY_PORT, &reg_0x02, 1, true);
    // // i2c_request(PCA9535D_I2C_ADDRESS ,PCA9535D_CONFIG, &reg_0x03, 1, true);

    // ESP_LOGI("READ ALL REGISTERS", "0 PCA9535D_INPUT_PORT: %d", reg_0x00);
    // ESP_LOGI("READ ALL REGISTERS", "1 PCA9535D_OUTPUT_PORT: %d", reg_0x01);
    // ESP_LOGI("READ ALL REGISTERS", "2 PCA9535D_POLARITY_PORT: %d", reg_0x02);
    // ESP_LOGI("READ ALL REGISTERS", "3 PCA9535D_CONFIG: %d", reg_0x03);



    //////////////////////////////////////////////////////////////////////////////////





    return button_status;
}
// -----------------------------------------------------------------------------------------------------
void test_pca9536d(void)
{
    static bool init_pca = false;
    if(init_pca == false)
    {
        i2c_master_init();
    }


	for(int i = 0; i <= 5; i++)
    {
		set_led(RED_LED, LED_ON);
		vTaskDelay(100/portTICK_PERIOD_MS);
		set_led(RED_LED, LED_OFF);
		vTaskDelay(100/portTICK_PERIOD_MS);
	}

	for(int i = 0; i <= 5; i++)
    {
		set_led(GREEN_LED, LED_ON);
		vTaskDelay(100/portTICK_PERIOD_MS);
		set_led(GREEN_LED, LED_OFF);
		vTaskDelay(100/portTICK_PERIOD_MS);
	}

	for(int i = 0; i <= 5; i++)
    {
		set_led(BLUE_LED, LED_ON);
		vTaskDelay(100/portTICK_PERIOD_MS);
		set_led(BLUE_LED, LED_OFF);
		vTaskDelay(100/portTICK_PERIOD_MS);
	}

    // Read button state
	// for(int i = 0; i <= 500; i++)
    // { 
	// 	bool button_state = read_key();
	// 	vTaskDelay(500/portTICK_PERIOD_MS);
		
	// 	ESP_LOGI("BOTTON STATE", "--------------------------------------------------> %d", button_state);
	// }
}
// -----------------------------------------------------------------------------------------------------


