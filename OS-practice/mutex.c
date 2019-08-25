#include "mutex.h"
#include <rtthread.h>
#include <rthw.h>
#include "usart.h"
//�̶߳�ջ�����ȼ���ʱ��Ƭ�궨��
#define THREAD_STACK_SIZE 512
#define THREAD_PRIORITY 10
#define THREAD_TIMESLICE 50

//���������ƿ�����
rt_mutex_t mutex = RT_NULL;
 
//�߳̿��ƿ�����
rt_thread_t thread_printf = RT_NULL;
rt_thread_t thread1 = RT_NULL;
rt_thread_t thread2 = RT_NULL;

static rt_uint8_t cnt = 0;
//�߳�1����
void thread1_printf(void *parameter)
{
	
	while(1)
	{
		rt_thread_delay(400);//1.��������ȼ� 
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

//�߳�2����
void thread1_demo(void *parameter)
{
	
	rt_err_t result;
	while(1)
	{
		printf("thread1 priority is %d\n",thread1->current_priority);
		rt_thread_delay(100);
		//��thread2�̹߳����ʱ���ڣ�thread1���󻥳��������ڸû�������ռ�У���ʱ���л������ĵ����ȼ��߳�thread2
		//���ȼ�����������thread1��ͬ�ĵȼ����̱߳����ѣ��ͷŻ�������Ȼ��thread2��û�����
		result = rt_mutex_take(mutex,RT_WAITING_FOREVER);
		if(result == RT_EOK)
		{
			printf("thread1_cnt:%d\n",cnt++);
			rt_mutex_release(mutex);
		}
	}
}

//�߳�3����
void thread2_demo(void *parameter)
{
	
	rt_err_t result;
	while(1)
	{
		printf("thread2 priority is %d\n",thread2->current_priority);
		//2.��thread2�Ȼ�û�������Ȼ������߳�һ��ʱ�䲢���иû�����
		result = rt_mutex_take(mutex,RT_WAITING_FOREVER);
		if(result == RT_EOK)
		{
		    printf("thread2_cnt:%d\n",cnt++);
		}
		rt_thread_delay(10);
		rt_mutex_release(mutex);
	}
}

//�����ź����������߳�
int mutex_sample(void)
{
	mutex = rt_mutex_create("mutex",RT_IPC_FLAG_FIFO);
	//�����߳�1
	thread_printf = rt_thread_create("thread_printf",
									  thread1_printf,
									  RT_NULL,
									  THREAD_STACK_SIZE,
									  THREAD_PRIORITY-1,
									  THREAD_TIMESLICE	
									);
	rt_thread_startup(thread_printf);
	//�����߳�2
	thread1 = rt_thread_create(		 "thread1_demo",
									  thread1_demo,
									  RT_NULL,
									  THREAD_STACK_SIZE,
									  THREAD_PRIORITY,
									  THREAD_TIMESLICE	
									);
	rt_thread_startup(thread1);
	
	//�����߳�3
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
