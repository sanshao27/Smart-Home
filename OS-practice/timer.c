#include <rtthread.h>
#include <rthw.h>
#include "timer.h"

//������̬��ʱ�����ƿ����
 rt_timer_t timer1;
 rt_timer_t timer2;

//��ʱ����1
void timeout1(void *parameter)
{
	static rt_uint32_t cnt = 0;
	rt_kprintf("timer1 out cnt: %u\r RT_TIMER_FLAG_PERIODIC--50\n",cnt++);
	if(cnt>1000)
		{
			rt_timer_stop(timer1);
		}
}

//��ʱ����2
void timeout2(void *parameter)
{
	rt_kprintf("timer2 out\r RT_TIMER_FLAG_ONE_SHOT--50\n" );
}

int timer_sample(void)
{
	//������ʱ��1�߳�
	timer1 = rt_timer_create("timer1",timeout1,RT_NULL,50,RT_TIMER_FLAG_PERIODIC);
	//������ʱ��1�߳�
	if(timer1 != RT_NULL)
		rt_timer_start(timer1);
	
	//������ʱ��2�߳�
	timer2 = rt_timer_create("timer2",timeout2,RT_NULL,50,RT_TIMER_FLAG_HARD_TIMER);
	//������ʱ��2�߳�
	if(timer2 != RT_NULL)
		rt_timer_start(timer2);	
	
	
	return 0;
}
