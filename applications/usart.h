#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
#include "stm32f1xx_hal.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//����1��ʼ��		   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/8/18
//�汾��V1.5
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
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

//��������
#define RXBUFFERSIZE            1 			//�����С
#define USART_REC_LEN  			1024  		//�����������ֽ��� 2048

//����1��ز���
#define EN_USART1_RX 			1			    //ʹ�ܣ�1��/��ֹ��0������1����
extern  uint8_t  USART_RX_BUF[USART_REC_LEN]; 	//���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern  uint16_t USART_RX_STA;         			//����״̬���	
extern  UART_HandleTypeDef UART1_Handler; 	    //UART1���
extern  uint8_t aRxBuffer[RXBUFFERSIZE];			//HAL��USART����Buffer

//����2��ز���
#define EN_USART2_RX 			1	
extern  uint8_t  USART_RX2_BUF[USART_REC_LEN]; 	//���ջ���,���USART_REC_LEN���ֽ�.ĩ�ֽ�Ϊ���з� 
extern  uint16_t USART_RX2_STA;         		//����״̬���	
extern  UART_HandleTypeDef UART2_Handler; 	    //UART3���
extern  uint8_t aRx2Buffer[RXBUFFERSIZE];		//HAL��USART����Buffer



//����봮���жϽ��գ��벻Ҫע�����º궨��
extern void Delay_ms(uint16_t ms);
void uart1_init(uint32_t bound);
void uart2_init(uint32_t bound);
int8_t usart2_recvData(uint8_t *str);
void usart2_senddata(char* c);
#endif
