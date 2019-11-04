#include "public.h"
/****************************�汾˵��*************************
2019/07/19-00:07
ʵ����:
1.��������ʱ�ӣ�ϵͳ����ģʽ���ƹ�Ŀ��أ�
2.����ң�أ�
3.GSM��������ϵͳ����ģʽ���Ͷ�ȡϵͳ״̬��Ϣ���Լ�������Ϣ��
ʾ�����÷�����ʾ��
4.������ͨ������ң�غʹ�����ʵ��ͨ��NRF2401���߿��Ƶ��ת�����ӻ�����STMS208MBоƬ
PS:�����������޹��ϣ�ʵ��Ԥ�ڹ���

2019/07/21-09:50
�Ľ����ݣ�
1.GSM����߳��еȴ�ISM800C������ʽȡ����while((USART_RX2_STA&0x8000)==0);��ʽ
���û�ȡϵͳtick��ʽʵ�ֳ�ʱ�˳���
2.�����ʼ�����֣�ʹ�����Զ���ʼ��ʽ��ȡ���˺�����ʽ��ʼ����ʽ��
��INIT_BOARD_EXPORT()��INIT_APP_EXPORT()
���Ϲ��ܾ�����֤����ʹ�� 

2019/07/30-23:29
�Ľ����ݣ�
�����LWIP���繦�ܣ�����ʵ���ˣ�DHCP��ȡ��̬IP�Ĺ��ܣ�����PINGͨ��

2019/08/06
�������ݣ�
ʵ����webԶ�����úͲ�����ȡ
*************************************************************/



/****************************�û�����*************************/
//ϵͳģʽ
uint8_t sys_status = HOME;
uint8_t curtain_status = 1;
/**********************��������������*************************/
uint8_t temp,humi,len;
char temp_buffer[] = {0};
char humi_buffer[] = {0};
char MQ_buffer[]= {0};
char HZ_buffer[]= {0};
float mq = 0;
float LS1_VAL= 0;
float hz = 0;
//ú����������
float delta_mq = 0;
//NRF2401�߳���ر���
rt_uint32_t val = 0;
_Bool color_flag = 0;
//ú��й©�����־�����־λ
//_Bool Security_mq = 0;
//_Bool Security_invaid = 0;
/************************************************************/

/***************�̶߳�ջ�����ȼ���ʱ��Ƭ�궨��***************/
#define THREAD_STACK_SIZE 512
#define THREAD_PRIORITY 5
#define THREAD_TIMESLICE 500

/**********************�����߳̿��ƿ�************************/
static struct rt_thread Touch_Thread;
static struct rt_thread Light_Thread;
static struct rt_thread Security_Thread;
static struct rt_thread IRDA_Thread;
static struct rt_thread GSM_Thread;
static struct rt_thread GSM_Thread_Read;
static struct rt_thread GSM_Thread_Send;
static struct rt_thread Get_Sensor_Status_Thread;
static struct rt_thread NRF2401_Thread;
/******************************�����ź���********************/
//����һ���ź������ƿ�,����ú��ֵ�����밲���ٽ绥�����--delta_mq
rt_sem_t mq_sem = RT_NULL;
//����һ���ź������ƿ�,����GSM��Ϣͬ��
rt_sem_t GSM_sem_read = RT_NULL;
//��ʱ�����ƿ��������---���ж�ʱ�������GSM��Ϣ����ʧ��
//rt_timer_t timer1_sensor_Status = RT_NULL;
/******************************��������********************/

//����GSM_SendSMS�̵߳���Ϣ����
static struct rt_mailbox GSM_mq_send;
static char mq_pool[16];

//���ں���ң�ص�NRF����Ϣ����
static struct rt_mailbox IRDA_NRF2401_send;
static char IRDA_NRF2401_pool[16];

/******************************�������������߳�*****************/
//���ڽ������Դ������ͺ���ң�ط�����������Ϣ
//������Ϣ�����ݽ���Ӧ��ָ��ͨ��������NRF2401���͵��ӻ���NRF2401��ִ����Ӧ�Ĳ���
ALIGN(RT_ALIGN_SIZE)
static char NRF2401_Thread_stack[512];
//NRF2401�߳� 
static void NRF2401_thread(void *parameter)
{
	//����Ϊ����ģʽ
	NRF24L01_TX_Mode();
	while(1)
	{
		//�ȴ������źţ�û���յ��̹߳���֪���������źŹ����Ż��Ѹ��߳�
		rt_mb_recv(&IRDA_NRF2401_send,&val,RT_WAITING_FOREVER);
		{
			    RED_LED = !RED_LED;
			    rt_enter_critical();
			    //���ݺ���ң�ص�ָ����в���
				switch((uint8_t)val)
				{
					    //����left,����OPEN
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
								//��ֹK/T����ɫ�ڰ���LEFT��right֮���Ǻ�ɫ�ģ����ǵ����ֹͣ������
								color_flag=0; 
								POINT_COLOR = BLACK;
								LCD_ShowString(x5_curtain_kt,290,200,16,16,"K/T");
								curtain_status = 1;
							}
							else
							printf("OPEN left sent failed!\n");
							break;
						}
						//����right,����CLOSE
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
						//����stop,����K/T
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
//GSMͨ���̶߳�ջ--��
ALIGN(RT_ALIGN_SIZE)
static char GSM_send_thread_stack[1024];
//GSM��Ϣ�����߳�---ר�����ڷ�����Ϣ���������䴫�ݹ�����ָ�����ѡ���Եķ����û�Ҫ����Ϣ 
static void GSM_send_thread(void *parameter)
{
	rt_uint32_t val = 0;
	while(1)
	{
		//�ȴ�������Ϣ�ź�����û�յ��߳̾͹���
		if(rt_mb_recv(&GSM_mq_send,&val,RT_WAITING_FOREVER)==RT_EOK)
		{
			printf("val:%ld\n",val); 
			rt_enter_critical(); //��ֹϵͳ����,�����ֹ���ȷ��򽫵�����Ϣ����ʧ��
			GSM_SendSMS((uint8_t)val); 
			rt_exit_critical();  //�ָ�ϵͳ����
		}
		rt_thread_mdelay(200);//����Ϊ200���޷��������ն��Ų�ִ��GSM_SendSMS����ֹϵͳ���Ⱥ�OK
	}
}

//GSMͨ���̶߳�ջ--��
ALIGN(RT_ALIGN_SIZE)
static char GSM_read_thread_stack[1024];
//GSM��ȡ��Ϣ�߳�---ר�Ŷ�ȡSMS��Ϣ�����ж��Ƿ���ָ����ָ��������ͨ���������
//����ָ�GSM��Ϣ�����߳�
static void GSM_read_thread(void *parameter)
{
	rt_tick_t current_tick=0;
	static char*pt = RT_NULL;
	//point���ڱ����pt�����ڴ�õ����ڴ���׵�ַ����������ͷ���
	static char*point = RT_NULL; 
	char GSM_buffer[] = {0};
	while(1)
	{
		//�ȴ���ȡ��Ϣ�ź���
	  if(rt_sem_take(GSM_sem_read,RT_WAITING_FOREVER)==RT_EOK)
	  {
		//��ӡ�������ISM800C��ԭʼ����
		printf("��ISM800C��ԭʼ���ݣ�%s\n",USART_RX2_BUF);
		//����256�ֽڵ��ڴ�ռ�
		pt = rt_malloc(256);
		point = pt;//������ĵ����ڴ���׵�ַ��ŵ�point�У�������ͷ�ʱ�õ�
				   //��һ������Ҫ����Ϊ����Ĵ���Ҫ�õ�strstr�������ú�����ı�
				   //pt��ֵ������ͷ�ʱ�ĵ�ַ������ʱ�õ��ĵ�ַ��ͬ�����ջᵼ��
				   //�ڴ���ͷ�ʧ�ܣ��Ӷ����¶��Դ���
		if(pt==RT_NULL)
		{
			rt_kprintf("try to get 256 byte memory failed!\n" );
           return;
	     }
		loop:
		printf("pt:0x%p\n",pt); 
		pt = rt_strstr((char*)USART_RX2_BUF,"+CMTI:");
		printf("pt:%s\n",pt);
		//�ж��Ƿ��յ����ţ�����ǽ�����Ŵ������
		 if(pt!=RT_NULL)
		 {
			//��ȡ����Ϣ��������ŵ�GSM_buffer
			sprintf(GSM_buffer,"%s",pt+12);
			printf("GSM_buffer:%s\n",GSM_buffer); 
			//��Ϣ��������99����ȫ��ɾ���������޷����ն���
			if(rt_strcmp(GSM_buffer,"99")>=0)
			{
				usart2_senddata("AT+CMGD=1,4\n"); 
				//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
				current_tick=rt_tick_get();
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>100000)
					{
						rt_kprintf("ISM800C�����ã�\n");
						rt_free(point);
						pt = RT_NULL;
						point= RT_NULL;
						return;
					}
				}
				USART_RX2_STA=0;
				printf("All messages have been deleted!\n");
				//��ת���ж��Ƿ��յ�����
				goto loop;
			}
			else
			 { 
				//��ȡ��������
				USART_RX2_STA=0;
				sprintf(GSM_buffer,"AT+CMGR=%s\n\n",pt+12);
				printf("%s\n",GSM_buffer);
				memset((uint8_t*)USART_RX2_BUF,0,sizeof(USART_RX2_BUF));
				rt_kprintf("���Ͷ���Ϣ����\n");
				usart2_senddata(GSM_buffer); 
				//�ȴ�����ISM800C�ķ���
				//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
				current_tick=rt_tick_get();
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>50000)
					{
						rt_kprintf("ISM800C�����ã�\n");
						rt_free(point);
						pt = RT_NULL;
						point= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("��һ�ζ���%s\n",USART_RX2_BUF);	
                memset((uint8_t*)USART_RX2_BUF,0,sizeof(USART_RX2_BUF));				 
				usart2_senddata(GSM_buffer);	
				//�ȴ�����ISM800C�ķ���
				//���û�ȡϵͳtick��ʽ�ж��Ƿ�ʱ������������ѭ�� 
				current_tick=rt_tick_get();
				while((USART_RX2_STA&0x8000)==0)
				{
					if((rt_tick_get()-current_tick)>50000)
					{
						rt_kprintf("ISM800C�����ã�\n");
						rt_free(point);
						pt = RT_NULL;
						point= RT_NULL;
						return;
					}
				}
				USART_RX2_STA = 0;
				printf("�ڶ��ζ���%s\n",USART_RX2_BUF);
				rt_kprintf("�ж��Ƿ��յ�report status\n");
				pt = rt_strstr((char*)USART_RX2_BUF,"report status");
				printf("pt:%p\n",pt);
				rt_kprintf("�ж��Ƿ�Ҫ����ϵͳ״̬\n");
				if(pt!=RT_NULL)
				{
					//ͨ�����Ų鿴ϵͳ״̬ 
					//��������
					rt_kprintf("�����ʼ�����ϵͳ״̬\n");
					//GSM_SendSMS(report_status); 
					rt_mb_send(&GSM_mq_send,report_status);
					goto LOOP;
				}
				rt_kprintf("�ж��Ƿ��յ�home\n");
				pt =  rt_strstr((char*)USART_RX2_BUF,"home"); 
				if(pt!=RT_NULL)
				{
					//ͨ���������ûؼ�ģʽ
					rt_enter_critical();//����ϵͳ����ģʽʱ����ֹϵͳ����
					sys_status = HOME;
					POINT_COLOR =GREEN;
					LCD_ShowString(lcddev.width-95,58,200,24,24," Home ");
					BEEP = 0;			//��ֹ�ڷ�����Ϣʱ������һֱ��
			        RED_LED   = 1;
					//���ͷ�����Ϣ���û���ʾ�Ƿ����óɹ�
					if(sys_status == HOME)
						rt_mb_send(&GSM_mq_send,set_ok);//��������״̬���û�
					else
						rt_mb_send(&GSM_mq_send,set_Err);
					rt_exit_critical(); //����ϵͳ����
					goto LOOP;
				}
			   rt_kprintf("�ж��Ƿ��յ�out\n");
			   pt =  rt_strstr((char*)USART_RX2_BUF,"out"); 
			   if(pt!=RT_NULL)
			   {
				    //ͨ�������������ģʽ
				    rt_enter_critical();//����ϵͳ����ģʽʱ����ֹϵͳ����
					sys_status = OUT;
					POINT_COLOR =RED;
					LCD_ShowString(lcddev.width-95,58,200,24,24," OUT ");
				    BEEP = 0;			//��ֹ�ڷ�����Ϣʱ������һֱ��
			        RED_LED   = 1;
				    //���ͷ�����Ϣ���û���ʾ�Ƿ����óɹ�
					if(sys_status == OUT)
						rt_mb_send(&GSM_mq_send,set_ok); //��������״̬
					else
						rt_mb_send(&GSM_mq_send,set_Err);
				    rt_exit_critical();  //����ϵͳ����
					goto LOOP;
			   }
			   //û���յ���ȷָ��ͷ�����ʾ��Ϣ
			   if(pt==RT_NULL)
			   {
					rt_kprintf("û���յ�SMS����\n");
				    rt_mb_send(&GSM_mq_send,tips);
			   }
			 }
		 }
LOOP:
		 memset((uint8_t*)USART_RX2_BUF,0,sizeof(USART_RX2_BUF));	
		 //�ڴ�ռ�ʹ�����Ҫ����rt_free(pt)���ͷ�����Ķѿռ�
         pt  =  point;//������ʱ�õ��ĵ�ַ����д��pt.		 
		 if(pt!=RT_NULL)//�ͷ�ǰ�ж��Ƿ�Ҫ�ͷŵ��ڴ�ռ��Ƿ����
		 {
			 printf("pt:0x%p\n",pt); 
			 rt_free(pt);//���ھ��ͷţ�û�о�ֱ������
			 rt_kprintf("�ڴ��:0x%p�Ѿ��ͷ�\n",pt);
			 pt = RT_NULL;
		 }
		 else
		 {
			rt_kprintf("û����Ҫ�ͷ��ڴ��pt\n");
		 }
		 
     }
	  rt_thread_mdelay(10);
  }
}
//GSMͨ���̶߳�ջ
ALIGN(RT_ALIGN_SIZE)
static char GSM_thread_stack[256];
//GSMͨ���߳�---ר�������ж��Ƿ���յ�GSM��Ϣ---�����յ���Ϣ���ͷ��ź���(��ֵ�ź���)������Ϣ��ȡ�߳� 
static void GSM_thread(void *parameter)
{
	while(1)
	{
		//�ж��Ƿ��յ�����
		if((USART_RX2_STA&0x8000))
		{
	       USART_RX2_STA=0;
		   //�յ��������ͷ��ź���GSM_sem_read���Ի����߳�GSM_read_thread
           rt_sem_release(GSM_sem_read);			
	     }
		rt_thread_mdelay(10);
    }
}
//�����̶߳�ջ
ALIGN(RT_ALIGN_SIZE)
static char IRDA_thread_stack[256];
//�����߳� 
static void IRDA_thread(void *parameter)
{
	//����ң�ز���
    uint8_t irda_value = 0;
	while(1)
	{
		irda_value = Remote_Scan();
		    if(irda_value!=0)
			{
				while(irda_value==Remote_Scan());//�ȴ������ɿ�
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
//�����̶߳�ջ
ALIGN(RT_ALIGN_SIZE)
static char Security_thread_stack[512];
static void security_thread(void *parameter)
{
	while(1)
	{
		//�ж��Ƿ�Ϊ���ģʽ
		if(sys_status==OUT)
		{
			//�ж��Ƿ����˴���
			if(RSD_STATUS|WB_STATUS)
			{
				rt_mb_send(&GSM_mq_send,Invaiding);
				BEEP = 1;//�������ⱨ��
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
		//�ж�ú�����
		if((mq<=3.4)&&(mq>=0.3))
		{
			rt_mb_send(&GSM_mq_send,MQ_LEAKING);//ͨ�����ŷ���ú��й©��Ϣ���û�
			BEEP = 1;   //�������ⱨ��
			RED_LED   = 0;
			rt_thread_mdelay(150);
			BEEP = 0;
			RED_LED   = 1;
		}
		//�жϻ������
		if((hz<=1.0)&&(hz!=0))
		{
			printf("hz= %f\n",hz);
			rt_mb_send(&GSM_mq_send,On_Fire);//ͨ�����ŷ��ͻ�����Ϣ���û�
			BEEP = 1;   //�������ⱨ��
			RED_LED   = 0;
			rt_thread_mdelay(150);
			BEEP = 0;
			RED_LED   = 1;
		}
		rt_thread_mdelay(50);
	}
}

//�����̶߳�ջ
ALIGN(RT_ALIGN_SIZE)
static char touch_thread_stack[512];
/******************�����߳�****************/
static void touch_thread(void *parameter)
{
	
	while(1)
	  {
		//������ɨ��
		tp_dev.scan(0); 	
        //�жϴ�����������		  
		if(tp_dev.sta&TP_PRES_DOWN)			
		{	
		 	if(tp_dev.x[0]<lcddev.width&&tp_dev.y[0]<lcddev.height) //��ⴥ����Χ�Ƿ���Һ����ʾ�ߴ�֮��
			{	
				//printf("X:%d\n",tp_dev.x[0]);//ͨ����ȡX,Y��ֵȷ��ÿ������������Ȼ���ж�
				//printf("Y:%d\n",tp_dev.y[0]);//�����µĵ��Ƿ��ڸ��������ж�ĳ�����ܰ�ť�Ƿ񱻰���
				//�ж�time�Ƿ񱻰���  
				if(TIME)
				{
					rt_enter_critical();			//��ֹϵͳ����
					HAL_NVIC_DisableIRQ(RTC_IRQn);	//����ǰ��ֹRTC�ж�					
					Set_Time();	       				//����ʱ������	
					rt_exit_critical();				//�ָ�ϵͳ����
				}
				//�ж�MODE���Ƿ񱻰���
				if(MODE)
				{
					
					//�л�ϵͳ����ģʽ
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
				//�ж�Bedroom_Light���Ƿ񱻰���
				if(Bedroom_Light)
				{
					Light_Control(1);
				}	
				//�ж�Livingroom_Light���Ƿ񱻰���
				if(Livingroom_Light)
				{
					Light_Control(2);
				}
				//�ж�Corridor_Light���Ƿ񱻰���
				if(Corridor_Light)
				{
					Light_Control(3);
				}
				//�ж�Curtain_CLOSE���Ƿ񱻰���
				if(Curtain_CLOSE)
				{
					 rt_mb_send(&IRDA_NRF2401_send,right); 
				}
				//�ж�Curtain_OPEN���Ƿ񱻰���
				if(Curtain_OPEN)
				{
					rt_mb_send(&IRDA_NRF2401_send,left);
				}
				//�ж�Curtain_KT���Ƿ񱻰���
				if(Curtain_KT)
				{
					rt_mb_send(&IRDA_NRF2401_send,stop);
				}
				//������������  
				while(tp_dev.sta&TP_PRES_DOWN)	
				{
					tp_dev.scan(0); 
				}
			}
		}
		rt_thread_mdelay(5);
	}
}

//�����̶߳�ջ
ALIGN(RT_ALIGN_SIZE)
static char Light_thread_stack[356];
/******************�����߳�****************/
static void Corridor_Light_thread(void *parameter)
{
	while(1)
	{
		//�ڼ�ģʽʱ
		if(sys_status==HOME)
		{
			//�ж��Ƿ���ҹ��
			if(get_LS1_Status(5,LS1))
			{
				//��ҹ�����ж��Ƿ����˾������ȵ����͵紫������΢��������
				if(RSD_STATUS|WB_STATUS)
				{
					rt_enter_critical();//��ֹ��ɫ��ʾ���̱����
					POINT_COLOR =RED;
					CORRIDOR_LIGHT = 0; //�����ȵ�
					//�Ƶ�״̬ͬ����LCD
					LCD_ShowString(0,272,200,16,16,"Corridor   Light:     ON ");
					rt_exit_critical(); 
					//��������ʱһ��ʱ��
					rt_thread_mdelay(8500);
					//��ֹ��ɫ��ʾ���̱����
					rt_enter_critical();
					//�ٹر�
					CORRIDOR_LIGHT = 1;
					POINT_COLOR =BLACK;
					LCD_ShowString(0,272,200,16,16,"Corridor   Light:     OFF");
					rt_exit_critical(); 
				}
			}
		}
		//���ģʽʱ���ر����ȵ�
		else if(sys_status==OUT)
		{
			//��ֹ��ɫ��ʾ���̱����
			rt_enter_critical();
			POINT_COLOR =BLACK;
			//�Ƶ�״̬ͬ����LCD
		    LCD_ShowString(0,272,200,16,16,"Corridor   Light:     OFF");
			rt_exit_critical(); 
		}
		rt_thread_mdelay(10);
	}
}


//���ڻ�ȡ������״̬
ALIGN(RT_ALIGN_SIZE)
static char Get_Sensor_Status_Thread_stack[1024];
//ÿ��200�������һ�� 
static void Get_Sensor_Status_thread(void *parameter)
{
	while(1)
	{
	  //��ȡ����������ֵ
	  LS1_VAL = get_aver_val(5,LS1);
	  //��ȡ��ʪ��ֵ
	  DHT11_Read_Data(&temp,&humi);
	  //���¶�ֵת��Ϊ�ַ���
	  sprintf(temp_buffer,"%d'C",temp);
	  //�ж��¶��Ƿ���23~27��֮�䣬��������ɫ��ʾ�¶�ֵ�������Ժ�ɫ��ʾ
	  if((temp>=23)&&(temp<=27))
	    POINT_COLOR =GREEN;
	  else
		POINT_COLOR =RED;
	  LCD_ShowString(105,124,200,16,16,temp_buffer);//���µ�LCD
	  //��ʪ��ֵת��Ϊ�ַ���
	  sprintf(humi_buffer,"%d%%",humi);
	   //�ж�ʪ���Ƿ���45%~65%��֮�䣬��������ɫ��ʾ�¶�ֵ�������Ժ�ɫ��ʾ
	  if((humi>=45)&&(humi<=65))
	    POINT_COLOR =GREEN;
	  else
		POINT_COLOR =RED;
	  LCD_ShowString(105,142,200,16,16,humi_buffer);//���µ�LCD
     
	  //��ȡú��ֵ
	  mq = get_aver_val(5,MQ);
	  sprintf(MQ_buffer,"%d.%.2d",(uint8_t)mq,(uint8_t)Get_decimal(mq,2));
	  //��ú��ֵ����0.4�Ժ�ɫ��ʾ����������ɫ��ʾ
	  if(mq>=0.3)
	    POINT_COLOR =RED;
	  else
		POINT_COLOR =GREEN;
	  LCD_ShowString(105,176,200,16,16,MQ_buffer);//���µ�LCD
	  
	  //���ִ�����ֵ
	  hz = get_aver_val(15,HZ);
	  //printf("hz = %f\n",hz);
	  sprintf(HZ_buffer,"%d.%.2d",(uint8_t)hz,(uint8_t)Get_decimal(hz,2));
	  //������ֵ����1����ɫ��ʾ�������Ժ�ɫ��ʾ
	  if(hz>=1.0)
	    POINT_COLOR =GREEN;
	  else
		POINT_COLOR =RED;
	  LCD_ShowString(105,158,200,16,16,HZ_buffer); //���µ�LCD
	  rt_thread_mdelay(200);                       //200ms����һ��
     }
}


//����ʵ��
int Smart_IOT_Center(void)
{
	rt_err_t result = 0;
	//��ʾϵͳģʽ
	POINT_COLOR =GREEN;
	LCD_ShowString(lcddev.width-95,58,200,24,24," Home ");
	//����һ��ú��ֵ����ٽ�����ź���--���ź���û��ʹ�õ��������κ�����GSM_Thread_stack is close to the bottom���ԣ�ԭ�����
	mq_sem = rt_sem_create("mq_sem",1,RT_IPC_FLAG_FIFO);
	if(mq_sem!=RT_NULL)
		printf("Successfully Create mq_sem!\n");
	else
		printf("Failed to Create mq_sem!\n");
#if 1
	//����һ��GSM�ź���--��
	GSM_sem_read = rt_sem_create("GSM_sem_read",0,RT_IPC_FLAG_FIFO);
	if(GSM_sem_read!=RT_NULL)
		printf("Successfully Create GSM_sem_read!\n");
	else
		printf("Failed to Create GSM_sem_read!\n");
	
	//����һ����������--��
	result = rt_mb_init( &IRDA_NRF2401_send,
						 "IRDA_NRF2401_send",
	                      &IRDA_NRF2401_pool[0],
						  sizeof(IRDA_NRF2401_pool)/4,
						  RT_IPC_FLAG_FIFO);
	if(result==RT_EOK)
		printf("Successfully Create IRDA_NRF2401_send!\n");
	else
		printf("Failed to Create IRDA_NRF2401_send!\n");
	
	//����һ��GSM����--��
	result = rt_mb_init( &GSM_mq_send,
						 "GSM_mb_send",
	                      &mq_pool[0],
						  sizeof(mq_pool)/4,
						  RT_IPC_FLAG_FIFO);
	if(result==RT_EOK)
		printf("Successfully Create GSM_mq_send!\n");
	else
		printf("Failed to Create GSM_mq_send!\n");
	
	//����IRDA_NRF2401�߳� 
	result = rt_thread_init(&NRF2401_Thread,
						   "NRF2401_Thread",
						    NRF2401_thread,
	                        RT_NULL,
						    &NRF2401_Thread_stack[0],
						    sizeof(NRF2401_Thread_stack),
						    THREAD_PRIORITY+1,
						    THREAD_TIMESLICE  	
						  );

	//����IRDA_NRF2401�߳� 
	if(result==RT_EOK)
	rt_thread_startup(&NRF2401_Thread);
	

#endif
    											
    //���������߳�
    result = rt_thread_init( &Touch_Thread,
							"touch_thread",
							 touch_thread,
							 RT_NULL,
							 &touch_thread_stack[0],
							 sizeof(touch_thread_stack),
							 5,
							 THREAD_TIMESLICE  	
							);
	//���������߳� 
	if(result==RT_EOK)
	rt_thread_startup(&Touch_Thread);
	
	//���������߳�
	result = rt_thread_init( &Light_Thread,
							"Light_Thread",
							 Corridor_Light_thread,
							 RT_NULL,
							 &Light_thread_stack[0],
							 sizeof(Light_thread_stack),
							 THREAD_PRIORITY+7,
							 THREAD_TIMESLICE  	
							);
	//���������߳�
	if(result==RT_EOK)
	rt_thread_startup(&Light_Thread);
	
    //����������״̬��ȡ�߳�
    result = rt_thread_init( &Get_Sensor_Status_Thread,
							"Get_Sensor_Status_Thread",
							 Get_Sensor_Status_thread,
							 RT_NULL,
							 &Get_Sensor_Status_Thread_stack[0],
							 sizeof(Get_Sensor_Status_Thread_stack),
							 THREAD_PRIORITY-2,
							 THREAD_TIMESLICE  	
							);
	//������״̬��ȡ�߳�
	if(result==RT_EOK)
	rt_thread_startup(&Get_Sensor_Status_Thread);	
	
	
	//�����������߳�
    result = rt_thread_init( &IRDA_Thread,
							"IRDA_Thread",
							 IRDA_thread,
							 RT_NULL,
							 &IRDA_thread_stack[0],
							 sizeof(IRDA_thread_stack),
							 THREAD_PRIORITY-1,
							 THREAD_TIMESLICE  	
							);
	//�����������߳� 
	if(result==RT_EOK)
	rt_thread_startup(&IRDA_Thread);
 	
	//����GSMͨ���߳�
    result = rt_thread_init( &GSM_Thread,
							"GSM_Thread",
							 GSM_thread,
							 RT_NULL,
							 &GSM_thread_stack[0],
							 sizeof(GSM_thread_stack),
							 THREAD_PRIORITY+9,
							 THREAD_TIMESLICE  	
							);
	//����GSMͨ���߳� 
	if(result==RT_EOK)
	rt_thread_startup(&GSM_Thread);
	
	//����GSM��ȡ��Ϣ�߳�
    result = rt_thread_init( &GSM_Thread_Read,
							"GSM_Thread_Read",
							 GSM_read_thread,
							 RT_NULL,
							 &GSM_read_thread_stack[0],
							 sizeof(GSM_read_thread_stack),
							 THREAD_PRIORITY+3,
							 THREAD_TIMESLICE  	
							);
	//����GSM��ȡ��Ϣ�߳�
	if(result==RT_EOK)
	rt_thread_startup(&GSM_Thread_Read);
	
	//����GSM������Ϣ�߳�
    result = rt_thread_init( &GSM_Thread_Send,
							"GSM_Thread_Send",
							 GSM_send_thread,
							 RT_NULL,
							 &GSM_send_thread_stack[0],
							 sizeof(GSM_send_thread_stack),
							 THREAD_PRIORITY+11,
							 THREAD_TIMESLICE  	
							);
	//����GSM��ȡ��Ϣ�߳�
	if(result==RT_EOK)
	rt_thread_startup(&GSM_Thread_Send);
	//���������߳�
    result = rt_thread_init( &Security_Thread,
							"security_thread",
							 security_thread,
							 RT_NULL,
							 &Security_thread_stack[0],
							 sizeof(Security_thread_stack),
							 THREAD_PRIORITY+14,
							 THREAD_TIMESLICE  	
							);
	//���������߳� 
	if(result==RT_EOK)
	rt_thread_startup(&Security_Thread);
	
	return 0;
}

//������
int main(void)
{
	#if 1
   //��ʼ��GSM
   scm800c_init();
   //lwip��ʼ��
   //ע�⣺NRF2401���ж����ſ��ܿ��ܻ���������ʼ��ʧ��
   if(lwip_comm_init()==0)
	   rt_kprintf("�����ʼ���ɹ�\n");
   else
	   rt_kprintf("�����ʼ��ʧ�ܣ�\n");
   //Web Serverģʽ
   httpd_init();  			
   //ʵ����ʼ��������	
   Smart_IOT_Center();
	#endif
  
}
