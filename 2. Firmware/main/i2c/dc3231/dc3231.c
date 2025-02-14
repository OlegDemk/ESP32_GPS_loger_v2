
#include "dc3231.h"


// -----------------------------------------------------------------------------------------------------
esp_err_t ds3231m_write_register(uint8_t reg, uint8_t value)
{
    return i2c_request(DS3231M_ADDRESS, reg, &value, 1, false, false);	    // false = write
}
// -----------------------------------------------------------------------------------------------------
esp_err_t ds3231m_read_register(uint8_t reg, uint8_t *value)
{
    return i2c_request(DS3231M_ADDRESS ,reg, value, 1, true, false);
}
// -----------------------------------------------------------------------------------------------------
esp_err_t bcd_to_dec(uint8_t bcd)
{
    return ((bcd >> 4)*10) + (bcd & 0x0F);
}
// -----------------------------------------------------------------------------------------------------
esp_err_t dec_to_bcd(uint8_t dec)
{
    return ((dec / 10) << 4) | (dec % 10);
}
// -----------------------------------------------------------------------------------------------------
esp_err_t ds3231m_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    ESP_ERROR_CHECK(ds3231m_write_register(REG_SECONDS, dec_to_bcd(seconds)));
    ESP_ERROR_CHECK(ds3231m_write_register(REG_MINUTES, dec_to_bcd(minutes)));
    ESP_ERROR_CHECK(ds3231m_write_register(REG_HOURS, dec_to_bcd(hours)));

    return ESP_OK;
}
// -----------------------------------------------------------------------------------------------------
esp_err_t ds3231m_set_date(uint8_t day, uint8_t date, uint8_t month, uint8_t year)
{
    ESP_ERROR_CHECK(ds3231m_write_register(REG_DAY, dec_to_bcd(day)));
    ESP_ERROR_CHECK(ds3231m_write_register(REG_DATE, dec_to_bcd(date)));
    ESP_ERROR_CHECK(ds3231m_write_register(REG_MONTH, dec_to_bcd(month)));
    ESP_ERROR_CHECK(ds3231m_write_register(REG_YEAR, dec_to_bcd(year)));

    return ESP_OK;
}
// -----------------------------------------------------------------------------------------------------
esp_err_t ds3231m_get_time(uint8_t *hours, uint8_t *minutes, uint8_t *seconds)
{
    uint8_t raw_seconds, raw_minutes, raw_hours;

    ESP_ERROR_CHECK(ds3231m_read_register(REG_SECONDS, &raw_seconds));
    ESP_ERROR_CHECK(ds3231m_read_register(REG_MINUTES, &raw_minutes));
    ESP_ERROR_CHECK(ds3231m_read_register(REG_HOURS, &raw_hours));

    *seconds = bcd_to_dec(raw_seconds);
    *minutes = bcd_to_dec(raw_minutes);
    *hours = bcd_to_dec(raw_hours);

    return ESP_OK;
}
// -----------------------------------------------------------------------------------------------------
esp_err_t ds3231m_get_date(uint8_t *day, uint8_t *date, uint8_t *month, uint8_t *year)
{
    uint8_t raw_day, raw_date, raw_month, raw_year;

    ESP_ERROR_CHECK(ds3231m_read_register(REG_DAY, &raw_day));
    ESP_ERROR_CHECK(ds3231m_read_register(REG_DATE, &raw_date));
    ESP_ERROR_CHECK(ds3231m_read_register(REG_MONTH, &raw_month));
    ESP_ERROR_CHECK(ds3231m_read_register(REG_YEAR, &raw_year));

    *day = bcd_to_dec(raw_day);
    *date = bcd_to_dec(raw_date);
    *month = bcd_to_dec(raw_month);
    *year = bcd_to_dec(raw_year);

    return ESP_OK;
}
// -----------------------------------------------------------------------------------------------------
void test_rtc_ds3231m(void)
{
    static bool init_rtc = false;
    if(init_rtc == false)
    {
        ESP_LOGI(TAG_RTC, "Set test time 11 12 13");
        ds3231m_set_time(11, 12, 13);
        ESP_LOGI(TAG_RTC, "Set test date 1 10 10 10");
        ds3231m_set_date(1, 10, 10, 10);
        init_rtc = true;
    }
    vTaskDelay(10/portTICK_PERIOD_MS);

    uint8_t houts, minutes, seconds;
    uint8_t day, date, month, year;

    ds3231m_get_time(&houts, &minutes, &seconds);
    ds3231m_get_date(&day, &date, &month, &year);
    ESP_LOGI(TAG_RTC, "TIME:   %d:%d:%d", houts, minutes, seconds);
    ESP_LOGI(TAG_RTC, "DATE:   %d %d:%d:%d",  day, date, month, year);
    vTaskDelay(1000/portTICK_PERIOD_MS);
}
// -----------------------------------------------------------------------------------------------------










