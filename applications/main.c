#include "public.h"
/****************************版本说明*************************
2019/07/19-00:07
实现了:
1.触摸设置时钟，系统工作模式，灯光的开关；
2.红外遥控；
3.GSM短信设置系统工作模式，和读取系统状态信息，以及错误信息提
示和设置反馈提示。
4.增加了通过红外遥控和触摸屏实现通过NRF2401无线控制电机转动，从机基于STMS208MB芯片
PS:功能完整，无故障，实现预期功能

2019/07/21-09:50
改进内容：
1.GSM相关线程中等待ISM800C反馈方式取消了while((USART_RX2_STA&0x8000)==0);方式
改用获取系统tick方式实现超时退出。
2.外设初始化部分，使用了自动初始方式，取消了函数显式初始化方式，
即INIT_BOARD_EXPORT()，INIT_APP_EXPORT()
以上功能经过验证可以使用 

2019/07/30-23:29
改进内容：
添加了LWIP网络功能，初步实现了，DHCP获取动态IP的功能，可以PING通。

2019/08/06
新增内容：
实现了web远程设置和参数获取
*************************************************************/



/****************************用户参数*************************/
//系统模式
uint8_t sys_status = HOME;
uint8_t curtain_status = 1;
/**********************传感器变量声明*************************/
uint8_t temp,humi,len;
char temp_buffer[] = {0};
char humi_buffer[] = {0};
char MQ_buffer[]= {0};
char HZ_buffer[]= {0};
float mq = 0;
float LS1_VAL= 0;
float hz = 0;
//煤气增量参数
float delta_mq = 0;
//NRF2401线程相关变量
rt_uint32_t val = 0;
_Bool color_flag = 0;
//煤气泄漏，入侵警报标志位
//_Bool Security_mq = 0;
//_Bool Security_invaid = 0;
/************************************************************/

/***************线程堆栈，优先级，时间片宏定义***************/
#define THREAD_STACK_SIZE 512
#define THREAD_PRIORITY 5
#define THREAD_TIMESLICE 500

/**********************创建线程控制块************************/
static struct rt_thread Touch_Thread;
static struct rt_thread Light_Thread;
static struct rt_thread Security_Thread;
static struct rt_thread IRDA_Thread;
static struct rt_thread GSM_Thread;
static struct rt_thread GSM_Thread_Read;
static struct rt_thread GSM_Thread_Send;
static struct rt_thread Get_Sensor_Status_Thread;
static struct rt_thread NRF2401_Thread;
/******************************创建信号量********************/
//创建一个信号量控制块,用于煤气值超标与安防临界互斥访问--delta_mq
rt_sem_t mq_sem = RT_NULL;
//创建一个信号量控制块,用于GSM消息同步
rt_sem_t GSM_sem_read = RT_NULL;
//定时器控制块变量声明---运行定时器会造成GSM信息发送失败
//rt_timer_t timer1_sensor_Status = RT_NULL;
/******************************创建邮箱********************/

//用于GSM_SendSMS线程的消息传递
static struct rt_mailbox GSM_mq_send;
static char mq_pool[16];

//用于红外遥控到NRF的消息传递
static struct rt_mailbox IRDA_NRF2401_send;
static char IRDA_NRF2401_pool[16];

/******************************创建各个功能线程*****************/
//用于接收来自触摸屏和红外遥控发来的邮箱信息
//根据信息的内容将相应的指令通过主机的NRF2401发送到从机的NRF2401来执行相应的操作
ALIGN(RT_ALIGN_SIZE)
static char NRF2401_Thread_stack[512];
//NRF2401线程 
static void NRF2401_thread(void *parameter)
{
	//设置为发送模式
	NRF24L01_TX_Mode();
	while(1)
	{
		//等待邮箱信号，没有收到线程挂起，知道有邮箱信号过来才唤醒该线程
		rt_mb_recv(&IRDA_NRF2401_send,&val,RT_WAITING_FOREVER);
		{
			    RED_LED = !RED_LED;
			    rt_enter_critical();
			    //根据红外遥控的指令进行操作
				switch((uint8_t)val)
				{
					    //红外left,触屏OPEN
						case left:
						{
							if(NRF24L01_TxPacket(" 8                              ")==TX_OK)
							{
								printf("OPEN sent ok!\n");
								POINT_COLOR =RED;
					            LCD_ShowString(x1_curtain_open,290,200,16,16,"OPEN");
								rt_thread_mdelay(500);
								POINT_COLOR =BLACK;
					            LCD_ShowString(x1_curtain_open,290,200,16,16,"OPEN");
								//防止K/T的颜色在按过LEFT和right之后还是红色的，但是电机是停止工作的
								color_flag=0; 
								POINT_COLOR = BLACK;
								LCD_ShowString(x5_curtain_kt,290,200,16,16,"K/T");
								curtain_status = 1;
							}
							else
							printf("OPEN left sent failed!\n");
							break;
						}
						//红外right,触屏CLOSE
						case right:
						{
							if(NRF24L01_TxPacket(" 9                              ")==TX_OK)
							{
								printf("CLOSE sent ok!\n");
								POINT_COLOR =RED;
					            LCD_ShowString(x3_curtain_close,290,200,16,16,"CLOSE");
								rt_thread_mdelay(500);
								POINT_COLOR =BLACK;
					            LCD_ShowString(x3_curtain_close,290,200,16,16,"CLOSE");
								color_flag=0; 
								POINT_COLOR = BLACK;
								LCD_ShowString(x5_curtain_kt,290,200,16,16,"K/T");
								curtain_status = 0;
							}
							else
							printf("CLOSE sent failed!\n");
							break;
						}
						//红外stop,触屏K/T
						case stop:
						{
							if(NRF24L01_TxPacket(" 7                              ")==TX_OK)
							{
							     printf("K/T sent ok!\n");
								 if(color_flag==0)
								 {
								   POINT_COLOR =RED;
								   color_flag=1;
					               LCD_ShowString(x5_curtain_kt,290,200,16,16,"K/T");
								 }
								 else
								 {
									color_flag=0; 
									POINT_COLOR = BLACK;
					                LCD_ShowString(x5_curtain_kt,290,200,16,16,"K/T");
								 }
							}
							else
							printf("K/T sent failed!\n");
							break;
						}
						default:break;
				}
				RED_LED = 1;
				rt_exit_critical();
		 }	
		rt_thread_mdelay(30);	
	}	 
}
//GSM通信线程堆栈--发
ALIGN(RT_ALIGN_SIZE)
static char GSM_send_thread_stack[1024];
//GSM信息发送线程---专门用于发送消息，根据邮箱传递过来的指令，进行选择性的发送用户要求信息 
static void GSM_send_thread(void *parameter)
{
	rt_uint32_t val = 0;
	while(1)
	{
		//等待发送消息信号量，没收到线程就挂起
		if(rt_mb_recv(&GSM_mq_send,&val,RT_WAITING_FOREVER)==RT_EOK)
		{
			printf("val:%ld\n",val); 
			rt_enter_critical(); //禁止系统调度,必须禁止调度否则将导致消息发送失败
			GSM_SendSMS((uint8_t)val); 
			rt_exit_critical();  //恢复系统调度
		}
		rt_thread_mdelay(200);//设置为200，无法正常接收短信并执行GSM_SendSMS，禁止系统调度后OK
	}
}

//GSM通信线程堆栈--读
ALIGN(RT_ALIGN_SIZE)
static char GSM_read_thread_stack[1024];
//GSM读取信息线程---专门读取SMS信息，并判断是否有指定的指令，如果有则通过邮箱机制
//传递指令到GSM消息发送线程
static void GSM_read_thread(void *parameter)
{
	rt_tick_t current_tick=0;
	static char*pt = RT_NULL;
	//point用于保存给pt分配内存得到的内存堆首地址，用于最后释放用
	static char*point = RT_NULL; 
	char GSM_buffer[] = {0};
	while(1)
	{
		//等待读取信息信号量
	  if(rt_sem_take(GSM_sem_read,RT_WAITING_FOREVER)==RT_EOK)
	  {
		//打印输出来自ISM800C的原始数据
		printf("自ISM800C的原始数据：%s\n",USART_RX2_BUF);
		//申请256字节的内存空间
		pt = rt_malloc(256);
		point = pt;//将申请的到的内存堆首地址存放到point中，待最后释放时用到
				   //这一步很重要，因为下面的代码要用到strstr函数，该函数会改变
				   //pt的值，如果释放时的地址与申请时得到的地址不同，最终会导致
				   //内存堆释放失败，从而导致断言错误。
		if(pt==RT_NULL)
		{
			rt_kprintf("try to get 256 byte memory failed!\n" );
           return;
	     }
		loop:
		printf("pt:0x%p\n",pt); 
		pt = rt_strstr((char*)USART_RX2_BUF,"+CMTI:");
		printf("pt:%s\n",pt);
		//判断是否收到短信，如果是进入短信处理程序
		 if(pt!=RT_NULL)
		 {
			//获取总信息量，并存放到GSM_buffer
			sprintf(GSM_buffer,"%s",pt+12);
			printf("GSM_buffer:%s\n",GSM_buffer); 
			//信息数量超过99个就全部删除，否则无法接收短信
			if(rt_strcmp(GSM_buffer,"99")>=0)
			{
				usart2_senddata("AT+CMGD=1,4\n"); 
				//采用获取系统tick方式判断是否超时，避免意外死循环 
				current_tick=rt_tick_get();
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("ISM800C不可用！\n");
						rt_free(point);
						pt = RT_NULL;
						point= RT_NULL;
						return;
					}
				}
				USART_RX2_STA=0;
				printf("All messages have been deleted!\n");
				//跳转到判断是否收到短信
				goto loop;
			}
			else
			 { 
				//读取短信内容
				USART_RX2_STA=0;
				sprintf(GSM_buffer,"AT+CMGR=%s\n\n",pt+12);
				printf("%s\n",GSM_buffer);
				memset((uint8_t*)USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
				rt_kprintf("发送读信息命令\n");
				usart2_senddata(GSM_buffer); 
				//等待来自ISM800C的反馈
				//采用获取系统tick方式判断是否超时，避免意外死循环 
				current_tick=rt_tick_get();
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>50000)
					{
						rt_kprintf("ISM800C不可用！\n");
						rt_free(point);
						pt = RT_NULL;
						point= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("第一次读：%s\n",USART_RX2_BUF);	
                memset((uint8_t*)USART_RX2_BUF,0,sizeof(USART_RX2_BUF));				 
				usart2_senddata(GSM_buffer);	
				//等待来自ISM800C的反馈
				//采用获取系统tick方式判断是否超时，避免意外死循环 
				current_tick=rt_tick_get();
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>50000)
					{
						rt_kprintf("ISM800C不可用！\n");
						rt_free(point);
						pt = RT_NULL;
						point= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("第二次读：%s\n",USART_RX2_BUF);
				rt_kprintf("判断是否收到report status\n");
				pt = rt_strstr((char*)USART_RX2_BUF,"report status");
				printf("pt:%p\n",pt);
				rt_kprintf("判断是否要发送系统状态\n");
				if(pt!=RT_NULL)
				{
					//通过短信查看系统状态 
					//发送邮箱
					rt_kprintf("发送邮件发送系统状态\n");
					//GSM_SendSMS(report_status); 
					rt_mb_send(&GSM_mq_send,report_status);
					goto LOOP;
				}
				rt_kprintf("判断是否收到home\n");
				pt =  rt_strstr((char*)USART_RX2_BUF,"home"); 
				if(pt!=RT_NULL)
				{
					//通过短信设置回家模式
					rt_enter_critical();//设置系统工作模式时，禁止系统调度
					sys_status = HOME;
					POINT_COLOR =GREEN;
					LCD_ShowString(lcddev.width-95,58,200,24,24," Home ");
					BEEP = 0;			//防止在发送消息时蜂鸣器一直响
			        RED_LED   = 1;
					//发送反馈信息给用户显示是否设置成功
					if(sys_status == HOME)
						rt_mb_send(&GSM_mq_send,set_ok);//反馈设置状态给用户
					else
						rt_mb_send(&GSM_mq_send,set_Err);
					rt_exit_critical(); //开放系统调度
					goto LOOP;
				}
			   rt_kprintf("判断是否收到out\n");
			   pt =  rt_strstr((char*)USART_RX2_BUF,"out"); 
			   if(pt!=RT_NULL)
			   {
				    //通过短信设置离家模式
				    rt_enter_critical();//设置系统工作模式时，禁止系统调度
					sys_status = OUT;
					POINT_COLOR =RED;
					LCD_ShowString(lcddev.width-95,58,200,24,24," OUT ");
				    BEEP = 0;			//防止在发送消息时蜂鸣器一直响
			        RED_LED   = 1;
				    //发送反馈信息给用户显示是否设置成功
					if(sys_status == OUT)
						rt_mb_send(&GSM_mq_send,set_ok); //发送设置状态
					else
						rt_mb_send(&GSM_mq_send,set_Err);
				    rt_exit_critical();  //开放系统调度
					goto LOOP;
			   }
			   //没有收到正确指令就发送提示信息
			   if(pt==RT_NULL)
			   {
					rt_kprintf("没有收到SMS命令\n");
				    rt_mb_send(&GSM_mq_send,tips);
			   }
			 }
		 }
LOOP:
		 memset((uint8_t*)USART_RX2_BUF,0,sizeof(USART_RX2_BUF));	
		 //内存空间使用完后要调用rt_free(pt)来释放申请的堆空间
         pt  =  point;//将申请时得到的地址重新写回pt.		 
		 if(pt!=RT_NULL)//释放前判断是否要释放的内存空间是否存在
		 {
			 printf("pt:0x%p\n",pt); 
			 rt_free(pt);//存在就释放，没有就直接跳过
			 rt_kprintf("内存堆:0x%p已经释放\n",pt);
			 pt = RT_NULL;
		 }
		 else
		 {
			rt_kprintf("没有需要释放内存堆pt\n");
		 }
		 
     }
	  rt_thread_mdelay(10);
  }
}
//GSM通信线程堆栈
ALIGN(RT_ALIGN_SIZE)
static char GSM_thread_stack[256];
//GSM通信线程---专门用于判断是否接收到GSM信息---若接收到信息则释放信号量(二值信号量)，给信息读取线程 
static void GSM_thread(void *parameter)
{
	while(1)
	{
		//判断是否收到短信
		if((USART_RX2_STA&0x8000))
		{
	       USART_RX2_STA=0;
		   //收到短信则释放信号量GSM_sem_read，以唤醒线程GSM_read_thread
           rt_sem_release(GSM_sem_read);			
	     }
		rt_thread_mdelay(10);
    }
}
//红外线程堆栈
ALIGN(RT_ALIGN_SIZE)
static char IRDA_thread_stack[256];
//红外线程 
static void IRDA_thread(void *parameter)
{
	//红外遥控参数
    uint8_t irda_value = 0;
	while(1)
	{
		irda_value = Remote_Scan();
		    if(irda_value!=0)
			{
				while(irda_value==Remote_Scan());//等待按键松开
				switch(irda_value)
				{
					case powr: 
					{
						rt_enter_critical();
						if(sys_status==HOME)
						{
						  POINT_COLOR =RED;
						  LCD_ShowString(lcddev.width-95,58,200,24,24," OUT  ");
						  sys_status = OUT;
						}
						else
						{
						  POINT_COLOR =GREEN;
						  LCD_ShowString(lcddev.width-95,58,200,24,24," Home ");
						  sys_status = HOME;
						}
						rt_exit_critical(); 
						break;
					}
					case up:    Light_Control(2);; break;
					case log:   Light_Control(3);; break;
					case left:  rt_mb_send(&IRDA_NRF2401_send,left);  break;
					case stop:  rt_mb_send(&IRDA_NRF2401_send,stop);  break;
					case right: rt_mb_send(&IRDA_NRF2401_send,right);  break;
					default:    break;
				}
			 }
			rt_thread_mdelay(10);
	}
}
//安防线程堆栈
ALIGN(RT_ALIGN_SIZE)
static char Security_thread_stack[512];
static void security_thread(void *parameter)
{
	while(1)
	{
		//判断是否为离家模式
		if(sys_status==OUT)
		{
			//判断是否有人闯入
			if(RSD_STATUS|WB_STATUS)
			{
				rt_mb_send(&GSM_mq_send,Invaiding);
				BEEP = 1;//启动声光报警
				RED_LED   = 0;
				rt_thread_mdelay(200);
				BEEP = 0;
				RED_LED   = 1;
			}
			else
			{
				;
			}
		}
		//判断煤气情况
		if((mq<=3.4)&&(mq>=0.3))
		{
			rt_mb_send(&GSM_mq_send,MQ_LEAKING);//通过短信发送煤气泄漏信息给用户
			BEEP = 1;   //启动声光报警
			RED_LED   = 0;
			rt_thread_mdelay(150);
			BEEP = 0;
			RED_LED   = 1;
		}
		//判断火灾情况
		if((hz<=1.0)&&(hz!=0))
		{
			printf("hz= %f\n",hz);
			rt_mb_send(&GSM_mq_send,On_Fire);//通过短信发送火灾信息给用户
			BEEP = 1;   //启动声光报警
			RED_LED   = 0;
			rt_thread_mdelay(150);
			BEEP = 0;
			RED_LED   = 1;
		}
		rt_thread_mdelay(50);
	}
}

//触摸线程堆栈
ALIGN(RT_ALIGN_SIZE)
static char touch_thread_stack[512];
/******************触摸线程****************/
static void touch_thread(void *parameter)
{
	
	while(1)
	  {
		//触摸屏扫描
		tp_dev.scan(0); 	
        //判断触摸屏被按下		  
		if(tp_dev.sta&TP_PRES_DOWN)			
		{	
		 	if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height) //检测触摸范围是否在液晶显示尺寸之内
			{	
				//printf("X:%d\n",tp_dev.x[0]);//通过读取X,Y的值确定每个按键的区域，然后判断
				//printf("Y:%d\n",tp_dev.y[0]);//被按下的点是否在该区域内判断某个功能按钮是否被按下
				//判断time是否被按下  
				if(TIME)
				{
					rt_enter_critical();			//禁止系统调度
					HAL_NVIC_DisableIRQ(RTC_IRQn);	//进入前禁止RTC中断					
					Set_Time();	       				//进入时间设置	
					rt_exit_critical();				//恢复系统调度
				}
				//判断MODE区是否被按下
				if(MODE)
				{
					
					//切换系统工作模式
					rt_enter_critical();
					if(sys_status==HOME)
					{
					  POINT_COLOR =RED;
					  LCD_ShowString(lcddev.width-95,58,200,24,24," OUT  ");
					  sys_status = OUT;
					}
					else
					{
					  POINT_COLOR =GREEN;
					  LCD_ShowString(lcddev.width-95,58,200,24,24," Home ");
					  sys_status = HOME;
					}
					rt_exit_critical(); 
				}
				//判断Bedroom_Light区是否被按下
				if(Bedroom_Light)
				{
					Light_Control(1);
				}	
				//判断Livingroom_Light区是否被按下
				if(Livingroom_Light)
				{
					Light_Control(2);
				}
				//判断Corridor_Light区是否被按下
				if(Corridor_Light)
				{
					Light_Control(3);
				}
				//判断Curtain_CLOSE区是否被按下
				if(Curtain_CLOSE)
				{
					 rt_mb_send(&IRDA_NRF2401_send,right); 
				}
				//判断Curtain_OPEN区是否被按下
				if(Curtain_OPEN)
				{
					rt_mb_send(&IRDA_NRF2401_send,left);
				}
				//判断Curtain_KT区是否被按下
				if(Curtain_KT)
				{
					rt_mb_send(&IRDA_NRF2401_send,stop);
				}
				//消除触摸抖动  
				while(tp_dev.sta&TP_PRES_DOWN)	
				{
					tp_dev.scan(0); 
				}
			}
		}
		rt_thread_mdelay(5);
	}
}

//照明线程堆栈
ALIGN(RT_ALIGN_SIZE)
static char Light_thread_stack[356];
/******************照明线程****************/
static void Corridor_Light_thread(void *parameter)
{
	while(1)
	{
		//在家模式时
		if(sys_status==HOME)
		{
			//判断是否是夜晚
			if(get_LS1_Status(5,LS1))
			{
				//是夜晚再判断是否有人经过走廊的热释电传感器和微波传感器
				if(RSD_STATUS|WB_STATUS)
				{
					rt_enter_critical();//防止颜色显示过程被打断
					POINT_COLOR =RED;
					CORRIDOR_LIGHT = 0; //打开走廊灯
					//灯的状态同步到LCD
					LCD_ShowString(0,272,200,16,16,"Corridor   Light:     ON ");
					rt_exit_critical(); 
					//点亮后延时一段时间
					rt_thread_mdelay(8500);
					//防止颜色显示过程被打断
					rt_enter_critical();
					//再关闭
					CORRIDOR_LIGHT = 1;
					POINT_COLOR =BLACK;
					LCD_ShowString(0,272,200,16,16,"Corridor   Light:     OFF");
					rt_exit_critical(); 
				}
			}
		}
		//离家模式时，关闭走廊灯
		else if(sys_status==OUT)
		{
			//防止颜色显示过程被打断
			rt_enter_critical();
			POINT_COLOR =BLACK;
			//灯的状态同步到LCD
		    LCD_ShowString(0,272,200,16,16,"Corridor   Light:     OFF");
			rt_exit_critical(); 
		}
		rt_thread_mdelay(10);
	}
}


//用于获取传感器状态
ALIGN(RT_ALIGN_SIZE)
static char Get_Sensor_Status_Thread_stack[1024];
//每隔200毫秒更新一次 
static void Get_Sensor_Status_thread(void *parameter)
{
	while(1)
	{
	  //获取光敏传感器值
	  LS1_VAL = get_aver_val(5,LS1);
	  //获取温湿度值
	  DHT11_Read_Data(&temp,&humi);
	  //将温度值转换为字符串
	  sprintf(temp_buffer,"%d'C",temp);
	  //判断温度是否在23~27度之间，是则以绿色显示温度值，否则以红色显示
	  if((temp>=23)&&(temp<=27))
	    POINT_COLOR =GREEN;
	  else
		POINT_COLOR =RED;
	  LCD_ShowString(105,124,200,16,16,temp_buffer);//更新到LCD
	  //将湿度值转换为字符串
	  sprintf(humi_buffer,"%d%%",humi);
	   //判断湿度是否在45%~65%度之间，是则以绿色显示温度值，否则以红色显示
	  if((humi>=45)&&(humi<=65))
	    POINT_COLOR =GREEN;
	  else
		POINT_COLOR =RED;
	  LCD_ShowString(105,142,200,16,16,humi_buffer);//更新到LCD
     
	  //获取煤气值
	  mq = get_aver_val(5,MQ);
	  sprintf(MQ_buffer,"%d.%.2d",(uint8_t)mq,(uint8_t)Get_decimal(mq,2));
	  //若煤气值大于0.4以红色显示，否则以绿色显示
	  if(mq>=0.3)
	    POINT_COLOR =RED;
	  else
		POINT_COLOR =GREEN;
	  LCD_ShowString(105,176,200,16,16,MQ_buffer);//更新到LCD
	  
	  //火灾传感器值
	  hz = get_aver_val(15,HZ);
	  //printf("hz = %f\n",hz);
	  sprintf(HZ_buffer,"%d.%.2d",(uint8_t)hz,(uint8_t)Get_decimal(hz,2));
	  //若火灾值大于1以绿色显示，否则以红色显示
	  if(hz>=1.0)
	    POINT_COLOR =GREEN;
	  else
		POINT_COLOR =RED;
	  LCD_ShowString(105,158,200,16,16,HZ_buffer); //更新到LCD
	  rt_thread_mdelay(200);                       //200ms更新一次
     }
}


//创建实例
int Smart_IOT_Center(void)
{
	rt_err_t result = 0;
	//显示系统模式
	POINT_COLOR =GREEN;
	LCD_ShowString(lcddev.width-95,58,200,24,24," Home ");
	//创建一个煤气值存放临界访问信号量--该信号量没有使用到，但屏蔽后会出现GSM_Thread_stack is close to the bottom断言，原因待解
	mq_sem = rt_sem_create("mq_sem",1,RT_IPC_FLAG_FIFO);
	if(mq_sem!=RT_NULL)
		printf("Successfully Create mq_sem!\n");
	else
		printf("Failed to Create mq_sem!\n");
#if 1
	//创建一个GSM信号量--读
	GSM_sem_read = rt_sem_create("GSM_sem_read",0,RT_IPC_FLAG_FIFO);
	if(GSM_sem_read!=RT_NULL)
		printf("Successfully Create GSM_sem_read!\n");
	else
		printf("Failed to Create GSM_sem_read!\n");
	
	//创建一个红外邮箱--发
	result = rt_mb_init( &IRDA_NRF2401_send,
						 "IRDA_NRF2401_send",
	                      &IRDA_NRF2401_pool[0],
						  sizeof(IRDA_NRF2401_pool)/4,
						  RT_IPC_FLAG_FIFO);
	if(result==RT_EOK)
		printf("Successfully Create IRDA_NRF2401_send!\n");
	else
		printf("Failed to Create IRDA_NRF2401_send!\n");
	
	//创建一个GSM邮箱--发
	result = rt_mb_init( &GSM_mq_send,
						 "GSM_mb_send",
	                      &mq_pool[0],
						  sizeof(mq_pool)/4,
						  RT_IPC_FLAG_FIFO);
	if(result==RT_EOK)
		printf("Successfully Create GSM_mq_send!\n");
	else
		printf("Failed to Create GSM_mq_send!\n");
	
	//创建IRDA_NRF2401线程 
	result = rt_thread_init(&NRF2401_Thread,
						   "NRF2401_Thread",
						    NRF2401_thread,
	                        RT_NULL,
						    &NRF2401_Thread_stack[0],
						    sizeof(NRF2401_Thread_stack),
						    THREAD_PRIORITY+1,
						    THREAD_TIMESLICE  	
						  );

	//启动IRDA_NRF2401线程 
	if(result==RT_EOK)
	rt_thread_startup(&NRF2401_Thread);
	

#endif
    											
    //创建触摸线程
    result = rt_thread_init( &Touch_Thread,
							"touch_thread",
							 touch_thread,
							 RT_NULL,
							 &touch_thread_stack[0],
							 sizeof(touch_thread_stack),
							 5,
							 THREAD_TIMESLICE  	
							);
	//启动触摸线程 
	if(result==RT_EOK)
	rt_thread_startup(&Touch_Thread);
	
	//创建照明线程
	result = rt_thread_init( &Light_Thread,
							"Light_Thread",
							 Corridor_Light_thread,
							 RT_NULL,
							 &Light_thread_stack[0],
							 sizeof(Light_thread_stack),
							 THREAD_PRIORITY+7,
							 THREAD_TIMESLICE  	
							);
	//启动照明线程
	if(result==RT_EOK)
	rt_thread_startup(&Light_Thread);
	
    //创建传感器状态获取线程
    result = rt_thread_init( &Get_Sensor_Status_Thread,
							"Get_Sensor_Status_Thread",
							 Get_Sensor_Status_thread,
							 RT_NULL,
							 &Get_Sensor_Status_Thread_stack[0],
							 sizeof(Get_Sensor_Status_Thread_stack),
							 THREAD_PRIORITY-2,
							 THREAD_TIMESLICE  	
							);
	//传感器状态获取线程
	if(result==RT_EOK)
	rt_thread_startup(&Get_Sensor_Status_Thread);	
	
	
	//创建红外检测线程
    result = rt_thread_init( &IRDA_Thread,
							"IRDA_Thread",
							 IRDA_thread,
							 RT_NULL,
							 &IRDA_thread_stack[0],
							 sizeof(IRDA_thread_stack),
							 THREAD_PRIORITY-1,
							 THREAD_TIMESLICE  	
							);
	//启动红外检测线程 
	if(result==RT_EOK)
	rt_thread_startup(&IRDA_Thread);
 	
	//创建GSM通信线程
    result = rt_thread_init( &GSM_Thread,
							"GSM_Thread",
							 GSM_thread,
							 RT_NULL,
							 &GSM_thread_stack[0],
							 sizeof(GSM_thread_stack),
							 THREAD_PRIORITY+9,
							 THREAD_TIMESLICE  	
							);
	//启动GSM通信线程 
	if(result==RT_EOK)
	rt_thread_startup(&GSM_Thread);
	
	//创建GSM读取信息线程
    result = rt_thread_init( &GSM_Thread_Read,
							"GSM_Thread_Read",
							 GSM_read_thread,
							 RT_NULL,
							 &GSM_read_thread_stack[0],
							 sizeof(GSM_read_thread_stack),
							 THREAD_PRIORITY+3,
							 THREAD_TIMESLICE  	
							);
	//启动GSM读取信息线程
	if(result==RT_EOK)
	rt_thread_startup(&GSM_Thread_Read);
	
	//创建GSM发送信息线程
    result = rt_thread_init( &GSM_Thread_Send,
							"GSM_Thread_Send",
							 GSM_send_thread,
							 RT_NULL,
							 &GSM_send_thread_stack[0],
							 sizeof(GSM_send_thread_stack),
							 THREAD_PRIORITY+11,
							 THREAD_TIMESLICE  	
							);
	//启动GSM读取信息线程
	if(result==RT_EOK)
	rt_thread_startup(&GSM_Thread_Send);
	//创建安防线程
    result = rt_thread_init( &Security_Thread,
							"security_thread",
							 security_thread,
							 RT_NULL,
							 &Security_thread_stack[0],
							 sizeof(Security_thread_stack),
							 THREAD_PRIORITY+14,
							 THREAD_TIMESLICE  	
							);
	//启动安防线程 
	if(result==RT_EOK)
	rt_thread_startup(&Security_Thread);
	
	return 0;
}

//主函数
int main(void)
{
	#if 1
   //初始化GSM
   scm800c_init();
   //lwip初始化
   //注意：NRF2401的中断引脚可能可能会造成网络初始化失败
   if(lwip_comm_init()==0)
	   rt_kprintf("网络初始化成功\n");
   else
	   rt_kprintf("网络初始化失败！\n");
   //Web Server模式
   httpd_init();  			
   //实例初始化并启动	
   Smart_IOT_Center();
	#endif
  
}
