

#include "ssd1306.h"

#define TAG "OLED"

uint8_t display_content[DISPLAY_PAGES][DISPLAY_WIDTH];

// -----------------------------------------------------------------------------------------------------
esp_err_t ssd1306_read_register(uint8_t reg, uint8_t *value)
{
    return i2c_request(SSD1306_ADDR ,reg, value, 1, true, false);
}
// -----------------------------------------------------------------------------------------------------
esp_err_t ssd1306_write_register(uint8_t reg, uint8_t value)
{
    // Передача адреси
    return i2c_request(SSD1306_ADDR, reg, &value, 1, false, false);	    // false = write
}
// -----------------------------------------------------------------------------------------------------
esp_err_t ssd1306_write_data(uint8_t *data, uint8_t length, uint8_t data_mode)          // Write massive of data   
{ 
    // Передача масиву даних
    return i2c_request(SSD1306_ADDR, data_mode, data, length, false, false);
}
// -----------------------------------------------------------------------------------------------------
esp_err_t oled_init(i2c_port_t i2c_port) 
{
        oled_clear();

        uint8_t init_sequence[] = {
                MULTIPLEX_RATIO, 0x3F,
                DISPLAY_OFFSET, 0x00,
                DISPLAY_LINE_START, 0x00,
                SEGMENT_REMAP_INVERSE,
                COM_OUTPUT_SCAN_DIR_REMAP,
                COM_PINS_HARDWARE_CONFIG, 0x12,
                CONTRAST_CONTROL, 0x7F,
                DISABLE_ENTIRE_DISPLAY,
                NORMAL_DISPLAY,
                DISPLAY_CLK_RATIO, 0x80,
                CHARGE_PUMP_SET, 0x14,
                DISPLAY_ON
        };

        // Write data into OLED screen memory
        esp_err_t ret = ssd1306_write_data(init_sequence, sizeof(init_sequence), COMMAND_MODE);
        if (ret != ESP_OK) 
        {
                ESP_LOGE(TAG, "Initialization failed: %s", esp_err_to_name(ret));
                return ret;
        }
        else
        {
                ESP_LOGI(TAG, "Initialization OK");
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);

        return ESP_OK;
}
// -----------------------------------------------------------------------------------------------------
esp_err_t oled_refresh(i2c_port_t i2c_port) 
{       
        esp_err_t ret = ESP_OK;

        for (uint8_t page = 0; page < DISPLAY_PAGES; page++)        // Iterate all pages
        {
                // Set start page 
                uint8_t page_cmd[] = { PAGE_START_ADDR | page };
                ret = ssd1306_write_data(page_cmd, sizeof(page_cmd), COMMAND_MODE);
                if (ret != ESP_OK) return ret;

                // Write page content into OLDE memory (before you have set DATA_MODE)   
                ret = ssd1306_write_data(display_content[page], DISPLAY_WIDTH, DATA_MODE);
                if (ret != ESP_OK) return ret;
        }
        return ret;
}
// -----------------------------------------------------------------------------------------------------
void oled_clear(void) 
{
        memset(display_content, 0, (DISPLAY_WIDTH * DISPLAY_PAGES));
}
// -----------------------------------------------------------------------------------------------------
void oled_print(const char *text, int row, int col, int color) 
{
        col *= FONT_WIDTH;

        for(int i = 0; text[i] != '\0'; i++) {
                uint16_t char_index = (text[i] <= 0) ? 0 : text[i];
                for(uint8_t j = 0; j < FONT_WIDTH; j++){
                        if(color) {
                                display_content[row][col] = FONTS[char_index][j];
                        }
                        else {
                                display_content[row][col] = ~FONTS[char_index][j];
                        }
                        col++;
                }
        }
}
// -----------------------------------------------------------------------------------------------------
void oled_draw_pixel(int x, int y, int color) 
{
        if(color == 1) {
                display_content[y/8][x] |= 1 << (y % 8);
        }
        else {
                display_content[y/8][x] &= ~(1 << (y % 8));
        }
}
// -----------------------------------------------------------------------------------------------------
void oled_draw_horizontal_line(int width, int x, int y, int color) 
{
        for(int i = x; i < x + width; i++) {
                oled_draw_pixel(i, y, color);
        }
}
// -----------------------------------------------------------------------------------------------------
void oled_draw_rectangle(int width, int height, int x, int y, int color, bool fill) 
{
        for(int j = y; j < y + height; j++) {
                for(int i = x; i < x + width; i++) {
                        if((fill == 1) || (j == y || j == y + height - 1 || i == x || i == x + width - 1)) {
                                oled_draw_pixel(i, j, color); 
                        }
                }
        }
}
// -----------------------------------------------------------------------------------------------------
void oled_print_text(char * text, uint8_t row, uint8_t col)
{       
        static bool init = false;

        if(((row < 0) || (row > 6)) || (col < 0) || (col > 16))
        {
                ESP_LOGE(TAG, "Wrong row or col row: %d, col: %d", row, col);
                return;
        }

        if(init == false)
        {       
                ESP_LOGI(TAG,"INIT OLED");
                oled_init(I2C_MASTER_NUM);
                oled_clear();
                init = true;
                oled_refresh(I2C_MASTER_NUM);
        }

        ESP_LOGI(TAG,"OLED print text");

        oled_print(text, row, col, 1);
        oled_refresh(I2C_MASTER_NUM);
}
// -----------------------------------------------------------------------------------------------------
void oled_clean_all(void)
{
        static bool init = false;    

        if(init == false)
        {       
                ESP_LOGI(TAG,"INIT OLED");
                oled_init(I2C_MASTER_NUM);
                oled_clear();
                init = true;
                oled_refresh(I2C_MASTER_NUM);
        }
        oled_clear();
        oled_refresh(I2C_MASTER_NUM);
}
// -----------------------------------------------------------------------------------------------------


