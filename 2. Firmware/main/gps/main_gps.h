/*
 * main_gps.h
 *
 *  Created on: Dec 25, 2023
 *      Author: odemki
 */

#ifndef MAIN_GPS_MAIN_GPS_H_
#define MAIN_GPS_MAIN_GPS_H_

#include "../main.h"
#include "../gps_data.h"

void turn_on_gps(void);
void turn_off_gps(void);
void gps_log_task(void *arg);

void init_on_off_gps_gpio(void);
void turn_on_gps_module(void);
void turn_off_gps_module(void);
int check_gps_data_valid(gps_data_gps_t *gps_data);
void show_gps_data(gps_data_gps_t *gps_data);
void send_data_to_client(gps_data_gps_t *gps_data);
void gps_log_off(void);
void send_one_point_gps_data(void);




#endif /* MAIN_GPS_MAIN_GPS_H_ */
