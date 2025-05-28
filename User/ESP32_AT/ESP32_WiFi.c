/*
 * ESP32_WiFi.c
 *
 *  Created on: May 24, 2025
 *      Author: 12114
 */

#include "ESP32_WiFi.h"

AT_WiFi_HandleTypeDef	ESP32_WiFi;


/*====================================================初始化==========================================================*/


/* @brief	加载WiFi数据并设置WiFi功能工作模式
 * @param	mode: a new state value of @ref WiFi_Mode in ESP32_WiFi.h
 *
 */
void ESP32_WiFi_Init(void){

	//加载数据
	switch(ESP32_WiFi.WiFi_MODE){
	case WiFi_Mixed:
		ESP32_WiFi.Station_Data.Wifi_ssid = Wifi_SSID;
		ESP32_WiFi.Station_Data.wifi_pwd  = Wifi_PWD;
		ESP32_WiFi.AP_Data.AP_ssid = AP_SSID;
		ESP32_WiFi.AP_Data.AP_pwd  = AP_PWD;
		break;

	case WiFi_Station:
		ESP32_WiFi.Station_Data.Wifi_ssid = Wifi_SSID;
		ESP32_WiFi.Station_Data.wifi_pwd  = Wifi_PWD;
		break;

	case WiFi_SoftAP:
		ESP32_WiFi.AP_Data.AP_ssid = AP_SSID;
		ESP32_WiFi.AP_Data.AP_pwd  = AP_PWD;
		break;

	case WiFi_RFClose:
		break;

	default:
		break;
	}


	if(ESP32_SendANDCheck(100,"OK","AT+CWMODE=%d,0",WIFI_MODE)){
		printf("\r\nWiFi_Init:Fail ");
	}
	else{
		ESP32_WiFi.WiFi_state = WiFi_Init;
		ESP32_WiFi.WiFi_MODE = WIFI_MODE;
		printf("\r\nWiFi_Init:Success ");
	}


	if(ESP32_SendANDCheck(2000,"OK","AT+CWJAP=%s,%s",Wifi_SSID,Wifi_PWD)){
		ESP32_WiFi.WiFi_state = WiFi_connectFail;
		printf("\r\nWiFi_connect:Fail ");
	}
	else{
		ESP32_WiFi.WiFi_state = WiFi_connected;
		printf("\r\nWiFi_connect:Success ");
	}

}


/* @brief	断开WiFi
 *
 */
void WiFi_DisConnect(){
	if(ESP32_SendANDCheck(1500,"OK","AT+CWQAP")){
		printf("\r\nWiFi_DisConnect:Fail ");
	}
	else{
		printf("\r\nWiFi_DisConnect:Success ");
		ESP32_WiFi.WiFi_state = WiFi_Init;
	}
}

/* @brief	重连初始化时成功连上的WiFi
 *
 */
void WiFi_Connect(){
	if(ESP32_SendANDCheck(1500,"OK","AT+CWJAP")){
		printf("\r\nWiFi_Connect:Fail ");
	}
	else{
		printf("\r\nWiFi_Connect:Success ");
	}
}


