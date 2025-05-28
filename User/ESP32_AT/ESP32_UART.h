/*
 * ESP32_link.h
 *
 *  Created on: May 24, 2025
 *      Author: 12114
 */

#ifndef ESP32_AT_ESP32_UART_H_
#define ESP32_AT_ESP32_UART_H_

#include "stm32f1xx_hal.h"
#include "usart.h"
#include "string.h"
#include <stdarg.h>

#define ATEtoUART1  1	//打印信息到串口一
#define ATEtoUART1_IQR  0	//打印信息到串口一()

#if(ATEtoUART1 == 1)
#include "../../Core/Inc/retarget.h"
#endif


#define	buff_Size	1024	//模块回复消息的缓冲区大小

typedef enum
{
	Free      =	0x00U,	//空闲
	Sending	  =	0x01U,	//发送中
	Waiting   =	0x02U,	//命令已发送等待响应
	Writing   = 0x03U,	//返回消息等待写入缓冲区
	SendERR	  =	0x04U,	//发送溢出
	Overload  = 0x05

} UART_State;

typedef enum
{
	Success   =	0x00U,	//上一条命令执行成功结束(空闲)
	Fail	  = 0x01U,	//命令失败
	CMDHandle = 0x02U,
	WaitForIn =	0x03U,	//模块进入长输入 > 状态
} CMD_State;

typedef struct{

	UART_HandleTypeDef*		Command_UART;

	UART_State 	Uart_State;				//模块串口状态
	CMD_State	Cmd_State;				//模块命令状态
	uint8_t		overmuch;	//有没有模块一直发送消息不停止的情况

	uint16_t	writeIndex;				//写索引
	uint16_t	readIndex;				//读索引

	uint8_t 	Reply_Data[buff_Size];	//数据缓冲区

}AT_UART_HandleTypeDef;


/*========================================外部声明===========================================*/

extern AT_UART_HandleTypeDef	ESP32_UART;

/*========================================循环缓冲区管理===========================================*/

void Add_ReadIndex(uint16_t length);
uint8_t Read_buffer(uint16_t i);
uint16_t Get_UNhandled();
uint16_t Get_Empty();
uint16_t Write_buffer(const uint8_t* data,uint16_t length);

/*========================================通信函数===========================================*/

void AT_Send(const char* __restrict__ Command, ...);
void AT_Send_callee(char* buffer,uint16_t length,uint16_t buf_length);
char* ESP32_UART_Checkcmd(char *str, uint32_t waittime);

uint8_t ESP32_SendANDCheck(uint32_t waittime,  char *ack, const char* __restrict__ Command, ...);

uint8_t ESP32_SendANDCheck_(char *cmd, char *ack, uint32_t waittime);

/*====================================================中断相关==========================================================*/

/*	ESP32发送完成处理,写在 HAL_UART_TxCpltCallback 发送完成回调 函数里
 */
void ESP32_TxCpltHandle(UART_HandleTypeDef *huart);

/* @brief	ESP32接收完成回调,写在 HAL_UARTEx_RxEventCallback 串口接收事件 中断函数里
 */
void ESP32_RxCpltHandle(UART_HandleTypeDef *huart,uint16_t Size);


/*====================================================初始化==========================================================*/

void ESP32_UART_Init(UART_HandleTypeDef *huartx);


#endif /* ESP32_AT_ESP32_UART_H_ */
