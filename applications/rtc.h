#ifndef __RTC_H
#define __RTC_H	  
#include "sys.h"
#include "stm32f1xx_hal.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������V3
//RTC��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2017/5/27
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

extern RTC_HandleTypeDef RTC_Handler;  //RTC���
												    
//ʱ��ṹ��
typedef struct 
{
	volatile uint8_t hour;
	volatile uint8_t min;
	volatile uint8_t sec;			
	//������������
	volatile uint16_t w_year;
	volatile uint8_t  w_month;
	volatile uint8_t  w_date;
	volatile uint8_t  week;	
}_calendar_obj;					 
extern _calendar_obj calendar;				    //�����ṹ��
void clock_demo(void);
int RTC_Init(void);        					//��ʼ��RTC,����0,ʧ��;1,�ɹ�;
uint8_t Is_Leap_Year(uint16_t year);			//ƽ��,�����ж�
uint8_t RTC_Get(void);         					//��ȡʱ��   
uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day);
uint8_t RTC_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec);		//����ʱ��	
uint8_t RTC_Alarm_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec);	//��������	
void Set_Time(void);
extern void Delay_ms(uint16_t ms);
#endif



















