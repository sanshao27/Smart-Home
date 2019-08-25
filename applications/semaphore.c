#include <rtthread.h>
#include <rthw.h>
#include "semaphore.h"
#include "bsp_ds18b20.h"
#include "usart.h"
#include "bsp_led.h"
//线程堆栈，优先级，时间片宏定义
#define THREAD_STACK_SIZE 512
#define THREAD_PRIORITY 10
#define THREAD_TIMESLICE 20

//事件宏定义
#define  EVENT_FLAG1 0X01
#define  EVENT_FLAG2 0X78
 

//信号量控制块声明
rt_sem_t sem = RT_NULL;
rt_sem_t lock = RT_NULL;
rt_event_t event = RT_NULL;
//线程控制块声明
rt_thread_t thread_delay = RT_NULL;
rt_thread_t thread_tem = RT_NULL;
rt_thread_t thread_led = RT_NULL;
//线程1定义
static void thread_delay_entry(void *parameter)
{
	static uint16_t cnt = 0;
	while(1)
	{
		cnt++;
		if(cnt>10000)
		{
			cnt = 0;
		}
		if(0 == cnt%100)
		{
			//printf("\ncnt:%d\n",cnt);
			
			//rt_sem_release(sem);
			//rt_sem_release(lock);
			//printf("\nsem value:%d\n",sem->value);
			rt_event_send(event,EVENT_FLAG1);
			rt_kprintf("\nsent event:0x%.2lx\n",EVENT_FLAG1);
			rt_thread_delay(350);
		}
	}
}


//线程2定义
static void thread_tem_entry(void *parameter)
{
	static rt_uint32_t result;
	while(1)
	{
		if(rt_event_recv(event,
						EVENT_FLAG1,
						RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,
						RT_WAITING_FOREVER,
						&result) == RT_EOK)
		{
			//rt_enter_critical();
			printf("\nGot an event:0x%.2lx\n",result);			
			printf("temperature:%.2f\n",DS18B20_Get_Temp());	
			rt_event_send(event,EVENT_FLAG2);
			rt_kprintf("sent event:0x%.2lx\n",EVENT_FLAG2);
			//rt_exit_critical(); 
		}
	}
}

//线程3定义
static void thread_led_entry(void *parameter)
{
	static rt_uint32_t result;
	while(1)
	{
		if(rt_event_recv(event,
						(EVENT_FLAG1|EVENT_FLAG2),
						RT_EVENT_FLAG_AND|RT_EVENT_FLAG_CLEAR,
						RT_WAITING_FOREVER,
						&result) == RT_EOK)
		{
			 //rt_enter_critical();
			 printf("\nGot an event:0x%.2lx\n",result);	
			 printf("Led demo is implementing\n");	
			 GREEN_LED = 0; 
			 RED_LED = 0;  
		     rt_thread_delay(1);
		     GREEN_LED = 1; 
			 RED_LED = 1;  
			 rt_thread_delay(1); 
			 printf("Led demo exit\n");	
			 //rt_exit_critical(); 
		}
	}
}

//创建信号量，启动线程
int semaphore_sample(void)
{
	//sem = rt_sem_create("sem",0,RT_IPC_FLAG_PRIO);
	//lock = rt_sem_create("lock",1,RT_IPC_FLAG_FIFO);
	event = rt_event_create("event",RT_IPC_FLAG_FIFO);
	if(event == RT_NULL)
	{
		rt_kprintf("Failed to create event!\n");
		return -1;
	}
	else
	{
		rt_kprintf("event created!\nevent value is:%d\n",event->set);
	}
	//创建线程1
	thread_delay = rt_thread_create("thread_delay",
									 thread_delay_entry,
								     RT_NULL,
									 THREAD_STACK_SIZE,
									 THREAD_PRIORITY-2,
									 THREAD_TIMESLICE  	
									);
	//启动线程1
	if(thread_delay != RT_NULL)
	{
		rt_thread_startup(thread_delay);
	}
	
	//创建线程2
	thread_tem = rt_thread_create(  "thread_tem",
									 thread_tem_entry,
								     RT_NULL,
									 THREAD_STACK_SIZE,
									 THREAD_PRIORITY-1,
									 THREAD_TIMESLICE  	
									);
	//启动线程2
	if(thread_tem != RT_NULL)
	{
		rt_thread_startup(thread_tem);
	}
	
	//创建线程3
	thread_led = rt_thread_create(  "thread_led",
									 thread_led_entry,
								     RT_NULL,
									 THREAD_STACK_SIZE,
									 THREAD_PRIORITY,
									 THREAD_TIMESLICE  	
									);
	//启动线程3
	if(thread_led != RT_NULL)
	{
		rt_thread_startup(thread_led);
	}
	
	return 0;
}
