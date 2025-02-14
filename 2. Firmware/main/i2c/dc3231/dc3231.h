#pragma once

#include "../../main.h"



#define DS3231M_ADDRESS         0x68

#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

#define REG_SECONDS             0x00
#define REG_MINUTES             0x01
#define REG_HOURS               0x02
#define REG_DAY                 0x03
#define REG_DATE                0x04
#define REG_MONTH               0x05
#define REG_YEAR                0x06

static const char *TAG_RTC = "RTC_DS3231M_DRIVER";


esp_err_t i2c_master_init_rtc(void);

void test_rtc_ds3231m(void);