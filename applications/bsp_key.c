#include "public.h" 

//按键初始化
void key_Init(void)
{
	 GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	 
	/*Configure GPIO pins : PE2 PE3 PE4 */
	GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pin : PA0 */
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;	 
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);	  
}

/************获取键值*************/
/*
形参：mode:	1:支持连续按，0：不支持连续按
返回值：UP,RIGHT,SELECT,LEFT,0
*/
uint8_t Key_Scan(uint8_t mode)
{
	//用于保存上一次按键按下的状态和按键松开的状态
	static _Bool key_up = 1; 
	if(mode)
		key_up = 1;
	//mode为1，则每次按键扫描的状态取决于是否有键按下
	//mode为0，只要每一次按下不松手，按键扫描程序因为mode为0，且key_up在第一次进入时被清零，则下次再
	//进入扫描程序时因为if判断不成立就直接跳过，不进行按键判断从而实现不支持不连续按功能，只有将按键松开
	//满足第二个if判断语句之后再将key_up置1，才能实现下一次按键检测
	if(key_up&&(WAKE_UP==1||KEY_RIGHT==0||KEY_SELECT==0||KEY_LEFT==0)) 
	{
		rt_thread_mdelay(20);
		key_up = 0; 
		if(mode)	                             //若使能了连续按功能，则每隔100毫秒判断一次连续按下的状态
		rt_thread_mdelay(90);
		if(WAKE_UP==1) return UP;
		if(KEY_RIGHT==0) return RIGHT;
		if(KEY_SELECT==0) return SELECT;
		if(KEY_LEFT==0) return LEFT;
	}
	else if(WAKE_UP==0&&KEY_RIGHT==1&&KEY_SELECT==1&&KEY_LEFT==1) //没有键按下时，标志位置1
	{
		key_up = 1;
	}
		return 0;
}
