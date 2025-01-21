
#include "../main.h"

#define TAG "PCA9536D_DRIVER"

#define I2C_MASTER_SCL_IO   22
#define I2C_MASTER_SDA_IO   21
#define I2C_MASTER_FREQ_HZ  100000
#define I2C_MASTER_NUM      I2C_NUM_0
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MASTER_RX_BUF_DISABLE 0

#define PCA9535D_I2C_ADDRESS        0x41

#define PCA9535D_INPUT_PORT         0x00
#define PCA9535D_OUTPUT_PORT        0x01
#define PCA9535D_POLARITY_PORT      0x02
#define PCA9535D_CONFIG             0x03

#define LED_ON 1
#define LED_OFF 0

#define RED_LED 0
#define GREEN_LED 1
#define BLUE_LED 2

void set_led(uint8_t LED, uint8_t state);
uint8_t read_key(void);
void test_pca9536d(void);