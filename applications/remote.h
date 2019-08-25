#ifndef __REMOTE_H
#define __REMOTE_H
#include "sys.h"
/***********红外遥控器编码宏定义*********/
#define powr	0Xa2
#define up		0X62
#define log		0XE2
#define left	0X22
#define stop	0X02
#define right	0XC2
#define voldec	0XE0
#define down	0XA8
#define volinc	0X90
#define back	0X52
#define one		0Xa2
#define two		0X98
#define three	0XB0
#define four	0X30
#define five	0X18
#define six	    0X79
#define seven	0X10
#define eight	0X38
#define nine	0X5A
#define zero 	0X42

//引脚宏定义
#define RDATA   PBin(9)		//红外数据输入脚

//红外遥控识别码(ID),每款遥控器的该值基本都不一样,但也有一样的.
//我们选用的遥控器识别码为0
#define REMOTE_ID 0      		   

extern uint8_t RmtCnt;	     //按键按下的次数

int Remote_Init(void);     //红外传感器接收头引脚初始化
uint8_t Remote_Scan(void);
#endif
