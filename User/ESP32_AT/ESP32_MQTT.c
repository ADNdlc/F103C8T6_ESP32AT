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
		if(!ESP32_SendANDCheck(400,"OK","AT+MQTTUSERCFG=%d,%d,%s,%s,\"\",%d,%d,%s",\
									   LinkID,scheme,client_id,username,cert_key_ID,CA_ID,path))
		{
#if(MQTT_Init_SorF_CFG == 1)
			printf("\r\nMQTT_USERCFG:Success ");
#endif
			break;
		}
		else{
#if(MQTT_Init_SorF_CFG == 1)
			printf("\r\nMQTT_USERCFG:Fail ");
#endif
		}
	}

	uint16_t PWSLength = sizeof(password) - 1;
	num = temp;
	while(num--){
		if(ESP32_SendANDCheck(500,">","AT+MQTTLONGPASSWORD=%d,%d",LinkID,PWSLength))
		{
#if(MQTT_Init_SorF_PWD == 1)
			printf("\r\nMQTT_PWD>:Fail ");
#endif
			continue;
		}
		else{
#if(MQTT_Init_SorF_PWD == 1)
			printf("\r\nMQTT_PWD>:Success ");//成功返回0
#endif
			ESP32_UART.Cmd_State = WaitForIn;
		}

		if(ESP32_UART.Cmd_State == WaitForIn){
			if(ESP32_SendANDCheck(700,"OK","%s",password))
			{
#if(MQTT_Init_SorF_PWD == 1)
				printf("\r\nMQTT_PWD:Fail ");
#endif
				continue;
			}
			else{
#if(MQTT_Init_SorF_PWD == 1)
				printf("\r\nMQTT_PWD:Success ");
#endif
				ESP32_MQTT.MQTT_state = MQTT_Init;
				break;
			}
		}

	}
}

/*======================================MQTT相关======================================*/

/* @brief	连接MQTT服务器
 *
 */
void MQTT_Connect(uint8_t num){

	while(num--){
		ESP32_MQTT.MQTT_state = MQTT_connecting;
		if(ESP32_SendANDCheck(2000,"+MQTTCONNECTED","AT+MQTTCONN=%d,%s,%d,0",LinkID,MQTT_host,MQTT_port))
		{
#if(MQTT_Connect_SorF == 1)
			printf("\r\nMQTT_Connect:Fail ");
#endif
			ESP32_MQTT.MQTT_state = MQTT_connectFail;
		}
		else{
#if(MQTT_Connect_SorF == 1)
			printf("\r\nMQTT_Connect:Success ");
#endif
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
#if(MQTT_DisConnect_SorF == 1)
			printf("\r\nMQTT_DisConnect:Fail ");
#endif
			ESP32_MQTT.MQTT_state = MQTTERR;
		}
		else{
#if(MQTT_DisConnect_SorF == 1)
			printf("\r\nMQTT_DisConnect:Success ");
#endif
			ESP32_MQTT.MQTT_state = MQTT_Init;
		}
	}
	else{
#if(MQTT_DisConnect_SorF == 1)
		printf("\r\nMQTT_Staate:NOconnected ");
#endif
	}
}


/* @brief	订阅主题
 * @param	num			最大尝试次数
 * @param	topic		主题
 * @param	qos			服务质量
 * @return	0/1  成功返回0
 *
 */
uint8_t MQTT_Subscribe(uint8_t num, const char* topic ,uint8_t qos){
	do{
		if(!ESP32_SendANDCheck(500,"OK","AT+MQTTSUB=0,%s,0",topic)){
#if(MQTT_Subscribe_SorF == 1)
		printf("\r\nMQTT_Subscribe:Success");
#endif
			return 0;
		}
		else{
#if(MQTT_Subscribe_SorF == 1)
		printf("\r\nMQTT_Subscribe:Fail");
#endif
			num--;
		}
	}while(num);
#if(MQTT_Subscribe_SorF == 1)
		printf("\r\nMQTT_Subscribe:Out");
#endif
	return 1;
}


/* @brief	取消订阅主题
 * @param	num			最大尝试次数
 * @param	topic		主题
 *
 * @return	0/1  成功返回0
 *
 */
uint8_t MQTT_DisSubscribe(uint8_t num, const char* topic){
	do{
		if(!ESP32_SendANDCheck(500,"OK","AT+MQTTUNSUB=0,%s,0",topic)){
#if(MQTT_DisSubscribe_SorF == 1)
		printf("\r\nMQTT_Subscribe:Success");
#endif
			return 0;
		}
		else{
#if(MQTT_DisSubscribe_SorF == 1)
		printf("\r\nMQTT_Subscribe:Fail retry...");
#endif
			num--;
		}
	}while(num);
#if(MQTT_DisSubscribe_SorF == 1)
		printf("\r\nMQTT_Subscribe:stop");
#endif
	return 1;
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
uint8_t MQTT_Publish_Data(uint8_t num, char* json, const char* topic, uint8_t qos, uint8_t retain) {
    // 发送MQTT命令
    uint16_t payload_length = strlen(json);//负载信息长度

    do {
		// 发送AT+MQTTPUBRAW命令,进入数据模式
		if(ESP32_SendANDCheck(500, ">", "AT+MQTTPUBRAW=%d,%s,%d,%d,%d",
							  LinkID, topic, payload_length, qos, retain))
		{
#if(MQTT_Publish_SorF == 1)
			printf("\r\nMQTT_Publish>:Fail retry...");
#endif
			num--;
			continue;
		}
		else{//进入数据模式
			 //发送JSON数据
			if(!ESP32_SendANDCheck(500, "+MQTTPUB:OK", "%s", json)) {
#if(MQTT_Publish_SorF == 1)
				printf("\r\nMQTT_Publish:Success");
#endif
				//网站因该会推送成功的消息
				//...处理

				free(json);
				return 0;
			}
			else {
#if(MQTT_Publish_SorF == 1)
				printf("\r\nMQTT_Publish:Fail retry...");
#endif
				num--;
				continue;
			}
		}

    }while(num);//重试

#if(MQTT_Publish_SorF == 1)
    printf("\r\nMQTT_Publish:Fail stop");
#endif

    free(json);
    return 2;
}

/*	@brief			检查云端命令
 *
 *	@param setRECV	检查接收到MQTT推送时模块的信息
 *
 *	@return			动作
 *
 */
uint8_t MQTT_Check_PropertySet(const char* setRECV){
	//+MQTTSUBRECV..一共70个..property/(set/reply)偏移70查看下发消息类型

	uint16_t SetID = 0;;
	char funcPoint[64];
	char PropertySet[32];

	char *strx = NULL;
	uint16_t size = 0;
	static uint8_t SetBuffr[256] = {0};//接收消息缓存

	if((size = Get_UNhandled())){
		ESP32_UART.Cmd_State = CMDHandle;
		for(uint16_t i=0; i < size; i++){//使用Read_buffer读出,不增加读指针
			SetBuffr[i] = Read_buffer(ESP32_UART.readIndex + i);
		}

#if(MQTT_Subscribe_Set == 1)
				printf("\r\nSetBuffr:|%s|\r\n",SetBuffr);
#endif

		ESP32_UART.Cmd_State = Success;
		strx = strstr((char*)SetBuffr,setRECV);
		if(strx!=NULL && (strx + 70 - (char*)SetBuffr) < sizeof(SetBuffr)){
			if(!strcmp((strx+70),"set")){//是设置消息
				//提取id
				strx = strstr((char*)SetBuffr,"id");
				if(( 1 == sscanf((strx+5),"%d",&SetID) )){
					//还没想好
				}
#if(MQTT_Subscribe_Set == 1)
				printf("MQTTSet:%d",SetID);
#endif
				//提取命令
				strx = strstr((char*)SetBuffr,"\"params\"");
				//strx现在指向"params....
				if(1 == sscanf((strx),"\"params\":{\"%s\":%s}",funcPoint,PropertySet)){
					if(strstr((char*)PropertySet,"true")){
						return 1;
					}
					else if(strstr((char*)PropertySet,"false")){
						return 2;
					}
				}

			}
			else if(!strcmp((strx+70),"reply")){//不是设置消息(是发送回复)
				return 0;
			}
			else{
				return 0;
			}
		}
		return 0;
	}
	return 0;
}


/*==============================================JSON字符相关=================================================*/


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

/*	@brief		根据设备构建JOSN信息
 * 				若需发送复杂信息，比如较大的字符串或者标识符等信息较长需要更精确的计算防止溢出
 *
 *	@param data	设备对象
 *	@return		JSON长度
 *
 */
char* MQTT_Bulid_JSON(Sensor* S){
    // 创建JSON缓冲区
    uint16_t json_length = Calculate_json_length(S);//长度
    char* json_buffer = malloc(json_length);		//地址
    if (!json_buffer) return NULL;						//内存分配失败

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

#if(MQTT_Bulid_JSON_Info == 1)
			printf("\r\nJSON:%s|end",json_buffer);
#endif

	return json_buffer;
}

