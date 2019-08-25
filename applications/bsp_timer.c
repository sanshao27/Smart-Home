#include "bsp_timer.h"
#include "stm32f1xx_hal.h"
#include "usart.h"	
#include "bsp_led.h"
#include <rthw.h>
#include <rtthread.h>
TIM_HandleTypeDef TIM5_Handler;      //定时器句柄 
//通用定时器5中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!
void TIM5_Init(uint16_t arr,uint16_t psc)
{  
    TIM5_Handler.Instance=TIM5;                          //通用定时器5
    TIM5_Handler.Init.Prescaler=psc;                     //分频系数
    TIM5_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //向上计数器
    TIM5_Handler.Init.Period=arr;                        //自动装载值
    TIM5_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM5_Handler);
    HAL_TIM_Base_Start_IT(&TIM5_Handler); //使能定时器5和定时器5更新中断：TIM_IT_UPDATE  
    __HAL_TIM_DISABLE(&TIM5_Handler);	
}

//定时器底册驱动，开启时钟，设置中断优先级
//此函数会被HAL_TIM_Base_Init()函数调用
//已经在stm32f1XX_hal_tim.c中处理
#if 1
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM5)
	{
		__HAL_RCC_TIM5_CLK_ENABLE();            //使能TIM3时钟
		HAL_NVIC_SetPriority(TIM5_IRQn,1,3);    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM5_IRQn);           //开启ITM5中断   
	}
}
#endif
//定时器5中断服务函数
void TIM5_IRQHandler(void)
{
   __HAL_TIM_CLEAR_IT(&TIM5_Handler, TIM_IT_UPDATE);
   USART_RX2_STA = 0x8000;						//定时器产生溢出说明串口数据接收完成，并标识接收完成标志位
	RED_LED   = 1;
	__HAL_TIM_DISABLE(&TIM5_Handler);			//停止定时器5
}

