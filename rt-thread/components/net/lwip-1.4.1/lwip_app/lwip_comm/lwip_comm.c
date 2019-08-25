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
//带OS版本的lwip实验,最多需要3个任务: lwip内核任务(必须有),dhcp任务(可选),DM9000接收任务(必须有)
//本例程三个任务都创建.
//另外,还需要用到2个信号量,用于合理的访问DM9000.

//lwip DHCP任务
static struct rt_thread LWIP_DHCP_Thread;
static struct rt_thread LWIP_DM9000_INPUT_Thread;
//设置任务优先级
#define LWIP_DHCP_TASK_PRIO       		3
ALIGN(RT_ALIGN_SIZE)
static char LWIP_DHCP_TASK_STK[512];	
//任务函数
void lwip_dhcp_task(void *pdata); 
//lwip DM9000数据接收处理任务
//设置任务优先级
#define LWIP_DM9000_INPUT_TASK_PRIO		2
ALIGN(RT_ALIGN_SIZE)
static char LWIP_DM9000_INPUT_TASK_STK[512];	
//任务函数
void lwip_dm9000_input_task(void *pdata); 

//DM9000接收数据信号量
rt_sem_t dm9000input;
//DM9000读写互锁控制信号量
rt_sem_t dm9000lock;		

//////////////////////////////////////////////////////////////////////////////////////////
__lwip_dev lwipdev;						//lwip控制结构体 
struct netif lwip_netif;				//定义一个全局的网络接口
extern u32_t memp_get_memorysize(void);	//在memp.c里面定义
extern u8_t *memp_memory;				//在memp.c里面定义.
extern u8_t *ram_heap;					//在mem.c里面定义.




//DM9000数据接收处理任务
void lwip_dm9000_input_task(void *pdata)
{
	//从网络缓冲区中读取接收到的数据包并将其发送给LWIP处理 
	ethernetif_input(&lwip_netif);
}
#if 0
//lwip内核部分,内存申请
//返回值:0,成功;
//    其他,失败
uint8_t lwip_comm_mem_malloc(void)
{
	uint32_t mempsize;
	uint32_t ramheapsize; 
	mempsize=memp_get_memorysize();			//得到memp_memory数组大小
	memp_memory=mymalloc(SRAMEX,mempsize);	//为memp_memory申请内存
	printf("memp_memory内存大小为:%d\r\n",mempsize);
	ramheapsize=LWIP_MEM_ALIGN_SIZE(MEM_SIZE)+2*LWIP_MEM_ALIGN_SIZE(4*3)+MEM_ALIGNMENT;//得到ram heap大小
	ram_heap=mymalloc(SRAMEX,ramheapsize);	//为ram_heap申请内存 
	printf("ram_heap内存大小为:%d\r\n",ramheapsize);
	//TCPIP_THREAD_TASK_STK=mymalloc(SRAMEX,TCPIP_THREAD_STACKSIZE*4);			//给内核任务申请堆栈 
	LWIP_DHCP_TASK_STK=mymalloc(SRAMEX,LWIP_DHCP_STK_SIZE*4);					//给dhcp任务申请堆栈 
	LWIP_DM9000_INPUT_TASK_STK=mymalloc(SRAMEX,LWIP_DM9000_INPUT_TASK_SIZE*4);	//给dm9000接收任务申请堆栈 
	if(!memp_memory||!ram_heap||!LWIP_DHCP_TASK_STK||!LWIP_DM9000_INPUT_TASK_STK)//有申请失败的
	{
		lwip_comm_mem_free();
		return 1;
	}
	return 0;	
}
//lwip内核部分,内存释放
void lwip_comm_mem_free(void)
{ 	
	myfree(SRAMIN,memp_memory);
	myfree(SRAMIN,ram_heap);
	//myfree(SRAMIN,TCPIP_THREAD_TASK_STK);
	myfree(SRAMIN,LWIP_DHCP_TASK_STK);
	myfree(SRAMIN,LWIP_DM9000_INPUT_TASK_STK);
}
#endif
//lwip 默认IP设置
//lwipx:lwip控制结构体指针
void lwip_comm_default_ip_set(__lwip_dev *lwipx)
{
	//默认远端IP为:192.168.1.100
	lwipx->remoteip[0]=192;	
	lwipx->remoteip[1]=168;
	lwipx->remoteip[2]=1;
	lwipx->remoteip[3]=100;
	//MAC地址设置(高三字节固定为:2.0.0,低三字节用STM32唯一ID)
	lwipx->mac[0]=dm9000cfg.mac_addr[0];
	lwipx->mac[1]=dm9000cfg.mac_addr[1];
	lwipx->mac[2]=dm9000cfg.mac_addr[2];
	lwipx->mac[3]=dm9000cfg.mac_addr[3];
	lwipx->mac[4]=dm9000cfg.mac_addr[4];
	lwipx->mac[5]=dm9000cfg.mac_addr[5]; 
	//默认本地IP为:192.168.1.30
	lwipx->ip[0]=192;	
	lwipx->ip[1]=168;
	lwipx->ip[2]=0;
	lwipx->ip[3]=30;
	//默认子网掩码:255.255.255.0
	lwipx->netmask[0]=255;	
	lwipx->netmask[1]=255;
	lwipx->netmask[2]=255;
	lwipx->netmask[3]=0;
	//默认网关:192.168.1.1
	lwipx->gateway[0]=192;	
	lwipx->gateway[1]=168;
	lwipx->gateway[2]=1;
	lwipx->gateway[3]=1;	
	lwipx->dhcpstatus=0;//没有DHCP	
} 

//LWIP初始化(LWIP启动的时候使用)
//返回值:0,成功
//      1,内存错误
//      2,DM9000初始化失败
//      3,网卡添加失败.
uint8_t lwip_comm_init(void)
{
	rt_err_t result = 0; 
	struct netif *Netif_Init_Flag;		//调用netif_add()函数时的返回值,用于判断网络初始化是否成功
	struct ip_addr ipaddr;  			//ip地址
	struct ip_addr netmask; 			//子网掩码
	struct ip_addr gw;      			//默认网关 
	//if(lwip_comm_mem_malloc())return 1;	//内存申请失败
	//创建数据接收信号量,必须在DM9000初始化之前创建
	rt_kprintf("创建dm9000input与dm9000lock信号量\n");
	dm9000input=rt_sem_create("dm9000input",0,RT_IPC_FLAG_FIFO);
	//创建互斥信号量,提高到优先级2	
 	dm9000lock=rt_sem_create("dm9000lock",1,RT_IPC_FLAG_FIFO);
	rt_kprintf("初始化DM9000AEP\n");
	//初始化DM9000AEP
	if(DM9000_Init())return 2;			
	rt_kprintf("tcpip_init初始化\n");
	//初始化tcp ip内核,该函数里面会创建tcpip_thread内核任务
	tcpip_init(NULL,NULL);				
	rt_kprintf("设置默认IP等信息\n");
	//设置默认IP等信息
	lwip_comm_default_ip_set(&lwipdev);	
	//使用动态IP
#if LWIP_DHCP		
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
#else
	IP4_ADDR(&ipaddr,lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	IP4_ADDR(&netmask,lwipdev.netmask[0],lwipdev.netmask[1] ,lwipdev.netmask[2],lwipdev.netmask[3]);
	IP4_ADDR(&gw,lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
	printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
	printf("静态IP地址........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
	printf("子网掩码..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
	printf("默认网关..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
#endif
    //向网卡列表中添加一个网口
	rt_kprintf("注册网卡\n"); 
	Netif_Init_Flag=netif_add(&lwip_netif,&ipaddr,&netmask,&gw,NULL,&ethernetif_init,&tcpip_input);
	//网口添加成功后,设置netif为默认值,并且打开netif网口
	if(Netif_Init_Flag != NULL) 	
	{
		//设置netif为默认网口
		rt_kprintf("设置netif为默认网口\n"); 
		netif_set_default(&lwip_netif); 
		//打开netif网口
		rt_kprintf("打开netif网口\n"); 
		netif_set_up(&lwip_netif);		
	}
	//进入临界区
	rt_enter_critical();  		
	//创建以太网数据接收任务
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
		rt_kprintf("以太网数据接收任务已启动\n");
	}
	rt_exit_critical();			//退出临界区
#if	LWIP_DHCP
	//创建DHCP任务
	lwip_comm_dhcp_creat();		
#endif		
	return 0;//操作OK.
}   
//如果使能了DHCP
#if LWIP_DHCP
//创建DHCP任务
void lwip_comm_dhcp_creat(void)
{
	rt_err_t result = 0; 
	rt_enter_critical();  		//进入临界区
	//创建DHCP任务 
	result = rt_thread_init( &LWIP_DHCP_Thread,
							"LWIP_DHCP_Thread",
							 lwip_dhcp_task,
							 RT_NULL,
							 &LWIP_DHCP_TASK_STK[0],
							 sizeof(LWIP_DHCP_TASK_STK),
							 LWIP_DHCP_TASK_PRIO,
							 200  	
							);
	//启动DHCP任务
	if(result==RT_EOK)
	{
	  rt_thread_startup(&LWIP_DHCP_Thread);
	  rt_kprintf("DHCP任务已启动\n");
	}
	//退出临界区
	rt_exit_critical();				
}

//删除DHCP任务
void lwip_comm_dhcp_delete(void)
{
	dhcp_stop(&lwip_netif); 		        //关闭DHCP
	rt_thread_delete(&LWIP_DHCP_Thread);	//删除DHCP任务
}
//DHCP处理任务
void lwip_dhcp_task(void *pdata)
{
	uint32_t ip=0,netmask=0,gw=0;
	dhcp_start(&lwip_netif);//开启DHCP 
	lwipdev.dhcpstatus=0;	//正在DHCP
	printf("正在查找DHCP服务器,请稍等...........\r\n");   
	while(1)
	{ 
		printf("正在获取地址...\r\n");
		ip=lwip_netif.ip_addr.addr;		//读取新IP地址
		netmask=lwip_netif.netmask.addr;//读取子网掩码
		gw=lwip_netif.gw.addr;			//读取默认网关 
		if(ip!=0)   					//当正确读取到IP地址的时候
		{
			lwipdev.dhcpstatus=2;	//DHCP成功
 			printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			//解析出通过DHCP获取到的IP地址
			lwipdev.ip[3]=(uint8_t)(ip>>24); 
			lwipdev.ip[2]=(uint8_t)(ip>>16);
			lwipdev.ip[1]=(uint8_t)(ip>>8);
			lwipdev.ip[0]=(uint8_t)(ip);
			printf("通过DHCP获取到IP地址..............%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			//解析通过DHCP获取到的子网掩码地址
			lwipdev.netmask[3]=(uint8_t)(netmask>>24);
			lwipdev.netmask[2]=(uint8_t)(netmask>>16);
			lwipdev.netmask[1]=(uint8_t)(netmask>>8);
			lwipdev.netmask[0]=(uint8_t)(netmask);
			printf("通过DHCP获取到子网掩码............%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			//解析出通过DHCP获取到的默认网关
			lwipdev.gateway[3]=(uint8_t)(gw>>24);
			lwipdev.gateway[2]=(uint8_t)(gw>>16);
			lwipdev.gateway[1]=(uint8_t)(gw>>8);
			lwipdev.gateway[0]=(uint8_t)(gw);
			printf("通过DHCP获取到的默认网关..........%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}else if(lwip_netif.dhcp->tries>LWIP_MAX_DHCP_TRIES) //通过DHCP服务获取IP地址失败,且超过最大尝试次数
		{  
			lwipdev.dhcpstatus=0XFF;//DHCP失败.
			//使用静态IP地址
			IP4_ADDR(&(lwip_netif.ip_addr),lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			IP4_ADDR(&(lwip_netif.netmask),lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			IP4_ADDR(&(lwip_netif.gw),lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			printf("DHCP服务超时,使用静态IP地址!\r\n");
			printf("网卡en的MAC地址为:................%d.%d.%d.%d.%d.%d\r\n",lwipdev.mac[0],lwipdev.mac[1],lwipdev.mac[2],lwipdev.mac[3],lwipdev.mac[4],lwipdev.mac[5]);
			printf("静态IP地址........................%d.%d.%d.%d\r\n",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);
			printf("子网掩码..........................%d.%d.%d.%d\r\n",lwipdev.netmask[0],lwipdev.netmask[1],lwipdev.netmask[2],lwipdev.netmask[3]);
			printf("默认网关..........................%d.%d.%d.%d\r\n",lwipdev.gateway[0],lwipdev.gateway[1],lwipdev.gateway[2],lwipdev.gateway[3]);
			break;
		}  
		Delay_ms(250); //延时250ms
	}
	dhcp_stop(&lwip_netif); //关闭DHCP
	rt_thread_mdelay(10000);//删除DHCP任务
}
#endif 



#endif

















































