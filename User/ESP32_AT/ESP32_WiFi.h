/*
 * ESP32_WiFi.h
 *
 *  Created on: May 24, 2025
 *      Author: 12114
 */

#ifndef ESP32_AT_ESP32_WIFI_H_
#define ESP32_AT_ESP32_WIFI_H_

#include "ESP32_UART.h"
/*==================================================WiFi信息========================================================*/

#define WIFI_MODE	WiFi_Mixed

#define Wifi_SSID	"\"test2\""
#define Wifi_PWD  	"\"yu778899\""


#define AP_SSID		"\"ESP_32\""
#define AP_PWD  	"\"yu778866\""


void ESP32_WiFi_Init(void);
void WiFi_DisConnect();
void WiFi_Connect();




/*===================================================WiFi========================================================*/

/*	<mode>：模式
	0: 无 Wi-Fi 模式，并且关闭 Wi-Fi RF
	1: Station 模式
	2: SoftAP 模式
	3: SoftAP+Station 模式
*/
typedef enum
{
	WiFi_RFClose =	0x00U,
	WiFi_Station =	0x01U,
	WiFi_SoftAP	 =	0x02U,
	WiFi_Mixed	 =	0x03U
} WiFi_Mode;


/*	WiFi状态
 */
typedef enum
{
	WiFi_connected	 =	0x00U,
	WiFi_Init		 =  0x01U,
	WiFi_connecting  =	0x02U,
	WiFi_connectFail =	0x03U,
	WiFiERR	 		 =	0x04U,
} WiFi_State;


/*	存储已保存的WiFi信息
 */
typedef struct
{
	char* Wifi_ssid;	//名
	char* wifi_pwd;		//密码

} Station_Data;


/*	存储自身的的WiFi信息
 */
typedef struct
{
	char* AP_ssid;	//名
	char* AP_pwd;	//密码
	uint8_t channel;	//信道号
	uint8_t max_conn;	//最大连接数
	uint8_t ecn;		//加密方式，详情见官方AT文档
	uint8_t ssid_hidden;//0: 广播 SSID（默认） 1: 不广播

} AP_Data;


/*	WiFi功能
 *
 */
typedef struct
{
	WiFi_Mode			WiFi_MODE;		//当前的WiFi模式
	volatile WiFi_State	WiFi_state;		//当前的WiFi状态

	Station_Data		Station_Data;	//记录的WiFi信息
	AP_Data				AP_Data;

} AT_WiFi_HandleTypeDef;

#endif /* ESP32_AT_ESP32_WIFI_H_ */
