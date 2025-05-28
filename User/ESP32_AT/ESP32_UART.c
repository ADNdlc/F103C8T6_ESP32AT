/*
 * ESP32_link.c
 *
 *  Created on: May 24, 2025
 *      Author: 12114
 */

#include "ESP32_UART.h"


extern	DMA_HandleTypeDef	hdma_usart3_rx;
DMA_HandleTypeDef*			hdma_AT_rx	= &hdma_usart3_rx;

#define UART_Size			512
uint8_t UART_buffer[UART_Size] =  {0};//串口接收缓存

AT_UART_HandleTypeDef	ESP32_UART={
	.Reply_Data = {0}//初始化缓冲区
};


/*========================================循环缓冲区管理===========================================*/

/*	@brief	增加读索引(已处理)
 *	@param	length:	要增加的长度
 *
*/
void Add_ReadIndex(uint16_t length){
	ESP32_UART.readIndex = (ESP32_UART.readIndex + length) % buff_Size;

//	ESP32_UART.readIndex += length;
//	ESP32_UART.readIndex %= buff_Size;
}


/*	@brief	读取缓冲区第i位数据,超过缓存区长度自动循环
 *	@param:	i要读取的数据指针
 *
*/
uint8_t Read_buffer(uint16_t i){
	uint16_t index = i % buff_Size;
	return ESP32_UART.Reply_Data[index];
}


/*	@brief		计算未处理的数据长度
 *	@return		未处理的数据长度
 *	@retval		0						缓冲区为空
 *	@retval		1~buff_Size-1			未处理的数据长度
 *	@retval		buff_Size(缓冲区大小)	缓冲区已满
 *
 */
uint16_t Get_UNhandled(){
	return (ESP32_UART.writeIndex + buff_Size - ESP32_UART.readIndex) % buff_Size;
}


/*	@brief		算缓冲区剩余空间
 *	@return		剩余空间大小
 *	@retval		0					缓冲区已满
 *	@retval		1~buff_Size-1		剩余空间
 *	@retval		buff_Size(缓冲区大小)	缓冲区为空
 *
 */
uint16_t Get_Empty(){
	return buff_Size - Get_UNhandled();
}


/*	@brief向缓冲区写入数据(自动置writeIndex大小)
 *	@param data 	要写入的数据指针
 *	@param length 	要写入的数据长度
 *	@return			写入的数据长度
 *
 */
uint16_t Write_buffer(const uint8_t* data,uint16_t length){
	//缓冲区不足
	if(Get_Empty() < length){
		ESP32_UART.overmuch = 1;
		return 0;
	}
	//使用memcpy函数将数据写入缓冲区
	if(ESP32_UART.writeIndex + length < buff_Size){
		memcpy(ESP32_UART.Reply_Data + ESP32_UART.writeIndex, data, length);
		ESP32_UART.writeIndex += length;
	}
	else{
		uint16_t firstLength = buff_Size - ESP32_UART.writeIndex;
		memcpy(ESP32_UART.Reply_Data + ESP32_UART.writeIndex, data, firstLength);
		memcpy(ESP32_UART.Reply_Data, data + firstLength, length - firstLength);
		ESP32_UART.writeIndex = length - firstLength;
	}
	return length;
}

/*========================================通信函数===========================================*/

/* @brief	发送AT指令(格式化发送，非阻塞，状态由中断控制)
 * @param	Command 格式化字符串
 * @param	...     入参
 *
 */
void AT_Send(const char* __restrict__ Command, ...){
	//if(ESP32_UART.AT_state == Success){

		char buffer[1024];  //缓冲区大小(至少要装下网站的token)
		va_list args;

		va_start(args, Command);
		vsnprintf(buffer, sizeof(buffer), Command, args);
		va_end(args);

		uint16_t length = strlen(buffer);

		if (length + 2 < sizeof(buffer)) {
			buffer[length] = '\r';
			buffer[length + 1] = '\n';
			buffer[length + 2] = '\0';  // 更新字符串结束符
			length += 2;  // 更新长度
		} else {
			ESP32_UART.Uart_State = SendERR;
			return;
		}

		ESP32_UART.Uart_State = Sending;//状态置位

#if(ATEtoUART1 == 1)
		printf("\r\nTxCMD:%s ",Command);
#endif
#if(ATEtoUART1_IQR == 1)
		printf("\r\nTx:Sending ");
#endif

		//发送命令(非阻塞)
		HAL_UART_Transmit_DMA(ESP32_UART.Command_UART, (uint8_t*)buffer, length);
}

/* @brief	发送AT指令(发送字符串)
 * @param	Command 格式化字符串
 * @param	...     入参
 *
 */
void AT_Send_callee(char* buffer,uint16_t length,uint16_t buf_length){

		if (length + 2 < buf_length) {
			buffer[length] = '\r';
			buffer[length + 1] = '\n';
			buffer[length + 2] = '\0';  // 更新字符串结束符
			length += 2;  // 更新长度
		} else {
			ESP32_UART.Uart_State = SendERR;
			return;
		}

		ESP32_UART.Uart_State = Sending;//状态置位

#if(ATEtoUART1 == 1)
		printf("\r\nTxCMD:%s ",buffer);
#endif
#if(ATEtoUART1_IQR == 1)
		printf("\r\nTx:Sending ");
#endif

		//发送命令(非阻塞)
		HAL_UART_Transmit_DMA(ESP32_UART.Command_UART, (uint8_t*)buffer, length);
}


/* @brief	检查返回消息判断命令执行情况
 *
 * @param	str 应该收到的内容
 * @return	在接收字符串 ESP32_UART.Reply_Data 中查找第一次出现字符串 str（不包含空结束字符）的位置
 *
 */
char* ESP32_UART_Checkcmd(char *str, uint32_t waittime){
	static uint8_t cmd_buffer[512];//这是所有消息公用的最后的存储区
	uint32_t Time=0;	//上次收到消息时间
	char *strx = NULL;
	uint16_t Size=0;
	uint8_t MAX_Cmd = 15;
	uint8_t length = strlen(str);


	while(MAX_Cmd){//最多接收MAX_Cmd条指令
		Time = HAL_GetTick();//获取本轮指令等待开始时间
		while((HAL_GetTick()-Time) <= waittime){
			if( (Size = Get_UNhandled()) ){//在超时时间内判断有没有消息(缓冲区是否为空)


				/*有回复消息*/
				ESP32_UART.Cmd_State = CMDHandle;
				for(uint16_t i=0; i < Size; i++){//使用Read_buffer读出
					cmd_buffer[i] = Read_buffer(ESP32_UART.readIndex + i);
				}
				Add_ReadIndex(Size);//增加读索引(已读出判断)

#if(ATEtoUART1 == 1)
				printf("\r\nRIndx:%d ",ESP32_UART.readIndex);
#endif

				/*开始判断*/
				strx = strstr((char*)cmd_buffer,str);

				//添加结束符
				if (strx != NULL && (strx+length+1 - (char*)cmd_buffer) < sizeof(cmd_buffer)){
				    strx[length] = '\0';
					ESP32_UART.Cmd_State = Success;

#if(ATEtoUART1 == 1)
				printf("\r\nCheck:%s ",strx);//发送信息
#endif
					return strx;

				} else {
					ESP32_UART.Cmd_State = Fail;
					MAX_Cmd--;
				}


			}
		}
		//超时
		ESP32_UART.Cmd_State = Fail;
		return strx;
	}
	//返回
	ESP32_UART.overmuch = 1;
	return strx;
}

/* @brief	发送AT指令并判断成败(格式化发送)
 * @param	cmd:	命令
 * @param	ack:	成功的回答
 * @param	waittime:超时时间
 * @return	0/1  成功返回0
 *
 */
uint8_t ESP32_SendANDCheck(uint32_t waittime,  char *ack, const char* __restrict__ Command, ...){
	char buffer[1024];  //缓冲区大小(至少要装下网站的token)
	uint8_t data_found = 0;
	uint32_t start_time = 0;
	va_list args;
	va_start(args, Command);
	vsnprintf(buffer, sizeof(buffer), Command, args);
	va_end(args);

	uint16_t length = strlen(buffer);
	uint16_t buf_length = sizeof(buffer);
	uint16_t Size = 0;

	//判断缓冲区Reply_Data是否有残留数据
	do {
	    data_found = 0; // 重置标志位
	    // 在500ms窗口期内持续检测
	    while (HAL_GetTick() - start_time <= 500) {

	        if ( (Size = Get_UNhandled()) ) {
	            Add_ReadIndex(Size); // 清空残留数据
	            data_found = 1;   	 // 标记数据残留，进行循环
	            start_time = HAL_GetTick(); // 重置超时计时

#if(ATEtoUART1 == 1)
	        	Size = Get_UNhandled();
	            printf("\r\nData:residual %d",Size);
#endif

	        }
	        HAL_Delay(50);
	    }
	    // 若500ms内无数据，退出循环
	} while (data_found);


#if(ATEtoUART1 == 1)
	Size = Get_UNhandled();
	printf("\r\nData:%d",Size);
#endif

	AT_Send_callee(buffer,length,buf_length);//发送

	char* bufIdx;
	bufIdx = ESP32_UART_Checkcmd(ack,waittime);
	if( bufIdx != NULL){//成功

#if(ATEtoUART1 == 1)
		printf("\r\nSC_Check:%s ",bufIdx);
#endif

		return 0;
	}
	else{

#if(ATEtoUART1 == 1)
		printf("\r\nSC_Check:NULL ");
#endif

		return 1;
	}
}


/* @brief	发送AT指令并判断成败(发送字符串)
 * @param	cmd:	命令
 * @param	ack:	成功的回答
 * @param	waittime:超时时间
 * @return	0/1  成功返回0
 *
 */
uint8_t ESP32_SendANDCheck_(char *cmd, char *ack, uint32_t waittime){
	AT_Send(cmd);
	char* buffer;
	buffer = ESP32_UART_Checkcmd(ack,waittime);
	if( buffer != NULL){//成功
#if(ATEtoUART1 == 1)
		printf("\r\nBack:%s ",buffer);
#endif
		return 0;
	}
	else{
#if(ATEtoUART1 == 1)
		printf("\r\nBack:NULL ");
#endif
		return 1;
	}
}

/*====================================================中断相关==========================================================*/

/*	ESP32发送完成处理,写在 HAL_UART_TxCpltCallback 发送完成回调 函数里
 */
void ESP32_TxCpltHandle(UART_HandleTypeDef *huart){
	//if(huart->Instance == ESP32_UART.Command_UART->Instance){
	if(huart == ESP32_UART.Command_UART){
		ESP32_UART.Uart_State = Waiting;//发送完成等待模块响应

#if(ATEtoUART1_IQR == 1)
		printf("\r\nTx:Waiting ");
#endif

	}
}

/* @brief	ESP32接收完成回调,写在 HAL_UARTEx_RxEventCallback 串口接收事件 中断函数里
 * 			当模块回复完一次消息就会进入此函数，此时消息存在ESP32_UART.Reply_Data中，在此调用消息处理
 *
 * @param	huart 传入中断的参数
 * @param	Size  传入中断的参数
 *
 */
void ESP32_RxCpltHandle(UART_HandleTypeDef *huart,uint16_t Size){
	//这里是接收中断内

#if(ATEtoUART1_IQR == 1)
		printf("\r\nRx:InIT ");//将刚收到的发送到串口一
#endif

	if(huart == ESP32_UART.Command_UART){//判断来源

#if(ATEtoUART1_IQR == 1)
		printf("\r\nRx:InU3 ");//将刚收到的发送到串口一
#endif

	//if(huart->Instance == ESP32_UART.Command_UART->Instance){
		if((ESP32_UART.Uart_State != Waiting)&&(ESP32_UART.Uart_State != Free)){
			ESP32_UART.Uart_State=Overload;printf("\r\nRx:Overload\r\n");return;
		}//过载停止接收

		ESP32_UART.Uart_State = Writing;//写入中
		Write_buffer(UART_buffer,Size);	//写入循环缓冲区
		ESP32_UART.Uart_State = Free;	//空闲

		HAL_UARTEx_ReceiveToIdle_DMA(ESP32_UART.Command_UART, (uint8_t*)UART_buffer, UART_Size);//开启接收，末参数为最大长度
		__HAL_DMA_DISABLE_IT(hdma_AT_rx,DMA_IT_HT);//关闭相关DMA接收过半中断


#if(ATEtoUART1 == 1)
		printf("\r\nBuffer:%s|end\r\n",UART_buffer);//打印此次收到内容
#endif
	}
}



/*====================================================初始化==========================================================*/

void ESP32_UART_Init(UART_HandleTypeDef *huartx){
	//绑定通讯口
	ESP32_UART.Command_UART = huartx;

	ESP32_UART.Uart_State	= Free;
	ESP32_UART.Cmd_State	= Success;
	ESP32_UART.overmuch 	= 0;

	ESP32_UART.readIndex = 0;//读写指针归位
	ESP32_UART.writeIndex = 0;

	//开启接收
	HAL_UARTEx_ReceiveToIdle_DMA(ESP32_UART.Command_UART,(uint8_t*)UART_buffer,UART_Size);//开启接收，末参数为最大长度
	__HAL_DMA_DISABLE_IT(hdma_AT_rx,DMA_IT_HT);//关闭相关DMA接收过半中断


	AT_Send("AT+RST");
	char* buffer;
	buffer = ESP32_UART_Checkcmd("ready",2000);
	if( buffer != NULL){//复位成功

#if(ATEtoUART1 == 1)
		printf("\r\nUART_Init:%s Success",buffer);
#endif

	}
	else{
		printf("\r\nUART_Init:InitFail ");
	}

	if(ESP32_SendANDCheck(200,"OK","ATE0"))
	{
		printf("\r\nUART_Init:ATE0_Fail ");
	}
	else{
		printf("\r\nUART_Init:ATE0_Success ");
	}

}


