#include "bsp_timer.h"
#include "stm32f1xx_hal.h"
#include "usart.h"	
#include "bsp_led.h"
#include <rthw.h>
#include <rtthread.h>
TIM_HandleTypeDef TIM5_Handler;      //��ʱ����� 
//ͨ�ö�ʱ��5�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!
void TIM5_Init(uint16_t arr,uint16_t psc)
{  
    TIM5_Handler.Instance=TIM5;                          //ͨ�ö�ʱ��5
    TIM5_Handler.Init.Prescaler=psc;                     //��Ƶϵ��
    TIM5_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //���ϼ�����
    TIM5_Handler.Init.Period=arr;                        //�Զ�װ��ֵ
    TIM5_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����
    HAL_TIM_Base_Init(&TIM5_Handler);
    HAL_TIM_Base_Start_IT(&TIM5_Handler); //ʹ�ܶ�ʱ��5�Ͷ�ʱ��5�����жϣ�TIM_IT_UPDATE  
    __HAL_TIM_DISABLE(&TIM5_Handler);	
}

//��ʱ���ײ�����������ʱ�ӣ������ж����ȼ�
//�˺����ᱻHAL_TIM_Base_Init()��������
//�Ѿ���stm32f1XX_hal_tim.c�д���
#if 1
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if(htim->Instance==TIM5)
	{
		__HAL_RCC_TIM5_CLK_ENABLE();            //ʹ��TIM3ʱ��
		HAL_NVIC_SetPriority(TIM5_IRQn,1,3);    //�����ж����ȼ�����ռ���ȼ�1�������ȼ�3
		HAL_NVIC_EnableIRQ(TIM5_IRQn);           //����ITM5�ж�   
	}
}
#endif
//��ʱ��5�жϷ�����
void TIM5_IRQHandler(void)
{
   __HAL_TIM_CLEAR_IT(&TIM5_Handler, TIM_IT_UPDATE);
   USART_RX2_STA = 0x8000;						//��ʱ���������˵���������ݽ�����ɣ�����ʶ������ɱ�־λ
	RED_LED   = 1;
	__HAL_TIM_DISABLE(&TIM5_Handler);			//ֹͣ��ʱ��5
}

