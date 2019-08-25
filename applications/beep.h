#ifndef __BEEP_H
#define __BEEP_H
#include "sys.h"


//LED端口定义
#define BEEP PBout(8)	// 蜂鸣器控制IO 

int BEEP_Init(void);//初始化
void beep_warning(void);
#endif
