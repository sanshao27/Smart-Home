#ifndef __BEEP_H
#define __BEEP_H
#include "sys.h"


//LED�˿ڶ���
#define BEEP PBout(8)	// ����������IO 

int BEEP_Init(void);//��ʼ��
void beep_warning(void);
#endif
