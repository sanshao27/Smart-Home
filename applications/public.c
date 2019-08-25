#include "public.h"


//延时
void delay200(uint16_t ms)
{
  uint16_t i,j;
  while(ms--)
  {
     for(j=0;j<5;j++); 
     for(i=0;i<5;i++); 
  }
}

/*******1毫秒延时 @24MHz*******/
void Delay_ms(uint16_t ms)
{
    
    uint16_t i,j;
    while(ms--)
    {
        for(i=0;i<80;i++)
        {
           for(j=0;j<100;j++); 
			 
        }
    }
}

//1微妙延时函数@2MHz
void Delay1us(uint16_t us)
{
    uint16_t i = 0;
    for(i=0;i<us;i++);

}

//1微妙延时函数@16MHz
void delay_2us(uint16_t nCount)   //16M 晶振时  延时 1个微妙
{
    nCount *= 10;            //等同于 nCount=nCount*3  相当于把nCount变量扩大3倍
    
    while(--nCount);
}

void Delay_1US(uint16_t nCount)   //16M 晶振时  延时 1个微妙
{
    nCount *= 5;            //等同于 nCount=nCount*3  相当于把nCount变量扩大3倍
    
    while(--nCount);
}
/************************************************************************************************
*   函 数 名: Get_decimal
*   功能说明: 获得数值小数部分
*   形    参：dt输入数据 deci小数位数,最多保留4位小数
*   返 回 值: 放大后的小数部分
*************************************************************************************************/
uint16_t Get_decimal(double dt,uint8_t deci)   //获得数值小数部分 dt为要打印的小数值，deci为要保留的小数
{
    long x1=0;
    uint32_t x2=0,x3=0;
    if(deci>4) deci=4;
    if(deci<1) deci=1;
    switch(deci)
    {
        case 1: x3=10; break;
        case 2: x3=10*10;break;
        case 3: x3=10*10*10;break;
        case 4: x3=10*10*10*10;break;
        default:break;
    }
    x1=(long)(dt*x3);  
    x2=(uint16_t)(x1%x3);
    return x2;
}
