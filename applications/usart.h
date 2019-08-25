#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
#include "stm32f1xx_hal.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//串口1初始化		   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/8/18
//版本：V1.5
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved
//********************************************************************************
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持

//公共参数
#define RXBUFFERSIZE            1 			//缓存大小
#define USART_REC_LEN  			1024  		//定义最大接收字节数 2048

//串口1相关参数
#define EN_USART1_RX 			1			    //使能（1）/禁止（0）串口1接收
extern  uint8_t  USART_RX_BUF[USART_REC_LEN]; 	//接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern  uint16_t USART_RX_STA;         			//接收状态标记	
extern  UART_HandleTypeDef UART1_Handler; 	    //UART1句柄
extern  uint8_t aRxBuffer[RXBUFFERSIZE];			//HAL库USART接收Buffer

//串口2相关参数
#define EN_USART2_RX 			1	
extern  uint8_t  USART_RX2_BUF[USART_REC_LEN]; 	//接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern  uint16_t USART_RX2_STA;         		//接收状态标记	
extern  UART_HandleTypeDef UART2_Handler; 	    //UART3句柄
extern  uint8_t aRx2Buffer[RXBUFFERSIZE];		//HAL库USART接收Buffer



//如果想串口中断接收，请不要注释以下宏定义
extern void Delay_ms(uint16_t ms);
void uart1_init(uint32_t bound);
void uart2_init(uint32_t bound);
int8_t usart2_recvData(uint8_t *str);
void usart2_senddata(char* c);
#endif
