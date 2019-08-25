#include "public.h"

/*****热释电传感器接口初始化*****/
int RSD_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOC_CLK_ENABLE(); 
	/*Configure GPIO pins : PC8 */
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	return 0;
}
//INIT_BOARD_EXPORT(RSD_init); 
//uint8_t RSD_count = 0;
//if(!RSD_STATUS)
//{
//	GREEN_LED = 1; 
//}
//else
//{
//	GREEN_LED = 0; 
//}
//离家时作为安防，在家时作为路灯开关
