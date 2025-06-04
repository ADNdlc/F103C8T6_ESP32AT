/*
 * Timestamp.h
 *
 *  Created on: May 30, 2025
 *      Author: 12114
 */

#ifndef ESP32_AT_TIMESTAMP_H_
#define ESP32_AT_TIMESTAMP_H_

#include "ESP32_WiFi.h"

/*ߜ----------------------------------------ߜ打印信息开关ߜ---------------------------------------ߜ*/
//SetServer:
#define SetServer_SorF  1	//连接成败信息

//GET_Time:
#define GET_Time_ack	1	//期望答复
#define GET_Time_sscanf 1	//sscanf成败
#define GET_Time_Server 1	//SNTP服务器状态

/*ߡ-------------------------------------ߡ打印信息开关ߡ-----------------------------------------ߡ*/


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

uint8_t SetServer(uint8_t Num);
uint8_t GET_Time(uint32_t waittime,uint8_t Num);
uint32_t cst_to_unix(const Time* t);

#endif /* ESP32_AT_TIMESTAMP_H_ */
