#include "public.h"
ADC_HandleTypeDef hadc3;
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef adc1;
/***************ADC初始化***************/
int adc_Init(void)
{
  RCC_PeriphCLKInitTypeDef ADC_CLKInit;
  ADC_CLKInit.PeriphClockSelection=RCC_PERIPHCLK_ADC;	    //ADC外设时钟
  ADC_CLKInit.AdcClockSelection=RCC_ADCPCLK2_DIV6;			//分频因子6时钟为72M/6=12MHz
  HAL_RCCEx_PeriphCLKConfig(&ADC_CLKInit);	
  /** Common config ---ADC3 */
  hadc3.Instance = ADC3;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
     rt_kprintf("Failed to initilize ADC3\n");
  }
  
  /** Common config ---ADC1 */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
     rt_kprintf("Failed to initilize ADC1\n");
  }
  //calibrate the target ADC
  HAL_ADCEx_Calibration_Start(&hadc3);
  HAL_ADCEx_Calibration_Start(&hadc1);
  //enable the adc ADC校准函数已经使能ADC
  //ADC_Enable(&hadc3);
  return 0;
}
//INIT_BOARD_EXPORT(adc_Init);

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC3)
  {
    /* ADC3/ADC1 clock enable */
    __HAL_RCC_ADC3_CLK_ENABLE();
    __HAL_RCC_ADC1_CLK_ENABLE();
	/* GPIOF/GPIOA clock enable */
    __HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();  
    /**ADC3 GPIO Configuration    PF8------> ADC3_IN6 ----LS1*/
    GPIO_InitStruct.Pin = GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
	  
	/**ADC3 GPIO Configuration    PA1------> ADC1_IN1 ----MQ*/  
	/**ADC3 GPIO Configuration    PA4------> ADC1_IN4 ----HZ*/
    GPIO_InitStruct.Pin = GPIO_PIN_1|GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

 if(adcHandle->Instance==ADC3)
  {
    /* Peripheral clock disable */
    __HAL_RCC_ADC3_CLK_DISABLE();
    __HAL_RCC_ADC1_CLK_DISABLE();
    /**ADC3 GPIO Configuration    
    PF8     ------> ADC3_IN6 
	PA4------> ADC1_IN4
    */
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_8);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_1|GPIO_PIN_4);
  }
} 

/***************获取ADC转换值***************/
uint16_t adc_get_val(uint32_t ch) 
{
  ADC_ChannelConfTypeDef sConfig = {0};
  //检测光敏传感器
  if(ch==LS1)
  {
	  /** Configure Regular Channel */
	  sConfig.Channel = LS1;
	  sConfig.Rank = ADC_REGULAR_RANK_1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
	  {
		 rt_kprintf("Failed to configuer ADC3's channel\n");
	  }
	  HAL_ADC_Start(&hadc3);
	  //HAL_ADC_PollForConversion(&hadc3,10);                //轮询转换
	  while(!HAL_ADC_GetState(&hadc3));
	  return HAL_ADC_GetValue(&hadc3);
  }
  
  //检测火灾传感器
  if(ch==HZ)
  {
	  /** Configure Regular Channel */
	  sConfig.Channel = HZ;
	  sConfig.Rank = ADC_REGULAR_RANK_1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	  {
		 rt_kprintf("Failed to configuer ADC1's channel 4\n");
	  }
	  HAL_ADC_Start(&hadc1);
	  //轮询转换
	  HAL_ADC_PollForConversion(&hadc1,10);                
	  while(!HAL_ADC_GetState(&hadc1));
	  return HAL_ADC_GetValue(&hadc1);
  }
  
  //检测煤气
  if(ch==MQ)
  {
	  /** Configure Regular Channel */
	  sConfig.Channel = MQ;
	  sConfig.Rank = ADC_REGULAR_RANK_1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_239CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	  {
		 rt_kprintf("Failed to configuer ADC1's channel\n");
	  }
	  HAL_ADC_Start(&hadc1);
	   //轮询转换,多通道转换时务必要开启，否则无法获取正确的转换值
	  HAL_ADC_PollForConversion(&hadc1,10);               
	  while(!HAL_ADC_GetState(&hadc1));
	  return HAL_ADC_GetValue(&hadc1);
  }
	return 0;
}	

/***************获取ADC转换平均值***************/
float get_aver_val(uint8_t time,uint32_t ch) 
{
	uint8_t i = 0;
	uint16_t temp = 0;
	float adc_value = 0;
	//获取time次AD值
	for(;i<time;i++)
	{
		temp += adc_get_val(ch);
		rt_thread_mdelay(1);
	}
	adc_value = temp/time*(3.3/4096);
	//返回平均AD值
	return adc_value;
}	

/***************获取光敏状态***************/
_Bool get_LS1_Status(uint8_t time,uint32_t ch) 
{
	uint8_t i = 0;
	uint16_t temp = 0;
	float adc_value = 0;
	//获取time次AD值
	for(;i<time;i++)
	{
		temp += adc_get_val(ch);
		rt_thread_mdelay(10);
	}
	adc_value = temp/time*(3.3/4096);
	if(adc_value>2.5)  
		return 1;    //夜晚
	else
	    return 0;	 //白天		
}
/***************获取芯片内部温度传感器的值***************/
//float Get_internal_tem(ADC_TypeDef* adcx,uint8_t ch,uint8_t time)
//{
//	float result = 0;
//	//获取AD值，然后转换成对应得电压值
//	result = (float)(get_aver_val(adcx,ch,time)*(3.3/4096)); 
//	//根据电压值换算成相应的温度值
//	result = (1.43-result)/0.0043+25;
//	//返回实际温度值
//	return result;
//}

//printf("\n光敏传感器值:%.2fV\n",get_aver_val(5,LS1));
//printf("煤气值:%.2fV\n",get_aver_val(5,MQ));
//rt_thread_mdelay(1500);
