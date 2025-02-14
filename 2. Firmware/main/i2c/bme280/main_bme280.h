/*
 * main_bme280.h
 *
 *  Created on: Dec 25, 2023
 *      Author: odemki
 */

#ifndef MAIN_BME280_MAIN_BME280_H_
#define MAIN_BME280_MAIN_BME280_H_

#include "../../main.h"
#include "bme280.h"

typedef struct{
	double temperature;
	double humidity;
	double pressure;
} bme280_data_t;


void BME280_delay_msek(u32 msek);
s8 bme280_i2c_manager_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);
s8 bme280_i2c_manager_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt);

void bme280_test_read_thp(bme280_data_t *data);

#endif /* MAIN_BME280_MAIN_BME280_H_ */
