/*
 * ESP32_link.c
 *
 *  Created on: May 24, 2025
 *      Author: 12114
 */

#include "ESP32_UART.h"
#include "ESP32_WiFi.h"
#include "ESP32_MQTT.h"

extern	DMA_HandleTypeDef	hdma_usart3_rx;
DMA_HandleTypeDef*			hdma_AT_rx	= &hdma_usart3_rx;

#define UART_Size			512
uint8_t UART_buffer[UART_Size] =  {0};//串口接收缓存


/*
  有没有一种可能会使用多个WiFi模块？那么所有函数还得加个入参？
  我觉得一个应该够了
 */
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


/*	@brief			向缓冲区写入数据(自动置writeIndex大小)
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


/*	@brief	在窗口时间里等待并且持续清空loopbuffer
 *
 */
void Clear_loopbuffer(uint16_t waittime){
	//判断缓冲区Reply_Data是否有残留数据
	uint32_t start_time = 0;
	uint16_t Size = 0;
	uint8_t data_found = 0;

	do {
	    data_found = 0; // 重置标志位
	    // 在500ms窗口期内持续检测
	    while (HAL_GetTick() - start_time <= waittime) {

	        if ( (Size = Get_UNhandled()) ) {
#if(Clear_UNhandled == 1)
	            printf("\r\nClear:UNhandled %d",Size);
#endif
	            Add_ReadIndex(Size); // 清空残留数据
	            data_found = 1;   	 // 标记数据残留，进行循环
	            start_time = HAL_GetTick(); // 重置超时计时
#if(Clear_cleared == 1)
	            printf("\r\nClear:cleared %d",Size);
#endif
	        }
	        HAL_Delay(50);
	    }
	    // 若500ms内无数据，退出循环
	} while (data_found);
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

#if(AT_Send_CMD == 1)
		printf("\r\n\r\nSendCMD:%s ",Command);
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

#if(AT_Send_CMD == 1)
		printf("\r\nSendCMD:%s ",buffer);
#endif
#if(ATEtoUART1_IQR == 1)
		printf("\r\nTx:Sending ");
#endif

		//发送命令(非阻塞)
		HAL_UART_Transmit_DMA(ESP32_UART.Command_UART, (uint8_t*)buffer, length);
}


/* @brief	发送16进制数组
 * @param	Command 数组
 *
 */
void AT_Send_HEX(uint8_t* buffer){

	uint16_t Size = sizeof(buffer);

	ESP32_UART.Uart_State = Sending;//状态置位

#if(AT_Send_CMD == 1)
	printf("\r\nSendHEX:%d %d...%d %d ",buffer[0],buffer[1],buffer[Size-2],buffer[Size-1]);
#endif
#if(ATEtoUART1_IQR == 1)
	printf("\r\nTx:Sending ");
#endif

	//发送命令(非阻塞)
	HAL_UART_Transmit_DMA(ESP32_UART.Command_UART, buffer, Size);
}


/* @brief	检查返回消息判断命令执行情况，
 * 			函数在本次指令响应接收字符串 cmd_buffer 中查找第一次出现 期望答复(str不包含空结束字符）的位置
 * 			没有找到等待下一次响应，直到重试次数用完或者接收到期望答复(str)，成功后此时strx指向cmd_buffer中
 * 			期望答复(str)的开头,并在str长度后截断剩下的所有响应,可以用extend延后截断位置,让返回指针能够访问
 *
 * @param	str 		应该收到的内容
 * @param	waittime 	等待时间
 * @param	extend 		是否延长，一般为0也就是只返回期望答复的字符串地址，如果答复后有要访问的数据用此参数延长
 *
 * @return	返回查找到的期望答复的地址
 *
 */
char* ESP32_UART_Checkcmd(char *ack, uint32_t waittime,uint8_t extend){
	static uint8_t cmd_buffer[512];//这是所有消息公用的最后的存储区
	uint32_t Time=0;//上次收到消息时间
	char *strx = NULL;
	uint16_t Size=0;
	uint8_t MAX_Cmd = 15;
	uint8_t length = strlen(ack);

	length += extend;

	while(MAX_Cmd){//最多接收MAX_Cmd条指令响应
		Time = HAL_GetTick();//获取本轮指令等待开始时间
		while((HAL_GetTick()-Time) <= waittime){
			if( (Size = Get_UNhandled()) ){//在超时时间内判断有没有消息(缓冲区是否为空)

				/*有回复消息*/
				ESP32_UART.Cmd_State = CMDHandle;
				for(uint16_t i=0; i < Size; i++){//使用Read_buffer读出
					cmd_buffer[i] = Read_buffer(ESP32_UART.readIndex + i);
				}
				Add_ReadIndex(Size);//增加读索引(已读出判断)

#if(Checkcmd_RIndx == 1)
				printf("\r\nCheckcmd:RIndx %d ",ESP32_UART.readIndex);
#endif

				/*开始判断*/
				strx = strstr((char*)cmd_buffer,ack);

				//添加结束符
				if (strx != NULL && (strx+length+1 - (char*)cmd_buffer) < sizeof(cmd_buffer)){
				    strx[length] = '\0';
					ESP32_UART.Cmd_State = Success;

#if(Checkcmd_buffer == 1)
				printf("\r\nCheckcmd:buffer|%s|end",cmd_buffer);//拷贝过来的回复消息
#endif
#if(Checkcmd_ack == 1)
				printf("\r\nCheckcmd:ack|%s|end",ack);		//返回的指针指向
#endif
					return strx;

				} else {//没找到等待下一次缓冲区不为空(接收到响应)
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
uint8_t ESP32_SendANDCheck(uint32_t waittime,  char *espack, const char* __restrict__ Command, ...){
	char buffer[1024];  //缓冲区大小(至少要装下网站的token)
	va_list args;
	va_start(args, Command);
	vsnprintf(buffer, sizeof(buffer), Command, args);
	va_end(args);

	uint16_t length = strlen(buffer);
	uint16_t buf_length = sizeof(buffer);

	Clear_loopbuffer(500);//清空消息缓冲区

	AT_Send_callee(buffer,length,buf_length);//发送

	char* xstr;
	xstr = ESP32_UART_Checkcmd(espack,waittime,0);
	if( xstr != NULL){

#if(SendANDCheck_ack == 1)
		printf("\r\nsADNc:ack|%s|end",xstr);//成功
#endif

		return 0;
	}
	else{

#if(SendANDCheck_ack == 1)
		printf("\r\nsADNc:ack NULL");//失败
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

	Clear_loopbuffer(500);//清空消息缓冲区

	AT_Send(cmd);
	char* strx;
	strx = ESP32_UART_Checkcmd(ack,waittime,0);
	if( strx != NULL){//成功
#if(SendANDCheck_ack == 1)
		printf("\r\nsADNc_:ack|%s|end",strx);
#endif
		return 0;
	}
	else{
#if(SendANDCheck_NULL == 1)
		printf("\r\nsADNc_:ack NULL");
#endif
		return 1;
	}
}

/*====================================================中断相关==========================================================*/

/*	ESP32发送完成处理,放在 HAL_UART_TxCpltCallback 发送完成回调 函数里
 */
void ESP32_TxCpltHandle(UART_HandleTypeDef *huart){
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

		if((ESP32_UART.Uart_State != Waiting)&&(ESP32_UART.Uart_State != Free)){
			ESP32_UART.Uart_State=Overload;printf("\r\nRx:Overload\r\n");return;
		}//过载停止接收

		ESP32_UART.Uart_State = Writing;//写入中
		Write_buffer(UART_buffer,Size);	//写入循环缓冲区
		ESP32_UART.Uart_State = Free;	//空闲

		HAL_UARTEx_ReceiveToIdle_DMA(ESP32_UART.Command_UART, (uint8_t*)UART_buffer, UART_Size);//开启接收，末参数为最大长度
		__HAL_DMA_DISABLE_IT(hdma_AT_rx,DMA_IT_HT);//关闭相关DMA接收过半中断

#if(ATEtoUART1 == 1)
		printf("\r\nRx:buf|%s|end",UART_buffer);//打印此次收到内容(耗时太长)
#endif
	}
}



/*====================================================初始化==========================================================*/

uint8_t ESP32_UART_Init(UART_HandleTypeDef *huartx){
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
	char* strx;
	strx = ESP32_UART_Checkcmd("ready",2000,0);
	if( strx != NULL){//复位成功

#if(UART_Init == 1)
		printf("\r\nUART_Init:%s Success",strx);
#endif

	}
	else{

		ESP32_WiFi.WiFi_state = StateERR;
		ESP32_MQTT.MQTT_state = MQTTERR;

#if(UART_Init == 1)
		printf("\r\nUART_Init:ready Fail ");
#endif

		return 1;
	}

	if(!ESP32_SendANDCheck(200,"OK","ATE0"))
	{
#if(UART_Init == 1)
		printf("\r\nUART_Init:ATE0_Success");
#endif
	}
	else{
#if(UART_Init == 1)
		printf("\r\nUART_Init:ATE0_Fail");
#endif
	}

	return 0;
}


