#ifndef __RTC_H
#define __RTC_H	  
#include "sys.h"
#include "stm32f1xx_hal.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板V3
//RTC驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2017/5/27
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

extern RTC_HandleTypeDef RTC_Handler;  //RTC句柄
												    
//时间结构体
typedef struct 
{
	volatile uint8_t hour;
	volatile uint8_t min;
	volatile uint8_t sec;			
	//公历日月年周
	volatile uint16_t w_year;
	volatile uint8_t  w_month;
	volatile uint8_t  w_date;
	volatile uint8_t  week;	
}_calendar_obj;					 
extern _calendar_obj calendar;				    //日历结构体
void clock_demo(void);
int RTC_Init(void);        					//初始化RTC,返回0,失败;1,成功;
uint8_t Is_Leap_Year(uint16_t year);			//平年,闰年判断
uint8_t RTC_Get(void);         					//获取时间   
uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day);
uint8_t RTC_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec);		//设置时间	
uint8_t RTC_Alarm_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec);	//设置闹钟	
void Set_Time(void);
extern void Delay_ms(uint16_t ms);
#endif



















