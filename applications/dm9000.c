#include "dm9000.h"
#include "usart.h"
#include "lwip_comm.h"
#include <rthw.h>
#include <rtthread.h>
#include "stm32f1xx_hal.h"
#if 1
//DM9000句柄
SRAM_HandleTypeDef DM9000_Handler;    		  		
//DM9000配置结构体
struct dm9000_config dm9000cfg;		
//外部函数声明
extern void Delay_ms(uint16_t ms);
//DM9000接收数据信号量
extern rt_sem_t dm9000input;
//DM9000读写互锁控制信号量
extern rt_sem_t dm9000lock;		

//初始化DM9000
//返回值:
//0,初始化成功
//1，DM9000A ID读取错误
uint8_t DM9000_Init(void)
{
	uint32_t temp;
	GPIO_InitTypeDef GPIO_Initure;
	FSMC_NORSRAM_TimingTypeDef FSMC_ReadWriteTim;
	
    __HAL_RCC_GPIOD_CLK_ENABLE();           	//开启GPIOD时钟
	__HAL_RCC_GPIOE_CLK_ENABLE();           	//开启GPIOE时钟
	__HAL_RCC_GPIOF_CLK_ENABLE();           	//开启GPIOF时钟
	__HAL_RCC_GPIOG_CLK_ENABLE();           	//开启GPIOG时钟
	
    GPIO_Initure.Pin=GPIO_PIN_7; 				//PD7
    GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  	//推挽输出
    GPIO_Initure.Pull=GPIO_PULLUP;          	//上拉
    GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;    //高速
    HAL_GPIO_Init(GPIOD,&GPIO_Initure);

	GPIO_Initure.Pin=GPIO_PIN_6; 				//PG6 DM9000中断引脚
	GPIO_Initure.Mode=GPIO_MODE_IT_FALLING;  	//中断，下降沿
	HAL_GPIO_Init(GPIOG,&GPIO_Initure);
	
	//PD0 1 4 5 8 8 9 10 14 15
	GPIO_Initure.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5|\
                     GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_14|GPIO_PIN_15;	
	GPIO_Initure.Mode=GPIO_MODE_AF_PP;  		//复用推挽输出
	HAL_GPIO_Init(GPIOD,&GPIO_Initure);
	
	//PE7 8 9 10 11 12 13 14 15
	GPIO_Initure.Pin=GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|\
                     GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;	
	GPIO_Initure.Mode=GPIO_MODE_AF_PP;  		//复用推挽输出
	HAL_GPIO_Init(GPIOE,&GPIO_Initure);
	
	//PF13
	GPIO_Initure.Pin=GPIO_PIN_13;
	GPIO_Initure.Mode=GPIO_MODE_AF_PP;  		//复用推挽输出
	HAL_GPIO_Init(GPIOF,&GPIO_Initure);
	
	//PG9
	GPIO_Initure.Pin=GPIO_PIN_9;
	GPIO_Initure.Mode=GPIO_MODE_AF_PP;  		//复用推挽输出
	HAL_GPIO_Init(GPIOG,&GPIO_Initure);
	
	//中断线6=PG6
    HAL_NVIC_SetPriority(EXTI9_5_IRQn,1,0);   	//抢占优先级为1，子优先级为0
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);         	//使能中断线6
	
	DM9000_Handler.Instance=FSMC_NORSRAM_DEVICE;                
	DM9000_Handler.Extended=FSMC_NORSRAM_EXTENDED_DEVICE;    
    
	DM9000_Handler.Init.NSBank=FSMC_NORSRAM_BANK2;     					//使用NE2
	DM9000_Handler.Init.DataAddressMux=FSMC_DATA_ADDRESS_MUX_DISABLE; 	//地址/数据线不复用
	DM9000_Handler.Init.MemoryType=FSMC_MEMORY_TYPE_SRAM;   			//SRAM
	DM9000_Handler.Init.MemoryDataWidth=FSMC_NORSRAM_MEM_BUS_WIDTH_16; 	//16位数据宽度
	DM9000_Handler.Init.BurstAccessMode=FSMC_BURST_ACCESS_MODE_DISABLE; 	//是否使能突发访问,仅对同步突发存储器有效,此处未用到
	DM9000_Handler.Init.WaitSignalPolarity=FSMC_WAIT_SIGNAL_POLARITY_LOW;	//等待信号的极性,仅在突发模式访问下有用
	DM9000_Handler.Init.WaitSignalActive=FSMC_WAIT_TIMING_BEFORE_WS;   	//存储器是在等待周期之前的一个时钟周期还是等待周期期间使能NWAIT
	DM9000_Handler.Init.WriteOperation=FSMC_WRITE_OPERATION_ENABLE;    	//存储器写使能
	DM9000_Handler.Init.WaitSignal=FSMC_WAIT_SIGNAL_DISABLE;           	//等待使能位,此处未用到
	DM9000_Handler.Init.ExtendedMode=FSMC_EXTENDED_MODE_DISABLE;        	//读写使用相同的时序
	DM9000_Handler.Init.AsynchronousWait=FSMC_ASYNCHRONOUS_WAIT_DISABLE;	//是否使能同步传输模式下的等待信号,此处未用到
	DM9000_Handler.Init.WriteBurst=FSMC_WRITE_BURST_DISABLE;           	//禁止突发写
  
  	//FMC读时序控制寄存器
	FSMC_ReadWriteTim.AddressSetupTime=0x00;       	//地址建立时间（ADDSET）为1个HCLK 1/72M=13.8ns
	FSMC_ReadWriteTim.AddressHoldTime=0x00;			//地址保持时间（ADDHLD）模式A未用到
	FSMC_ReadWriteTim.DataSetupTime=0x03;			//数据保存时间为3个HCLK	=4*13.8=55ns
	FSMC_ReadWriteTim.BusTurnAroundDuration=0X00;
	FSMC_ReadWriteTim.AccessMode=FSMC_ACCESS_MODE_A;//模式A
	HAL_SRAM_Init(&DM9000_Handler,&FSMC_ReadWriteTim,&FSMC_ReadWriteTim);	
  
	//FMC读时序控制寄存器
	FSMC_ReadWriteTim.AddressSetupTime=0x00;       	//地址建立时间（ADDSET）为1个HCLK 1/72M=13.8ns
	FSMC_ReadWriteTim.AddressHoldTime=0x00;			//地址保持时间（ADDHLD）模式A未用到
	FSMC_ReadWriteTim.DataSetupTime=0x03;			//数据保存时间为3个HCLK	=4*13.8=55ns
	FSMC_ReadWriteTim.BusTurnAroundDuration=0X00;
	FSMC_ReadWriteTim.AccessMode=FSMC_ACCESS_MODE_A;//模式A
	HAL_SRAM_Init(&DM9000_Handler,&FSMC_ReadWriteTim,&FSMC_ReadWriteTim);	
	

	temp=*(volatile uint32_t*)(0x1FFFF7E8);				//获取STM32的唯一ID的前24位作为MAC地址后三字节
	dm9000cfg.mode=DM9000_AUTO;	
 	dm9000cfg.queue_packet_len=0;
	//DM9000的SRAM的发送和接收指针自动返回到开始地址，并且开启接收中断
	dm9000cfg.imr_all = IMR_PAR|IMR_PRI; 
	//初始化MAC地址
	dm9000cfg.mac_addr[0]=2;
	dm9000cfg.mac_addr[1]=0;
	dm9000cfg.mac_addr[2]=0;
	dm9000cfg.mac_addr[3]=(temp>>16)&0XFF;	//低三字节用STM32的唯一ID
	dm9000cfg.mac_addr[4]=(temp>>8)&0XFFF;
	dm9000cfg.mac_addr[5]=temp&0XFF;
	//初始化组播地址
	dm9000cfg.multicase_addr[0]=0Xff;
	dm9000cfg.multicase_addr[1]=0Xff;
	dm9000cfg.multicase_addr[2]=0Xff;
	dm9000cfg.multicase_addr[3]=0Xff;
	dm9000cfg.multicase_addr[4]=0Xff;
	dm9000cfg.multicase_addr[5]=0Xff;
	dm9000cfg.multicase_addr[6]=0Xff;
	dm9000cfg.multicase_addr[7]=0Xff; 
	
	DM9000_Reset();							//复位DM9000
	Delay_ms(200);
	temp=DM9000_Get_DeiviceID();			//获取DM9000ID
	printf("DM9000 ID:%#x\r\n",temp);
	if(temp!=DM9000_ID) return 1; 			//读取ID错误
	DM9000_Set_PHYMode(dm9000cfg.mode);		//设置PHY工作模式
	
	DM9000_WriteReg(DM9000_NCR,0X00);
	DM9000_WriteReg(DM9000_TCR,0X00);		//发送控制寄存器清零
	DM9000_WriteReg(DM9000_BPTR,0X3F);	
	DM9000_WriteReg(DM9000_FCTR,0X38);
	DM9000_WriteReg(DM9000_FCR,0X00);
	DM9000_WriteReg(DM9000_SMCR,0X00);		//特殊模式
	DM9000_WriteReg(DM9000_NSR,NSR_WAKEST|NSR_TX2END|NSR_TX1END);//清除发送状态
	DM9000_WriteReg(DM9000_ISR,0X0F);		//清除中断状态
	DM9000_WriteReg(DM9000_TCR2,0X80);		//切换LED到mode1 	
	//设置MAC地址和组播地址
	DM9000_Set_MACAddress(dm9000cfg.mac_addr);		//设置MAC地址
	DM9000_Set_Multicast(dm9000cfg.multicase_addr);	//设置组播地址
	DM9000_WriteReg(DM9000_RCR,RCR_DIS_LONG|RCR_DIS_CRC|RCR_RXEN);
	DM9000_WriteReg(DM9000_IMR,IMR_PAR); 
	temp=DM9000_Get_SpeedAndDuplex();		//获取DM9000的连接速度和双工状态
	if(temp!=0XFF)							//连接成功，通过串口显示连接速度和双工状态
	{
		printf("DM9000 Speed:%dMbps,Duplex:%s duplex mode\r\n",(temp&0x02)?10:100,(temp&0x01)?"Full":"Half");
	}else printf("DM9000 Establish Link Failed!\r\n");
	DM9000_WriteReg(DM9000_IMR,dm9000cfg.imr_all);	//设置中断
	return 0;		
}
//外部中断线6的中断服务函数(放exti.c)
void EXTI9_5_IRQHandler(void)
{
	rt_enter_critical();  
	EXTI->PR=1<<6;  			//清除LINE6上的中断标志位   
	//为避免与NRF2401中断引脚冲突，网络数据接收采用独立线程查询的方式处理
//	while(DM9000_INT==0)
//	{
//		DMA9000_ISRHandler();
//	}
	rt_exit_critical();	
} 

//读取DM9000指定寄存器的值
//reg:寄存器地址
//返回值：DM9000指定寄存器的值
uint16_t DM9000_ReadReg(uint16_t reg)
{
	DM9000->REG=reg;
	return DM9000->DATA; 
}
//向DM9000指定寄存器中写入指定值
//reg:要写入的寄存器
//data:要写入的值
void DM9000_WriteReg(uint16_t reg,uint16_t data)
{
	DM9000->REG=reg;
	DM9000->DATA=data;
}
//读取DM9000的PHY的指定寄存器
//reg:要读的PHY寄存器
//返回值:读取到的PHY寄存器值
uint16_t DM9000_PHY_ReadReg(uint16_t reg)
{
	uint16_t temp;
	DM9000_WriteReg(DM9000_EPAR,DM9000_PHY|reg);
	DM9000_WriteReg(DM9000_EPCR,0X0C);				//选中PHY，发送读命令
	Delay_ms(10);
	DM9000_WriteReg(DM9000_EPCR,0X00);				//清除读命令
	temp=(DM9000_ReadReg(DM9000_EPDRH)<<8)|(DM9000_ReadReg(DM9000_EPDRL));
	return temp;
}
//向DM9000的PHY寄存器写入指定值
//reg:PHY寄存器
//data:要写入的值
void DM9000_PHY_WriteReg(uint16_t reg,uint16_t data)
{
	DM9000_WriteReg(DM9000_EPAR,DM9000_PHY|reg);
	DM9000_WriteReg(DM9000_EPDRL,(data&0xff));		//写入低字节
	DM9000_WriteReg(DM9000_EPDRH,((data>>8)&0xff));	//写入高字节
	DM9000_WriteReg(DM9000_EPCR,0X0A);				//选中PHY,发送写命令
	Delay_ms(50);
	DM9000_WriteReg(DM9000_EPCR,0X00);				//清除写命令	
} 
//获取DM9000的芯片ID
//返回值：DM9000的芯片ID值
uint32_t DM9000_Get_DeiviceID(void)
{
	uint32_t value;
	value =DM9000_ReadReg(DM9000_VIDL);
	value|=DM9000_ReadReg(DM9000_VIDH) << 8;
	value|=DM9000_ReadReg(DM9000_PIDL) << 16;
	value|=DM9000_ReadReg(DM9000_PIDH) << 24;
	return value;
}
//获取DM9000的连接速度和双工模式
//返回值：	0,100M半双工
//			1,100M全双工
//			2,10M半双工
//			3,10M全双工
//			0XFF,连接失败！
uint8_t DM9000_Get_SpeedAndDuplex(void)
{
	uint8_t temp;
	uint8_t i=0;	
	if(dm9000cfg.mode==DM9000_AUTO)					//如果开启了自动协商模式一定要等待协商完成
	{
		while(!(DM9000_PHY_ReadReg(0X01)&0X0020))	//等待自动协商完成
		{
			Delay_ms(100);					
			i++;
			if(i>100)return 0XFF;					//自动协商失败
		}	
	}else											//自定义模式,一定要等待连接成功
	{
		while(!(DM9000_ReadReg(DM9000_NSR)&0X40))	//等待连接成功
		{
			Delay_ms(100);					
			i++;
			if(i>100)return 0XFF;					//连接失败			
		}
	}
	temp =((DM9000_ReadReg(DM9000_NSR)>>6)&0X02);	//获取DM9000的连接速度
	temp|=((DM9000_ReadReg(DM9000_NCR)>>3)&0X01);	//获取DM9000的双工状态
	return temp;
}

//设置DM900的PHY工作模式
//mode:PHY模式
void DM9000_Set_PHYMode(uint8_t mode)
{
	uint16_t BMCR_Value,ANAR_Value;	
	switch(mode)
	{
		case DM9000_10MHD:		//10M半双工
			BMCR_Value=0X0000;
			ANAR_Value=0X21;
			break;
		case DM9000_10MFD:		//10M全双工
			BMCR_Value=0X0100;
			ANAR_Value=0X41;
			break;
		case DM9000_100MHD:		//100M半双工
			BMCR_Value=0X2000;
			ANAR_Value=0X81;
			break;
		case DM9000_100MFD:		//100M全双工
			BMCR_Value=0X2100;
			ANAR_Value=0X101;
			break;
		case DM9000_AUTO:		//自动协商模式
			BMCR_Value=0X1000;
			ANAR_Value=0X01E1;
			break;		
	}
	DM9000_PHY_WriteReg(DM9000_PHY_BMCR,BMCR_Value);
	DM9000_PHY_WriteReg(DM9000_PHY_ANAR,ANAR_Value);
 	DM9000_WriteReg(DM9000_GPR,0X00);	//使能PHY
}
//设置DM9000的MAC地址
//macaddr:指向MAC地址
void DM9000_Set_MACAddress(uint8_t *macaddr)
{
	uint8_t i;
	for(i=0;i<6;i++)
	{
		DM9000_WriteReg(DM9000_PAR+i,macaddr[i]);
	}
}
//设置DM9000的组播地址
//multicastaddr:指向多播地址
void DM9000_Set_Multicast(uint8_t *multicastaddr)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		DM9000_WriteReg(DM9000_MAR+i,multicastaddr[i]);
	}
}  
//复位DM9000
void DM9000_Reset(void)
{
	//复位DM9000,复位步骤参考<DM9000 Application Notes V1.22>手册29页
	DM9000_RST = 0;								//DM9000硬件复位
	Delay_ms(10);
	DM9000_RST = 1; 							//DM9000硬件复位结束
	Delay_ms(100);								//一定要有这个延时，让DM9000准备就绪！
 	DM9000_WriteReg(DM9000_GPCR,0x01);			//第一步:设置GPCR寄存器(0X1E)的bit0为1 
	DM9000_WriteReg(DM9000_GPR,0);				//第二步:设置GPR寄存器(0X1F)的bit1为0，DM9000内部的PHY上电
 	DM9000_WriteReg(DM9000_NCR,(0x02|NCR_RST));	//第三步:软件复位DM9000 
	do 
	{
		Delay_ms(25); 	
	}while(DM9000_ReadReg(DM9000_NCR)&1);		//等待DM9000软复位完成
	DM9000_WriteReg(DM9000_NCR,0);
	DM9000_WriteReg(DM9000_NCR,(0x02|NCR_RST));	//DM9000第二次软复位
	do 
	{
		Delay_ms(25);	
	}while (DM9000_ReadReg(DM9000_NCR)&1);
}  

//通过DM9000发送数据包
//p:pbuf结构体指针
void DM9000_SendPacket(struct pbuf *p)
{
	struct pbuf *q;
	uint16_t pbuf_index = 0;
	uint8_t word[2], word_index = 0;	
 	//printf("send len:%d\r\n",p->tot_len);
	rt_sem_take(dm9000lock,RT_WAITING_FOREVER); 			//请求互斥信号量,锁定DM9000 
	DM9000_WriteReg(DM9000_IMR,IMR_PAR);		//关闭网卡中断 
	DM9000->REG=DM9000_MWCMD;					//发送此命令后就可以将要发送的数据搬到DM9000 TX SRAM中	
	q=p;
	//向DM9000的TX SRAM中写入数据，一次写入两个字节数据
	//当要发送的数据长度为奇数的时候，我们需要将最后一个字节单独写入DM9000的TX SRAM中
 	while(q)
	{
		if (pbuf_index < q->len)
		{
			word[word_index++] = ((uint8_t*)q->payload)[pbuf_index++];
			if (word_index == 2)
			{
				DM9000->DATA=((uint16_t)word[1]<<8)|word[0];
				word_index = 0;
			}
		}else
		{
			q=q->next;
			pbuf_index = 0;
		}
	}
	//还有一个字节未写入TX SRAM
	if(word_index==1)DM9000->DATA=word[0];
	//向DM9000写入发送长度
	DM9000_WriteReg(DM9000_TXPLL,p->tot_len&0XFF);
	DM9000_WriteReg(DM9000_TXPLH,(p->tot_len>>8)&0XFF);		//设置要发送数据的数据长度
	DM9000_WriteReg(DM9000_TCR,0X01);						//启动发送 
	while((DM9000_ReadReg(DM9000_ISR)&0X02)==0);			//等待发送完成 
	DM9000_WriteReg(DM9000_ISR,0X02);						//清除发送完成中断 
 	DM9000_WriteReg(DM9000_IMR,dm9000cfg.imr_all);			//DM9000网卡接收中断使能
	rt_sem_release(dm9000lock);								//发送互斥信号量,解锁DM9000
}
//DM9000接收数据包
//接收到的数据包存放在DM9000的RX FIFO中，地址为0X0C00~0X3FFF
//接收到的数据包的前四个字节并不是真实的数据，而是有特定含义的
//byte1:表明是否接收到数据，为0x00或者0X01，如果两个都不是的话一定要软件复位DM9000
//		0x01，接收到数据
//		0x00，未接收到数据
//byte2:第二个字节表示一些状态信息，和DM9000的RSR(0X06)寄存器一致的
//byte3:本帧数据长度的低字节
//byte4:本帧数据长度的高字节
//返回值：pbuf格式的接收到的数据包
struct pbuf *DM9000_Receive_Packet(void)
{
	struct pbuf* p;
	struct pbuf* q;
    uint32_t rxbyte;
	volatile uint16_t rx_status, rx_length;
    uint16_t* data;
	uint16_t dummy; 
	int len;
	p=NULL; 
	rt_sem_take(dm9000lock,RT_WAITING_FOREVER); 				//请求信号量,锁定DM9000
__error_retry:	
	DM9000_ReadReg(DM9000_MRCMDX);					//假读
	rxbyte=(uint8_t)DM9000->DATA;						//进行第二次读取 
	if(rxbyte)										//接收到数据
	{
		if(rxbyte>1)								//rxbyte大于1，接收到的数据错误,挂了		
		{
            printf("dm9000 rx: rx error, stop device\r\n");
			DM9000_WriteReg(DM9000_RCR,0x00);
			DM9000_WriteReg(DM9000_ISR,0x80);		 
			return (struct pbuf*)p;
		}
		DM9000->REG=DM9000_MRCMD;
		rx_status=DM9000->DATA;
        rx_length=DM9000->DATA;  
		//if(rx_length>512)printf("rxlen:%d\r\n",rx_length);
        p=pbuf_alloc(PBUF_RAW,rx_length,PBUF_POOL);	//pbufs内存池分配pbuf
		if(p!=NULL)									//内存申请成功
        {
            for(q=p;q!=NULL;q=q->next)
            {
                data=(uint16_t*)q->payload;
                len=q->len;
                while(len>0)
                {
					*data=DM9000->DATA;
                    data++;
                    len-= 2;
                }
            }
        }else										//内存申请失败
		{
			printf("pbuf内存申请失败:%d\r\n",rx_length);
            data=&dummy;
			len=rx_length;
			while(len)
			{
				*data=DM9000->DATA;
				len-=2;
			}
        }	
		//根据rx_status判断接收数据是否出现如下错误：FIFO溢出、CRC错误
		//对齐错误、物理层错误，如果有任何一个出现的话丢弃该数据帧，
		//当rx_length小于64或者大于最大数据长度的时候也丢弃该数据帧
		if((rx_status&0XBF00) || (rx_length < 0X40) || (rx_length > DM9000_PKT_MAX))
		{
			printf("rx_status:%#x\r\n",rx_status);
			if (rx_status & 0x100)printf("rx fifo error\r\n");
            if (rx_status & 0x200)printf("rx crc error\r\n");
            if (rx_status & 0x8000)printf("rx length error\r\n");
            if (rx_length>DM9000_PKT_MAX)
			{
				printf("rx length too big\r\n");
				DM9000_WriteReg(DM9000_NCR, NCR_RST); 	//复位DM9000
				Delay_ms(5);
			}
			if(p!=NULL)pbuf_free((struct pbuf*)p);		//释放内存
			p=NULL;
			goto __error_retry;
		}
	}else
    {
        DM9000_WriteReg(DM9000_ISR,ISR_PTS);			//清除所有中断标志位
        dm9000cfg.imr_all=IMR_PAR|IMR_PRI;				//重新接收中断 
        DM9000_WriteReg(DM9000_IMR, dm9000cfg.imr_all);
    } 
	rt_sem_release(dm9000lock);							//发送互斥信号量,解锁DM9000
	return (struct pbuf*)p; 
}
//中断处理函数
void DMA9000_ISRHandler(void)
{
	uint16_t int_status;
	uint16_t last_io; 
	last_io = DM9000->REG;
	int_status=DM9000_ReadReg(DM9000_ISR); 
	DM9000_WriteReg(DM9000_ISR,int_status);				//清除中断标志位，DM9000的ISR寄存器的bit0~bit5写1清零
	if(int_status & ISR_ROS)printf("overflow \r\n");
    if(int_status & ISR_ROOS)printf("overflow counter overflow \r\n");	
	if(int_status & ISR_PRS)		//接收中断
	{  
		//printf("Incoming data!\n");
 		//rt_sem_release(dm9000input);//处理接收到数据帧 
	} 
	if(int_status&ISR_PTS)			//发送中断
	{ 
									//接收中断处理,这里没用到
	}
	DM9000->REG=last_io;	
}










#endif













