#include "netif/ethernetif.h" 
#include "dm9000.h"  
#include "lwip_comm.h" 
#include "netif/etharp.h"  
#include "string.h"  
#include <rthw.h>
#include <rtthread.h>
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK ս�������� V3
//ethernetif ����	   
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
#if 1

extern rt_sem_t dm9000input;		//DM9000���������ź���

//��ethernetif_init()�������ڳ�ʼ��Ӳ��
//netif:�����ṹ��ָ�� 
//����ֵ:ERR_OK,����
//       ����,ʧ��
static err_t low_level_init(struct netif *netif)
{
	//INT8U err;
	netif->hwaddr_len = ETHARP_HWADDR_LEN; //����MAC��ַ����,Ϊ6���ֽ�
	//��ʼ��MAC��ַ,����ʲô��ַ���û��Լ�����,���ǲ����������������豸MAC��ַ�ظ�
	netif->hwaddr[0]=lwipdev.mac[0]; 
	netif->hwaddr[1]=lwipdev.mac[1]; 
	netif->hwaddr[2]=lwipdev.mac[2];
	netif->hwaddr[3]=lwipdev.mac[3];
	netif->hwaddr[4]=lwipdev.mac[4];
	netif->hwaddr[5]=lwipdev.mac[5];
	netif->mtu=1500; //��������䵥Ԫ,����������㲥��ARP����
	netif->flags = NETIF_FLAG_BROADCAST|NETIF_FLAG_ETHARP|NETIF_FLAG_LINK_UP; 
	return ERR_OK;
} 
//���ڷ������ݰ�����ײ㺯��(lwipͨ��netif->linkoutputָ��ú���)
//netif:�����ṹ��ָ��
//p:pbuf���ݽṹ��ָ��
//����ֵ:ERR_OK,��������
//       ERR_MEM,����ʧ��
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	DM9000_SendPacket(p);//��������
	return ERR_OK;
}
//���ڽ������ݰ�����ײ㺯��
//neitif:�����ṹ��ָ��
//����ֵ:pbuf���ݽṹ��ָ��
static struct pbuf * low_level_input(struct netif *netif)
{  
	
	return DM9000_Receive_Packet();
}

err_t ethernetif_input(struct netif *netif)
{
	err_t err;
	struct pbuf *p;
	while(1)
	{
		//�����ź���
		//rt_sem_take(dm9000input,RT_WAITING_FOREVER);
		//rt_kprintf("receving data!\n");
		while(1)
		{
			p=low_level_input(netif);   //����low_level_input������������
			if(p!=NULL)
			{
				err=netif->input(p, netif); //����netif�ṹ���е�input�ֶ�(һ������)���������ݰ�
				if(err!=ERR_OK)
				{
					LWIP_DEBUGF(NETIF_DEBUG,("ethernetif_input: IP input error\n"));
					pbuf_free(p);
					p = NULL;
				} 
			}else break; 
		}
		rt_thread_mdelay(10);	
	 }
} 

//ʹ��low_level_init()��������ʼ������
//netif:�����ṹ��ָ��
//����ֵ:ERR_OK,����
//       ����,ʧ��
err_t ethernetif_init(struct netif *netif)
{
	LWIP_ASSERT("netif!=NULL",(netif!=NULL));
#if LWIP_NETIF_HOSTNAME			//LWIP_NETIF_HOSTNAME 
	netif->hostname="lwip";  	//��ʼ������
#endif 
	netif->name[0]=IFNAME0; 	//��ʼ������netif��name�ֶ�
	netif->name[1]=IFNAME1; 	//���ļ��ⶨ�����ﲻ�ù��ľ���ֵ
	netif->output=etharp_output;//IP�㷢�����ݰ�����
	netif->linkoutput=low_level_output;//ARPģ�鷢�����ݰ�����
	low_level_init(netif); 		//�ײ�Ӳ����ʼ������
	return ERR_OK;
}


#if 0
#ifdef RT_USING_FINSH
#include <finsh.h>
void set_if(char* netif_name, char* ip_addr, char* gw_addr, char* nm_addr)
{
    struct ip_addr *ip;
    struct ip_addr addr;
    struct netif * netif = netif_list;

    if(strlen(netif_name) > sizeof(netif->name))
    {
        rt_kprintf("network interface name too long!\r\n");
        return;
    }

    while(netif != RT_NULL)
    {
        if(strncmp(netif_name, netif->name, sizeof(netif->name)) == 0)
            break;

        netif = netif->next;
        if( netif == RT_NULL )
        {
            rt_kprintf("network interface: %s not found!\r\n", netif_name);
            return;
        }
    }

    ip = (struct ip_addr *)&addr;

    /* set ip address */
    if ((ip_addr != RT_NULL) && ipaddr_aton(ip_addr, &addr))
    {
        netif_set_ipaddr(netif, ip);
    }

    /* set gateway address */
    if ((gw_addr != RT_NULL) && ipaddr_aton(gw_addr, &addr))
    {
        netif_set_gw(netif, ip);
    }

    /* set netmask address */
    if ((nm_addr != RT_NULL) && ipaddr_aton(nm_addr, &addr))
    {
        netif_set_netmask(netif, ip);
    }
}
//FINSH_FUNCTION_EXPORT(set_if, set network interface address);

#if LWIP_DNS
#include <lwip/dns.h>
void set_dns(char* dns_server)
{
    struct ip_addr addr;

    if ((dns_server != RT_NULL) && ipaddr_aton(dns_server, &addr))
    {
        dns_setserver(0, &addr);
    }
}
//FINSH_FUNCTION_EXPORT(set_dns, set DNS server address);
#endif

void list_if(void)
{
    rt_ubase_t index;
    struct netif * netif;

    rt_enter_critical();

    netif = netif_list;

    while( netif != RT_NULL )
    {
        rt_kprintf("network interface: %c%c%s\n",
                   netif->name[0],
                   netif->name[1],
                   (netif == netif_default)?" (Default)":"");
        rt_kprintf("MTU: %d\n", netif->mtu);
        rt_kprintf("MAC: ");
        for (index = 0; index < netif->hwaddr_len; index ++)
            rt_kprintf("%02x ", netif->hwaddr[index]);
        rt_kprintf("\nFLAGS:");
        if (netif->flags & NETIF_FLAG_UP) rt_kprintf(" UP");
        else rt_kprintf(" DOWN");
        if (netif->flags & NETIF_FLAG_LINK_UP) rt_kprintf(" LINK_UP");
        else rt_kprintf(" LINK_DOWN");
        if (netif->flags & NETIF_FLAG_DHCP) rt_kprintf(" DHCP");
        if (netif->flags & NETIF_FLAG_POINTTOPOINT) rt_kprintf(" PPP");
        if (netif->flags & NETIF_FLAG_ETHARP) rt_kprintf(" ETHARP");
        if (netif->flags & NETIF_FLAG_IGMP) rt_kprintf(" IGMP");
        rt_kprintf("\n");
        rt_kprintf("ip address: %s\n", ipaddr_ntoa(&(netif->ip_addr)));
        rt_kprintf("gw address: %s\n", ipaddr_ntoa(&(netif->gw)));
        rt_kprintf("net mask  : %s\n", ipaddr_ntoa(&(netif->netmask)));
        rt_kprintf("\r\n");

        netif = netif->next;
    }

#if LWIP_DNS
    {
        struct ip_addr ip_addr;

        for(index=0; index<DNS_MAX_SERVERS; index++)
        {
            ip_addr = dns_getserver(index);
            rt_kprintf("dns server #%d: %s\n", index, ipaddr_ntoa(&(ip_addr)));
        }
    }
#endif /**< #if LWIP_DNS */

    rt_exit_critical();
}
//FINSH_FUNCTION_EXPORT(list_if, list network interface information);

#if LWIP_TCP
#include <lwip/tcp.h>
#include <lwip/tcp_impl.h>

void list_tcps(void)
{
    rt_uint32_t num = 0;
    struct tcp_pcb *pcb;
    char local_ip_str[16];
    char remote_ip_str[16];

    extern struct tcp_pcb *tcp_active_pcbs;
    extern union tcp_listen_pcbs_t tcp_listen_pcbs;
    extern struct tcp_pcb *tcp_tw_pcbs;
    extern const char *tcp_state_str[];

    rt_enter_critical();
    rt_kprintf("Active PCB states:\n");
    for(pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next)
    {
        strcpy(local_ip_str, ipaddr_ntoa(&(pcb->local_ip)));
        strcpy(remote_ip_str, ipaddr_ntoa(&(pcb->remote_ip)));

        rt_kprintf("#%d %s:%d <==> %s:%d snd_nxt 0x%08X rcv_nxt 0x%08X ",
                   num++,
                   local_ip_str,
                   pcb->local_port,
                   remote_ip_str,
                   pcb->remote_port,
                   pcb->snd_nxt,
                   pcb->rcv_nxt);
        rt_kprintf("state: %s\n", tcp_state_str[pcb->state]);
    }

    rt_kprintf("Listen PCB states:\n");
    num = 0;
    for(pcb = (struct tcp_pcb *)tcp_listen_pcbs.pcbs; pcb != NULL; pcb = pcb->next)
    {
        rt_kprintf("#%d local port %d ", num++, pcb->local_port);
        rt_kprintf("state: %s\n", tcp_state_str[pcb->state]);
    }

    rt_kprintf("TIME-WAIT PCB states:\n");
    num = 0;
    for(pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next)
    {
        strcpy(local_ip_str, ipaddr_ntoa(&(pcb->local_ip)));
        strcpy(remote_ip_str, ipaddr_ntoa(&(pcb->remote_ip)));

        rt_kprintf("#%d %s:%d <==> %s:%d snd_nxt 0x%08X rcv_nxt 0x%08X ",
                   num++,
                   local_ip_str,
                   pcb->local_port,
                   remote_ip_str,
                   pcb->remote_port,
                   pcb->snd_nxt,
                   pcb->rcv_nxt);
        rt_kprintf("state: %s\n", tcp_state_str[pcb->state]);
    }
    rt_exit_critical();
}
//FINSH_FUNCTION_EXPORT(list_tcps, list all of tcp connections);
#endif

#if LWIP_UDP
#include "lwip/udp.h"
void list_udps(void)
{
    struct udp_pcb *pcb;
    rt_uint32_t num = 0;
    char local_ip_str[16];
    char remote_ip_str[16];

    rt_enter_critical();
    rt_kprintf("Active UDP PCB states:\n");
    for (pcb = udp_pcbs; pcb != NULL; pcb = pcb->next)
    {
        strcpy(local_ip_str, ipaddr_ntoa(&(pcb->local_ip)));
        strcpy(remote_ip_str, ipaddr_ntoa(&(pcb->remote_ip)));

        rt_kprintf("#%d %d %s:%d <==> %s:%d \n",
                   num, (int)pcb->flags,
                   local_ip_str,
                   pcb->local_port,
                   remote_ip_str,
                   pcb->remote_port);

        num++;
    }
    rt_exit_critical();
}
//FINSH_FUNCTION_EXPORT(list_udps, list all of udp connections);
#endif /* LWIP_UDP */

#endif

#endif


#endif


