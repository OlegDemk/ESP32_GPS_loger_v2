
#ifndef GPS_DATA_H
#define GPS_DATA_H

#include <stdint.h>

typedef struct {
    uint8_t hour;      /*!< Hour */
    uint8_t minute;    /*!< Minute */
    uint8_t second;    /*!< Second */
    uint16_t thousand; /*!< Thousand */
} time_gps_t;

typedef struct {
    uint8_t day;   /*!< Day (start from 1) */
    uint8_t month; /*!< Month (start from 1) */
    uint16_t year; /*!< Year (start from 2000) */
} date_gps_t;

typedef struct {
    float latitude;
    float longitude;
    float altitude;
    float speed;
    time_gps_t time;
    date_gps_t date;
    uint8_t sats_in_view;
} gps_data_gps_t;

#endif // GPS_DATA_H