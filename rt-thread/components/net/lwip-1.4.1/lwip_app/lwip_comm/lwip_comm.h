#ifndef __LWIP_COMM_H
#define __LWIP_COMM_H 
#include "dm9000.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK ս�������� V3
//lwipͨ������ ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/3/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//*******************************************************************************
//�޸���Ϣ
//��
////////////////////////////////////////////////////////////////////////////////// 	   

#define LWIP_MAX_DHCP_TRIES		4   //DHCP������������Դ���

//lwip���ƽṹ��
typedef struct  
{
	uint8_t mac[6];      //MAC��ַ
	uint8_t remoteip[4];	//Զ������IP��ַ 
	uint8_t ip[4];       //����IP��ַ
	uint8_t netmask[4]; 	//��������
	uint8_t gateway[4]; 	//Ĭ�����ص�IP��ַ
	
	volatile uint8_t dhcpstatus;	//dhcp״̬ 
					//0,δ��ȡDHCP��ַ;
					//1,����DHCP��ȡ״̬
					//2,�ɹ���ȡDHCP��ַ
					//0XFF,��ȡʧ��.
}__lwip_dev;
extern __lwip_dev lwipdev;	//lwip���ƽṹ��

void DM9000_LWIP_CreateTask(void);
void lwip_pkt_handle(void *pdata);
void lwip_comm_default_ip_set(__lwip_dev *lwipx);
uint8_t lwip_comm_mem_malloc(void);
void lwip_comm_mem_free(void);
uint8_t lwip_comm_init(void);
void lwip_comm_dhcp_creat(void);
void lwip_comm_dhcp_delete(void);
void lwip_comm_destroy(void);
void lwip_comm_delete_next_timeout(void);

//void lwip_dhcp_task(void *pdata);

#endif













