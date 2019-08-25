#ifndef __SPI_H
#define __SPI_H
#include "sys.h"

#include "stm32f1xx_hal.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32F103开发板
//SPI驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2017/5/30
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

extern SPI_HandleTypeDef SPI2_Handler;  //SPI句柄

void SPI2_Init(void);
void SPI2_SetSpeed(uint8_t SPI_BaudRatePrescaler);
uint8_t SPI2_ReadWriteByte(uint8_t TxData);
#endif
