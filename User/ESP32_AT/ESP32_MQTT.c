/*
 * ESP32_MQTT.c
 *
 *  Created on: May 25, 2025
 *      Author: 12114
 */

#include "ESP32_MQTT.h"

AT_MQTT_HandleTypeDef	ESP32_MQTT;


void ESP32_MQTT_Init(uint8_t num){
	uint8_t temp = num;

	ESP32_MQTT.MQTT_Data.MQTT_LinkID	= LinkID;
	ESP32_MQTT.MQTT_Data.MQTT_client_id	= client_id;
	ESP32_MQTT.MQTT_Data.MQTT_password	= password;
	ESP32_MQTT.MQTT_Data.MQTT_scheme	= scheme;
	ESP32_MQTT.MQTT_Data.MQTT_username	= username;

	while(num--){
		if(ESP32_SendANDCheck(400,"OK","AT+MQTTUSERCFG=%d,%d,%s,%s,\"\",%d,%d,%s",LinkID,scheme,client_id,username,cert_key_ID,CA_ID,path))
		{
			printf("\r\nMQTT_USERCFG:Fail ");//成功返回0
		}
		else{
			printf("\r\nMQTT_USERCFG:Success ");
			break;
		}
	}

	uint16_t PWSLength = sizeof(password) - 1;

	num = temp;
	while(num--){
		if(ESP32_SendANDCheck(400,">","AT+MQTTLONGPASSWORD=%d,%d",LinkID,PWSLength))
		{
			printf("\r\nMQTT_PWD>:Fail ");
			continue;
		}
		else{
			printf("\r\nMQTT_PWD>:Success ");//成功返回0
			ESP32_UART.Cmd_State = WaitForIn;
		}


		if(ESP32_UART.Cmd_State == WaitForIn){
			if(ESP32_SendANDCheck(500,"OK","%s",password))
			{
				printf("\r\nMQTT_PWD:Fail ");
			}
			else{
				printf("\r\nMQTT_PWD:Success ");
				ESP32_MQTT.MQTT_state = MQTT_Init;
				break;
			}
		}

	}
}

/* @brief	连接MQTT服务器
 *
 */
void MQTT_Connect(uint8_t num){

	while(num--){
		ESP32_MQTT.MQTT_state = MQTT_connecting;
		if(ESP32_SendANDCheck(2000,"OK","AT+MQTTCONN=%d,%s,%d,0",LinkID,MQTT_host,MQTT_port))
		{
			printf("\r\nMQTT_Connect:Fail ");
			ESP32_MQTT.MQTT_state = MQTT_connectFail;
		}
		else{
			printf("\r\nMQTT_Connect:Success ");
			ESP32_MQTT.MQTT_state = MQTT_connected;
		}
	}
}


/* @brief	断开MQTT服务器
 *
 */
void MQTT_DisConnect(){

	if(ESP32_MQTT.MQTT_state == MQTT_connected){

		if(ESP32_SendANDCheck(50,"OK","AT+MQTTCLEAN=0"))
		{
			printf("\r\nMQTT_Connect:Fail ");
			ESP32_MQTT.MQTT_state = MQTTERR;
		}
		else{
			printf("\r\nMQTT_Connect:Success ");
			ESP32_MQTT.MQTT_state = MQTT_Init;
		}
	}
}
