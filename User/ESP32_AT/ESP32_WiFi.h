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




/*ߜ----------------------------------------ߜ打印信息开关ߜ---------------------------------------ߜ*/
//ESP32_WiFi_Init:
#define WiFi_Init			1	//UART初始化信息

//WiFi_GetMODE:
#define WF_GetMODE_ack		1	//是否检查到的答复内容
#define WF_GetMODE_sscanf	1	//sscanf步骤执行情况

//WiFi_SetState:
#define WF_SetState			1	//打印获取到的状态,判断有没有设置成功
#define WF_SetState_SorF	1	//成败信息

//WiFi_GetState:
#define WF_GetState_ack		1	//期望答复
#define WF_GetState_sscanf	1	//打印获取到的状态

//WiFi_DisConnect:
#define WF_DisConnect_state	1	//打印获取到的状态
#define WF_DisConnect_SorF	1	//成败信息

//WiFi_Connect:
#define WF_Connect_state	1	//打印获取到的状态
#define WF_Connect_SorF		1	//成败信息

/*ߡ-------------------------------------ߡ打印信息开关ߡ-----------------------------------------ߡ*/




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
	WiFi_Mixed	 =	0x03U,
	ModeERR		 =	-1//没有这种模式
} WiFi_Mode;


/*	WiFi状态
 * 	<state>：模块返回码
 *	0: ESP32 station 尚未进行任何 Wi-Fi 连接
 *	1: ESP32 station 已经连接上 AP，但尚未获取到 IPv4 地址
 *	2: ESP32 station 已经连接上 AP，并已经获取到 IPv4 地址
 *	3: ESP32 station 正在进行 Wi-Fi 连接或 Wi-Fi 重连
 *	4: ESP32 station 处于 Wi-Fi 断开状态
 *
 *	模块行为：
 *	如果设置了自动连接AP，上电后自动尝试重连上次连接的WiFi
 *	失败后进入4，然后3和4间隔切换(大概2s)，直到发送AT+CWQAP停止连接稳定在4或连接上AP
 */
typedef enum
{
	WiFi_connected	 =	0x00U,//2  可以上网
	WiFi_NOIP 		 =	0x01U,//1
	WiFi_NOconnect	 =  0x02U,//0
	WiFi_connecting  =	0x03U,//3
	WiFi_DISconnect  =	0x04U,//4
	StateERR		 =	-1//没有这种状态
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

extern AT_WiFi_HandleTypeDef	ESP32_WiFi;

/*===================================================初始化========================================================*/
uint8_t ESP32_WiFi_Init(uint8_t Num);

/*====================================================WiFi模式==========================================================*/
WiFi_Mode WiFi_GetMODE(void);
uint8_t WiFi_SetState(WiFi_Mode mode, uint8_t auto_connect);

/*===================================================WiFi状态==========================================================*/
WiFi_State WiFi_GetState(void);
void WiFi_DisConnect();
uint8_t WiFi_Connect(char *SSID, char *PWD);

#endif /* ESP32_AT_ESP32_WIFI_H_ */
