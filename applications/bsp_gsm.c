#include "public.h"
#if 1
char cmd_1A[] ={0x1A};
extern uint8_t temp,humi;
extern float mq;
extern rt_timer_t timer1_sensor_Status ;
//GSM����
int scm800c_init(void)
{
	rt_tick_t current_tick=0;
	//����Ϊ�ı���ʽ
	usart2_senddata("AT+CMGF=1\n");
	current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>5000)
		{
			rt_kprintf("ISM800C��ʼ���ȴ�������ʱ��\n");
			return 0;
		}
	}
	USART_RX2_STA =0;
	usart2_senddata("AT+CMGF=1\n");
	//�ȴ�����
	current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>5000)
		{
			rt_kprintf("ISM800C��ʼ���ȴ�������ʱ��\n");
			return 0;
		}
	}
    if((USART_RX2_STA&0x8000))
	  {
		if(rt_strstr((char*)USART_RX2_BUF,"OK") !=NULL)
		{
			printf("�ı���ʽ���óɹ�\n");
		}
		else if(rt_strstr((char*)USART_RX2_BUF,"ERROR") !=NULL) 
		{
		   printf("�ı���ʽ����ʧ��\n");
		}
		USART_RX2_STA=0;
		//���ڻ������
		memset(USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
	  }			
	else
	{
		printf("��Ӧ��\n");
	}
	 

	//����GSM�ַ���
	usart2_senddata("AT+CSCS=\"GSM\"\n");
	//�ȴ�����
    current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>5000)
		{
			rt_kprintf("ISM800C��ʼ���ȴ�������ʱ��\n");
			return 0;
		}
	}
	if((USART_RX2_STA&0x8000))
	   {   
		if(rt_strstr((char*)USART_RX2_BUF,"OK") !=NULL)
		{
			printf("GSM�ַ������óɹ�\n");
		}
		else if(rt_strstr((char*)USART_RX2_BUF,"ERROR") !=NULL) 
		{
		   printf("GSM�ַ�������ʧ��\n");
		}
		USART_RX2_STA=0;
		//���ڻ������
		memset(USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
	  }			
	 else
	{
		printf("��Ӧ��\n");
	}

	//���ö�������
	usart2_senddata("AT+CNMI=2,1\n");
	//�ȴ�����
    current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>100000)
		{
			rt_kprintf("�ȴ�ISM800C������ʱ��\n");
			return 0;
		}
	}
	if((USART_RX2_STA&0x8000))
	  {
		
		if(rt_strstr((char*)USART_RX2_BUF,"OK") !=NULL)
		{
			printf("�����������óɹ�\n");
		}
		else if(rt_strstr((char*)USART_RX2_BUF,"ERROR") !=NULL) 
		{
		   printf("������������ʧ��\n");
		}
		USART_RX2_STA=0;
		//���ڻ������
		memset(USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
	  }	
     else
	{
		printf("��Ӧ��\n");
	}	
	return 0;
}
//INIT_APP_EXPORT(scm800c_init);




//���Ͷ���
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
	//���ISM800C�Ƿ����,��������ֱ���˳����ͳ���
	usart2_senddata("AT\n");
	//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
	current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>100000)
		{
			rt_kprintf("ISM800C�����ã�\n");
			rt_free(prt_backup);
			prt = RT_NULL;
			prt_backup= RT_NULL;
			return;
		}
	}
	USART_RX2_STA = 0;
    if(rt_strstr((char*)USART_RX2_BUF,"OK")==RT_NULL)
    {
		printf("ISM800C�����ã���Ϣ������ֹ��\n");
		return;
	}		
	USART_RX2_STA=0;
	memset((uint8_t*)USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
	//���÷�����Ϣָ��
	usart2_senddata("AT+CMGF=1\n");
	//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
	current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>100000)
		{
			rt_kprintf("�ȴ�ISM800C������ʱ��\n");
			rt_free(prt_backup);
			prt = RT_NULL;
			prt_backup= RT_NULL;
			return;
		}
	}
	USART_RX2_STA = 0;	
	printf("USART_RX2_BUF��%s\n",USART_RX2_BUF);
	
	usart2_senddata("AT+CSCS=\"GSM\"\n");
	//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
	current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>100000)
		{
			rt_kprintf("�ȴ�ISM800C������ʱ��\n");
			rt_free(prt_backup);
			prt = RT_NULL;
			prt_backup= RT_NULL;
			return;
		}
	}
	USART_RX2_STA = 0;	
	printf("USART_RX2_BUF��%s\n",USART_RX2_BUF);
	
	usart2_senddata("AT+CNMI=2,1\n");
	//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
	current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>100000)
		{
			rt_kprintf("�ȴ�ISM800C������ʱ��\n");
			rt_free(prt_backup);
			prt = RT_NULL;
			prt_backup= RT_NULL;
			return;
		}
	}
	USART_RX2_STA = 0;	
	printf("USART_RX2_BUF��%s\n",USART_RX2_BUF);
	usart2_senddata("AT+CMGS=\"17511681733\"\n");
	//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
	current_tick=rt_tick_get();
	while((USART_RX2_STA&0x8000)==0)
	{
		if((rt_tick_get()-current_tick)>100000)
		{
			rt_kprintf("�ȴ�ISM800C������ʱ��\n");
			rt_free(prt_backup);
			prt = RT_NULL;
			prt_backup= RT_NULL;
			return;
		}
	}
	USART_RX2_STA = 0;	 
	printf("USART_RX2_BUF��%s\n",USART_RX2_BUF);
    prt =  rt_strstr((char*)USART_RX2_BUF,"> ");
	printf("prt%p\n",prt);
	//�ж��Ƿ����óɹ�
    if(prt!=RT_NULL)
    {
		//�����û�Ҫ��ѡ��������
		switch(content)
		{
			//����ϵͳ״̬��Ϣ
			case report_status:
			{
				printf("����report_status���������\n" );
				sprintf(temp_buffer,"Temperature:%d'C\n",temp);
				usart2_senddata(temp_buffer);
				current_tick=rt_tick_get();
				//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("�ȴ�ISM800C������ʱ��\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF��%s\n",USART_RX2_BUF);
				sprintf(humi_buffer,"Humidity:%d%%\n",humi);
				usart2_senddata(humi_buffer);
				current_tick=rt_tick_get();
				//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("�ȴ�ISM800C������ʱ��\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF��%s\n",USART_RX2_BUF);
				sprintf(MQ_buffer,  "M       Q:%d.%.2d\n",(uint8_t)mq,(uint8_t)Get_decimal(mq,2)); 
				usart2_senddata(MQ_buffer);
				current_tick=rt_tick_get();
				//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("�ȴ�ISM800C������ʱ��\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF��%s\n",USART_RX2_BUF);
				break;
			}
			//����ok������Ϣ
			case set_ok:
			{
				usart2_senddata("set_ok");
				current_tick=rt_tick_get();
				//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("�ȴ�ISM800C������ʱ��\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF��%s\n",USART_RX2_BUF);
				break;
			}
			//���ʹ�����ʾ��Ϣ
			case set_Err:
			{
				usart2_senddata("set_Err");
				current_tick=rt_tick_get();
				//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("�ȴ�ISM800C������ʱ��\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF��%s\n",USART_RX2_BUF);
				break;
			}
			//ú��й©��Ϣ
			case MQ_LEAKING:
			{
				usart2_senddata("MQ_LEAKING");
				current_tick=rt_tick_get();
				//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("�ȴ�ISM800C������ʱ��\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF��%s\n",USART_RX2_BUF);
				break;
			}
			//������Ϣ
			case Invaiding:
			{
				usart2_senddata("Invaiding");
				current_tick=rt_tick_get();
				//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("�ȴ�ISM800C������ʱ��\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF��%s\n",USART_RX2_BUF);
				break;
			}
			//��ʾ��Ϣ
			case tips:
			{
				usart2_senddata("Wrong Command!!\n");
				current_tick=rt_tick_get();
				//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("�ȴ�ISM800C������ʱ��\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				usart2_senddata("1.Send \"report status\"to get sensor status.\n");
				current_tick=rt_tick_get();
				//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("�ȴ�ISM800C������ʱ��\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				usart2_senddata("2.Send \"out\"to set OUT mode.\n");
				current_tick=rt_tick_get();
				//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("�ȴ�ISM800C������ʱ��\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				usart2_senddata("3.Send \"home\"to set HOME mode.\n");
				//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
				current_tick=rt_tick_get();
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("�ȴ�ISM800C������ʱ��\n");
						rt_free(prt_backup);
						prt = RT_NULL;
						prt_backup= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("22USART_RX2_BUF��%s\n",USART_RX2_BUF);
				break;
			}
			default:break;
		}
		memset((uint8_t*)USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
	    printf("\n��ʼ������Ϣ\n" );
		//��ʼ����--0X1A
		usart2_senddata((char*)cmd_1A);
		//��ȡ������Ϣ
		memset((uint8_t*)USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
		//�ȴ�������Ϣ
		//while((USART_RX2_STA&0x8000)==0);//ʹ�ö�ʱ���������Ϣ����ʧ��(rt_timer_t),ԭ�����
		//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
		current_tick=rt_tick_get();
		while((USART_RX2_STA&0x8000)==0)
		{
			if((rt_tick_get()-current_tick)>100000)
			{
				rt_kprintf("�ȴ�ISM800C������ʱ��\n");
				rt_free(prt_backup);
				prt = RT_NULL;
				prt_backup= RT_NULL;
				return;
			}
		}
		printf("�ȴ�ʱ��Ϊ:%ldms\n",(rt_tick_get()-current_tick));
		USART_RX2_STA = 0;
		printf("11USART_RX2_BUF��%s\n",USART_RX2_BUF);
		//�ж��Ƿ��յ�������Ϣ
	    prt= rt_strstr((char*)USART_RX2_BUF,"+CMGS: ");
		printf("prt:%p\n",prt);
	    if(prt!=RT_NULL)
		   {
				printf("��Ϣ����ok\n");
			    //����ISM800Cģʽ���ڵȴ�������Ϣģʽ
				usart2_senddata(cmd_1A);
		   }
	   else
		   {
				printf("��Ϣ����ʧ��\n");
				usart2_senddata(cmd_1A); 
		   }
	}
	//û�����óɹ�������ʾ��Ϣ������
	else
	  {
		  printf("����ָ��û�еõ�ģ��������Ӧ\n");
		  usart2_senddata(cmd_1A);  
	  }
	//������ڴ���ͷ�
	  prt = prt_backup;
	  if(prt==RT_NULL)
	  {
		printf("GSM_SendSMSģ���ڴ���ͷ���Ч��\n");
	  }
	  else
	  {
		rt_free(prt);
		rt_kprintf("GSM_SendSMSģ���ڴ��:0x%p�Ѿ��ͷ�\n",prt);
		prt = RT_NULL;
	  }
	  memset((uint8_t*)USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
	 
}
#endif

 
