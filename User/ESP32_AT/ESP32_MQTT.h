/*
 * ESP32_MQTT.h
 *
 *  Created on: May 25, 2025
 *      Author: 12114
 */

#ifndef ESP32_AT_ESP32_MQTT_H_
#define ESP32_AT_ESP32_MQTT_H_

#include "ESP32_UART.h"
#include "sensor_data.h"

/*==================================================MQTT信息========================================================*/

//服务器地址和端口
#define MQTT_host	"\"mqtts.heclouds.com\""	//域名
#define MQTT_port	1883						//端口

//MQTT连接设置
#define	LinkID		0						//MQTT连接ID,目前模块只支持0
#define	scheme		1						//连接方式


#define msg_ID "\"123\""			//消息id号，用户自定义，String类型的数字，长度限制不超过13位
#define mqtt_version "\"1.0\""		//物模型版本号，可选字段，不填默认为1.0


#define	client_id	"\"temperatureAndHumidity\""	//网站的	“设备名称/ID”
#define	username	"\"SQKg9n0Ii0\""				//用户名，用于登陆 MQTT broker, 网站的"产品ID"

					/*密码，使用tokon工具生成*/
#define	password	"version=2018-10-31&res=products%2FSQKg9n0Ii0%2Fdevices%2FtemperatureAndHumidity&et=1757458587&method=md5&sign=YCozJxz%2BPX0Qf1coXSUd0A%3D%3D"


//这些参数现在还没用仅作占位和格式构建
#define cert_key_ID	0
#define CA_ID		0
#define path		"\"\""


//DHT11使用的MQTT主题和信息（这是在网站定义的）
#define dht11

#ifdef dht11

/*================主题===================*/
//https://open.iot.10086.cn/doc/v5/fuse/detail/920
#define Info_Topic		"\"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post/reply\""//这是信息回复主题,即响应Topic
#define publish_Topic	"\"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/post\""//本地数据推送主题，即请求Topic
#define subscribe_Topic	"\"$sys/SQKg9n0Ii0/temperatureAndHumidity/thing/property/set\""


//传感器数据推送格式
#define Data_Info		"{\"id\":\"123\",\"version\":\"1.0\",\"params\":{\"currentTemperature\":{\"value\":22,\"time\":1747458287111},\"currenthumidity\":{\"value\":33,\"time\":1747458287111}}}"

#endif

/*ߜ----------------------------------------ߜ打印信息开关ߜ---------------------------------------ߜ*/
//ESP32_MQTT_Init:
#define MQTT_Init_SorF_CFG	1	//CFG成败信息
#define MQTT_Init_SorF_PWD	1	//PWD成败信息

//MQTT_Connect:
#define MQTT_Connect_SorF		1	//连接成败信息

//MQTT_DisConnect:
#define MQTT_DisConnect_SorF 	1	//断开成败信息

//MQTT_Subscribe:
#define MQTT_Subscribe_SorF	 	1	//订阅成败信息

#define MQTT_Subscribe_Set		1	//云设置主题有信息

//MQTT_DisSubscribe:
#define MQTT_DisSubscribe_SorF	1	//取消订阅成败信息

//MQTT_Publish_Data:
#define MQTT_Publish_SorF		1	//取消订阅成败信息

//MQTT_Bulid_JSON:
#define MQTT_Bulid_JSON_Info	1	//打印发布JSON

/*ߡ-------------------------------------ߡ打印信息开关ߡ-----------------------------------------ߡ*/



/*===================================================MQTT========================================================*/

typedef enum
{
	MQTT_connected	 =	0x00U,
	MQTT_Init		 =  0x01U,
	MQTT_connecting  =	0x02U,
	MQTT_connectFail =	0x03U,
	MQTTERR	 		 =	0x04U,

} MQTT_State;


/*	存储MQTT用户属性
 */
typedef struct
{
	uint8_t		MQTT_LinkID;		//MQTT连接ID
	uint8_t		MQTT_scheme;		//连接方式
	char*		MQTT_client_id;		//网站的	“设备名称/ID”
	char*		MQTT_username;		//用户名，用于登陆 MQTT broker, 网站的"产品ID"
	char*		MQTT_password;		//密码，使用tokon工具生成

} MQTT_Data;


/*	MQTT功能
 */
typedef struct
{
	volatile MQTT_State	MQTT_state;
	MQTT_Data			MQTT_Data;


} AT_MQTT_HandleTypeDef;



extern AT_MQTT_HandleTypeDef	ESP32_MQTT;

/*===================================================连接函数========================================================*/

void ESP32_MQTT_Init(uint8_t num);
void MQTT_Connect(uint8_t num);
void MQTT_DisConnect();

uint16_t Calculate_json_length_simple(Sensor* S);
uint16_t Calculate_json_length(Sensor* data);
char* MQTT_Bulid_JSON(Sensor* S);
/*===================================================发布订阅函数========================================================*/

uint8_t MQTT_Subscribe(uint8_t num, const char* topic ,uint8_t qos);
uint8_t MQTT_DisSubscribe(uint8_t num, const char* topic);
uint8_t MQTT_Publish_Data(uint8_t num, char* json, const char* topic, uint8_t qos, uint8_t retain);
uint8_t MQTT_Check_PropertySet(const char* setRECV);

#endif /* ESP32_AT_ESP32_MQTT_H_ */
