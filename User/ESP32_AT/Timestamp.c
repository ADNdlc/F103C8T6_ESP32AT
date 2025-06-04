/*
 * Timestamp.c
 *
 *  Created on: May 30, 2025
 *      Author: 12114
 */

#include "Timestamp.h"

// 星期缩写数组（索引0不使用，1-7对应周一到周日），这个根据模块返回信息写的
const char* week_buffer[] = {"", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

// 月份缩写数组（索引1-12对应1-12月）
const char* month_buffer[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// 月份天数表 [0-11]，用于查表计算
static const uint8_t days_in_month[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, // 平年
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}  // 闰年
};

Time ESP_time = {
	.year = 0,
	.month_n = 0,
	.day = 0,
	.hour = 0,
	.min = 0,
	.sec = 0,
	.week_n = 0,
	.str_info.week = {0},
	.str_info.month = {0},
	.ServerON = 0
};

/* @brief	将字符串转换为月份和星期
 *
 */
void Time_transform(void){
	for(uint8_t i=1; i<13; i++){
		if(0==strcmp((ESP_time.str_info.month),month_buffer[i])){
			ESP_time.month_n = i;
			break;
		}
	}
	for(uint8_t i=1; i<8; i++){
		if(0==strcmp((ESP_time.str_info.week),week_buffer[i])){
			ESP_time.week_n = i;
			break;
		}
	}
}

/* @brief	设置使用哪个SNTP服务器
 *
 */
uint8_t SetServer(uint8_t Num){

	do{
		//设置SNTP服务器
		Clear_loopbuffer(500);
		if(ESP32_SendANDCheck(3000,"+TIME_UPDATED","AT+CIPSNTPCFG=1,%d,%s",timezone,SNTPServer)){
#if(SetServer_SorF == 1)
			printf("\r\nSNTPServer:Fail retry..");
#endif
			Num--;
			continue;//重试
		}
		else{
			ESP_time.ServerON = 1;
#if(SetServer_SorF == 1)
			printf("\r\nSNTPServer:Success");//成功返回0
#endif
			return 0;
		}
	}while(Num);
#if(SetServer_SorF == 1)
			printf("\r\nSNTPServer:Fail stop");
#endif
	return 1;
}


/* @brief	连接SNTP服务器并查询时间，服务器返回结果存在ESP_time成员中
 *
 * @param	waittime 等待时间
 * @param	Num 	 重试次数
 *
 * @return  0成功  1服务器掉线  2失败
 */
uint8_t GET_Time(uint32_t waittime,uint8_t Num){
	char *strx = NULL;

if(ESP_time.ServerON){//ServerON==1

		do{//查询时间重试

			Clear_loopbuffer(500);

			AT_Send("AT+CIPSNTPTIME?");//发送查询时间命令(服务器返回的是CST时间)

			strx = ESP32_UART_Checkcmd("SNTPTIME:",700,24);//没找到返回Null

			if(strx){
				//此时时间在ESP32_UART_Checkcmd的cmd_buffer数组里

#if(GET_Time_ack == 1)
					printf("\r\nGET_Time:ack|%s|end",strx);
#endif

				int tmp_year, tmp_day, tmp_hour, tmp_min, tmp_sec;
				if(7 == sscanf(strx,"SNTPTIME:%3s %3s %d %d:%d:%d %d",
						   ESP_time.str_info.week,
						   ESP_time.str_info.month,
						   &tmp_day,          // 使用int临时变量
						   &tmp_hour, &tmp_min, &tmp_sec,
						   &tmp_year)        // 使用int临时变量
				){
					//成功提取
					// 赋值到结构体（范围检查）
					ESP_time.year = (tmp_year >= 0 && tmp_year <= 65535) ? (uint16_t)tmp_year : 2023;
					ESP_time.day = (uint8_t)tmp_day;
					ESP_time.hour = (uint8_t)tmp_hour;
					ESP_time.min = (uint8_t)tmp_min;
					ESP_time.sec = (uint8_t)tmp_sec;


					Time_transform();//字符串数据转换
#if(GET_Time_sscanf == 1)
					printf("\r\nGET_Time:sscanf OK");
#endif
					return 0;

				}else{
#if(GET_Time_sscanf == 1)
					printf("\r\nGET_Time:sscanf Fail retry..");
#endif
					Num--;
					continue;//重试
				}


			}//strx判空
			else
			{
				Num--;
#if(GET_Time_ack == 1)
				printf("\r\nGET_Time:ack NULL retry..");
#endif
				continue;//重试
			}
		}while(Num);//查询时间重试
		return 2;

	}//ServerON==1
	else{
#if(GET_Time_Server == 1)
		printf("\r\nSNTPServer:NO");
#endif
		return 1;
	}
}



// 判断闰年 (内联函数提高效率)
static inline uint8_t is_leap_year(uint16_t year) {
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}


/* @brief	转换时间戳
 *
 * @param	t	存储了时间信息的Time的地址
 * @return	Unix时间戳
 *
 */
uint32_t cst_to_unix(const Time* t) {
    // 1. 计算年份贡献的天数 (1970到year-1)
    uint32_t total_days = 0;
    for (uint16_t y = 1970; y < t->year; y++) {
        total_days += is_leap_year(y) ? 366 : 365;
    }

    // 2. 计算当年月份贡献的天数 (1月到month-1)
    uint8_t is_leap = is_leap_year(t->year);
    for (uint8_t m = 0; m < t->month_n - 1; m++) {
        total_days += days_in_month[is_leap][m];
    }

    // 3. 添加当月天数 (注意: 天数从1开始)
    total_days += (t->day - 1);

    // 4. 计算总秒数 (天转秒 + 时间部分)
    uint32_t total_sec = total_days * 86400UL  // 每天86400秒
                       + t->hour * 3600UL     // 小时转秒
                       + t->min * 60UL        // 分钟转秒
                       + t->sec;              // 秒

    // 5. 减去CST时区偏移 (UTC+8 -> 减28800秒)
    return total_sec - timezone * 3600UL;
}
