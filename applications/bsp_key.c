#include "public.h" 

//������ʼ��
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

/************��ȡ��ֵ*************/
/*
�βΣ�mode:	1:֧����������0����֧��������
����ֵ��UP,RIGHT,SELECT,LEFT,0
*/
uint8_t Key_Scan(uint8_t mode)
{
	//���ڱ�����һ�ΰ������µ�״̬�Ͱ����ɿ���״̬
	static _Bool key_up = 1; 
	if(mode)
		key_up = 1;
	//modeΪ1����ÿ�ΰ���ɨ���״̬ȡ�����Ƿ��м�����
	//modeΪ0��ֻҪÿһ�ΰ��²����֣�����ɨ�������ΪmodeΪ0����key_up�ڵ�һ�ν���ʱ�����㣬���´���
	//����ɨ�����ʱ��Ϊif�жϲ�������ֱ�������������а����жϴӶ�ʵ�ֲ�֧�ֲ����������ܣ�ֻ�н������ɿ�
	//����ڶ���if�ж����֮���ٽ�key_up��1������ʵ����һ�ΰ������
	if(key_up&&(WAKE_UP==1||KEY_RIGHT==0||KEY_SELECT==0||KEY_LEFT==0)) 
	{
		rt_thread_mdelay(20);
		key_up = 0; 
		if(mode)	                             //��ʹ�������������ܣ���ÿ��100�����ж�һ���������µ�״̬
		rt_thread_mdelay(90);
		if(WAKE_UP==1) return UP;
		if(KEY_RIGHT==0) return RIGHT;
		if(KEY_SELECT==0) return SELECT;
		if(KEY_LEFT==0) return LEFT;
	}
	else if(WAKE_UP==0&&KEY_RIGHT==1&&KEY_SELECT==1&&KEY_LEFT==1) //û�м�����ʱ����־λ��1
	{
		key_up = 1;
	}
		return 0;
}
