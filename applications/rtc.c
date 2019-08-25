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

RTC_HandleTypeDef RTC_Handler;  //RTC句柄

_calendar_obj calendar;//时钟结构体 

//实时时钟配置
//初始化RTC时钟,同时检测时钟是否工作正常
//BKP->DR1用于保存是否第一次配置的设置
//返回0:正常
//其他:错误代码
int RTC_Init(void)
{
	RTC_Handler.Instance=RTC; 
	RTC_Handler.Init.AsynchPrediv=32767; 	//时钟周期设置(有待观察,看是否跑慢了?)理论值：32767	
	if(HAL_RTC_Init(&RTC_Handler)!=HAL_OK) return 1;
	
	if(HAL_RTCEx_BKUPRead(&RTC_Handler,RTC_BKP_DR1)!=0X5050)//是否第一次配置
	{
		RTC_Set(2017,5,27,17,7,0); //设置日期和时间，2017年5月27日，17点02分0秒		 									  
		HAL_RTCEx_BKUPWrite(&RTC_Handler,RTC_BKP_DR1,0X5050);//标记已经初始化过了
	 	printf("FIRST TIME\n");
	}
	
	__HAL_RTC_ALARM_ENABLE_IT(&RTC_Handler,RTC_IT_SEC); 	//允许秒中断
	__HAL_RTC_ALARM_ENABLE_IT(&RTC_Handler,RTC_IT_ALRA); 	//允许闹钟中断
    HAL_NVIC_SetPriority(RTC_IRQn,0x01,0x02); 				//抢占优先级1,子优先级2
    HAL_NVIC_EnableIRQ(RTC_IRQn);	 
	RTC_Get();//更新时间 
  return 0;
}
//INIT_BOARD_EXPORT(RTC_Init);

//RTC底层驱动，时钟配置
//此函数会被HAL_RTC_Init()调用
//hrtc:RTC句柄
void HAL_RTC_MspInit(RTC_HandleTypeDef* hrtc)
{
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

    __HAL_RCC_PWR_CLK_ENABLE();	//使能电源时钟PWR
	HAL_PWR_EnableBkUpAccess();	//取消备份区域写保护
	__HAL_RCC_BKP_CLK_ENABLE();	//使能BSP时钟
	
	RCC_OscInitStruct.OscillatorType=RCC_OSCILLATORTYPE_LSE;//LSE配置
    RCC_OscInitStruct.PLL.PLLState=RCC_PLL_NONE;
    RCC_OscInitStruct.LSEState=RCC_LSE_ON;                  //RTC使用LSE
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    PeriphClkInitStruct.PeriphClockSelection=RCC_PERIPHCLK_RTC;//外设为RTC
    PeriphClkInitStruct.RTCClockSelection=RCC_RTCCLKSOURCE_LSE;//RTC时钟源为LSE
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
        
    __HAL_RCC_RTC_ENABLE();//RTC时钟使能
}

//RTC时钟中断
//每秒触发一次  	 
void RTC_IRQHandler(void)
{		 
	if(__HAL_RTC_ALARM_GET_FLAG(&RTC_Handler,RTC_FLAG_SEC)!=RESET)  //秒中断
	{
	 	__HAL_RTC_ALARM_CLEAR_FLAG(&RTC_Handler,RTC_FLAG_SEC); 		//清除秒中断
		RTC_Get();				//更新时间   
		clock_demo();		    //将时间显示到屏幕										
	}		

	if(__HAL_RTC_ALARM_GET_FLAG(&RTC_Handler,RTC_FLAG_SEC)!=RESET)  //闹钟中断
	{
		__HAL_RTC_ALARM_CLEAR_FLAG(&RTC_Handler,RTC_FLAG_ALRAF); 	//清除闹钟中断   
		RTC_Get();				//更新时间 
        clock_demo();		
		printf("ALARM A!\r\n");
	}	
	__HAL_RTC_ALARM_CLEAR_FLAG(&RTC_Handler,RTC_FLAG_OW); 		//清除溢出  	    						 	   	 
}

//判断是否是闰年函数
//月份   1  2  3  4  5  6  7  8  9  10 11 12
//闰年   31 29 31 30 31 30 31 31 30 31 30 31
//非闰年 31 28 31 30 31 30 31 31 30 31 30 31
//year:年份
//返回值:该年份是不是闰年.1,是.0,不是
uint8_t Is_Leap_Year(uint16_t year)
{			  
	if(year%4==0) //必须能被4整除
	{ 
		if(year%100==0) 
		{ 
			if(year%400==0)return 1;//如果以00结尾,还要能被400整除 	   
			else return 0;   
		}else return 1;   
	}else return 0;	
}	

//设置时钟
//把输入的时钟转换为秒钟
//以1970年1月1日为基准
//1970~2099年为合法年份
//返回值:0,成功;其他:错误代码.
//月份数据表											 
uint8_t const table_week[12]={0,3,3,6,1,4,6,2,5,0,3,5}; //月修正数据表	  
//平年的月份日期表
const uint8_t mon_table[12]={31,28,31,30,31,30,31,31,30,31,30,31};
//syear,smon,sday,hour,min,sec：年月日时分秒
//返回值：设置结果。0，成功；1，失败。
uint8_t RTC_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t t;
	uint32_t seccount=0;
	//RTC_DateTypeDef RTC_DateStructure;
	//RTC_TimeTypeDef RTC_TimeStructure;
	
	if(syear<1970||syear>2099)return 1;	   
	for(t=1970;t<syear;t++)	//把所有年份的秒钟相加
	{
		if(Is_Leap_Year(t))seccount+=31622400;//闰年的秒钟数
		else seccount+=31536000;			  //平年的秒钟数
	}
	smon-=1;
	for(t=0;t<smon;t++)	   //把前面月份的秒钟数相加
	{
		seccount+=(uint32_t)mon_table[t]*86400;//月份秒钟数相加
		if(Is_Leap_Year(syear)&&t==1)seccount+=86400;//闰年2月份增加一天的秒钟数	   
	}
	seccount+=(uint32_t)(sday-1)*86400;//把前面日期的秒钟数相加 
	seccount+=(uint32_t)hour*3600;//小时秒钟数
    seccount+=(uint32_t)min*60;	 //分钟秒钟数
	seccount+=sec;//最后的秒钟加上去
	
//	RTC_DateStructure.Year=syear;
//	RTC_DateStructure.Month=smon;
//	RTC_DateStructure.Date=sday;
//	HAL_RTC_SetDate(&RTC_Handler,&RTC_DateStructure,RTC_FORMAT_BIN);
//	
//	RTC_TimeStructure.Hours=hour;
//	RTC_TimeStructure.Minutes=min;
//	RTC_TimeStructure.Seconds=sec;
//	HAL_RTC_SetTime(&RTC_Handler,&RTC_TimeStructure,RTC_FORMAT_BIN);
	

	//设置时钟
    RCC->APB1ENR|=1<<28;//使能电源时钟
    RCC->APB1ENR|=1<<27;//使能备份时钟
	PWR->CR|=1<<8;    //取消备份区写保护
	//上面三步是必须的!
	RTC->CRL|=1<<4;   //允许配置 
	RTC->CNTL=seccount&0xffff;
	RTC->CNTH=seccount>>16;
	RTC->CRL&=~(1<<4);//配置更新
	while(!(RTC->CRL&(1<<5)));//等待RTC寄存器操作完成 
	
	RTC_Get();//设置完之后更新一下数据 	
	return 0;	    
}
//初始化闹钟		  
//以1970年1月1日为基准
//1970~2099年为合法年份
//syear,smon,sday,hour,min,sec：闹钟的年月日时分秒   
//返回值:0,成功;其他:错误代码.
uint8_t RTC_Alarm_Set(uint16_t syear,uint8_t smon,uint8_t sday,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint16_t t;
	uint32_t seccount=0;
	if(syear<1970||syear>2099)return 1;	   
	for(t=1970;t<syear;t++)	//把所有年份的秒钟相加
	{
		if(Is_Leap_Year(t))seccount+=31622400;//闰年的秒钟数
		else seccount+=31536000;			  //平年的秒钟数
	}
	smon-=1;
	for(t=0;t<smon;t++)	   //把前面月份的秒钟数相加
	{
		seccount+=(uint32_t)mon_table[t]*86400;//月份秒钟数相加
		if(Is_Leap_Year(syear)&&t==1)seccount+=86400;//闰年2月份增加一天的秒钟数	   
	}
	seccount+=(uint32_t)(sday-1)*86400;//把前面日期的秒钟数相加 
	seccount+=(uint32_t)hour*3600;//小时秒钟数
    seccount+=(uint32_t)min*60;	 //分钟秒钟数
	seccount+=sec;//最后的秒钟加上去 			    
	//设置时钟
    RCC->APB1ENR|=1<<28;//使能电源时钟
    RCC->APB1ENR|=1<<27;//使能备份时钟
	PWR->CR|=1<<8;    //取消备份区写保护
	//上面三步是必须的!
	RTC->CRL|=1<<4;   //允许配置 
	RTC->ALRL=seccount&0xffff;
	RTC->ALRH=seccount>>16;
	RTC->CRL&=~(1<<4);//配置更新
	while(!(RTC->CRL&(1<<5)));//等待RTC寄存器操作完成  
	return 0;	    
}
//得到当前的时间，结果保存在calendar结构体里面
//返回值:0,成功;其他:错误代码.
uint8_t RTC_Get(void)
{
	static uint16_t daycnt=0;
	uint32_t timecount=0; 
	uint32_t temp=0;
	uint16_t temp1=0;	  
 	timecount=RTC->CNTH;//得到计数器中的值(秒钟数)
	timecount<<=16;
	timecount+=RTC->CNTL;			 

 	temp=timecount/86400;   //得到天数(秒钟数对应的)
	if(daycnt!=temp)//超过一天了
	{	  
		daycnt=temp;
		temp1=1970;	//从1970年开始
		while(temp>=365)
		{				 
			if(Is_Leap_Year(temp1))//是闰年
			{
				if(temp>=366)temp-=366;//闰年的秒钟数
				else break;  
			}
			else temp-=365;	  //平年 
			temp1++;  
		}   
		calendar.w_year=temp1;//得到年份
		temp1=0;
		while(temp>=28)//超过了一个月
		{
			if(Is_Leap_Year(calendar.w_year)&&temp1==1)//当年是不是闰年/2月份
			{
				if(temp>=29)temp-=29;//闰年的秒钟数
				else break; 
			}
			else 
			{
				if(temp>=mon_table[temp1])temp-=mon_table[temp1];//平年
				else break;
			}
			temp1++;  
		}
		calendar.w_month=temp1+1;	//得到月份
		calendar.w_date=temp+1;  	//得到日期 
	}
	temp=timecount%86400;     		//得到秒钟数   	   
	calendar.hour=temp/3600;     	//小时
	calendar.min=(temp%3600)/60; 	//分钟	
	calendar.sec=(temp%3600)%60; 	//秒钟
	calendar.week=RTC_Get_Week(calendar.w_year,calendar.w_month,calendar.w_date);//获取星期   
	return 0;
}	 
//获得现在是星期几
//功能描述:输入公历日期得到星期(只允许1901-2099年)
//year,month,day：公历年月日 
//返回值：星期号																						 
uint8_t RTC_Get_Week(uint16_t year,uint8_t month,uint8_t day)
{	
	uint16_t temp2;
	uint8_t yearH,yearL;
	
	yearH=year/100;	yearL=year%100; 
	// 如果为21世纪,年份数加100  
	if (yearH>19)yearL+=100;
	// 所过闰年数只算1900年之后的  
	temp2=yearL+yearL/4;
	temp2=temp2%7; 
	temp2=temp2+day+table_week[month-1];
	if (yearL%4==0&&month<3)temp2--;
	return(temp2%7);
}



//将获取到的时间显示到LCD
void clock_demo(void)
{	
	static char year_mon_dat[10] = {0};
	static char hour_min_sec[8] = {0};
	POINT_COLOR = RED;
	//刷新年月日
	sprintf(year_mon_dat,"%.2d-%.2d-%.2d",calendar.w_year,calendar.w_month,calendar.w_date);
	LCD_ShowString(20,43,200,16,16,year_mon_dat);
	//刷新小时，分钟，秒
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
//设置时间
void Set_Time(void)
{
	//两个数组用于存放sprintf转换的字符
	static char set_year_mon_dat[10] = {0};
	static char set_hour_min_sec[8] = {0};
	//设置闪烁标志位，进入时间设置时默认年份闪烁
	static uint8_t time_flag_flash = 1;
	static uint8_t key_value = 0;
	static uint8_t status = 0;
	static uint16_t set_value = 0;
	//LCD清屏
	LCD_Clear(WHITE);
	POINT_COLOR =BLACK;
	//禁止RTC中断
	HAL_NVIC_DisableIRQ(RTC_IRQn);
	//拷贝当前时间到设置时间结构体变量中
	Set_time.year = calendar.w_year;
	Set_time.mon = calendar.w_month;
	Set_time.day = calendar.w_date;
	Set_time.hour = calendar.hour;
	Set_time.min = calendar.min;
	Set_time.sec = calendar.sec;
	Set_time.week = calendar.week;
	
	//显示数字触摸键盘
	Draw_Number_Pad();
	//显示设置时间
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
		//循环检测触摸状态
		tp_dev.scan(0); 		 
		if(tp_dev.sta&TP_PRES_DOWN)			//触摸屏被按下
		{	
		 	if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height) //检测触摸范围是否在液晶显示尺寸之内
			{	
				//判断某个时间项是否被按下
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
				//判断某个数字是否被按下
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
				//消除触摸抖动  
				while(tp_dev.sta&TP_PRES_DOWN)	
				{
					tp_dev.scan(0); 
				}
			}
		}
		//time_flag_flash标志位的值循环闪烁某个设定值
		switch(time_flag_flash)
		{
				case 1://年份闪烁
						{
							POINT_COLOR = BLACK;
							LCD_ShowString(84,43,200,24,24,"  ");
							Delay_ms(100);
							sprintf(set_year_mon_dat,"%.2d-%.2d-%.2d",Set_time.year,Set_time.mon,Set_time.day);
							LCD_ShowString(60,43,200,24,24,set_year_mon_dat);
							Delay_ms(100);			
							break;
						}
				case 2://月份闪烁
						{
							POINT_COLOR = BLACK;
							LCD_ShowString(120,43,200,24,24,"  ");
							Delay_ms(100);
							sprintf(set_year_mon_dat,"%.2d-%.2d-%.2d",Set_time.year,Set_time.mon,Set_time.day);
							LCD_ShowString(60,43,200,24,24,set_year_mon_dat);
							Delay_ms(100);	
						    break;
						}
				case 3://日期闪烁
						{
							POINT_COLOR = BLACK;
							LCD_ShowString(156,43,200,24,24,"  ");
							Delay_ms(100);
							sprintf(set_year_mon_dat,"%.2d-%.2d-%.2d",Set_time.year,Set_time.mon,Set_time.day);
							LCD_ShowString(60,43,200,24,24,set_year_mon_dat);
							Delay_ms(100);	
							break;
						}
				case 11://小时闪烁
						{
							POINT_COLOR = BLACK;
							LCD_ShowString(65,70,200,24,24,"  "); 
							Delay_ms(100);
							sprintf(set_hour_min_sec,"%.2d:%.2d:%.2d",Set_time.hour,Set_time.min,Set_time.sec);
	                        LCD_ShowString(65,70,200,24,24,set_hour_min_sec); 
							Delay_ms(100);	
							time_flag_flash = 11;//去掉就无法闪烁
						    break;
						}
				case 12://分钟闪烁
						{
							POINT_COLOR = BLACK;
							LCD_ShowString(101,70,200,24,24,"  "); 
							Delay_ms(100);
							sprintf(set_hour_min_sec,"%.2d:%.2d:%.2d",Set_time.hour,Set_time.min,Set_time.sec);
	                        LCD_ShowString(65,70,200,24,24,set_hour_min_sec); 
							Delay_ms(100);	 
							time_flag_flash = 12;//去掉就无法闪烁
						    break;
						}
				case 13://秒闪烁
						{
							POINT_COLOR = BLACK;
							LCD_ShowString(137,70,200,24,24,"  "); 
							Delay_ms(100);
							sprintf(set_hour_min_sec,"%.2d:%.2d:%.2d",Set_time.hour,Set_time.min,Set_time.sec);
	                        LCD_ShowString(65,70,200,24,24,set_hour_min_sec); 
							Delay_ms(100);
							time_flag_flash = 13;//去掉就无法闪烁							
						    break;
						}
				case 7://星期闪烁
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
	//取消则退出时间设置
	if(status == 'C')
	  {
		    LCD_Clear(WHITE);
		    Draw_window();
		  	HAL_NVIC_EnableIRQ(RTC_IRQn);
		    //rt_timer_start(timer1_sensor_Status);
		    GREEN_LED = 1;
	  }
	//设置完成将时间写入RTC
	if(status == 'E')
	  {
		LCD_Clear(WHITE);
		
        //设置设定的时间到RTC
		RTC_Set(Set_time.year,Set_time.mon,Set_time.day,Set_time.hour,Set_time.min,Set_time.sec);
		//标记已经初始化过了时钟
		HAL_RTCEx_BKUPWrite(&RTC_Handler,RTC_BKP_DR1,0X5050);
		
		//显示主窗口
		Draw_window();
		//使能RTC秒中断
		HAL_NVIC_EnableIRQ(RTC_IRQn);
		//重新启动软件定时器
		//rt_timer_start(timer1_sensor_Status);
		GREEN_LED = 1;
	  }
}








