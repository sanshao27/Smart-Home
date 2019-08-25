#include "public.h"

int BEEP_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOB_CLK_ENABLE();           //����GPIOBʱ��
    GPIO_Initure.Pin=GPIO_PIN_8; 			//PB8
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //�������
    GPIO_Initure.Pull=GPIO_PULLUP;          //����
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;     //����
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_RESET);	//��������Ӧ����GPIOB8���ͣ�
	BEEP = 1;
	Delay_ms(50);
	BEEP = 0;
  return 0;
}
//INIT_BOARD_EXPORT(BEEP_Init);

//����
void beep_warning(void)
{
	uint8_t i = 50;
	while(i--)
	{
		BEEP = 1; 
		RED_LED = 0;  
		rt_thread_delay(100);
		BEEP = 0; 
		RED_LED = 1;  
		rt_thread_delay(20); 
	}
	
}
