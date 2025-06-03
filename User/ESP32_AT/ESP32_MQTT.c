/*
 * ESP32_MQTT.c
 *
 *  Created on: May 25, 2025
 *      Author: 12114
 */

#include "ESP32_MQTT.h"
#include <inttypes.h> // 要添加这个头文件

AT_MQTT_HandleTypeDef	ESP32_MQTT= {
	.MQTT_state = MQTTERR
};

void ESP32_MQTT_Init(uint8_t num){
	uint8_t temp = num;

	ESP32_MQTT.MQTT_Data.MQTT_LinkID	= LinkID;
	ESP32_MQTT.MQTT_Data.MQTT_client_id	= client_id;
	ESP32_MQTT.MQTT_Data.MQTT_password	= password;
	ESP32_MQTT.MQTT_Data.MQTT_scheme	= scheme;
	ESP32_MQTT.MQTT_Data.MQTT_username	= username;

	while(num--){
		if(ESP32_SendANDCheck(400,"OK","AT+MQTTUSERCFG=%d,%d,%s,%s,\"\",%d,%d,%s",\
									   LinkID,scheme,client_id,username,cert_key_ID,CA_ID,path))
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
		if(ESP32_SendANDCheck(500,">","AT+MQTTLONGPASSWORD=%d,%d",LinkID,PWSLength))
		{
			printf("\r\nMQTT_PWD>:Fail ");
			continue;
		}
		else{
			printf("\r\nMQTT_PWD>:Success ");//成功返回0
			ESP32_UART.Cmd_State = WaitForIn;
		}


		if(ESP32_UART.Cmd_State == WaitForIn){
			if(ESP32_SendANDCheck(700,"OK","%s",password))
			{
				printf("\r\nMQTT_PWD:Fail ");
				continue;
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
		if(ESP32_SendANDCheck(2000,"+MQTTCONNECTED","AT+MQTTCONN=%d,%s,%d,0",LinkID,MQTT_host,MQTT_port))
		{
			printf("\r\nMQTT_Connect:Fail ");
			ESP32_MQTT.MQTT_state = MQTT_connectFail;
		}
		else{
			printf("\r\nMQTT_Connect:Success ");
			ESP32_MQTT.MQTT_state = MQTT_connected;
			Clear_loopbuffer(2000);
			break;
		}
	}
}


/* @brief	断开MQTT服务器
 *
 */
void MQTT_DisConnect(){

	if(ESP32_MQTT.MQTT_state == MQTT_connected){

		if(ESP32_SendANDCheck(200,"OK","AT+MQTTCLEAN=0"))
		{
			printf("\r\nMQTT_DisConnect:Fail ");
			ESP32_MQTT.MQTT_state = MQTTERR;
		}
		else{
			printf("\r\nMQTT_DisConnect:Success ");
			ESP32_MQTT.MQTT_state = MQTT_Init;
		}
	}
	else{
		printf("\r\nMQTT_Staate:NOconnected ");
	}
}


/*	@brief		计算JSON字符串长度(大致)
 * 				若需发送复杂信息，比如较大的字符串或者标识符等信息较长需要更精确的计算防止溢出
 *
 *	@param data	设备对象
 *	@return		JSON长度
 *
 */
uint16_t Calculate_json_length_simple(Sensor* S) {
    uint16_t length = 200; // 基础长度
    for (int i = 0; i < S->count; i++) {
        length += strlen(S->data_points[i].name) * 2 + 50;
    }
    return length;
}


/*	@brief		计算更精确的JSON字符串长度
 *	@param data	设备对象
 *	@return		JSON长度(用于malloc分配空间)
 *
 */
uint16_t Calculate_json_length(Sensor* data) {
    // 基础长度
    uint16_t length = 50; // {"id":"","version":"","params":{}}

    // 添加设备ID和版本号长度
    length += strlen(data->device_id) + strlen(data->version);

    // 添加每个数据点长度
    for (int i = 0; i < data->count; i++) {
        DataPoint* point = &data->data_points[i];

        // 数据点名称：引号+转义字符
        length += 4 + strlen(point->name) * 2;

        // 值部分：根据数据类型计算
        switch(point->type) {
            case DATA_int:
                length += 30; // {"value":,"time":} + 最大整数长度
                break;
            case DATA_float:
            case DATA_double:
                length += 40;
                break;
            case DATA_string:
                length += 20 + strlen(point->value.string_value);//计算字符串信息长度
                break;
        }

        // 时间戳部分：20位数字(实际使用13位)
        length += 20;

        // 分隔符：逗号（最后一个数据点除外）
        if (i < data->count - 1) length += 1;
    }

    // 添加结束符和额外的缓冲
    length += 10; // 结束符和额外缓冲

    return length;
}


/*	@brief			发布传感器数据
 *  @param num		重试次数
 *
 *	@param data		设备对象
 *	@param topic	主题
 *	@param qos		服务质量
 *	@param retain	服务器保存信息吗
 *
 *	@return	执行情况	0发送成功 1内存溢出 2发送失败
 *
 */
uint8_t MQTT_Publish_Data(uint8_t num, Sensor* S, const char* topic, uint8_t qos, uint8_t retain) {
    // 创建JSON缓冲区
    uint16_t json_length = Calculate_json_length(S);
    char* json_buffer = malloc(json_length);
    if (!json_buffer) return 1;

    // 构建JSON头部
    snprintf(json_buffer, json_length,
             "{\"id\":%s,\"version\":%s,\"params\":{",
             S->device_id, S->version);

    // 添加数据点JSON
    char temp_buffer[128];//数据点很大时此数组有溢出风险，此时snprintf会截断剩下的信息导致格式错误

    for (int i = 0; i < S->count; i++) {
        DataPoint* point = &S->data_points[i];

        // 根据数据类型格式化值
        switch(point->type) {
            case DATA_int:
                snprintf(temp_buffer, sizeof(temp_buffer),
                         "%s:{\"value\":%d,\"time\":%lu000}",
                         point->name, point->value.int_value, S->data_points->timestamp);
                break;
            case DATA_float:
                snprintf(temp_buffer, sizeof(temp_buffer),
                         "%s:{\"value\":%.2f,\"time\":%lu000}",
                         point->name, point->value.float_value, S->data_points->timestamp);
                break;
            case DATA_double:
                snprintf(temp_buffer, sizeof(temp_buffer),
                         "%s:{\"value\":%.2f,\"time\":%lu000}",
                         point->name, point->value.double_value, S->data_points->timestamp);
                break;
            case DATA_string:
                snprintf(temp_buffer, sizeof(temp_buffer),
                         "%s:{\"value\":%s,\"time\":%lu000}",
                         point->name, point->value.string_value, S->data_points->timestamp);
                break;
        }

        strcat(json_buffer, temp_buffer);//追加到json_buffer
        if (i < S->count - 1) strcat(json_buffer, ",");//追加,
    }

    // 完成JSON构建
    strcat(json_buffer, "}}");

#if(ATEtoUART1 == 1)
			printf("\r\nJSON:%s|end",json_buffer);
#endif

    // 发送MQTT命令
    uint16_t payload_length = strlen(json_buffer);//负载信息长度
    do {
		// 发送AT+MQTTPUBRAW命令,进入数据模式
		if(ESP32_SendANDCheck(500, ">", "AT+MQTTPUBRAW=%d,%s,%d,%d,%d",
							  LinkID, topic, payload_length, qos, retain))
		{
#if(ATEtoUART1 == 1)
			printf("\r\nMQTT_PUBRAW:Fail retry...");
#endif
			num--;
			continue;
		}
		else{//进入数据模式
			 //发送JSON数据
			if(ESP32_SendANDCheck(500, "+MQTTPUB:OK", "%s", json_buffer)) {

#if(ATEtoUART1 == 1)
				printf("\r\nMQTT_Publish:Fail retry...");
#endif
				num--;
				continue;
			}
			else {
#if(ATEtoUART1 == 1)
				printf("\r\nMQTT_Publish:Success");
#endif
				//网站因该会推送成功的消息
				//...处理

				free(json_buffer);
				return 0;
			}
		}

    }while(num);//重试

#if(ATEtoUART1 == 1)
    printf("\r\nMQTT_Publish:Fail stop");
#endif

    free(json_buffer);
    return 2;
}


