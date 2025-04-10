

#ifndef MAIN_SSD1366_H_
#define MAIN_SSD1366_H_


#include "../../main.h"
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "font.h"

#define DISPLAY_WIDTH       128
#define DISPLAY_HEIGHT      64
#define DISPLAY_PAGES       8
#define DISPLAY_COLS        DISPLAY_WIDTH/FONT_WIDTH
#define DISPLAY_ROWS        DISPLAY_HEIGHT/DISPLAY_PAGES
#define DISPLAY_TIMEOUT_MS  500     

#define SSD1306_ADDR        0x3C

// Control bytes
#define COMMAND_MODE 0x00
#define SINGLE_COMMAND_MODE 0x80
#define DATA_MODE 0x40

// Fundamental Command
#define DISPLAY_LINE_START 0x40
#define CONTRAST_CONTROL 0x81
#define DISABLE_ENTIRE_DISPLAY 0xA4
#define DISPLAY_ON_REG02 0xA5
#define NORMAL_DISPLAY 0xA6
#define DISPLAY_INVERSE  0xA7
#define DISPLAY_RESET 0xAE
#define DISPLAY_ON 0xAF

// Addressing Setting
#define MEM_ADDR_MODE 0x20
#define LOWER_COL_START_ADDR 0x00
#define HIGHER_COL_START_ADDR 0x10
#define PAGE_START_ADDR 0xB0

// Scrolling
#define DEACTIVATE_SCROLL 0x2E
#define ACTIVATE_SCROLL 0x2F
#define VERTICAL_AND_RIGHT_HOR_SCROLL 0x29
#define DUMMY_BYTE 0x00
#define SIX_FRAMES_PER_SEC 0x00
#define VERTICAL_OFFSET_ONE 0x01

// Hardware Config
#define DISPLAY_START_LINE 0x40
#define SEGMENT_REMAP_NORMAL 0xA0
#define SEGMENT_REMAP_INVERSE 0xA1
#define MULTIPLEX_RATIO 0xA8
#define COM_OUTPUT_SCAN_DIR_NORMAL 0xC0
#define COM_OUTPUT_SCAN_DIR_REMAP 0xC8
#define DISPLAY_OFFSET 0xD3
#define COM_PINS_HARDWARE_CONFIG 0xDA

// Timing & Driving Scheme
#define DISPLAY_CLK_RATIO 0xD5
#define PRE_CHARGE_PER 0xD9
#define VCOMH_DESELECT_LEVEL 0xDB
#define NOOPERATION 0xE3

// Charge Pump
#define CHARGE_PUMP_SET 0x8D

#define ALIGN_CENTER(width_display, obj_size)  (width_display/2 - obj_size/2)
#define ALIGN_RIGHT(width_display, obj_size)   (width_display - obj_size)
#define ALIGN_LEFT(width_display, obj_size)    (0)

esp_err_t oled_init(i2c_port_t i2c_port);
esp_err_t oled_refresh(i2c_port_t i2c_port);

void oled_clear(void);
void oled_print(const char *text, int row, int col, int color);
void oled_draw_pixel(int x, int y, int color);
void oled_draw_horizontal_line(int width, int x, int y, int color);
void oled_draw_rectangle(int width, int height, int x, int y, int color, bool fill);

esp_err_t ssd1306_read_register(uint8_t reg, uint8_t *value);
void oled_print_text(char * text, uint8_t row, uint8_t col);
void oled_test(void);
void oled_clean_all(void);


#endif /* MAIN_SSD1366_H_ */

