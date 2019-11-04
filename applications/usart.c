#include "sys.h"
#include "usart.h"	
#include "bsp_led.h"
#include "stm32f1xx_hal.h"
#include "string.h"

//********************************************************************************
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//加入了对printf的支持
//增加了串口接收命令功能.
//修正了printf第一个字符丢失的bug
//V1.4修改说明
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
//V1.5修改说明
//1,增加了对UCOSII的支持  
 

//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 0
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
    USART1->DR = (uint8_t) ch;      
	return ch;
}
#endif 


#if EN_USART1_RX   //如果使能了接收
//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
uint8_t USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
uint8_t USART_RX2_BUF[USART_REC_LEN];
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
uint16_t USART_RX_STA=0;       //接收状态标记	  
uint16_t USART_RX2_STA; 
uint8_t aRxBuffer[RXBUFFERSIZE];  //HAL库使用的串口接收缓冲
uint8_t aRx2Buffer[RXBUFFERSIZE];
UART_HandleTypeDef UART1_Handler; //UART1句柄
UART_HandleTypeDef UART2_Handler; //UART2句柄  
//初始化IO 串口1 
//bound:波特率
void uart1_init(uint32_t bound)
{	
	//UART 初始化设置
	UART1_Handler.Instance=USART1;					    //USART1
	UART1_Handler.Init.BaudRate=bound;				    //波特率
	UART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //字长为8位数据格式
	UART1_Handler.Init.StopBits=UART_STOPBITS_1;	    //一个停止位
	UART1_Handler.Init.Parity=UART_PARITY_NONE;		    //无奇偶校验位
	UART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //无硬件流控
	UART1_Handler.Init.Mode=UART_MODE_TX_RX;		    //收发模式
	if(HAL_UART_Init(&UART1_Handler)==HAL_OK)			//HAL_UART_Init()会使能UART1
		printf("uart1 Initilized!\n");				    
	HAL_UART_Receive_IT(&UART1_Handler, (uint8_t *)aRxBuffer, RXBUFFERSIZE);//该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量
}


void uart2_init(uint32_t bound)
{	
	//UART 初始化设置
	UART2_Handler.Instance=USART2;					    //USART2
	UART2_Handler.Init.BaudRate=bound;				    //波特率
	UART2_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //字长为8位数据格式
	UART2_Handler.Init.StopBits=UART_STOPBITS_1;	    //一个停止位
	UART2_Handler.Init.Parity=UART_PARITY_NONE;		    //无奇偶校验位
	UART2_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //无硬件流控
	UART2_Handler.Init.Mode=UART_MODE_TX_RX;		    //收发模式
	UART2_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UART2_Handler.Init.OverSampling = UART_OVERSAMPLING_16;
	if(HAL_UART_Init(&UART2_Handler)==HAL_OK)			//HAL_UART_Init()会使能UART1
		printf("\nuart2 Initilized!\n");
	HAL_UART_Receive_IT(&UART2_Handler, (uint8_t *)aRx2Buffer, RXBUFFERSIZE);//该函数会开启接收中断：标志位UART_IT_RXNE，并且设置接收缓冲以及接收缓冲接收最大数据量
    TIM5_Init(29999,35); //定时30ms,时间不能太短，否则不能正确接收比较长的的不定长信息
}

//UART底层初始化，时钟使能，引脚配置，中断配置
//此函数会被HAL_UART_Init()调用
//huart:串口句柄

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO端口设置
	GPIO_InitTypeDef GPIO_Initure;
	if(huart->Instance==USART1)                 //如果是串口1，进行串口1 MSP初始化
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();			//使能GPIOA时钟
		__HAL_RCC_USART1_CLK_ENABLE();			//使能USART1时钟
		__HAL_RCC_AFIO_CLK_ENABLE();

		GPIO_Initure.Pin=GPIO_PIN_9;			//PA9
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//复用推挽输出
		GPIO_Initure.Pull=GPIO_PULLUP;			//上拉
		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;//高速
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PA9

		GPIO_Initure.Pin=GPIO_PIN_10;			//PA10
		GPIO_Initure.Mode=GPIO_MODE_AF_INPUT;	//模式要设置为复用输入模式！	
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//初始化PA10
		
#if EN_USART1_RX
		HAL_NVIC_EnableIRQ(USART1_IRQn);				//使能USART1中断通道
		HAL_NVIC_SetPriority(USART1_IRQn,3,3);			//抢占优先级3，子优先级3	
#endif			
	}
	
	else if(huart->Instance==USART2)                 //如果是串口1，进行串口1 MSP初始化
	{
		//使能USART3时钟
		__HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
		//PA2复用推挽输出
		GPIO_Initure.Pin = GPIO_PIN_2;
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOA, &GPIO_Initure);
		//A3模式要设置为复用输入模式！	
		GPIO_Initure.Pin = GPIO_PIN_3;
		GPIO_Initure.Mode = GPIO_MODE_INPUT;
		GPIO_Initure.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_Initure);

#if EN_USART2_RX
	    /* USART2 interrupt Init */
		HAL_NVIC_SetPriority(USART2_IRQn, 0, 1);  //抢占优先级3，子优先级0	
		HAL_NVIC_EnableIRQ(USART2_IRQn);
#endif
		}
}

#if 0
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)//如果是串口1
	{
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(aRxBuffer[0]!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(aRxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0] ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}
	}
	else if(huart->Instance==USART2)//如果是串口2
	{
		if((USART_RX2_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX2_STA&0x4000)//接收到了0x0d
			{
				if(aRx2Buffer[0]!=0x0a)USART_RX2_STA=0;//接收错误,重新开始
				else USART_RX2_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(aRx2Buffer[0]==0x0d)USART_RX2_STA|=0x4000;
				else
				{
					USART_RX2_BUF[USART_RX2_STA&0X3FFF]=aRx2Buffer[0] ;
					USART_RX2_STA++;
					if(USART_RX2_STA>(USART_REC_LEN-1))USART_RX2_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}
	}
}
 
//串口1中断服务程序
void USART1_IRQHandler(void)                	
{ 
	uint32_t timeout=0;
	HAL_UART_IRQHandler(&UART1_Handler);	//调用HAL库中断处理公用函数
	timeout=0;
    while (HAL_UART_GetState(&UART1_Handler) != HAL_UART_STATE_READY)//等待就绪
	{
	 timeout++;////超时处理
     if(timeout>HAL_MAX_DELAY) break;		
	
	}
    
	timeout=0;
	while(HAL_UART_Receive_IT(&UART1_Handler, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK)//一次处理完成之后，重新开启中断并设置RxXferCount为1
	{
	 timeout++; //超时处理
	 if(timeout>HAL_MAX_DELAY) break;	
	}
} 


//串口1中断服务程序
void USART2_IRQHandler(void)  
{
    uint32_t timeout=0;
	HAL_UART_IRQHandler(&UART2_Handler);	//调用HAL库中断处理公用函数
	timeout=0;
    while (HAL_UART_GetState(&UART2_Handler) != HAL_UART_STATE_READY)//等待就绪
	{
	  timeout++;////超时处理
      if(timeout>HAL_MAX_DELAY) break;		
	}
     
	timeout=0;
	while(HAL_UART_Receive_IT(&UART2_Handler, (uint8_t *)aRx2Buffer, RXBUFFERSIZE) != HAL_OK)//一次处理完成之后，重新开启中断并设置RxXferCount为1
	{
	  timeout++; //超时处理
	  if(timeout>HAL_MAX_DELAY) break;	
	}
}	
#endif	

#if 0
/*下面代码我们直接把中断控制逻辑写在中断服务函数内部。*/
void USART1_IRQHandler(void)                	
{ 
	uint8_t Res;
	//HAL_StatusTypeDef err;
	//接收中断(接收到的数据必须是0x0d 0x0a结尾)
	if((__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_RXNE)!=RESET))  
	{
		Res=USART1->DR; 
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}   		 
	}
	HAL_UART_IRQHandler(&UART1_Handler);	
} 

#endif
//串口2中断
void USART2_IRQHandler(void)                	
{ 
	uint8_t Res;
	__HAL_TIM_ENABLE(&TIM5_Handler);	//当串口2接收到数据时启动定时器5，用于判断一个不定长的
	if((__HAL_UART_GET_FLAG(&UART2_Handler,UART_FLAG_RXNE)!=RESET))  
	{ 
		Res=USART2->DR; 
		TIM5->CNT =0;       			//接收数据的过程中不断将TIM3的计数器清零，当数据接收完成，
										//没有串口中断产生时，停止清零，TIM3产生溢出，标识串口数据接收完成
		RED_LED = !RED_LED;
		USART_RX2_BUF[USART_RX2_STA&0X7FFF]=Res;
		USART_RX2_STA++;
		if(USART_RX2_STA>(USART_REC_LEN-1))
			USART_RX2_STA=0;//接收数据错误,重新开始接收	  
	}   		 
	HAL_UART_IRQHandler(&UART2_Handler);	
} 

//串口2发送字符串
void usart2_senddata(char* c)
{
	while(*c!='\0')
	{
		while ((__HAL_UART_GET_FLAG(&UART2_Handler, UART_FLAG_TXE) == RESET));
		UART2_Handler.Instance->DR = *c++;
	}
}

#endif
