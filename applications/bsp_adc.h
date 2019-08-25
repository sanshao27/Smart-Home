#ifndef __BSP_ADC_H
#define __BSP_ADC_H
#include "stdint.h"

/*ADC通道宏定义*/
#define LS1 ADC_CHANNEL_6    //ADC3
#define HZ  ADC_CHANNEL_4    //ADC1
#define MQ  ADC_CHANNEL_1    //ADC1
/*函数声明*/
int adc_Init(void);
uint16_t adc_get_val(uint32_t ch);
float get_aver_val(uint8_t time,uint32_t ch) ;
_Bool get_LS1_Status(uint8_t time,uint32_t ch);
//float Get_internal_tem(ADC_TypeDef* adcx,uint8_t ch,uint8_t time);
#endif
