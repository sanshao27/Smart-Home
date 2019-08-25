#ifndef __BSP_LED_H
#define __BSP_LED_H


/* ----------------------------------Includes --------------------------------*/
#include "stm32f1xx_hal.h"
#include "public.h"

//宏定义
#define GREEN_LED PEout(5)
#define RED_LED   PBout(5)
#define BEDROOM_LIGHT RED_LED 
#define LIVINGROOM_LIGHT RED_LED 
#define CORRIDOR_LIGHT GREEN_LED 
//函数声明
int LED_Init(void);
void Led_Demo(void *paramet);;
void LED_Flash(void *cnt);
void Light_Control(uint8_t val); 
#endif

