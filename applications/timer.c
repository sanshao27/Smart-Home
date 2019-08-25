#include <rtthread.h>
#include <rthw.h>
#include "timer.h"

//创建动态定时器控制块变量
 rt_timer_t timer1;
 rt_timer_t timer2;

//超时函数1
void timeout1(void *parameter)
{
	static rt_uint32_t cnt = 0;
	rt_kprintf("timer1 out cnt: %u\r RT_TIMER_FLAG_PERIODIC--50\n",cnt++);
	if(cnt>1000)
		{
			rt_timer_stop(timer1);
		}
}

//超时函数2
void timeout2(void *parameter)
{
	rt_kprintf("timer2 out\r RT_TIMER_FLAG_ONE_SHOT--50\n" );
}

int timer_sample(void)
{
	//创建定时器1线程
	timer1 = rt_timer_create("timer1",timeout1,RT_NULL,50,RT_TIMER_FLAG_PERIODIC);
	//启动定时器1线程
	if(timer1 != RT_NULL)
		rt_timer_start(timer1);
	
	//创建定时器2线程
	timer2 = rt_timer_create("timer2",timeout2,RT_NULL,50,RT_TIMER_FLAG_HARD_TIMER);
	//启动定时器2线程
	if(timer2 != RT_NULL)
		rt_timer_start(timer2);	
	
	
	return 0;
}
