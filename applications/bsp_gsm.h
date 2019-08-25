#ifndef __BSP_GSM_H
#define __BSP_GSM_H
#include "stm32f1xx_hal.h"

//��������궨��
#define report_status 11
#define home          22
#define out           33
#define set_ok        44
#define set_Err       55
#define MQ_LEAKING    66
#define Invaiding     77
#define tips          88

//��������
int scm800c_init(void);
void GSM_SendSMS(uint8_t content);
#endif
