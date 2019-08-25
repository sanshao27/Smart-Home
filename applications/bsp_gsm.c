#include "public.h"
#if 1
char cmd_1A[] ={0x1A};
extern uint8_t temp,humi;
extern float mq;
extern rt_timer_t timer1_sensor_Status ;
//GSM配置
int scm800c_init(void)
{
	rt_tick_t current_tick=0;
	//设置为文本格式
	usart2_senddata("AT+CMGF=1\n");
	current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>5000)
		{
			rt_kprintf("ISM800C初始化等待反馈超时！\n");
			return 0;
		}
	}
	USART_RX2_STA =0;
	usart2_senddata("AT+CMGF=1\n");
	//等待反馈
	current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>5000)
		{
			rt_kprintf("ISM800C初始化等待反馈超时！\n");
			return 0;
		}
	}
    if((USART_RX2_STA&0x8000))
	  {
		if(rt_strstr((char*)USART_RX2_BUF,"OK") !=NULL)
		{
			printf("文本格式设置成功\n");
		}
		else if(rt_strstr((char*)USART_RX2_BUF,"ERROR") !=NULL) 
		{
		   printf("文本格式设置失败\n");
		}
		USART_RX2_STA=0;
		//串口缓存清空
		memset(USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
	  }			
	else
	{
		printf("无应答\n");
	}
	 

	//设置GSM字符集
	usart2_senddata("AT+CSCS=\"GSM\"\n");
	//等待反馈
    current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>5000)
		{
			rt_kprintf("ISM800C初始化等待反馈超时！\n");
			return 0;
		}
	}
	if((USART_RX2_STA&0x8000))
	   {   
		if(rt_strstr((char*)USART_RX2_BUF,"OK") !=NULL)
		{
			printf("GSM字符集设置成功\n");
		}
		else if(rt_strstr((char*)USART_RX2_BUF,"ERROR") !=NULL) 
		{
		   printf("GSM字符集设置失败\n");
		}
		USART_RX2_STA=0;
		//串口缓存清空
		memset(USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
	  }			
	 else
	{
		printf("无应答\n");
	}

	//设置短信提醒
	usart2_senddata("AT+CNMI=2,1\n");
	//等待反馈
    current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>100000)
		{
			rt_kprintf("等待ISM800C反馈超时！\n");
			return 0;
		}
	}
	if((USART_RX2_STA&0x8000))
	  {
		
		if(rt_strstr((char*)USART_RX2_BUF,"OK") !=NULL)
		{
			printf("短信提醒设置成功\n");
		}
		else if(rt_strstr((char*)USART_RX2_BUF,"ERROR") !=NULL) 
		{
		   printf("短信提醒设置失败\n");
		}
		USART_RX2_STA=0;
		//串口缓存清空
		memset(USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
	  }	
     else
	{
		printf("无应答\n");
	}	
	return 0;
}
//INIT_APP_EXPORT(scm800c_init);




//发送短信
void GSM_SendSMS(uint8_t content)
{
	rt_tick_t current_tick=0;
	char temp_buffer[] = {0};
    char humi_buffer[] = {0};
	char MQ_buffer[]= {0};
	static char*prt = RT_NULL;
	static char*prt_backup = RT_NULL;
	prt = rt_malloc(256);
	if(prt==RT_NULL)
	{
		printf("Failed to allocate mem for prt!\n");
		return;
	}
	prt_backup = prt;
	//检测ISM800C是否可用,不可用则直接退出发送程序
	usart2_senddata("AT\n");
	//采用获取系统tick方式判断是否超时，避免意外死循环 
	current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>100000)
		{
			rt_kprintf("ISM800C不可用！\n");
			rt_free(prt_backup);
			prt = RT_NULL;
			prt_backup= RT_NULL;
			return;
		}
	}
	USART_RX2_STA = 0;
    if(rt_strstr((char*)USART_RX2_BUF,"OK")==RT_NULL)
    {
		printf("ISM800C不可用，消息发送终止！\n");
		return;
	}		
	USART_RX2_STA=0;
	memset((uint8_t*)USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
	//设置发送信息指令
	usart2_senddata("AT+CMGF=1\n");
	//采用获取系统tick方式判断是否超时，避免意外死循环 
	current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>100000)
		{
			rt_kprintf("等待ISM800C反馈超时！\n");
			rt_free(prt_backup);
			prt = RT_NULL;
			prt_backup= RT_NULL;
			return;
		}
	}
	USART_RX2_STA = 0;	
	printf("USART_RX2_BUF：%s\n",USART_RX2_BUF);
	
	usart2_senddata("AT+CSCS=\"GSM\"\n");
	//采用获取系统tick方式判断是否超时，避免意外死循环 
	current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>100000)
		{
			rt_kprintf("等待ISM800C反馈超时！\n");
			rt_free(prt_backup);
			prt = RT_NULL;
			prt_backup= RT_NULL;
			return;
		}
	}
	USART_RX2_STA = 0;	
	printf("USART_RX2_BUF：%s\n",USART_RX2_BUF);
	
	usart2_senddata("AT+CNMI=2,1\n");
	//采用获取系统tick方式判断是否超时，避免意外死循环 
	current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>100000)
		{
			rt_kprintf("等待ISM800C反馈超时！\n");
			rt_free(prt_backup);
			prt = RT_NULL;
			prt_backup= RT_NULL;
			return;
		}
	}
	USART_RX2_STA = 0;	
	printf("USART_RX2_BUF：%s\n",USART_RX2_BUF);
	usart2_senddata("AT+CMGS=\"17511681733\"\n");
	//采用获取系统tick方式判断是否超时，避免意外死循环 
	current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>100000)
		{
			rt_kprintf("等待ISM800C反馈超时！\n");
			rt_free(prt_backup);
			prt = RT_NULL;
			prt_backup= RT_NULL;
			return;
		}
	}
	USART_RX2_STA = 0;	 
	printf("USART_RX2_BUF：%s\n",USART_RX2_BUF);
    prt =  rt_strstr((char*)USART_RX2_BUF,"> ");
	printf("prt%p\n",prt);
	//判断是否设置成功
    if(prt!=RT_NULL)
    {
		//根据用户要求选择发送内容
		switch(content)
		{
			//发送系统状态信息
			case report_status:
			{
				printf("发送report_status请求的内容\n" );
				sprintf(temp_buffer,"Temperature:%d'C\n",temp);
				usart2_senddata(temp_buffer);
				current_tick=rt_tick_get();
				//采用获取系统tick方式判断是否超时，避免意外死循环 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("等待ISM800C反馈超时！\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF：%s\n",USART_RX2_BUF);
				sprintf(humi_buffer,"Humidity:%d%%\n",humi);
				usart2_senddata(humi_buffer);
				current_tick=rt_tick_get();
				//采用获取系统tick方式判断是否超时，避免意外死循环 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("等待ISM800C反馈超时！\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF：%s\n",USART_RX2_BUF);
				sprintf(MQ_buffer,  "M       Q:%d.%.2d\n",(uint8_t)mq,(uint8_t)Get_decimal(mq,2)); 
				usart2_senddata(MQ_buffer);
				current_tick=rt_tick_get();
				//采用获取系统tick方式判断是否超时，避免意外死循环 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("等待ISM800C反馈超时！\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF：%s\n",USART_RX2_BUF);
				break;
			}
			//发送ok反馈信息
			case set_ok:
			{
				usart2_senddata("set_ok");
				current_tick=rt_tick_get();
				//采用获取系统tick方式判断是否超时，避免意外死循环 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("等待ISM800C反馈超时！\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF：%s\n",USART_RX2_BUF);
				break;
			}
			//发送错误提示信息
			case set_Err:
			{
				usart2_senddata("set_Err");
				current_tick=rt_tick_get();
				//采用获取系统tick方式判断是否超时，避免意外死循环 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("等待ISM800C反馈超时！\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF：%s\n",USART_RX2_BUF);
				break;
			}
			//煤气泄漏信息
			case MQ_LEAKING:
			{
				usart2_senddata("MQ_LEAKING");
				current_tick=rt_tick_get();
				//采用获取系统tick方式判断是否超时，避免意外死循环 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("等待ISM800C反馈超时！\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF：%s\n",USART_RX2_BUF);
				break;
			}
			//入侵信息
			case Invaiding:
			{
				usart2_senddata("Invaiding");
				current_tick=rt_tick_get();
				//采用获取系统tick方式判断是否超时，避免意外死循环 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("等待ISM800C反馈超时！\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF：%s\n",USART_RX2_BUF);
				break;
			}
			//提示信息
			case tips:
			{
				usart2_senddata("Wrong Command!!\n");
				current_tick=rt_tick_get();
				//采用获取系统tick方式判断是否超时，避免意外死循环 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("等待ISM800C反馈超时！\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				usart2_senddata("1.Send \"report status\"to get sensor status.\n");
				current_tick=rt_tick_get();
				//采用获取系统tick方式判断是否超时，避免意外死循环 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("等待ISM800C反馈超时！\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				usart2_senddata("2.Send \"out\"to set OUT mode.\n");
				current_tick=rt_tick_get();
				//采用获取系统tick方式判断是否超时，避免意外死循环 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("等待ISM800C反馈超时！\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				usart2_senddata("3.Send \"home\"to set HOME mode.\n");
				//采用获取系统tick方式判断是否超时，避免意外死循环 
				current_tick=rt_tick_get();
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("等待ISM800C反馈超时！\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF：%s\n",USART_RX2_BUF);
				break;
			}
			default:break;
		}
		memset((uint8_t*)USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
	    printf("\n开始发送消息\n" );
		//开始发送--0X1A
		usart2_senddata((char*)cmd_1A);
		//获取反馈信息
		memset((uint8_t*)USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
		//等待反馈信息
		//while((USART_RX2_STA&0x8000)==0);//使用定时器会造成信息发送失败(rt_timer_t),原因待解
		//采用获取系统tick方式判断是否超时，避免意外死循环 
		current_tick=rt_tick_get();
		while((USART_RX2_STA&0x8000)==0)
		{
			if((rt_tick_get()-current_tick)>100000)
			{
				rt_kprintf("等待ISM800C反馈超时！\n");
				rt_free(prt_backup);
				prt = RT_NULL;
				prt_backup= RT_NULL;
				return;
			}
		}
		printf("等待时长为:%ldms\n",(rt_tick_get()-current_tick));
		USART_RX2_STA = 0;
		printf("11USART_RX2_BUF：%s\n",USART_RX2_BUF);
		//判断是否收到反馈信息
	    prt= rt_strstr((char*)USART_RX2_BUF,"+CMGS: ");
		printf("prt:%p\n",prt);
	    if(prt!=RT_NULL)
		   {
				printf("信息发送ok\n");
			    //避免ISM800C模式处在等待接收信息模式
				usart2_senddata(cmd_1A);
		   }
	   else
		   {
				printf("信息发送失败\n");
				usart2_senddata(cmd_1A); 
		   }
	}
	//没有设置成功返回提示信息到串口
	else
	  {
		  printf("发送指令没有得到模块正常响应\n");
		  usart2_senddata(cmd_1A);  
	  }
	//申请的内存堆释放
	  prt = prt_backup;
	  if(prt==RT_NULL)
	  {
		printf("GSM_SendSMS模块内存堆释放无效！\n");
	  }
	  else
	  {
		rt_free(prt);
		rt_kprintf("GSM_SendSMS模块内存堆:0x%p已经释放\n",prt);
		prt = RT_NULL;
	  }
	  memset((uint8_t*)USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
	 
}
#endif

 
