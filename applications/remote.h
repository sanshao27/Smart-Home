#ifndef __REMOTE_H
#define __REMOTE_H
#include "sys.h"
/***********����ң��������궨��*********/
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

//���ź궨��
#define RDATA   PBin(9)		//�������������

//����ң��ʶ����(ID),ÿ��ң�����ĸ�ֵ��������һ��,��Ҳ��һ����.
//����ѡ�õ�ң����ʶ����Ϊ0
#define REMOTE_ID 0      		   

extern uint8_t RmtCnt;	     //�������µĴ���

int Remote_Init(void);     //���⴫��������ͷ���ų�ʼ��
uint8_t Remote_Scan(void);
#endif
