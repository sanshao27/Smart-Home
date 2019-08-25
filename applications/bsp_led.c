#include "public.h"

/***************************LED 引脚初始化********************************/
int LED_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PE5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
  GREEN_LED = 0;
  RED_LED   = 0;
  Delay_ms(1);
  GREEN_LED = 1;
  RED_LED   = 1;
  return 0;
}
//INIT_BOARD_EXPORT(LED_Init);

/***************************LED DEMO********************************/
void Led_Demo(void *parameter)
{
	//while(1)
	//{
		GREEN_LED = 1; 
		RED_LED = 1;  
		rt_thread_delay(200);
		GREEN_LED = 0; 
		RED_LED = 0;  
		rt_thread_delay(1); 
	//}
}

//LED闪烁
void LED_Flash(void *parameter)
{
	static rt_uint32_t cnt = 0;
	static rt_uint16_t count = 0;
	cnt = (rt_uint32_t)parameter;
	count++;
	if(count<cnt)
	{
		GREEN_LED = 1; 
		RED_LED = 1; 
	}
	else
	{
		count = 0;
		GREEN_LED = 0; 
		RED_LED = 0; 
		printf("temperature:%.2f\n",DS18B20_Get_Temp()); 
		printf("LS1 value:%.2fV\n\n",get_aver_val(5,LS1));
	}
}


//照明线程
void Light_Control(uint8_t val)
{
	//处理来自红外遥控和触摸屏的命令
	switch(val)
	{
		case 1:
		{
			rt_enter_critical();
			BEDROOM_LIGHT = !BEDROOM_LIGHT;
			if(BEDROOM_LIGHT)
			{
				POINT_COLOR =BLACK;
				LCD_ShowString(0,236,200,16,16,"Bedroom    Light:     OFF");	
			}
			else
			{
				POINT_COLOR =RED;
				LCD_ShowString(0,236,200,16,16,"Bedroom    Light:     ON ");
			}
			rt_exit_critical(); 
			break;
		}
		case 2:
		{
			rt_enter_critical();
			LIVINGROOM_LIGHT = !LIVINGROOM_LIGHT;
			if(LIVINGROOM_LIGHT)
			{
				POINT_COLOR =BLACK;
				LCD_ShowString(0,254,200,16,16,"Livingroom Light:     OFF");
			}
			else
			{
				POINT_COLOR =RED;
				LCD_ShowString(0,254,200,16,16,"Livingroom Light:     ON ");
			}
			rt_exit_critical(); 
			break;
		}
		case 3:
		{
			rt_enter_critical();
			if(CORRIDOR_LIGHT)
			{
				POINT_COLOR =RED;
				CORRIDOR_LIGHT = 0;
				LCD_ShowString(0,272,200,16,16,"Corridor   Light:     ON ");
			}
			else
			{
				POINT_COLOR =BLACK;
				CORRIDOR_LIGHT = 1;
				LCD_ShowString(0,272,200,16,16,"Corridor   Light:     OFF");
			}
			rt_exit_critical(); 
			break;
		}
		default:
		{
			val = 0;
			break;
		}
	}
}
