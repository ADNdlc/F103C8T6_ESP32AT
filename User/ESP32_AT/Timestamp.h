/*
 * Timestamp.h
 *
 *  Created on: May 30, 2025
 *      Author: 12114
 */

#ifndef ESP32_AT_TIMESTAMP_H_
#define ESP32_AT_TIMESTAMP_H_

#include "ESP32_WiFi.h"


/*==================================================时间戳服务器========================================================*/

#define SNTPServer  "\"cn.ntp.org.cn\""
#define timezone	8		//时区


typedef struct {
    uint16_t year;
    uint8_t month_n;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t week_n;

    uint8_t ServerON;

    struct {
        char month[4];
        char week[4];
    } str_info;
} Time;

extern  Time ESP_time;

void SetServer(uint8_t Num);
void GET_Time(uint32_t waittime,uint8_t Num);
uint32_t cst_to_unix(const Time* t);

#endif /* ESP32_AT_TIMESTAMP_H_ */
