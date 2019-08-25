#include "usart.h"
#include "rtc.h" 	
#include "lcd.h"
#include "touch.h" 
#include "bsp_led.h"
#include <rtthread.h>
#include <rthw.h>
struct time
	{
		uint16_t year;
		uint8_t mon;
		uint8_t week;
		uint8_t day;
		uint8_t hour;
		uint8_t min;
		uint8_t sec;
	};
struct time current_time ={0,0,2,0,0,0,0};
struct time Set_time ={0,0,2,0,0,0,0};
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

RTC_HandleTypeDef RTC_Handler;  //RTC���

_calendar_obj calendar;//ʱ�ӽṹ�� 

//ʵʱʱ������
//��ʼ��RTCʱ��,ͬʱ���ʱ���Ƿ�������
//BKP->DR1���ڱ����Ƿ��һ�����õ�����
//����0:����
//����:�������
int RTC_Init(void)
{
	RTC_Handler.Instance=RTC; 
	RTC_Handler.Init.AsynchPrediv=32767; 	//ʱ����������(�д��۲�,���Ƿ�������?)����ֵ��32767	
	if(HAL_RTC_Init(&RTC_Handler)!=HAL_OK) return 1;
	
	if(HAL_RTCEx_BKUPRead(&RTC_Handler,RTC_BKP_DR1)!=0X5050)//�Ƿ��һ������
	{
		RTC_Set(2017,5,27,17,7,0); //�������ں�ʱ�䣬2017��5��27�գ�17��02��0��		 									  
		HAL_RTCEx_BKUPWrite(&RTC_Handler,RTC_BKP_DR1,0X5050);//����Ѿ���ʼ������
	 	printf("FIRST TIME\n");
	}
	
	__HAL_RTC_ALARM_ENABLE_IT(&RTC_Handler,RTC_IT_SEC); 	//�������ж�
	__HAL_RTC_ALARM_ENABLE_IT(&RTC_Handler,RTC_IT_ALRA); 	//���������ж�
    HAL_NVIC_SetPriority(RTC_IRQn,0x01,0x02); 				//��ռ���ȼ�1,�����ȼ�2
    HAL_NVIC_EnableIRQ(RTC_IRQn);	 
	RTC_Get();//����ʱ�� 
  return 0;
}
//INIT_BOARD_EXPORT(RTC_Init);

//RTC�ײ�������ʱ������
//�˺����ᱻHAL_RTC_Init()����
//hrtc:RTC���
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    __HAL_RCC_PWR_CLK_ENABLE();	//ʹ�ܵ�Դʱ��PWR
	HAL_PWR_EnableBkUpAccess();	//ȡ����������д����
	__HAL_RCC_BKP_CLK_ENABLE();	//ʹ��BSPʱ��
	
	RCC_OscInitStruct.OscillatorType=RCC_OSCILLATORTYPE_LSE;//LSE����
    RCC_OscInitStruct.PLL.PLLState=RCC_PLL_NONE;
    RCC_OscInitStruct.LSEState=RCC_LSE_ON;                  //RTCʹ��LSE
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    PeriphClkInitStruct.PeriphClockSelection=RCC_PERIPHCLK_RTC;//����ΪRTC
    PeriphClkInitStruct.RTCClockSelection=RCC_RTCCLKSOURCE_LSE;//RTCʱ��ԴΪLSE
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
        
    __HAL_RCC_RTC_ENABLE();//RTCʱ��ʹ��
}

//RTCʱ���ж�
//ÿ�봥��һ��  	 
void RTC_IRQHandler(void)
{		 
	if(__HAL_RTC_ALARM_GET_FLAG(&RTC_Handler,RTC_FLAG_SEC)!=RESET)  //���ж�
	{
	 	__HAL_RTC_ALARM_CLEAR_FLAG(&RTC_Handler,RTC_FLAG_SEC); 		//������ж�
		RTC_Get();				//����ʱ��   
		clock_demo();		    //��ʱ����ʾ����Ļ										
	}		

	if(__HAL_RTC_ALARM_GET_FLAG(&RTC_Handler,RTC_FLAG_SEC)!=RESET)  //�����ж�
	{
		__HAL_RTC_ALARM_CLEAR_FLAG(&RTC_Handler,RTC_FLAG_ALRAF); 	//��������ж�   
		RTC_Get();				//����ʱ�� 
        clock_demo();		
		printf("ALARM A!\r\n");
	}	
	__HAL_RTC_ALARM_CLEAR_FLAG(&RTC_Handler,RTC_FLAG_OW); 		//������  	    						 	   	 
}

//�ж��Ƿ������꺯��
//�·�   1  2  3  4  5  6  7  8  9  10 11 12
//����   31 29 31 30 31 30 31 31 30 31 30 31
//������ 31 28 31 30 31 30 31 31 30 31 30 31
//year:���
//����ֵ:������ǲ�������.1,��.0,����
uint8_t Is_Leap_Year(uint16_t year)
{			  
	if(year%4==0) //�����ܱ�4����
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//�����00��β,��Ҫ�ܱ�400���� 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	

//����ʱ��
//�������ʱ��ת��Ϊ����
//��1970��1��1��Ϊ��׼
//1970~2099��Ϊ�Ϸ����
//����ֵ:0,�ɹ�;����:�������.
//�·����ݱ�											 
uint8_t const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //���������ݱ�	  
//ƽ����·����ڱ�
const uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
//syear,smon,sday,hour,min,sec��������ʱ����
//����ֵ�����ý����0���ɹ���1��ʧ�ܡ�
uint8_t RTC_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t t;
	uint32_t seccount=0;
	//RTC_DateTypeDef RTC_DateStructure;
	//RTC_TimeTypeDef RTC_TimeStructure;
	
	if(syear<1970||syear>2099)return 1;	   
	for(t=1970;t<syear;t++)	//��������ݵ��������
	{
		if(Is_Leap_Year(t))seccount+=31622400;//�����������
		else seccount+=31536000;			  //ƽ���������
	}
	smon-=1;
	for(t=0;t<smon;t++)	   //��ǰ���·ݵ����������
	{
		seccount+=(uint32_t)mon_table[t]*86400;//�·����������
		if(Is_Leap_Year(syear)&&t==1)seccount+=86400;//����2�·�����һ���������	   
	}
	seccount+=(uint32_t)(sday-1)*86400;//��ǰ�����ڵ���������� 
	seccount+=(uint32_t)hour*3600;//Сʱ������
    seccount+=(uint32_t)min*60;	 //����������
	seccount+=sec;//�������Ӽ���ȥ
	
//	RTC_DateStructure.Year=syear;
//	RTC_DateStructure.Month=smon;
//	RTC_DateStructure.Date=sday;
//	HAL_RTC_SetDate(&RTC_Handler,&RTC_DateStructure,RTC_FORMAT_BIN);
//	
//	RTC_TimeStructure.Hours=hour;
//	RTC_TimeStructure.Minutes=min;
//	RTC_TimeStructure.Seconds=sec;
//	HAL_RTC_SetTime(&RTC_Handler,&RTC_TimeStructure,RTC_FORMAT_BIN);
	

	//����ʱ��
    RCC->APB1ENR|=1<<28;//ʹ�ܵ�Դʱ��
    RCC->APB1ENR|=1<<27;//ʹ�ܱ���ʱ��
	PWR->CR|=1<<8;    //ȡ��������д����
	//���������Ǳ����!
	RTC->CRL|=1<<4;   //�������� 
	RTC->CNTL=seccount&0xffff;
	RTC->CNTH=seccount>>16;
	RTC->CRL&=~(1<<4);//���ø���
	while(!(RTC->CRL&(1<<5)));//�ȴ�RTC�Ĵ���������� 
	
	RTC_Get();//������֮�����һ������ 	
	return 0;	    
}
//��ʼ������		  
//��1970��1��1��Ϊ��׼
//1970~2099��Ϊ�Ϸ����
//syear,smon,sday,hour,min,sec�����ӵ�������ʱ����   
//����ֵ:0,�ɹ�;����:�������.
uint8_t RTC_Alarm_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t t;
	uint32_t seccount=0;
	if(syear<1970||syear>2099)return 1;	   
	for(t=1970;t<syear;t++)	//��������ݵ��������
	{
		if(Is_Leap_Year(t))seccount+=31622400;//�����������
		else seccount+=31536000;			  //ƽ���������
	}
	smon-=1;
	for(t=0;t<smon;t++)	   //��ǰ���·ݵ����������
	{
		seccount+=(uint32_t)mon_table[t]*86400;//�·����������
		if(Is_Leap_Year(syear)&&t==1)seccount+=86400;//����2�·�����һ���������	   
	}
	seccount+=(uint32_t)(sday-1)*86400;//��ǰ�����ڵ���������� 
	seccount+=(uint32_t)hour*3600;//Сʱ������
    seccount+=(uint32_t)min*60;	 //����������
	seccount+=sec;//�������Ӽ���ȥ 			    
	//����ʱ��
    RCC->APB1ENR|=1<<28;//ʹ�ܵ�Դʱ��
    RCC->APB1ENR|=1<<27;//ʹ�ܱ���ʱ��
	PWR->CR|=1<<8;    //ȡ��������д����
	//���������Ǳ����!
	RTC->CRL|=1<<4;   //�������� 
	RTC->ALRL=seccount&0xffff;
	RTC->ALRH=seccount>>16;
	RTC->CRL&=~(1<<4);//���ø���
	while(!(RTC->CRL&(1<<5)));//�ȴ�RTC�Ĵ����������  
	return 0;	    
}
//�õ���ǰ��ʱ�䣬���������calendar�ṹ������
//����ֵ:0,�ɹ�;����:�������.
uint8_t RTC_Get(void)
{
	static uint16_t daycnt=0;
	uint32_t timecount=0; 
	uint32_t temp=0;
	uint16_t temp1=0;	  
 	timecount=RTC->CNTH;//�õ��������е�ֵ(������)
	timecount<<=16;
	timecount+=RTC->CNTL;			 

 	temp=timecount/86400;   //�õ�����(��������Ӧ��)
	if(daycnt!=temp)//����һ����
	{	  
		daycnt=temp;
		temp1=1970;	//��1970�꿪ʼ
		while(temp>=365)
		{				 
			if(Is_Leap_Year(temp1))//������
			{
				if(temp>=366)temp-=366;//�����������
				else break;  
			}
			else temp-=365;	  //ƽ�� 
			temp1++;  
		}   
		calendar.w_year=temp1;//�õ����
		temp1=0;
		while(temp>=28)//������һ����
		{
			if(Is_Leap_Year(calendar.w_year)&&temp1==1)//�����ǲ�������/2�·�
			{
				if(temp>=29)temp-=29;//�����������
				else break; 
			}
			else 
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];//ƽ��
				else break;
			}
			temp1++;  
		}
		calendar.w_month=temp1+1;	//�õ��·�
		calendar.w_date=temp+1;  	//�õ����� 
	}
	temp=timecount%86400;     		//�õ�������   	   
	calendar.hour=temp/3600;     	//Сʱ
	calendar.min=(temp%3600)/60; 	//����	
	calendar.sec=(temp%3600)%60; 	//����
	calendar.week=RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);//��ȡ����   
	return 0;
}	 
//������������ڼ�
//��������:���빫�����ڵõ�����(ֻ����1901-2099��)
//year,month,day������������ 
//����ֵ�����ں�																						 
uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day)
{	
	uint16_t temp2;
	uint8_t yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// ���Ϊ21����,�������100  
	if (yearH>19)yearL+=100;
	// ����������ֻ��1900��֮���  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}



//����ȡ����ʱ����ʾ��LCD
void clock_demo(void)
{	
	static char year_mon_dat[10] = {0};
	static char hour_min_sec[8] = {0};
	POINT_COLOR = RED;
	//ˢ��������
	sprintf(year_mon_dat,"%.2d-%.2d-%.2d",calendar.w_year,calendar.w_month,calendar.w_date);
	LCD_ShowString(20,43,200,16,16,year_mon_dat);
	//ˢ��Сʱ�����ӣ���
	sprintf(hour_min_sec,"%.2d:%.2d:%.2d",calendar.hour,calendar.min,calendar.sec);
	LCD_ShowString(25,61,200,16,16,hour_min_sec);
	switch(calendar.week)
		{
			case 0:
				LCD_ShowString(32,78,200,16,16,"Sunday    ");
				break;
			case 1:
				LCD_ShowString(32,78,200,16,16,"Monday    ");
				break;
			case 2:
				LCD_ShowString(26,78,200,16,16,"Tuesday   ");
				break;
			case 3:
				LCD_ShowString(20,78,200,16,16,"Wednesday ");
				break;
			case 4:
				LCD_ShowString(20,78,200,16,16,"Thursday  ");
				break;
			case 5:
				LCD_ShowString(32,78,200,16,16,"Friday    ");
				break;
			case 6:
				LCD_ShowString(23,78,200,16,16,"Saturday  ");
				break;  
		}
}
extern  rt_timer_t timer1_sensor_Status;
//����ʱ��
void Set_Time(void)
{
	//�����������ڴ��sprintfת�����ַ�
	static char set_year_mon_dat[10] = {0};
	static char set_hour_min_sec[8] = {0};
	//������˸��־λ������ʱ������ʱĬ�������˸
	static uint8_t time_flag_flash = 1;
	static uint8_t key_value = 0;
	static uint8_t status = 0;
	static uint16_t set_value = 0;
	//LCD����
	LCD_Clear(WHITE);
	POINT_COLOR =BLACK;
	//��ֹRTC�ж�
	HAL_NVIC_DisableIRQ(RTC_IRQn);
	//������ǰʱ�䵽����ʱ��ṹ�������
	Set_time.year = calendar.w_year;
	Set_time.mon = calendar.w_month;
	Set_time.day = calendar.w_date;
	Set_time.hour = calendar.hour;
	Set_time.min = calendar.min;
	Set_time.sec = calendar.sec;
	Set_time.week = calendar.week;
	
	//��ʾ���ִ�������
	Draw_Number_Pad();
	//��ʾ����ʱ��
	LCD_ShowString(50,0,200,24,24," Set    Time ");
	LCD_DrawLine(0,26, 240, 26);
	sprintf(set_year_mon_dat,"%.2d-%.2d-%.2d",calendar.w_year,calendar.w_month,calendar.w_date);
	LCD_ShowString(60,43,200,24,24,set_year_mon_dat);
	sprintf(set_hour_min_sec,"%.2d:%.2d:%.2d",calendar.hour,calendar.min,calendar.sec);
	LCD_ShowString(65,70,200,24,24,set_hour_min_sec);
	switch(calendar.week)
		{
			case 0:
				LCD_ShowString(62,98,200,24,24,"Sunday   ");
				break;
			case 1:
				LCD_ShowString(62,98,200,24,24,"Monday   ");
				break;
			case 2:
				LCD_ShowString(62,98,200,24,24,"Tuesday  ");
				break;
			case 3:
				LCD_ShowString(62,98,200,24,24,"Wednesday");
				break;
			case 4:
				LCD_ShowString(62,98,200,24,24,"Thursday ");
				break;
			case 5:
				LCD_ShowString(62,98,200,24,24,"Friday   ");
				break;
			case 6:
				LCD_ShowString(62,98,200,24,24,"Saturday ");
				break;  
		}
	while(1)
	{
		//ѭ����ⴥ��״̬
		tp_dev.scan(0); 		 
		if(tp_dev.sta&TP_PRES_DOWN)			//������������
		{	
		 	if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height) //��ⴥ����Χ�Ƿ���Һ����ʾ�ߴ�֮��
			{	
				//�ж�ĳ��ʱ�����Ƿ񱻰���
				if(SET_YEAR)
				{
					 time_flag_flash =1;
					 set_value = 0;
					 Set_time.year = set_value+2000;
				}
				if(SET_MON)
				{
					time_flag_flash = 2;
					GREEN_LED = !GREEN_LED;
					set_value = 0;
					Set_time.mon = set_value;
				}
				if(SET_DAY)
				{
					time_flag_flash = 3;
					GREEN_LED = !GREEN_LED;
					set_value = 0;
					Set_time.day = set_value;
				}
				if(SET_HOUR)
				{
					time_flag_flash = 11;
					GREEN_LED = !GREEN_LED;
					set_value = 0;
					Set_time.hour = set_value;
				}
				if(SET_MIN)
				{
					time_flag_flash = 12;
					GREEN_LED = !GREEN_LED;
					set_value = 0;
					Set_time.min = set_value;
				}
				if(SET_SEC)
				{
					time_flag_flash = 13;
					GREEN_LED = !GREEN_LED;
					set_value = 0;
					Set_time.sec = set_value;
				}
				if(SET_WEEK)
				{
					time_flag_flash = 7;
					GREEN_LED = !GREEN_LED;
					set_value = 0;
					Set_time.week = set_value;
				}
				//�ж�ĳ�������Ƿ񱻰���
				if(ZERO)
				{
					key_value =0;
					switch(time_flag_flash)
					{
						case 1:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>99)
										set_value =0;
									Set_time.year = set_value+2000;
									
									break;
								}
					    case 2:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=13)
										set_value =0;
									Set_time.mon = set_value;
									
									break;
								}
						case 3:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=32)
										set_value =0;
									Set_time.day = set_value;
									
									break;
								}
						case 11:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=24)
										set_value =0;
									Set_time.hour = set_value;
									
									break;
								}
						case 12:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.min = set_value;
									
									break;
								}
						case 13:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.sec = set_value;
									
									break;
								}
						case 7:
								{
									Set_time.week = 0;
								}
						default:break;
					}
				}
				if(ONE)
				{
					key_value =1;
					switch(time_flag_flash)
					{
						case 1:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>99)
										set_value =0;
									Set_time.year = set_value+2000;
									
									break;
								}
					    case 2:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=13)
										set_value =0;
									Set_time.mon = set_value;
									
									break;
								}
						case 3:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=32)
										set_value =0;
									Set_time.day = set_value;
									
									break;
								}
						case 11:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=24)
										set_value =0;
									Set_time.hour = set_value;
									
									break;
								}
						case 12:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.min = set_value;
									
									break;
								}
						case 13:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.sec = set_value;
									
									break;
								}
						case 7:
								{
									Set_time.week = 1;
								}
						default:break;
					}
				}
				if(TWO)
				{
					key_value =2;
					switch(time_flag_flash)
					{
						case 1:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>99)
										set_value =0;
									Set_time.year = set_value+2000;
									
									break;
								}
					    case 2:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=13)
										set_value =0;
									Set_time.mon = set_value;
									
									break;
								}
						case 3:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=32)
										set_value =0;
									Set_time.day = set_value;
									
									break;
								}
						case 11:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=24)
										set_value =0;
									Set_time.hour = set_value;
									
									break;
								}
						case 12:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.min = set_value;
									
									break;
								}
						case 13:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.sec = set_value;
									
									break;
								}
						case 7:
								{
								
									Set_time.week = 2;
								}
						default:break;
					}
				}
				if(THREE)
				{
					key_value =3;
					switch(time_flag_flash)
					{
						case 1:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>99)
										set_value =0;
									Set_time.year = set_value+2000;
									
									break;
								}
					    case 2:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=13)
										set_value =0;
									Set_time.mon = set_value;
									
									break;
								}
						case 3:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=32)
										set_value =0;
									Set_time.day = set_value;
									
									break;
								}
						case 11:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=24)
										set_value =0;
									Set_time.hour = set_value;
									
									break;
								}
						case 12:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.min = set_value;
									
									break;
								}
						case 13:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.sec = set_value;
									
									break;
								}
						case 7:
								{
									
									Set_time.week = 3;
								}
						default:break;
					}
				}
				if(FOUR)
				{
					key_value =4;
					switch(time_flag_flash)
					{
						case 1:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>99)
										set_value =0;
									Set_time.year = set_value+2000;
									
									break;
								}
					    case 2:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=13)
										set_value =0;
									Set_time.mon = set_value;
									
									break;
								}
						case 3:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=32)
										set_value =0;
									Set_time.day = set_value;
									
									break;
								}
						case 11:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=24)
										set_value =0;
									Set_time.hour = set_value;
									
									break;
								}
						case 12:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.min = set_value;
									
									break;
								}
						case 13:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.sec = set_value;
									
									break;
								}
						case 7:
								{
									
									Set_time.week = 4;
								}
						default:break;
					}
				}
				if(FIVE)
				{
					key_value =5;
					switch(time_flag_flash)
					{
						case 1:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>99)
										set_value =0;
									Set_time.year = set_value+2000;
									
									break;
								}
					    case 2:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=13)
										set_value =0;
									Set_time.mon = set_value;
									
									break;
								}
						case 3:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=32)
										set_value =0;
									Set_time.day = set_value;
									
									break;
								}
						case 11:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=24)
										set_value =0;
									Set_time.hour = set_value;
									
									break;
								}
						case 12:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.min = set_value;
									
									break;
								}
						case 13:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.sec = set_value;
									
									break;
								}
						case 7:
								{
									
									Set_time.week = 5;
								}
						default:break;
					}
				}
				if(SIX)
				{
					key_value =6;
					switch(time_flag_flash)
					{
						case 1:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>99)
										set_value =0;
									Set_time.year = set_value+2000;
									
									break;
								}
					    case 2:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=13)
										set_value =0;
									Set_time.mon = set_value;
									
									break;
								}
						case 3:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=32)
										set_value =0;
									Set_time.day = set_value;
									
									break;
								}
						case 11:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=24)
										set_value =0;
									Set_time.hour = set_value;
									
									break;
								}
						case 12:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.min = set_value;
									
									break;
								}
						case 13:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.sec = set_value;
									
									break;
								}
						case 7:
								{
									
									Set_time.week = 6;
								}
						default:break;
					}
				}
				if(SEVEN)
				{
					key_value =7;
					switch(time_flag_flash)
					{
						case 1:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>99)
										set_value =0;
									Set_time.year = set_value+2000;
									
									break;
								}
					    case 2:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=13)
										set_value =0;
									Set_time.mon = set_value;
									
									break;
								}
						case 3:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=32)
										set_value =0;
									Set_time.day = set_value;
									
									break;
								}
						case 11:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=24)
										set_value =0;
									Set_time.hour = set_value;
									
									break;
								}
						case 12:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.min = set_value;
									
									break;
								}
						case 13:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.sec = set_value;
									
									break;
								}
						case 7:
								{
									
									Set_time.week = 0;
									break;
								}
						default:break;
					}
				}
				if(EIGHT)
				{
					key_value =8;
					switch(time_flag_flash)
					{
						case 1:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>99)
										set_value =0;
									Set_time.year = set_value+2000;
									
									break;
								}
					    case 2:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=13)
										set_value =0;
									Set_time.mon = set_value;
									
									break;
								}
						case 3:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=32)
										set_value =0;
									Set_time.day = set_value;
									
									break;
								}
						case 11:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=24)
										set_value =0;
									Set_time.hour = set_value;
									
									break;
								}
						case 12:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.min = set_value;
									
									break;
								}
						case 13:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.sec = set_value;
									
									break;
								}
						case 7:
								{
									Set_time.week = 9;
									POINT_COLOR = RED;
									LCD_ShowString(60,98,200,24,24," Wrong No.!  ");
									break;
								}
						default:break;
					}
				}
				if(NINE)
				{
					key_value =9;
					switch(time_flag_flash)
					{
						case 1:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>99)
										set_value =0;
									Set_time.year = set_value+2000;
									
									break;
								}
					    case 2:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=13)
										set_value =0;
									Set_time.mon = set_value;
									
									break;
								}
						case 3:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=32)
										set_value =0;
									Set_time.day = set_value;
									
									break;
								}
						case 11:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=24)
										set_value =0;
									Set_time.hour = set_value;
									
									break;
								}
						case 12:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.min = set_value;
									
									break;
								}
						case 13:
								{
									set_value = (set_value*10) + key_value;
									if(set_value>=61)
										set_value =0;
									Set_time.sec = set_value;
									
									break;
								}
						case 7:
								{
									Set_time.week = 9;
									POINT_COLOR = RED;
									LCD_ShowString(60,98,200,24,24," Wrong No.!  ");
								}
						default:break;
					}
				}
				if(CANCLE)
				{
					status = 'C';
					goto loop;
				}
				if(OK)
				{
					status = 'E';
					goto loop;
				}
				//������������  
				while(tp_dev.sta&TP_PRES_DOWN)	
				{
					tp_dev.scan(0); 
				}
			}
		}
		//time_flag_flash��־λ��ֵѭ����˸ĳ���趨ֵ
		switch(time_flag_flash)
		{
				case 1://�����˸
						{
							POINT_COLOR = BLACK;
							LCD_ShowString(84,43,200,24,24,"  ");
							Delay_ms(100);
							sprintf(set_year_mon_dat,"%.2d-%.2d-%.2d",Set_time.year,Set_time.mon,Set_time.day);
							LCD_ShowString(60,43,200,24,24,set_year_mon_dat);
							Delay_ms(100);			
							break;
						}
				case 2://�·���˸
						{
							POINT_COLOR = BLACK;
							LCD_ShowString(120,43,200,24,24,"  ");
							Delay_ms(100);
							sprintf(set_year_mon_dat,"%.2d-%.2d-%.2d",Set_time.year,Set_time.mon,Set_time.day);
							LCD_ShowString(60,43,200,24,24,set_year_mon_dat);
							Delay_ms(100);	
						    break;
						}
				case 3://������˸
						{
							POINT_COLOR = BLACK;
							LCD_ShowString(156,43,200,24,24,"  ");
							Delay_ms(100);
							sprintf(set_year_mon_dat,"%.2d-%.2d-%.2d",Set_time.year,Set_time.mon,Set_time.day);
							LCD_ShowString(60,43,200,24,24,set_year_mon_dat);
							Delay_ms(100);	
							break;
						}
				case 11://Сʱ��˸
						{
							POINT_COLOR = BLACK;
							LCD_ShowString(65,70,200,24,24,"  "); 
							Delay_ms(100);
							sprintf(set_hour_min_sec,"%.2d:%.2d:%.2d",Set_time.hour,Set_time.min,Set_time.sec);
	                        LCD_ShowString(65,70,200,24,24,set_hour_min_sec); 
							Delay_ms(100);	
							time_flag_flash = 11;//ȥ�����޷���˸
						    break;
						}
				case 12://������˸
						{
							POINT_COLOR = BLACK;
							LCD_ShowString(101,70,200,24,24,"  "); 
							Delay_ms(100);
							sprintf(set_hour_min_sec,"%.2d:%.2d:%.2d",Set_time.hour,Set_time.min,Set_time.sec);
	                        LCD_ShowString(65,70,200,24,24,set_hour_min_sec); 
							Delay_ms(100);	 
							time_flag_flash = 12;//ȥ�����޷���˸
						    break;
						}
				case 13://����˸
						{
							POINT_COLOR = BLACK;
							LCD_ShowString(137,70,200,24,24,"  "); 
							Delay_ms(100);
							sprintf(set_hour_min_sec,"%.2d:%.2d:%.2d",Set_time.hour,Set_time.min,Set_time.sec);
	                        LCD_ShowString(65,70,200,24,24,set_hour_min_sec); 
							Delay_ms(100);
							time_flag_flash = 13;//ȥ�����޷���˸							
						    break;
						}
				case 7://������˸
						{
							POINT_COLOR = BLACK;
							switch(Set_time.week)
							{
								case 0:
										{
											LCD_ShowString(60,98,200,24,24,"              "); 
											Delay_ms(100);
											LCD_ShowString(62,98,200,24,24," Sunday       ");
											Delay_ms(100);
											break;
										}
								case 1:
									{
											LCD_ShowString(60,98,200,24,24,"             "); 
											Delay_ms(100);
											LCD_ShowString(62,98,200,24,24," Monday      ");
											Delay_ms(100);
											break;
									}
								case 2:
									{
											LCD_ShowString(60,98,200,24,24,"             "); 
											Delay_ms(100);
											LCD_ShowString(62,98,200,24,24," Tuesday     ");
											Delay_ms(100);
											break;
									}
								case 3:
									{
											LCD_ShowString(62,98,200,24,24,"             "); 
											Delay_ms(100);
											LCD_ShowString(62,98,200,24,24,"Wednesday    ");
											Delay_ms(100);
											break;
									}
								case 4:
									{
											LCD_ShowString(62,98,200,24,24,"            "); 
											Delay_ms(100);
											LCD_ShowString(62,98,200,24,24,"Thursday    ");
											Delay_ms(100);
											break;
									}
								case 5:
									{
											LCD_ShowString(60,98,200,24,24,"           "); 
											Delay_ms(100);
											LCD_ShowString(62,98,200,24,24," Friday    ");
											Delay_ms(100);
											break;
									}
								case 6:
									{
											LCD_ShowString(62,98,200,24,24,"           "); 
											Delay_ms(100);
											LCD_ShowString(62,98,200,24,24,"Saturday   ");
											Delay_ms(100);
											break;
									} 
							}
							break;
						}
				default: break;
		}
	}
	loop: 
	//ȡ�����˳�ʱ������
	if(status == 'C')
	  {
		    LCD_Clear(WHITE);
		    Draw_window();
		  	HAL_NVIC_EnableIRQ(RTC_IRQn);
		    //rt_timer_start(timer1_sensor_Status);
		    GREEN_LED = 1;
	  }
	//������ɽ�ʱ��д��RTC
	if(status == 'E')
	  {
		LCD_Clear(WHITE);
		
        //�����趨��ʱ�䵽RTC
		RTC_Set(Set_time.year,Set_time.mon,Set_time.day,Set_time.hour,Set_time.min,Set_time.sec);
		//����Ѿ���ʼ������ʱ��
		HAL_RTCEx_BKUPWrite(&RTC_Handler,RTC_BKP_DR1,0X5050);
		
		//��ʾ������
		Draw_window();
		//ʹ��RTC���ж�
		HAL_NVIC_EnableIRQ(RTC_IRQn);
		//�������������ʱ��
		//rt_timer_start(timer1_sensor_Status);
		GREEN_LED = 1;
	  }
}








