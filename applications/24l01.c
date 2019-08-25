#include "24l01.h"
#include "spi.h"

const char TX_ADDRESS[TX_ADR_WIDTH]={0xff,0xff,0xff,0xff,0xff}; //发送地址
const char RX_ADDRESS[RX_ADR_WIDTH]={0xff,0xff,0xff,0xff,0xff}; //发送地址

//针对NRF24L01修改SPI1驱动
void NRF24L01_SPI_Init(void)
{
    __HAL_SPI_DISABLE(&SPI2_Handler);               //先关闭SPI2
    SPI2_Handler.Init.CLKPolarity=SPI_POLARITY_LOW; //串行同步时钟的空闲状态为低电平
    SPI2_Handler.Init.CLKPhase=SPI_PHASE_1EDGE;     //串行同步时钟的第1个跳变沿（上升或下降）数据被采样
    HAL_SPI_Init(&SPI2_Handler);
    __HAL_SPI_ENABLE(&SPI2_Handler);                //使能SPI2
}

//初始化24L01的IO口
int NRF24L01_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    __HAL_RCC_GPIOB_CLK_ENABLE();			//开启GPIOB时钟
    __HAL_RCC_GPIOG_CLK_ENABLE();			//开启GPIOG时钟
    
	//GPIOB12初始化设置:推挽输出
    GPIO_Initure.Pin=GPIO_PIN_12; 			//PB12
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  //推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;//高速
    HAL_GPIO_Init(GPIOB,&GPIO_Initure);     //初始化

	//GPIOG7,8推挽输出
    GPIO_Initure.Pin=GPIO_PIN_7|GPIO_PIN_8;	//PG7，8  //CS/CE
    HAL_GPIO_Init(GPIOG,&GPIO_Initure);     //初始化
	
	//GPIOG6上拉输入
//	GPIO_Initure.Pin=GPIO_PIN_6;			//PG6     //NRF_IRQ
//	GPIO_Initure.Mode=GPIO_MODE_INPUT;      //输入
//	HAL_GPIO_Init(GPIOG,&GPIO_Initure);     //初始化
	
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);//PB12输出1,防止SPI FLASH干扰NRF的通信  //W25Q128的片选信号
    
	SPI2_Init();    		                //初始化SPI1
    NRF24L01_SPI_Init();                    //针对NRF的特点修改SPI的设置	
	NRF24L01_CE=0; 			                //使能24L01
	NRF24L01_CSN=1;			                //SPI片选取消	
	if(NRF24L01_Check()==0)
		  printf("NRF2401 is OK!\n");
	  else
		  printf("NRF2401 is failed to response!\n");	
  return 0;
}
//INIT_BOARD_EXPORT(NRF24L01_Init);
//检测24L01是否存在
//返回值:0，成功;1，失败	
uint8_t NRF24L01_Check(void)
{
	char buf[5]={0XA5,0XA5,0XA5,0XA5,0XA5};
	uint8_t i;
	SPI2_SetSpeed(SPI_BAUDRATEPRESCALER_8); //spi速度为10.5Mhz（（24L01的最大SPI时钟为10Mhz,这里大一点没关系）    	 
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,buf,5);//写入5个字节的地址.	
	NRF24L01_Read_Buf(TX_ADDR,buf,5); //读出写入的地址  
	for(i=0;i<5;i++)if(buf[i]!=0XA5)break;	 							   
	if(i!=5)return 1;//检测24L01错误	
	return 0;		 //检测到24L01
}	 	 
//SPI写寄存器
//reg:指定寄存器地址
//value:写入的值
uint8_t NRF24L01_Write_Reg(uint8_t reg,uint8_t value)
{
	uint8_t status;	
   	NRF24L01_CSN=0;                 //使能SPI传输
  	status =SPI2_ReadWriteByte(reg);//发送寄存器号 
  	SPI2_ReadWriteByte(value);      //写入寄存器的值
  	NRF24L01_CSN=1;                 //禁止SPI传输	   
  	return(status);       		    //返回状态值
}
//读取SPI寄存器值
//reg:要读的寄存器
uint8_t NRF24L01_Read_Reg(uint8_t reg)
{
	  uint8_t reg_val;	    
   	NRF24L01_CSN=0;             //使能SPI传输		
  	SPI2_ReadWriteByte(reg);    //发送寄存器号
  	reg_val=SPI2_ReadWriteByte(0XFF);//读取寄存器内容
  	NRF24L01_CSN=1;             //禁止SPI传输		    
  	return(reg_val);            //返回状态值
}	
//在指定位置读出指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值 
uint8_t NRF24L01_Read_Buf(uint8_t reg,char *pBuf,uint8_t len)
{
	uint8_t status,uint8_t_ctr;	       
  	NRF24L01_CSN=0;            //使能SPI传输
  	status=SPI2_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值   	   
	for(uint8_t_ctr=0;uint8_t_ctr<len;uint8_t_ctr++)pBuf[uint8_t_ctr]=SPI2_ReadWriteByte(0XFF);//读出数据
  	NRF24L01_CSN=1;            //关闭SPI传输
  	return status;             //返回读到的状态值
}
//在指定位置写指定长度的数据
//reg:寄存器(位置)
//*pBuf:数据指针
//len:数据长度
//返回值,此次读到的状态寄存器值
uint8_t NRF24L01_Write_Buf(uint8_t reg, char *pBuf, uint8_t len)
{
	uint8_t status,uint8_t_ctr;	    
	NRF24L01_CSN=0;             //使能SPI传输
  	status = SPI2_ReadWriteByte(reg);//发送寄存器值(位置),并读取状态值
  	for(uint8_t_ctr=0; uint8_t_ctr<len; uint8_t_ctr++)//写入数据
        SPI2_ReadWriteByte(*pBuf++);  
  	NRF24L01_CSN=1;             //关闭SPI传输
  	return status;              //返回读到的状态值
}				   
//启动NRF24L01发送一次数据
//txbuf:待发送数据首地址
//返回值:发送完成状况
uint8_t NRF24L01_TxPacket(char *txbuf)
{
	uint8_t sta;
 	SPI2_SetSpeed(SPI_BAUDRATEPRESCALER_8); //spi速度为6.75Mhz（24L01的最大SPI时钟为10Mhz）   
	NRF24L01_CE=0;
  	NRF24L01_Write_Buf(WR_TX_PLOAD,txbuf,TX_PLOAD_WIDTH);//写数据到TX BUF  32个字节
 	NRF24L01_CE=1;                          //启动发送	   
	while(NRF24L01_IRQ!=0);                 //等待发送完成
	sta=NRF24L01_Read_Reg(STATUS);          //读取状态寄存器的值	   
	printf("sta :0x%x\n",sta);
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&MAX_TX)                          //达到最大重发次数
	{
		NRF24L01_Write_Reg(FLUSH_TX,0xff);  //清除TX FIFO寄存器 
		return MAX_TX; 
	}
	if(sta&TX_OK)                           //发送完成
	{
		return TX_OK;
	}
	return 0xff;//其他原因发送失败
}
//启动NRF24L01发送一次数据
//rxbuf:待数据首地址
//返回值:0，接收完成；其他，错误代码
uint8_t NRF24L01_RxPacket(char *rxbuf)
{
	uint8_t sta;		    							   
	SPI2_SetSpeed(SPI_BAUDRATEPRESCALER_8); //spi速度为6.75Mhz（24L01的最大SPI时钟为10Mhz）   
	sta=NRF24L01_Read_Reg(STATUS);          //读取状态寄存器的值    	 
	NRF24L01_Write_Reg(NRF_WRITE_REG+STATUS,sta); //清除TX_DS或MAX_RT中断标志
	if(sta&RX_OK)//接收到数据
	{
		NRF24L01_Read_Buf(RD_RX_PLOAD,rxbuf,RX_PLOAD_WIDTH);//读取数据
		NRF24L01_Write_Reg(FLUSH_RX,0xff);  //清除RX FIFO寄存器 
		return 0; 
	}	   
	return 1;//没收到任何数据
}					    
//该函数初始化NRF24L01到RX模式
//设置RX地址,写RX数据宽度,选择RF频道,波特率和LNA HCURR
//当CE变高后,即进入RX模式,并可以接收数据了		   
void NRF24L01_RX_Mode(void)
{
	NRF24L01_CE=0;	  
  	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(char*)RX_ADDRESS,RX_ADR_WIDTH);//写RX节点地址
  	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);       //使能通道0的自动应答    
  	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01);   //使能通道0的接收地址  	 
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,0);	        //设置RF通信频率		  
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RX_PW_P0,RX_PLOAD_WIDTH);//选择通道0的有效数据宽度 	    
  	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);    //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
  	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0f);     //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式 
  	NRF24L01_CE=1; //CE为高,进入接收模式 
}						 
//该函数初始化NRF24L01到TX模式
//设置TX地址,写TX数据宽度,设置RX自动应答的地址,填充TX发送数据,选择RF频道,波特率和LNA HCURR
//PWR_UP,CRC使能
//当CE变高后,即进入TX模式,并可以接收数据了		   
//CE为高大于10us,则启动发送.	 
void NRF24L01_TX_Mode(void)
{														 
	NRF24L01_CE=0;	    
	NRF24L01_Write_Buf(NRF_WRITE_REG+TX_ADDR,(char*)TX_ADDRESS,TX_ADR_WIDTH);//写TX节点地址 
	NRF24L01_Write_Buf(NRF_WRITE_REG+RX_ADDR_P0,(char*)RX_ADDRESS,RX_ADR_WIDTH); //设置TX节点地址,主要为了使能ACK	  
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_AA,0x01);     //使能通道0的自动应答    
	NRF24L01_Write_Reg(NRF_WRITE_REG+EN_RXADDR,0x01); //使能通道0的接收地址  
	NRF24L01_Write_Reg(NRF_WRITE_REG+SETUP_RETR,0x1a);//设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_CH,0);       //设置RF通道为0
	NRF24L01_Write_Reg(NRF_WRITE_REG+RF_SETUP,0x0f);  //设置TX发射参数,0db增益,2Mbps,低噪声增益开启   
	NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG,0x0e);    //配置基本工作模式的参数;PWR_UP,EN_CRC,16BIT_CRC,接收模式,开启所有中断
	NRF24L01_CE=1;//CE为高,10us后启动发送
}



//NRF24L01_TX_Mode();	
//uint8_t key_value = 0;
////char buff[] = {" 2                             "};
//char rx_buff[32];

//while(1)
//{
//    key_value = Key_Scan(0);
//	if(key_value!=0)
//	{
//		rt_kprintf("key_value:%d\n",key_value);
//		//设置为发送模式
//		NRF24L01_CE=0;
//		NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0e);
//		NRF24L01_CE=1;
//		switch(key_value)
//		{
//				case UP:
//				{
//					if(NRF24L01_TxPacket(" 1                              ")==TX_OK)
//					printf("sent ok!\n");
//					else
//					printf("sent failed!\n");
//					break;
//					}
//				case LEFT:
//				{
//					if(NRF24L01_TxPacket(" 9                              ")==TX_OK)
//					printf("sent ok!\n");
//					else
//					printf("sent failed!\n");
//					break;
//				}
//				case SELECT:
//				{
//					if(NRF24L01_TxPacket(" 7                              ")==TX_OK)
//					printf("sent ok!\n");
//					else
//					printf("sent failed!\n");
//					break;
//				}
//				case RIGHT:
//				{
//					if(NRF24L01_TxPacket(" 8                              ")==TX_OK)
//					printf("sent ok!\n");
//					else
//					printf("sent failed!\n");
//					break;
//				}
//				default:break;
//		}
//	   //重新设置为接收模式
//	   NRF24L01_CE=0;
//	   NRF24L01_Write_Reg(NRF_WRITE_REG+CONFIG, 0x0f); 	
//	   NRF24L01_CE=1;    
//	  }
//    //等待查询NRF模块是否收到数据
//	if(NRF24L01_IRQ==0)
//	{
//		if(NRF24L01_RxPacket(rx_buff)==0)
//		printf("data :%s\n",rx_buff);
//		LCD_Clear(WHITE);
//		LCD_ShowString(30,80,200,12,12,rx_buff);
//		RED_LED   = !RED_LED;
//	}  
//}
