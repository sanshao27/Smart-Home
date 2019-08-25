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

//消息队列控制块变量声明
static  rt_mq_t msg1;
static  rt_mq_t msg2;
static  rt_mailbox_t msg2_ack;
//消息队列池
//static uint8_t msg_pool[2048] ={0};

//定时器控制块变量声明
static rt_timer_t timer1_led_flash = RT_NULL;

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
	static char* msg2_ack_buffer= RT_NULL;
	rt_err_t err;
	rt_uint8_t key_value;
	while(1)
	{	
		key_value = Key_Scan(CONST_PRESS_DISALBE);
		switch(key_value)
		{
			case UP:
			{
				rt_mq_send(msg2,key_up_press,sizeof(key_up_press));//发送消息队列
				rt_mb_recv(msg2_ack,(rt_uint32_t*)&msg2_ack_buffer,RT_WAITING_FOREVER);//接收反馈消息
				rt_kprintf("msg2_ack:%s\n",msg2_ack_buffer);//输出反馈结果
				break;
			}
			case RIGHT:
			{
				err = rt_mq_send(msg1,key_right_press,sizeof(key_right_press));
				switch(err)
				{
					case RT_EOK:   rt_kprintf("msg sent successfully!\n");break;
					case -RT_EFULL: rt_kprintf("queue is full!\n");break;
					case -RT_ERROR: rt_kprintf("the length of the  msg to be sent is greater than the MAX msg length!\n");break;
					default : break;
				}
				break;
			}
			case LEFT:
			{
				rt_mq_send(msg1,key_left_press,sizeof(key_left_press));
				break;
			}
			case SELECT:
			{
				rt_mq_send(msg1,key_select_press,sizeof(key_select_press));
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
	rt_err_t result ;
	static char key_id[16];
	while(1)
	{
		result = rt_mq_recv(msg2,&key_id,sizeof(key_id),RT_WAITING_FOREVER);
		switch(result)
		{	
			case RT_EOK:
					{
						if(strcmp(key_id,"WAKE_UP") ==0)
						{	
							rt_kprintf("\nGot a msg:%s\n",key_id);	
							rt_mb_send(msg2_ack,(rt_uint32_t)"RT_EOK");//发送反馈结果
							printf("temperature:%.2f\n",DS18B20_Get_Temp()); 
							
						}
						break;
					}
			case -RT_ETIMEOUT: rt_kprintf("\nmsg receive timeout!\n");	break;
			case -RT_ERROR:    rt_kprintf("\nmsg receive error!\n");break;
			default:break;
		}
		rt_thread_delay(1);
	}
}
ALIGN(RT_ALIGN_SIZE)
static char thread3_stack[512];
//线程3定义
static void thread3_entry(void *parameter)
{
	static rt_uint8_t timer1_flag = 0;
	static char key_ID[16];
	while(1)
	{
		if(rt_mq_recv(msg1,&key_ID,sizeof(key_ID),RT_WAITING_FOREVER)==RT_EOK)
		{
			if(strcmp(key_ID,key_left_press) ==0)
			{
				     printf("\nGot a msg:%s\n",key_ID);	
					 GREEN_LED = !GREEN_LED;
			}
			
			if(strcmp(key_ID,key_right_press) ==0)
			{
				     printf("\nGot a msg:%s\n",key_ID);
				     RED_LED = !RED_LED;
			}
			
			if(strcmp(key_ID,key_select_press) ==0)
			{
				printf("\nGot a msg:%s\n",key_ID);
				timer1_flag = !timer1_flag;
			    if(timer1_flag)
				{
					rt_timer_start(timer1_led_flash);
					printf("\ntimer1_led_flash starts!\n" );
				}
				else
				{
					rt_timer_stop(timer1_led_flash);
					printf("\ntimer1_led_flash stopped!\n" );
				}
			}
		}
	    rt_thread_delay(1);
	}
}

//启动线程
int messagequeue_sample(void)
{
	//创建软件定时器
	timer1_led_flash = rt_timer_create(
										"timer1_led_flash",
										 LED_Flash,   //超时回调函数
										 (void*)500,  //超时函数入口参数
										 10,          //定时长度10个 OS Tick
										 RT_TIMER_FLAG_PERIODIC|RT_TIMER_FLAG_SOFT_TIMER);//软定时，周期性执行

	
	//创建第一个邮箱
	msg2_ack = rt_mb_create("msg2_ack",
							12,
							RT_IPC_FLAG_FIFO);
	if(msg2_ack != RT_NULL)
	{
		printf("msg2_ack size:%d\n",msg2_ack->size);
	}
	
	//创建第一个消息队列
	msg1 = rt_mq_create("msg1",
						 12,
						 3,
						 RT_IPC_FLAG_FIFO);
	if(msg1 != RT_NULL)
	{
		printf("msg1 size:%d\n",msg1->msg_size);
	}
	//创建第二个消息队列
	msg2 = rt_mq_create("msg2",
						 16,
					     10,
						 RT_IPC_FLAG_FIFO);
	if(msg2 != RT_NULL)
	{
		printf("msg2 size:%d\n",msg2->msg_size);
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

//若是有多个线程使用消息队列，应该一个线程配备一个独立的消息队列，否则多个线程公用一个消息队列将出现异常
