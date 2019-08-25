#include "mutex.h"
#include <rtthread.h>
#include <rthw.h>
#include "usart.h"
//线程堆栈，优先级，时间片宏定义
#define THREAD_STACK_SIZE 512
#define THREAD_PRIORITY 10
#define THREAD_TIMESLICE 50

//互斥量控制块声明
rt_mutex_t mutex = RT_NULL;
 
//线程控制块声明
rt_thread_t thread_printf = RT_NULL;
rt_thread_t thread1 = RT_NULL;
rt_thread_t thread2 = RT_NULL;

static rt_uint8_t cnt = 0;
//线程1定义
void thread1_printf(void *parameter)
{
	
	while(1)
	{
		rt_thread_delay(400);//1.挂起高优先级 
		if(thread1->current_priority != thread2->current_priority)
		{
			rt_kprintf("test failed!\n");
			rt_kprintf("thread1 priority is: %d\n",thread1->current_priority);
			rt_kprintf("thread2 priority is: %d\n",thread2->current_priority);
			//return;
		}
		else
		{
			rt_kprintf("\ntest OK!\n");
			rt_kprintf("thread1 priority is: %d\n",thread1->current_priority);
			rt_kprintf("thread2 priority is: %d\n",thread2->current_priority);
			
		}
	}
}

//线程2定义
void thread1_demo(void *parameter)
{
	
	rt_err_t result;
	while(1)
	{
		printf("thread1 priority is %d\n",thread1->current_priority);
		rt_thread_delay(100);
		//在thread2线程挂起的时间内，thread1请求互斥量，由于该互斥量被占有，这时持有互斥量的低优先级线程thread2
		//优先级被提升到与thread1相同的等级，线程被唤醒，释放互斥量，然后thread2获得互斥量
		result = rt_mutex_take(mutex,RT_WAITING_FOREVER);
		if(result == RT_EOK)
		{
			printf("thread1_cnt:%d\n",cnt++);
			rt_mutex_release(mutex);
		}
	}
}

//线程3定义
void thread2_demo(void *parameter)
{
	
	rt_err_t result;
	while(1)
	{
		printf("thread2 priority is %d\n",thread2->current_priority);
		//2.让thread2先获得互斥量，然后挂起线程一段时间并持有该互斥量
		result = rt_mutex_take(mutex,RT_WAITING_FOREVER);
		if(result == RT_EOK)
		{
		    printf("thread2_cnt:%d\n",cnt++);
		}
		rt_thread_delay(10);
		rt_mutex_release(mutex);
	}
}

//创建信号量，启动线程
int mutex_sample(void)
{
	mutex = rt_mutex_create("mutex",RT_IPC_FLAG_FIFO);
	//创建线程1
	thread_printf = rt_thread_create("thread_printf",
									  thread1_printf,
									  RT_NULL,
									  THREAD_STACK_SIZE,
									  THREAD_PRIORITY-1,
									  THREAD_TIMESLICE	
									);
	rt_thread_startup(thread_printf);
	//创建线程2
	thread1 = rt_thread_create(		 "thread1_demo",
									  thread1_demo,
									  RT_NULL,
									  THREAD_STACK_SIZE,
									  THREAD_PRIORITY,
									  THREAD_TIMESLICE	
									);
	rt_thread_startup(thread1);
	
	//创建线程3
	thread2 = rt_thread_create(		 "thread2_demo",
									  thread2_demo,
									  RT_NULL,
									  THREAD_STACK_SIZE+7,
									  THREAD_PRIORITY,
									  THREAD_TIMESLICE	
									);
	rt_thread_startup(thread2);
	return 0;
}
