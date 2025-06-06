/*
 * ESP32_link.h
 *
 *  Created on: May 24, 2025
 *      Author: 12114
 *
 *
 */

#ifndef ESP32_AT_ESP32_UART_H_
#define ESP32_AT_ESP32_UART_H_

#include "stm32f1xx_hal.h"
#include "usart.h"
#include "string.h"
#include <stdarg.h>
#include <stdio.h>

/*ߜ----------------------------------------ߜ打印信息开关ߜ---------------------------------------ߜ*/

#define ATEtoUART1_IQR  0	//中断内打印(不要这样做)

//Clear_loopbuffer:
#define Clear_UNhandled 1	//循环缓冲区没有处理的数据大小
#define Clear_cleared	1	//查看有没有清理成功

//三个AT_Send:
#define AT_Send_CMD		1	//将所有发送给模块的指令和内容通过printf的串口输出

//ESP32_UART_Checkcmd:
#define Checkcmd_RIndx	1	//当前循环缓冲区的读指针位置
#define Checkcmd_buffer 1	//Checkcmd函数全局缓存区内容
#define Checkcmd_ack	1	//检查到的期望答复

//ESP32_SendANDCheck:
#define SendANDCheck_ack	1	//打印检查到的期望答复或失败时打印NULL

//ESP32_UART_Init:
#define UART_Init		1	//UART初始化信息


/*ߡ-------------------------------------ߡ打印信息开关ߡ-----------------------------------------ߡ*/

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
	UART_HandleTypeDef* Command_UART;

	UART_State 	Uart_State;				//模块串口状态
	CMD_State	Cmd_State;				//模块命令状态
	uint8_t		overmuch;				//接收不过来了

	uint16_t	writeIndex;				//写索引
	uint16_t	readIndex;				//读索引

	uint8_t 	Reply_Data[buff_Size];	//数据缓冲区

} AT_UART_HandleTypeDef;


/*===========================================外部声明=========================================*/

extern AT_UART_HandleTypeDef	ESP32_UART;

/*========================================循环缓冲区管理=======================================*/

void Add_ReadIndex(uint16_t length);
uint8_t Read_buffer(uint16_t i);
uint16_t Get_UNhandled();
uint16_t Get_Empty();
uint16_t Write_buffer(const uint8_t* data,uint16_t length);

void Clear_loopbuffer(uint16_t waittime);
/*===========================================通信函数==========================================*/

void AT_Send(const char* __restrict__ Command, ...);
void AT_Send_callee(char* buffer,uint16_t length,uint16_t buf_length);
void AT_Send_HEX(uint8_t* buffer);
char* ESP32_UART_Checkcmd(char *str, uint32_t waittime,uint8_t extend);

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

uint8_t ESP32_UART_Init(UART_HandleTypeDef *huartx);


#endif /* ESP32_AT_ESP32_UART_H_ */
