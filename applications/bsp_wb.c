#include "public.h"


/*****微波传感器接口初始化*****/
int WB_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOG_CLK_ENABLE(); 
	/*Configure GPIO pins : PG15 */
	GPIO_InitStruct.Pin = GPIO_PIN_15;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
	WB_STATUS = 0;
	return 0;
}
//INIT_BOARD_EXPORT(WB_init);

//if(RSD_STATUS|WB_STATUS)
//{
//	GREEN_LED = 0;
//	printf("有人！\n"); 
//}
//else
//{
//	GREEN_LED = 1; 
//}
//DHT11_Read_Data(&temp,&humi);
//printf("温度:%d\n,湿度:%d\n",temp,humi); 
//printf("LS1 value:%.2fV\n\n",get_aver_val(5,LS1));
//printf("煤气值:%.2fV\n",get_aver_val(5,MQ));
//rt_thread_mdelay(1500);
