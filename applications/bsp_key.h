#ifndef __BSP_KEY_H
#define __BSP_KEY_H
#include "stdint.h"
#include "sys.h"
//按键扫描程序返回值
#define UP     1
#define LEFT   2
#define SELECT 3
#define RIGHT  4

//按键宏定义
#define WAKE_UP    PAin(0)
#define KEY_LEFT   PEin(4)
#define KEY_SELECT PEin(3)
#define KEY_RIGHT  PEin(2)

//按键工作模式宏定义
#define CONST_PRESS_ENALBE  1
#define CONST_PRESS_DISALBE  0

/******函数声明*****/
void key_Init(void);
uint8_t Key_Scan(uint8_t mode);
#endif
