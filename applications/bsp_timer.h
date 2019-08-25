#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
#include "stm32f1xx_hal.h"	
extern TIM_HandleTypeDef TIM5_Handler;      //¶¨Ê±Æ÷¾ä±ú 

void TIM5_Init(uint16_t arr,uint16_t psc);
#endif

