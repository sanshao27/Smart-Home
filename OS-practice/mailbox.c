#include <rtthread.h>
#include <rthw.h>
#include "mailbox.h"
#include "bsp_ds18b20.h"
#include "usart.h"
#include "bsp_led.h"



//线程堆栈，优先级，时间片宏定义
#define THREAD_STACK_SIZE 512
#define THREAD_PRIORITY 10
#define THREAD_TIMESLICE 50

//线程控制块声明
static struct rt_thread thread3_led;
static struct rt_thread thread2_tem ;
static struct rt_thread thread1_key ;

//邮箱控制块变量声明
static  rt_mailbox_t mail;
static  rt_mailbox_t mail1;

//邮箱池
//static char mail_pool[4] ={0};

//按键消息
static char key_up_press[] = {"WAKE_UP"};
static char key_left_press[] = {"KEY_LEFT"};
static char key_right_press[] = {"KEY_RIGHT"};
static char key_select_press[] = {"KEY_SELECT"};

ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[512];
//线程1定义
static void thread1_entry(void *parameter)
{
	rt_uint8_t key_value;
	while(1)
	{	
		key_value = Key_Scan(CONST_PRESS_ENALBE);
		switch(key_value)
		{
			case UP:
			{
				rt_mb_send(mail1,(rt_uint32_t)&key_up_press);
				
				break;
			}
			case RIGHT:
			{
				rt_mb_send(mail,(rt_uint32_t)&key_right_press);
				break;
			}
			case LEFT:
			{
				rt_mb_send(mail,(rt_uint32_t)&key_left_press);
				break;
			}
			case SELECT:
			{
				rt_mb_send(mail,(rt_uint32_t)&key_select_press);
				break;
			}
			default:
			{
				break;
			}
		}
	 rt_thread_mdelay(1);		
	}
}

ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[512];
//线程2定义
static void thread2_entry(void *parameter)
{
	static char* key_id= RT_NULL;
	while(1)
	{
		if(rt_mb_recv(mail1,(rt_uint32_t*)&key_id,RT_WAITING_FOREVER)==RT_EOK)//(rt_uint32_t*)的作用是将char类型的内存地址转换成rt_uint32_t类型
		{
				
			if(strcmp(key_id,"WAKE_UP") ==0)
			{	
				rt_kprintf("\nGot a mail:%s\n",key_id);	
				printf("temperature:%.2f\n",DS18B20_Get_Temp()); 
			}
		}
	}
}

ALIGN(RT_ALIGN_SIZE)
static char thread3_stack[512];
//线程3定义
static void thread3_entry(void *parameter)
{
	static char* key_ID= RT_NULL;
	while(1)
	{
		if(rt_mb_recv(mail,(rt_uint32_t*)&key_ID,RT_WAITING_FOREVER)==RT_EOK)
		{
			if(strcmp(key_ID,key_left_press) ==0)
			{
				     printf("\nGot a mail:%s\n",key_ID);	
					 GREEN_LED = !GREEN_LED;
			}
			
			if(strcmp(key_ID,key_right_press) ==0)
			{
				     printf("\nGot a mail:%s\n",key_ID);
				     RED_LED = !RED_LED;
			}
			
			if(strcmp(key_ID,key_select_press) ==0)
			{
				     printf("\nGot a mail:%s\n",key_ID);
				     RED_LED = !RED_LED;
				     GREEN_LED = !GREEN_LED;
			}
		}
	    rt_thread_delay(200);
	}
}

//创建信号量，启动线程
int mailbox_sample(void)
{
	//创建第一个邮箱
	mail = rt_mb_create("mail",
						12,
						RT_IPC_FLAG_FIFO);
	if(mail != RT_NULL)
	{
		printf("mailbox size:%d\n",mail->size);
	}
	//创建第二个邮箱
	mail1 = rt_mb_create("mail1",
						1,
						RT_IPC_FLAG_FIFO);
	if(mail1 != RT_NULL)
	{
		printf("mail1box size:%d\n",mail1->size);
	}
	//创建线程1
   rt_thread_init(  &thread1_key,
					"thread1_key",
					 thread1_entry,
					 RT_NULL,
					 &thread1_stack[0],
					 sizeof(thread1_stack),
					 THREAD_PRIORITY-1,
					 THREAD_TIMESLICE  	
					);
	//启动线程1
	rt_thread_startup(&thread1_key);
	
	//创建线程2
    rt_thread_init(&thread2_tem,
					"thread_tem",
					 thread2_entry,
					 RT_NULL,
					 &thread2_stack[0], 
					 sizeof(thread2_stack),
					 THREAD_PRIORITY,
					 THREAD_TIMESLICE  	
					);
	//启动线程2
	rt_thread_startup(&thread2_tem);
	//创建线程3
	rt_thread_init(  &thread3_led,
					 "thread_led",
					 thread3_entry,
					 RT_NULL,
					 &thread3_stack[0], 
					 sizeof(thread3_stack),
					 THREAD_PRIORITY+1,
					 THREAD_TIMESLICE  	
					);
	//启动线程3
	rt_thread_startup(&thread3_led);
	
	return 0;
}

//若是有多个线程使用邮箱，应该一个线程配备一个独立的邮箱，否则多个线程公用一个邮箱将出现异常
