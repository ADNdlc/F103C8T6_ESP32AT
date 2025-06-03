/*
 * ESP32_WiFi.c
 *
 *  Created on: May 24, 2025
 *      Author: 12114
 */

#include "ESP32_WiFi.h"

AT_WiFi_HandleTypeDef	ESP32_WiFi={
	.WiFi_state = StateERR
};


/*====================================================初始化==========================================================*/


/* @brief	加载WiFi数据并设置WiFi功能工作模式然后连接WiFi
 * @param	mode: a new state value of @ref WiFi_Mode in ESP32_WiFi.h
 *
 */
uint8_t ESP32_WiFi_Init(uint8_t Num){
	uint32_t Time=0;
	//加载数据
	ESP32_WiFi.WiFi_MODE = WIFI_MODE;

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

	do{
		if(!WiFi_SetState(ESP32_WiFi.WiFi_MODE,1)){
			break;
		}
		else{
			Num--;
		}
	}while(Num);

	Num +=1;
	do{
	//上电检查一遍WiFi状态
	ESP32_WiFi.WiFi_state = WiFi_GetState();
	Time = HAL_GetTick();

		switch(ESP32_WiFi.WiFi_state){
		case WiFi_connected://可上网
			return 0;

		case WiFi_NOIP://正在获取IP或失败
			HAL_Delay(500);
			continue;

		case WiFi_connecting://正连接或重连,在窗口时间内等待重连(WiFi会在4s左右尝试重连4s后切换到断开状态)
			while(HAL_GetTick()-Time<=4500){
				//查询模块是否切换状态
				if(WiFi_GetState() == WiFi_connected){//重连成功,可以上网,初始化成功
					ESP32_WiFi.WiFi_state = WiFi_GetState();
					printf("\r\nWF_Connect:Success ");
#if(ATEtoUART1 == 1)
					printf("\r\nWF_Connect:%d",ESP32_WiFi.WiFi_state);
#endif
					return 0;
				}
				else if(WiFi_GetState() == WiFi_DISconnect){//重连失败会进入WiFi_DISconnect,密码可能变更,重新配置WiFi连接
					WiFi_DisConnect();//终止重连
					Num--;
					continue;
				}//重连失败
				HAL_Delay(500);
			}//正连接或重连
			continue;//超时重试

		case WiFi_NOconnect://无连接
		case WiFi_DISconnect://断开状态，尝试连接
			WiFi_DisConnect();
			if(!WiFi_Connect(Wifi_SSID, Wifi_PWD)){
				//成功
				return 0;
			}
			else{
				//失败
				Num--;
				continue;
			}

		case StateERR://获取错误
			continue;

		}
	}while(Num);

	return 1;
}

/*====================================================WiFi模式==========================================================*/

/* @brief	查询WiFi模式
 * 			此函数不会更新WiFi_MODE
 *
 * @return	WiFi模式
 */
WiFi_Mode WiFi_GetMODE(void){
	unsigned short MODE = 0;
	char* strx = NULL;
	Clear_loopbuffer(500);
	AT_Send("AT+CWMODE?");//查询WiFi状态
	strx = ESP32_UART_Checkcmd("+CWMODE:",500,1);//格局结果返回WiFi_State
	if(strx){
#if(ATEtoUART1 == 1)
		printf("\r\nWF_GetMODE:strx|%s|end",strx);
#endif

		if(sscanf(strx,"+CWMODE:%hu",&MODE)){
#if(ATEtoUART1 == 1)
		printf("\r\nWF_GetMODE:sscanf%d",MODE);
#endif
			switch(MODE){
			case 0:
				return WiFi_RFClose;
			case 1:
				return WiFi_Station;
			case 2:
				return WiFi_SoftAP;
			case 3:
				return WiFi_Mixed;
			default:
				return ModeERR;
			}
		}
		else{
#if(ATEtoUART1 == 1)
			printf("\r\nWF_GetMODE:sscanfFail");
#endif
		}

	}//strxOK
	else{
#if(ATEtoUART1 == 1)
		printf("\r\nWF_GetState:strxNULL");
#endif
	}//strxNULL
	return ModeERR;
}


/* @brief	设置WiFi模式
 * @param	mode			WiFi_Mode模式
 * @param	auto_connect	自动重连
 *
 * @return	0/1  成功返回0
 */
uint8_t WiFi_SetState(WiFi_Mode mode, uint8_t auto_connect){
	if(ESP32_SendANDCheck(500,"OK","AT+CWMODE=%d,%d",mode ,auto_connect)){
		printf("\r\nWF_SetState:Fail1");
		ESP32_WiFi.WiFi_MODE = WiFi_GetMODE();
		return 1;
	}
	else{
		if(WiFi_GetMODE() == mode){
			printf("\r\nWF_SetState:Success ");
			ESP32_WiFi.WiFi_MODE = mode;
#if(ATEtoUART1 == 1)
		printf("\r\nWF_SetState:%d",ESP32_WiFi.WiFi_MODE);
#endif
			return 0;
		}
		else{
			printf("\r\nWF_SetState:Fail2");
			return 2;
		}
	}
}

/*===================================================WiFi状态==========================================================*/

/* @brief	查询WiFi状态
 * 			此函数不会更新WiFi_state
 *
 * @return	WiFi状态
 */
WiFi_State WiFi_GetState(void){
	unsigned short state = 0;
	char* strx = NULL;

	Clear_loopbuffer(500);
	AT_Send("AT+CWSTATE?");//查询WiFi状态
	strx = ESP32_UART_Checkcmd("+CWSTATE:",500,1);//格局结果返回WiFi_State
	if(strx){
#if(ATEtoUART1 == 1)
		printf("\r\nWF_GetState:strx|%s|end",strx);
#endif
		if(sscanf(strx,"+CWSTATE:%hu",&state)){
#if(ATEtoUART1 == 1)
		printf("\r\nWF_GetState:sscanf%d",state);
#endif
			switch(state){
			case 0:
				return WiFi_NOconnect;
			case 1:
				return WiFi_NOIP;
			case 2:
				return WiFi_connected;
			case 3:
				return WiFi_connecting;
			case 4:
				return WiFi_DISconnect;
			default:
				return StateERR;
			}
		}
		else{
#if(ATEtoUART1 == 1)
			printf("\r\nWF_GetState:sscanfFail");
#endif
		}
	}//strxOK
	else{
#if(ATEtoUART1 == 1)
		printf("\r\nWF_GetState:strxNULL");
#endif
	}//strxNULL
	return StateERR;
}


/* @brief	断开WiFi
 *
 */
void WiFi_DisConnect(){
	if(ESP32_SendANDCheck(500,"OK","AT+CWQAP")){
		printf("\r\nWF_DisConnect:Fail ");
	}
	else{
		printf("\r\nWF_DisConnect:Success ");
		ESP32_WiFi.WiFi_state = WiFi_GetState();
#if(ATEtoUART1 == 1)
		printf("\r\nWF_DisConnect:%d",ESP32_WiFi.WiFi_state);
#endif
	}
}

/* @brief	连接WiFi
 * @param	SSID			名字
 * @param	PWD	自动重连		密码
 *
 * @return	0/1	 成功返回0
 */
uint8_t WiFi_Connect(char *SSID, char *PWD){
	if(ESP32_SendANDCheck(16000,"WIFI GOT IP","AT+CWJAP=%s,%s",SSID ,PWD)){
		ESP32_WiFi.WiFi_state = WiFi_GetState();
		printf("\r\nWF_Connect:Fail1");
		return 1;
	}
	else{
		if(WiFi_GetState() == WiFi_connected){
			ESP32_WiFi.WiFi_state = WiFi_GetState();
			printf("\r\nWF_Connect:Success ");
#if(ATEtoUART1 == 1)
		printf("\r\nWF_Connect:%d",ESP32_WiFi.WiFi_state);
#endif
			return 0;
		}
		else{
			ESP32_WiFi.WiFi_state = WiFi_GetState();
			printf("\r\nWF_Connect:Fail2");
			return 2;
		}
	}
}



