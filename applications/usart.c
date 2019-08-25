#include "sys.h"
#include "usart.h"	
#include "bsp_led.h"
#include "stm32f1xx_hal.h"
#include "string.h"

//********************************************************************************
//V1.3�޸�˵�� 
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
//V1.5�޸�˵��
//1,�����˶�UCOSII��֧��  
 

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 0
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (uint8_t) ch;      
	return ch;
}
#endif 


#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���   	
uint8_t USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
uint8_t USART_RX2_BUF[USART_REC_LEN];
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
uint16_t USART_RX_STA=0;       //����״̬���	  
uint16_t USART_RX2_STA; 
uint8_t aRxBuffer[RXBUFFERSIZE];  //HAL��ʹ�õĴ��ڽ��ջ���
uint8_t aRx2Buffer[RXBUFFERSIZE];
UART_HandleTypeDef UART1_Handler; //UART1���
UART_HandleTypeDef UART2_Handler; //UART2���  
//��ʼ��IO ����1 
//bound:������
void uart1_init(uint32_t bound)
{	
	//UART ��ʼ������
	UART1_Handler.Instance=USART1;					    //USART1
	UART1_Handler.Init.BaudRate=bound;				    //������
	UART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //�ֳ�Ϊ8λ���ݸ�ʽ
	UART1_Handler.Init.StopBits=UART_STOPBITS_1;	    //һ��ֹͣλ
	UART1_Handler.Init.Parity=UART_PARITY_NONE;		    //����żУ��λ
	UART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //��Ӳ������
	UART1_Handler.Init.Mode=UART_MODE_TX_RX;		    //�շ�ģʽ
	if(HAL_UART_Init(&UART1_Handler)==HAL_OK)			//HAL_UART_Init()��ʹ��UART1
		printf("uart1 Initilized!\n");				    
	HAL_UART_Receive_IT(&UART1_Handler, (uint8_t *)aRxBuffer, RXBUFFERSIZE);//�ú����Ὺ�������жϣ���־λUART_IT_RXNE���������ý��ջ����Լ����ջ���������������
}


void uart2_init(uint32_t bound)
{	
	//UART ��ʼ������
	UART2_Handler.Instance=USART2;					    //USART2
	UART2_Handler.Init.BaudRate=bound;				    //������
	UART2_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //�ֳ�Ϊ8λ���ݸ�ʽ
	UART2_Handler.Init.StopBits=UART_STOPBITS_1;	    //һ��ֹͣλ
	UART2_Handler.Init.Parity=UART_PARITY_NONE;		    //����żУ��λ
	UART2_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //��Ӳ������
	UART2_Handler.Init.Mode=UART_MODE_TX_RX;		    //�շ�ģʽ
	UART2_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    UART2_Handler.Init.OverSampling = UART_OVERSAMPLING_16;
	if(HAL_UART_Init(&UART2_Handler)==HAL_OK)			//HAL_UART_Init()��ʹ��UART1
		printf("\nuart2 Initilized!\n");
	HAL_UART_Receive_IT(&UART2_Handler, (uint8_t *)aRx2Buffer, RXBUFFERSIZE);//�ú����Ὺ�������жϣ���־λUART_IT_RXNE���������ý��ջ����Լ����ջ���������������
    TIM5_Init(19999,35); //��ʱ20ms,ʱ�䲻��̫�̣���������ȷ���ձȽϳ��ĵĲ�������Ϣ
}

//UART�ײ��ʼ����ʱ��ʹ�ܣ��������ã��ж�����
//�˺����ᱻHAL_UART_Init()����
//huart:���ھ��

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_Initure;
	if(huart->Instance==USART1)                 //����Ǵ���1�����д���1 MSP��ʼ��
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();			//ʹ��GPIOAʱ��
		__HAL_RCC_USART1_CLK_ENABLE();			//ʹ��USART1ʱ��
		__HAL_RCC_AFIO_CLK_ENABLE();

		GPIO_Initure.Pin=GPIO_PIN_9;			//PA9
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//�����������
		GPIO_Initure.Pull=GPIO_PULLUP;			//����
		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;//����
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PA9

		GPIO_Initure.Pin=GPIO_PIN_10;			//PA10
		GPIO_Initure.Mode=GPIO_MODE_AF_INPUT;	//ģʽҪ����Ϊ��������ģʽ��	
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PA10
		
#if EN_USART1_RX
		HAL_NVIC_EnableIRQ(USART1_IRQn);				//ʹ��USART1�ж�ͨ��
		HAL_NVIC_SetPriority(USART1_IRQn,3,3);			//��ռ���ȼ�3�������ȼ�3	
#endif			
	}
	
	else if(huart->Instance==USART2)                 //����Ǵ���1�����д���1 MSP��ʼ��
	{
		//ʹ��USART3ʱ��
		__HAL_RCC_USART2_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
		//PA2�����������
		GPIO_Initure.Pin = GPIO_PIN_2;
		GPIO_Initure.Mode = GPIO_MODE_AF_PP;
		GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
		HAL_GPIO_Init(GPIOA, &GPIO_Initure);
		//A3ģʽҪ����Ϊ��������ģʽ��	
		GPIO_Initure.Pin = GPIO_PIN_3;
		GPIO_Initure.Mode = GPIO_MODE_INPUT;
		GPIO_Initure.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_Initure);

#if EN_USART2_RX
	    /* USART2 interrupt Init */
		HAL_NVIC_SetPriority(USART2_IRQn, 0, 1);  //��ռ���ȼ�3�������ȼ�0	
		HAL_NVIC_EnableIRQ(USART2_IRQn);
#endif
		}
}

#if 0
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance==USART1)//����Ǵ���1
	{
		if((USART_RX_STA&0x8000)==0)//����δ���
		{
			if(USART_RX_STA&0x4000)//���յ���0x0d
			{
				if(aRxBuffer[0]!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
			}
			else //��û�յ�0X0D
			{	
				if(aRxBuffer[0]==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=aRxBuffer[0] ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}
	}
	else if(huart->Instance==USART2)//����Ǵ���2
	{
		if((USART_RX2_STA&0x8000)==0)//����δ���
		{
			if(USART_RX2_STA&0x4000)//���յ���0x0d
			{
				if(aRx2Buffer[0]!=0x0a)USART_RX2_STA=0;//���մ���,���¿�ʼ
				else USART_RX2_STA|=0x8000;	//��������� 
			}
			else //��û�յ�0X0D
			{	
				if(aRx2Buffer[0]==0x0d)USART_RX2_STA|=0x4000;
				else
				{
					USART_RX2_BUF[USART_RX2_STA&0X3FFF]=aRx2Buffer[0] ;
					USART_RX2_STA++;
					if(USART_RX2_STA>(USART_REC_LEN-1))USART_RX2_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}
	}
}
 
//����1�жϷ������
void USART1_IRQHandler(void)                	
{ 
	uint32_t timeout=0;
	HAL_UART_IRQHandler(&UART1_Handler);	//����HAL���жϴ����ú���
	timeout=0;
    while (HAL_UART_GetState(&UART1_Handler) != HAL_UART_STATE_READY)//�ȴ�����
	{
	 timeout++;////��ʱ����
     if(timeout>HAL_MAX_DELAY) break;		
	
	}
    
	timeout=0;
	while(HAL_UART_Receive_IT(&UART1_Handler, (uint8_t *)aRxBuffer, RXBUFFERSIZE) != HAL_OK)//һ�δ������֮�����¿����жϲ�����RxXferCountΪ1
	{
	 timeout++; //��ʱ����
	 if(timeout>HAL_MAX_DELAY) break;	
	}
} 


//����1�жϷ������
void USART2_IRQHandler(void)  
{
    uint32_t timeout=0;
	HAL_UART_IRQHandler(&UART2_Handler);	//����HAL���жϴ����ú���
	timeout=0;
    while (HAL_UART_GetState(&UART2_Handler) != HAL_UART_STATE_READY)//�ȴ�����
	{
	  timeout++;////��ʱ����
      if(timeout>HAL_MAX_DELAY) break;		
	}
     
	timeout=0;
	while(HAL_UART_Receive_IT(&UART2_Handler, (uint8_t *)aRx2Buffer, RXBUFFERSIZE) != HAL_OK)//һ�δ������֮�����¿����жϲ�����RxXferCountΪ1
	{
	  timeout++; //��ʱ����
	  if(timeout>HAL_MAX_DELAY) break;	
	}
}	
#endif	

#if 0
/*�����������ֱ�Ӱ��жϿ����߼�д���жϷ������ڲ���*/
void USART1_IRQHandler(void)                	
{ 
	uint8_t Res;
	//HAL_StatusTypeDef err;
	//�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	if((__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_RXNE)!=RESET))  
	{
		Res=USART1->DR; 
		if((USART_RX_STA&0x8000)==0)//����δ���
		{
			if(USART_RX_STA&0x4000)//���յ���0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
			}
			else //��û�յ�0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}   		 
	}
	HAL_UART_IRQHandler(&UART1_Handler);	
} 

#endif
//����2�ж�
void USART2_IRQHandler(void)                	
{ 
	uint8_t Res;
	__HAL_TIM_ENABLE(&TIM5_Handler);	//������2���յ�����ʱ������ʱ��5�������ж�һ����������
	if((__HAL_UART_GET_FLAG(&UART2_Handler,UART_FLAG_RXNE)!=RESET))  
	{ 
		Res=USART2->DR; 
		TIM5->CNT =0;       			//�������ݵĹ����в��Ͻ�TIM3�ļ��������㣬�����ݽ�����ɣ�
										//û�д����жϲ���ʱ��ֹͣ���㣬TIM3�����������ʶ�������ݽ������
		RED_LED = !RED_LED;
		USART_RX2_BUF[USART_RX2_STA&0X7FFF]=Res;
		USART_RX2_STA++;
		if(USART_RX2_STA>(USART_REC_LEN-1))
			USART_RX2_STA=0;//�������ݴ���,���¿�ʼ����	  
	}   		 
	HAL_UART_IRQHandler(&UART2_Handler);	
} 

//����2�����ַ���
void usart2_senddata(char* c)
{
	while(*c!='\0')
	{
		while ((__HAL_UART_GET_FLAG(&UART2_Handler, UART_FLAG_TXE) == RESET));
		UART2_Handler.Instance->DR = *c++;
	}
}

#endif
