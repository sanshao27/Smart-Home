#include "public.h"

int BEEP_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOB_CLK_ENABLE();           //开启GPIOB时钟
    GPIO_Initure.Pin=GPIO_PIN_8; 			//PB8
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;     //高速
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_8,GPIO_PIN_RESET);	//蜂鸣器对应引脚GPIOB8拉低，
	BEEP = 1;
	Delay_ms(50);
	BEEP = 0;
  return 0;
}
//INIT_BOARD_EXPORT(BEEP_Init);

//报警
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
