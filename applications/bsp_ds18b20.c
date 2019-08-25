#include "public.h"

//给串口提供数据
uint16_t indoor=0,outdoor=0;
//线上温度传感器的ROM值，通过void Read_ROM()函数获取，然后写入
uint8_t ROM1_SN[8] = {0x28,0x92,0xa9,0x5c,0x2,0x0,0x0,0x84};
uint8_t ROM2_SN[8] = {0x28,0x16,0xc6,0x5c,0x2,0x0,0x0,0xcc};
//温度通道标志位
uint8_t flag = 0;


//复位DS18B20
void DS18B20_Rst(void)	   
{             
	    
	DS18B20_IO_OUT(); 	//SET PG11 OUTPUT
	
    DS18B20_DQ_OUT=0; 	//拉低DQ
	
    delayus(750);    	//拉低750us
	
    DS18B20_DQ_OUT=1; 	//DQ=1 
	delayus(15);     	//15US
}
//等待DS18B20的回应
//返回1:未检测到DS18B20的存在
//返回0:存在
uint8_t DS18B20_Check(void) 	   
{   
	uint8_t retry=0;
	DS18B20_IO_IN();	//SET PG11 INPUT
    delayus(5);	
    while(DS18B20_DQ_IN&&(retry<200))
	{
		retry++;
		delayus(500);
	}	
   
	if(retry>=200) return 1;
	else retry=0;
    while (!DS18B20_DQ_IN&&retry<240)
	{
		retry++;
		delayus(1);
	}
	if(retry>=240)return 1;	    
	return 0;
}
//从DS18B20读取一个位
//返回值：1/0
uint8_t DS18B20_Read_Bit(void) 	 
{
    uint8_t data;
	DS18B20_IO_OUT();	//SET PG11 OUTPUT
    DS18B20_DQ_OUT=0; 
	delayus(2);
    DS18B20_DQ_OUT=1; 
	DS18B20_IO_IN();	//SET PG11 INPUT
	delayus(12);
	if(DS18B20_DQ_IN)data=1;
    else data=0;	 
    delayus(50);           
    return data;
}
//从DS18B20读取一个字节
//返回值：读到的数据
uint8_t DS18B20_Read_Byte(void)     
{        
    uint8_t i,j,dat;
    dat=0;
	for (i=1;i<=8;i++) 
	{
        j=DS18B20_Read_Bit();
        dat=(j<<7)|(dat>>1);
    }						    
    return dat;
}
//写一个字节到DS18B20
//dat：要写入的字节
void DS18B20_Write_Byte(uint8_t dat)     
 {             
    uint8_t j;
    uint8_t testb;
	DS18B20_IO_OUT();	//SET PG11 OUTPUT;
    for (j=1;j<=8;j++) 
	{
        testb=dat&0x01;
        dat=dat>>1;
        if (testb) 
        {
            DS18B20_DQ_OUT=0;	// Write 1
            delayus(2);                            
            DS18B20_DQ_OUT=1;
            delayus(60);             
        }
        else 
        {
            DS18B20_DQ_OUT=0;	// Write 0
            delayus(60);             
            DS18B20_DQ_OUT=1;
            delayus(2);                          
        }
    }
}
//开始温度转换
void DS18B20_Start(void) 
{   						               
    DS18B20_Rst();	   
	DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);	// skip rom
    DS18B20_Write_Byte(0x44);	// convert
} 

//初始化DS18B20的IO口 DQ 同时检测DS的存在
//返回1:不存在
//返回0:存在    	 
int DS18B20_Init(void)
{
   GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_11, GPIO_PIN_RESET);

  /*Configure GPIO pin : PG11 */
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
  DS18B20_Rst();  
  return DS18B20_Check();
}
//INIT_BOARD_EXPORT(DS18B20_Init);
//从ds18b20得到温度值
//精度：0.1C
//返回值：温度值 （-550~1250） 
float DS18B20_Get_Temp(void)
{
	//__disable_irq();
	float tem_val;
    uint8_t temp;
    uint8_t TL,TH;
	short tem;
    DS18B20_Start ();  			// ds1820 start convert
    DS18B20_Rst();
    DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);	// skip rom
    DS18B20_Write_Byte(0xbe);	// convert	    
    TL=DS18B20_Read_Byte(); 	// LSB   
    TH=DS18B20_Read_Byte(); 	// MSB  
	    	  
    if(TH>7)
    {
        TH=~TH;
        TL=~TL; 
        temp=0;					//温度为负  
    }else temp=1;				//温度为正	  	  
    tem=TH; 					//获得高八位
    tem<<=8;    
    tem+=TL;					     //获得底八位
    tem_val=(float)tem*0.0625+0.5;   //转换     
	//__enable_irq();
	if(temp)return tem_val; 		//返回实际温度值 
	else return -tem_val;    
	
}

//打印显示温度值
void show_temp(void *parameter)
{
	printf("temperature:%.2f\n",DS18B20_Get_Temp());	 
}

/****************读取DS18B20 ROM序列****************/
void Read_ROM(void)
{
    printf("\r\n");
    printf("\r\n");
    printf("Ready to get DS18B20's ROM SN..........\r\n");
    uint8_t i=0;
    uint8_t ROM_SN[8] = {0};
    DS18B20_Rst();
    delayus(1000);
    //读ROM
    DS18B20_Write_Byte(0x33);
    for(i = 0;i<8;i++)
    {
        ROM_SN[i] = DS18B20_Read_Byte();
    }
    //打印输出ROM值
    printf("DS18B20 ROM SN is:\r");
    for(i = 0;i<8;i++)
    {
        printf("0x%x,",ROM_SN[i]);
    }
    printf("\r\n");
    printf("\r\n");
    printf("\r\n");
}
