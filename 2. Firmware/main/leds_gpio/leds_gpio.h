/*
 * gpio.h
 *
 *  Created on: Dec 28, 2023
 *      Author: odemki
 */

#ifndef MAIN_LEDS_GPIO_LEDS_GPIO_H_
#define MAIN_LEDS_GPIO_LEDS_GPIO_H_

#include "../main.h"
#include "gps_data.h"

void init_output_gpio(void);
void RGB_LEDs_blink(int times, int delay);
void make_blink(int color, int delay_ms, int times);
void gps_signal_led_indication(gps_data_gps_t *gps_data);

#endif /* MAIN_LEDS_GPIO_LEDS_GPIO_H_ */
