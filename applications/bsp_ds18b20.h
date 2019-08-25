#ifndef __BSP_DS18B20_H_
#define __BSP_DS18B20_H_

#include "sys.h"
#include  "stdint.h"
#include  "stm32f103xe.h"

//IO方向设置
#define DS18B20_IO_IN()  {GPIOG->CRH&=0XFFFF0FFF;GPIOG->CRH|=8<<12;}
#define DS18B20_IO_OUT() {GPIOG->CRH&=0XFFFF0FFF;GPIOG->CRH|=3<<12;}

////IO操作函数											   
#define	DS18B20_DQ_OUT PGout(11) //数据端口	PA0 
#define	DS18B20_DQ_IN  PGin(11)  //数据端口	PA0 
   	
int DS18B20_Init(void);//初始化DS18B20
float DS18B20_Get_Temp(void);//获取温度
void show_temp(void *parameter);
void DS18B20_Start(void);//开始温度转换
void DS18B20_Write_Byte(uint8_t dat);//写入一个字节
uint8_t DS18B20_Read_Byte(void);//读出一个字节
uint8_t DS18B20_Read_Bit(void);//读出一个位
uint8_t DS18B20_Check(void);//检测是否存在DS18B20
void DS18B20_Rst(void);//复位DS18B20    
void Read_ROM(void);//读取ROM序列
#endif
