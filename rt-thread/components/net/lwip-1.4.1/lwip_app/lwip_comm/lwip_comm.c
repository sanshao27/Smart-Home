#include "lwip_comm.h" 
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/init.h"
#include "ethernetif.h" 
#include "lwip/timers.h"
#include "lwip/tcp_impl.h"
#include "lwip/ip_frag.h"
#include "lwip/tcpip.h" 
#include "malloc.h"
#include "usart.h"  
#include <stdio.h>
#include <rthw.h>
#include <rtthread.h>

#if 1   
//��OS�汾��lwipʵ��,�����Ҫ3������: lwip�ں�����(������),dhcp����(��ѡ),DM9000��������(������)
//�������������񶼴���.
//����,����Ҫ�õ�2���ź���,���ں���ķ���DM9000.

//lwip DHCP����
static struct rt_thread LWIP_DHCP_Thread;
static struct rt_thread LWIP_DM9000_INPUT_Thread;
//�����������ȼ�
#define LWIP_DHCP_TASK_PRIO       		3
ALIGN(RT_ALIGN_SIZE)
static char LWIP_DHCP_TASK_STK[512];	
//������
void lwip_dhcp_task(void *pdata); 
//lwip DM9000���ݽ��մ�������
//�����������ȼ�
#define LWIP_DM9000_INPUT_TASK_PRIO		2
ALIGN(RT_ALIGN_SIZE)
static char LWIP_DM9000_INPUT_TASK_STK[512];	
//������
void lwip_dm9000_input_task(void *pdata); 

//DM9000���������ź���
rt_sem_t dm9000input;
//DM9000��д���������ź���
rt_sem_t dm9000lock;		

//////////////////////////////////////////////////////////////////////////////////////////
__lwip_dev lwipdev;						//lwip���ƽṹ�� 
struct netif lwip_netif;				//����һ��ȫ�ֵ�����ӿ�
extern u32_t memp_get_memorysize(void);	//��memp.c���涨��
extern u8_t *memp_memory;				//��memp.c���涨��.
extern u8_t *ram_heap;					//��mem.c���涨��.




//DM9000���ݽ��մ�������
void lwip_dm9000_input_task(void *pdata)
{
	//�����绺�����ж�ȡ���յ������ݰ������䷢�͸�LWIP���� 
	ethernetif_input(&lwip_netif);
}
#if 0
//lwip�ں˲���,�ڴ�����
//����ֵ:0,�ɹ�;
//    ����,ʧ��
uint8_t lwip_comm_mem_malloc(void)
{
	uint32_t mempsize;
	uint32_t ramheapsize; 
	mempsize=memp_get_memorysize();			//�õ�memp_memory�����С
	memp_memory=mymalloc(SRAMEX,mempsize);	//Ϊmemp_memory�����ڴ�
	printf("memp_memory�ڴ��СΪ:%d\r\n",mempsize);
	ramheapsize=LWIP_MEM_ALIGN_SIZE(MEM_SIZE)+2*LWIP_MEM_ALIGN_SIZE(4*3)+MEM_ALIGNMENT;//�õ�ram heap��С
	ram_heap=mymalloc(SRAMEX,ramheapsize);	//Ϊram_heap�����ڴ� 
	printf("ram_heap�ڴ��СΪ:%d\r\n",ramheapsize);
	//TCPIP_THREAD_TASK_STK=mymalloc(SRAMEX,TCPIP_THREAD_STACKSIZE*4);			//���ں����������ջ 
	LWIP_DHCP_TASK_STK=mymalloc(SRAMEX,LWIP_DHCP_STK_SIZE*4);					//��dhcp���������ջ 
	LWIP_DM9000_INPUT_TASK_STK=mymalloc(SRAMEX,LWIP_DM9000_INPUT_TASK_SIZE*4);	//��dm9000�������������ջ 
	if(!memp_memory||!ram_heap||!LWIP_DHCP_TASK_STK||!LWIP_DM9000_INPUT_TASK_STK)//������ʧ�ܵ�
	{
		lwip_comm_mem_free();
		return 1;
	}
	return 0;	
}
//lwip�ں˲���,�ڴ��ͷ�
void lwip_comm_mem_free(void)
{ 	
	myfree(SRAMIN,memp_memory);
	myfree(SRAMIN,ram_heap);
	//myfree(SRAMIN,TCPIP_THREAD_TASK_STK);
	myfree(SRAMIN,LWIP_DHCP_TASK_STK);
	myfree(SRAMIN,LWIP_DM9000_INPUT_TASK_STK);
}
#endif
//lwip Ĭ��IP����
//lwipx:lwip���ƽṹ��ָ��
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	//Ĭ��Զ��IPΪ:192.168.1.100
	lwipx->remoteip[0]=192;	
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=1;
	lwipx->remoteip[3]=100;
	//MAC��ַ����(�����ֽڹ̶�Ϊ:2.0.0,�����ֽ���STM32ΨһID)
	lwipx->mac[0]=dm9000cfg.mac_addr[0];
	lwipx->mac[1]=dm9000cfg.mac_addr[1];
	lwipx->mac[2]=dm9000cfg.mac_addr[2];
	lwipx->mac[3]=dm9000cfg.mac_addr[3];
	lwipx->mac[4]=dm9000cfg.mac_addr[4];
	lwipx->mac[5]=dm9000cfg.mac_addr[5]; 
	//Ĭ�ϱ���IPΪ:192.168.1.30
	lwipx->ip[0]=192;	
	lwipx->ip[1]=168;
	lwipx->ip[2]=0;
	lwipx->ip[3]=30;
	//Ĭ����������:255.255.255.0
	lwipx->netmask[0]=255;	
	lwipx->netmask[1]=255;
	lwipx->netmask[2]=255;
	lwipx->netmask[3]=0;
	//Ĭ������:192.168.1.1
	lwipx->gateway[0]=192;	
	lwipx->gateway[1]=168;
	lwipx->gateway[2]=1;
	lwipx->gateway[3]=1;	
	lwipx->dhcpstatus=0;//û��DHCP	
} 

//LWIP��ʼ��(LWIP������ʱ��ʹ��)
//����ֵ:0,�ɹ�
//      1,�ڴ����
//      2,DM9000��ʼ��ʧ��
//      3,�������ʧ��.
uint8_t lwip_comm_init(void)
{
	rt_err_t result = 0; 
	struct netif *Netif_Init_Flag;		//����netif_add()����ʱ�ķ���ֵ,�����ж������ʼ���Ƿ�ɹ�
	struct ip_addr ipaddr;  			//ip��ַ
	struct ip_addr netmask; 			//��������
	struct ip_addr gw;      			//Ĭ������ 
	//if(lwip_comm_mem_malloc())return 1;	//�ڴ�����ʧ��
	//�������ݽ����ź���,������DM9000��ʼ��֮ǰ����
	rt_kprintf("����dm9000input��dm9000lock�ź���\n");
	dm9000input=rt_sem_create("dm9000input",0,RT_IPC_FLAG_FIFO);
	//���������ź���,��ߵ����ȼ�2	
 	dm9000lock=rt_sem_create("dm9000lock",1,RT_IPC_FLAG_FIFO);
	rt_kprintf("��ʼ��DM9000AEP\n");
	//��ʼ��DM9000AEP
	if(DM9000_Init())return 2;			
	rt_kprintf("tcpip_init��ʼ��\n");
	//��ʼ��tcp ip�ں�,�ú�������ᴴ��tcpip_thread�ں�����
	tcpip_init(NULL,NULL);				
	rt_kprintf("����Ĭ��IP����Ϣ\n");
	//����Ĭ��IP����Ϣ
	lwip_comm_default_ip_set(&lwipdev);	
	//ʹ�ö�̬IP
#if LWIP_DHCP		
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
	printf("��̬IP��ַ........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	printf("��������..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
	printf("Ĭ������..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
#endif
    //�������б������һ������
	rt_kprintf("ע������\n"); 
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&tcpip_input);
	//������ӳɹ���,����netifΪĬ��ֵ,���Ҵ�netif����
	if(Netif_Init_Flag != NULL) 	
	{
		//����netifΪĬ������
		rt_kprintf("����netifΪĬ������\n"); 
		netif_set_default(&lwip_netif); 
		//��netif����
		rt_kprintf("��netif����\n"); 
		netif_set_up(&lwip_netif);		
	}
	//�����ٽ���
	rt_enter_critical();  		
	//������̫�����ݽ�������
    result = rt_thread_init( &LWIP_DM9000_INPUT_Thread,
							"LWIP_DM9000_INPUT_Thread",
							 lwip_dm9000_input_task,
							 RT_NULL,
							 &LWIP_DM9000_INPUT_TASK_STK[0],
							 sizeof(LWIP_DM9000_INPUT_TASK_STK),
							 LWIP_DM9000_INPUT_TASK_PRIO,
							 200  	
							);
	if(result==RT_EOK)
	{
		rt_thread_startup(&LWIP_DM9000_INPUT_Thread);
		rt_kprintf("��̫�����ݽ�������������\n");
	}
	rt_exit_critical();			//�˳��ٽ���
#if	LWIP_DHCP
	//����DHCP����
	lwip_comm_dhcp_creat();		
#endif		
	return 0;//����OK.
}   
//���ʹ����DHCP
#if LWIP_DHCP
//����DHCP����
void lwip_comm_dhcp_creat(void)
{
	rt_err_t result = 0; 
	rt_enter_critical();  		//�����ٽ���
	//����DHCP���� 
	result = rt_thread_init( &LWIP_DHCP_Thread,
							"LWIP_DHCP_Thread",
							 lwip_dhcp_task,
							 RT_NULL,
							 &LWIP_DHCP_TASK_STK[0],
							 sizeof(LWIP_DHCP_TASK_STK),
							 LWIP_DHCP_TASK_PRIO,
							 200  	
							);
	//����DHCP����
	if(result==RT_EOK)
	{
	  rt_thread_startup(&LWIP_DHCP_Thread);
	  rt_kprintf("DHCP����������\n");
	}
	//�˳��ٽ���
	rt_exit_critical();				
}

//ɾ��DHCP����
void lwip_comm_dhcp_delete(void)
{
	dhcp_stop(&lwip_netif); 		        //�ر�DHCP
	rt_thread_delete(&LWIP_DHCP_Thread);	//ɾ��DHCP����
}
//DHCP��������
void lwip_dhcp_task(void *pdata)
{
	uint32_t ip=0,netmask=0,gw=0;
	dhcp_start(&lwip_netif);//����DHCP 
	lwipdev.dhcpstatus=0;	//����DHCP
	printf("���ڲ���DHCP������,���Ե�...........\r\n");   
	while(1)
	{ 
		printf("���ڻ�ȡ��ַ...\r\n");
		ip=lwip_netif.ip_addr.addr;		//��ȡ��IP��ַ
		netmask=lwip_netif.netmask.addr;//��ȡ��������
		gw=lwip_netif.gw.addr;			//��ȡĬ������ 
		if(ip!=0)   					//����ȷ��ȡ��IP��ַ��ʱ��
		{
			lwipdev.dhcpstatus=2;	//DHCP�ɹ�
 			printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			//������ͨ��DHCP��ȡ����IP��ַ
			lwipdev.ip[3]=(uint8_t)(ip>>24); 
			lwipdev.ip[2]=(uint8_t)(ip>>16);
			lwipdev.ip[1]=(uint8_t)(ip>>8);
			lwipdev.ip[0]=(uint8_t)(ip);
			printf("ͨ��DHCP��ȡ��IP��ַ..............%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			//����ͨ��DHCP��ȡ�������������ַ
			lwipdev.netmask[3]=(uint8_t)(netmask>>24);
			lwipdev.netmask[2]=(uint8_t)(netmask>>16);
			lwipdev.netmask[1]=(uint8_t)(netmask>>8);
			lwipdev.netmask[0]=(uint8_t)(netmask);
			printf("ͨ��DHCP��ȡ����������............%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			//������ͨ��DHCP��ȡ����Ĭ������
			lwipdev.gateway[3]=(uint8_t)(gw>>24);
			lwipdev.gateway[2]=(uint8_t)(gw>>16);
			lwipdev.gateway[1]=(uint8_t)(gw>>8);
			lwipdev.gateway[0]=(uint8_t)(gw);
			printf("ͨ��DHCP��ȡ����Ĭ������..........%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES) //ͨ��DHCP�����ȡIP��ַʧ��,�ҳ�������Դ���
		{  
			lwipdev.dhcpstatus=0XFF;//DHCPʧ��.
			//ʹ�þ�̬IP��ַ
			IP4_ADDR(&(lwip_netif.ip_addr),lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			IP4_ADDR(&(lwip_netif.netmask),lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			IP4_ADDR(&(lwip_netif.gw),lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			printf("DHCP����ʱ,ʹ�þ�̬IP��ַ!\r\n");
			printf("����en��MAC��ַΪ:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			printf("��̬IP��ַ........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			printf("��������..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			printf("Ĭ������..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}  
		Delay_ms(250); //��ʱ250ms
	}
	dhcp_stop(&lwip_netif); //�ر�DHCP
	rt_thread_mdelay(10000);//ɾ��DHCP����
}
#endif 



#endif

















































