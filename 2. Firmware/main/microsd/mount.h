/*
 * mount.h
 *
 *  Created on: Jan 30, 2024
 *      Author: odemki
 */

#ifndef MAIN_MICROSD_MOUNT_H_
#define MAIN_MICROSD_MOUNT_H_

#include "../main.h"

#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_vfs_fat.h"
#include "esp_spiffs.h"
#include "sdkconfig.h"
#include "soc/soc_caps.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#if SOC_SDMMC_HOST_SUPPORTED
#include "driver/sdmmc_host.h"
#endif
#include "sdmmc_cmd.h"
#include "../gps_data.h"

esp_err_t example_mount_storage(const char *base_path);
uint8_t log_data(char* base_path, char* file_nmae, char* data, char* log_info);

uint8_t log_data1(char* base_path, char* file_nmae, gps_data_gps_t *gps_data);
uint8_t log_data2(char* base_path, char* file_nmae, gps_data_gps_t *gps_data, int gps_point_counter);

uint8_t log_data_into_micro_sd(int name_of_file, float latitude, float longitude);

void task_get_gps_data_one_time(void* ignode);

#endif /* MAIN_MICROSD_MOUNT_H_ */
