#ifndef __LWIP_COMM_H
#define __LWIP_COMM_H 
#include "dm9000.h" 
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK 战舰开发板 V3
//lwip通用驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//创建日期:2015/3/15
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//*******************************************************************************
//修改信息
//无
////////////////////////////////////////////////////////////////////////////////// 	   

#define LWIP_MAX_DHCP_TRIES		4   //DHCP服务器最大重试次数

//lwip控制结构体
typedef struct  
{
	uint8_t mac[6];      //MAC地址
	uint8_t remoteip[4];	//远端主机IP地址 
	uint8_t ip[4];       //本机IP地址
	uint8_t netmask[4]; 	//子网掩码
	uint8_t gateway[4]; 	//默认网关的IP地址
	
	volatile uint8_t dhcpstatus;	//dhcp状态 
					//0,未获取DHCP地址;
					//1,进入DHCP获取状态
					//2,成功获取DHCP地址
					//0XFF,获取失败.
}__lwip_dev;
extern __lwip_dev lwipdev;	//lwip控制结构体

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













