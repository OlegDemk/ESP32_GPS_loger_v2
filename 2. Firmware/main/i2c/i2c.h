#pragma once

#include "../main.h"

#define I2C_MASTER_SCL_IO   22
#define I2C_MASTER_SDA_IO   21
#define I2C_MASTER_FREQ_HZ  400000		// 100000
#define I2C_MASTER_NUM      I2C_NUM_1   // was I2C_NUM_0 
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

typedef struct {
	uint8_t device_address;
	uint8_t register_address;
	uint8_t  *data;
	size_t length;
	bool is_read;  						// true = read, false = write
	bool scaner_request;
	SemaphoreHandle_t done_signal;		// Signal of end 
}i2c_request_t;

#define I2C_QUEUE_ITEM_SIZE sizeof(i2c_request_t)
#define I2C_QUEUE_LENGTH 40

void i2c_master_init(void);
esp_err_t i2c_request(uint8_t device_address, uint8_t register_address, uint8_t *data, size_t length, bool is_resd, bool is_scaner_request);
void i2c_manager_task(void *pvParameters);
void i2c_staner(void);


