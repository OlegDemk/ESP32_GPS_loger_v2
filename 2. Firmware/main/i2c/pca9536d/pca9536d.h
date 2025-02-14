#pragma once

#include "../../main.h"


// #define TAG "PCA9536D_DRIVER"


#define PCA9535D_I2C_ADDRESS        0x41

#define PCA9535D_INPUT_PORT         0x00
#define PCA9535D_OUTPUT_PORT        0x01
#define PCA9535D_POLARITY_PORT      0x02
#define PCA9535D_CONFIG             0x03

#define LED_ON 1
#define LED_OFF 0

#define BLUE_LED 0 
#define GREEN_LED 1
#define RED_LED 2

esp_err_t pca9536d_write_output(uint8_t value);
esp_err_t pca9536d_read_input(uint8_t *value);
esp_err_t pca9536d_read_config(uint8_t *value);

void set_led(uint8_t LED, uint8_t state);
uint8_t read_key(void);
void test_pca9536d(void);
void make_led_blink(int led, int delay, int count_repeats);