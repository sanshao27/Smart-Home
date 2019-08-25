#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include "fs.h"
#include "lwip_comm.h"
#include "bsp_led.h"
#include "beep.h"
#include "bsp_adc.h"
#include "rtc.h" 
#include "lcd.h"
#include <string.h>
#include <stdlib.h>
extern uint8_t temp,humi;  
extern uint8_t sys_status; 
extern float LS1_VAL;
extern float hz;
//CGI��URI����
#define NUM_CONFIG_CGI_URIS	3  
//SSI��TAG����
#define NUM_CONFIG_SSI_TAGS	7 

//����LED,BEEP��ģʽ��CGI handler
const char* LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* BEEP_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* MODE_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* CURTAIN_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
//SSI��Tag
static const char *ppcTAGs[]=  
{
	"t",          //�¶�ֵ
	"h",          //ʪ��ֵ
	"f",		  //�𾯴�����ֵ
	"m",		  //ú��������ֵ
	"l",          //����������ֵ
	"s", 		  //ʱ��
	"y"  		  //����
};

//cgi����
static const tCGI ppcURLs[]= 
{
	{"/leds.cgi",LEDS_CGI_Handler},
	{"/beep.cgi",BEEP_CGI_Handler},
	{"/mode.cgi",MODE_CGI_Handler},
	//{"/status_update.cgi,STATUS_UPADATE_Handler"},
};


//��web�ͻ��������������ʱ��,ʹ�ô˺�����CGI handler����
static int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
	int iLoop;
	for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
	{
		if(strcmp(pcToFind,pcParam[iLoop]) == 0)
		{
			return (iLoop); //����iLOOP
		}
	}
	return (-1);
}


//SSIHandler����Ҫ�õ��Ĵ����ڲ��¶ȴ������ĺ���
void Temperate_Handler(char *pcInsert)
{
	char Digit1=0, Digit2=0; 
    Digit1 = temp/10;
    Digit2 = temp% 10;
	//��ӵ�html�е�����
	*pcInsert 	  = (char)(Digit1+0x30);
	*(pcInsert+1) = (char)(Digit2+0x30);
}

//SSIHandler����Ҫ�õ��Ĵ���RTCʱ��ĺ���
void RTCTime_Handler(char *pcInsert)
{
	uint8_t hour,min,sec;
	hour = calendar.hour;
	min  = calendar.min;
	sec  = calendar.sec;
	*pcInsert = 	(char)((hour/10) + 0x30);
	*(pcInsert+1) = (char)((hour%10) + 0x30);
	*(pcInsert+2) = ':';
	*(pcInsert+3) = (char)((min/10) + 0x30);
	*(pcInsert+4) = (char)((min%10) + 0x30);
	*(pcInsert+5) = ':';
	*(pcInsert+6) = (char)((sec/10) + 0x30);
	*(pcInsert+7) = (char)((sec%10) + 0x30);
}

//SSIHandler����Ҫ�õ��Ĵ���RTC���ڵĺ���
void RTCdate_Handler(char *pcInsert)
{
	uint16_t year,month,date,week;
	year  = calendar.w_year;
	month = calendar.w_month;
	date  = calendar.w_date;
	week  = calendar.week;
	*pcInsert = '2';
	*(pcInsert+1) = '0';
	*(pcInsert+2) = (char)(((year%100)/10) + 0x30);
	*(pcInsert+3) = (char)((year%10) + 0x30);
	*(pcInsert+4) = '-';
	*(pcInsert+5) = (char)((month/10) + 0x30);
	*(pcInsert+6) = (char)((month%10) + 0x30);
	*(pcInsert+7) = '-';
	*(pcInsert+8) = (char)((date/10) + 0x30);
	*(pcInsert+9) = (char)((date%10) + 0x30);
	*(pcInsert+10) = ' ';
	*(pcInsert+11) = 'w';
	*(pcInsert+12) = 'e';
	*(pcInsert+13) = 'e';
	*(pcInsert+14) = 'k';
	*(pcInsert+15) = ':';
	*(pcInsert+16) = (char)(week + 0x30);
	
}


//SSIHandler����Ҫ�õ��Ĵ���ʪ�ȵĺ���
void Humidity_Handler(char *pcInsert)
{
    char Digit1=0, Digit2=0; 
    Digit1 = humi/10;
    Digit2 = humi% 10;
	//��ӵ�html�е�����
	*pcInsert 	  = (char)(Digit1+0x30);
	*(pcInsert+1) = (char)(Digit2+0x30);
}

//SSIHandler����Ҫ�õ��Ĵ���𾯴������ĺ���
void Fire_Handler(char *pcInsert)
{
	 char Digit1=0, Digit2=0,Digit3=0; 
	 uint16_t FIRE = 0;
	 FIRE = get_aver_val(5,HZ)*100;
	if(FIRE<10)
	{
		Digit3 = FIRE;
		goto __loop;
	}
	if(FIRE<100)
	{
		Digit2 = FIRE/10;
	    Digit3 = FIRE%10;
		goto __loop;
	}
	if(FIRE>100)
	{
       Digit1 = FIRE/100;
	   Digit2 = FIRE%100/10;
	   Digit3 = FIRE%10;
	}
	__loop:    
	//��ӵ�html�е�����
	*pcInsert 	  = (char)(Digit1+0x30);
	*(pcInsert+1) = '.';
	*(pcInsert+2) = (char)(Digit2+0x30);
	*(pcInsert+3) = (char)(Digit3+0x30);
}

//SSIHandler����Ҫ�õ��Ĵ���ú���������ĺ���
void MQ_Handler(char *pcInsert)
{
	uint16_t Mq = 0;
	char Digit1=0, Digit2=0,Digit3=0; 
	Mq = get_aver_val(1,MQ)*100;
	if(Mq<10)
	{
		Digit3 = Mq;
		goto __loop;
	}
	if(Mq<100)
	{
		Digit2 = Mq/10;
	    Digit3 = Mq%10;
		goto __loop;
	}
	if(Mq>100)
	{
       Digit1 = Mq/100;
	   Digit2 = Mq%100/10;
	   Digit3 = Mq%10;
	}
	__loop:    
	//��ӵ�html�е�����
	*pcInsert 	  = (char)(Digit1+0x30);
	*(pcInsert+1) = '.';
	*(pcInsert+2) = (char)(Digit2+0x30);
	*(pcInsert+3) = (char)(Digit3+0x30);
}

//SSIHandler����Ҫ�õ��Ĺ����������ĺ���
void Light_sensor_Handler(char *pcInsert)
{
	uint16_t ls = 0;
	char Digit1=0, Digit2=0,Digit3=0; 
	ls= (uint16_t)(LS1_VAL*100);
	if(ls<10)
	{
		Digit3 = ls;
		goto __loop;
	}
	if(ls<100)
	{
		Digit2 = ls/10;
	    Digit3 = ls%10;
		goto __loop;
	}
	if(ls>100)
	{
		
       Digit1 = ls/100;
	   Digit2 = ls%100/10;
	   Digit3 = ls%10;
	}
	__loop:    
	//��ӵ�html�е�����
	*pcInsert 	  = (char)(Digit1+0x30);
	*(pcInsert+1) = '.';
	*(pcInsert+2) = (char)(Digit2+0x30);
	*(pcInsert+3) = (char)(Digit3+0x30);
}
//SSI��Handler���
static uint16_t SSIHandler(int iIndex,char *pcInsert,int iInsertLen)
{	 
	switch(iIndex)
	{
		case 0:  //�¶�ֵ
		       {
				 Temperate_Handler(pcInsert);
				 break;
			   }
		case 1: //ʪ��ֵ 
		       {
				 Humidity_Handler(pcInsert);
				 break;
			   }			   
		case 2: //�𾯴�����
			  {
				Fire_Handler(pcInsert);
				break;
			  }
		case 3: //ú��������
			 {	
				MQ_Handler(pcInsert); 
				break;
			 }
		case 4://���մ�����
			 {	
				Light_sensor_Handler(pcInsert); 
				break;
			 }
        case 5://ʱ��
			 {	
				RTCTime_Handler(pcInsert); 
				break;
			 }
	   case 6://����
			 {	
				RTCdate_Handler(pcInsert); 
				break;
			 }
	}
	return strlen(pcInsert);
}



//CGI LED���ƾ��
const char* LEDS_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
	uint8_t i=0;  //ע������Լ���GET�Ĳ����Ķ�����ѡ��iֵ��Χ
	iIndex = FindCGIParameter("light",pcParam,iNumParams);  //�ҵ�led��������
	//ֻ��һ��CGI��� iIndex=0
	if (iIndex != -1)
	{ 
		for (i=0; i<iNumParams; i++) //���CGI����
		{
		  //������"Bedroom_light" ���ڿ���Bedroom_light�Ƶ�
		  if (strcmp(pcParam[i] , "light")==0)  
		  {
			if(strcmp(pcValue[i], "Bedroom_lightON") ==0)  //�ı�LED1״̬
			{
				 RED_LED =0; //��Bedroom_light
				 POINT_COLOR =RED;
				 LCD_ShowString(0,236,200,16,16,"Bedroom    Light:     ON ");
			}
			else if(strcmp(pcValue[i],"Bedroom_lightOFF") == 0)
			{
				 RED_LED=1; //�ر�Bedroom_light
				 POINT_COLOR =BLACK;
				 LCD_ShowString(0,236,200,16,16,"Bedroom    Light:     OFF");
			}
		  }
		 
		   //������"Corridor_light" ���ڿ���Corridor_light�Ƶ�
		  if (strcmp(pcParam[i] , "light")==0)  
		  {
			if(strcmp(pcValue[i], "Corridor_lightON") ==0) 
			{				
				GREEN_LED =0; //��Corridor_light
				POINT_COLOR =RED;
				LCD_ShowString(0,272,200,16,16,"Corridor   Light:     ON ");
			}
			else if(strcmp(pcValue[i],"Corridor_lightOFF") == 0)
			{
				GREEN_LED=1; //�ر�Corridor_light
				POINT_COLOR =BLACK;
				LCD_ShowString(0,272,200,16,16,"Corridor   Light:     OFF");
			}
		  }
		}
	 }
	//ˢ����ҳ
	//alloff
	if(sys_status==OUT&&BEEP==0&&RED_LED==1&&GREEN_LED==1)
		return "/alloff.shtml";
	//allon
	else if(sys_status==HOME&&BEEP==1&&RED_LED==0&&GREEN_LED==0)
		return "/allon.shtml";
	//bedroomon
	else if(sys_status==OUT&&BEEP==0&&RED_LED==0&&GREEN_LED==1)
		return "/bedroomon.shtml";
	//bedroomon_corridoron
	else if(sys_status==OUT&&BEEP==0&&RED_LED==0&&GREEN_LED==0)
		return "/bedroomon_corridoron.shtml";
	//beepon
	else if(sys_status==OUT&&BEEP==1&&RED_LED==1&&GREEN_LED==1)
		return "/beepon.shtml";
	//beepon_Bedroomon
	else if(sys_status==OUT&&BEEP==1&&RED_LED==0&&GREEN_LED==1)
		return "/beepon_Bedroomon.shtml";
	//beepon_bedroomon_corridoron
	else if(sys_status==OUT&&BEEP==1&&RED_LED==0&&GREEN_LED==0)
		return "/beepon_bedroomon_corridoron.shtml";
	//beepon_corridoron
	else if(sys_status==OUT&&BEEP==1&&RED_LED==1&&GREEN_LED==0)
		return "/beepon_corridoron.shtml";
	//corridoron
	else if(sys_status==OUT&&BEEP==0&&RED_LED==1&&GREEN_LED==0)
		return "/corridoron.shtml";
	//modehome
	else if(sys_status==HOME&&BEEP==0&&RED_LED==1&&GREEN_LED==1)
		return "/modehome.shtml";
	//modehome_bedroomon
	else if(sys_status==HOME&&BEEP==0&&RED_LED==0&&GREEN_LED==1)
		return "/modehome_bedroomon.shtml";
	//modehome_bedroomon_corridoron
	else if(sys_status==HOME&&BEEP==0&&RED_LED==0&&GREEN_LED==0)
		return "/modehome_bedroomon_corridoron.shtml";
	//modehome_beepon
	else if(sys_status==HOME&&BEEP==1&&RED_LED==1&&GREEN_LED==1)
		return "/modehome_beepon.shtml";
	//modehome_beepon_bedroomon
	else if(sys_status==HOME&&BEEP==1&&RED_LED==0&&GREEN_LED==1)
		return "/modehome_beepon_bedroomon.shtml";
	//modehome_beepon_corridoron
	else if(sys_status==HOME&&BEEP==1&&RED_LED==1&&GREEN_LED==0)
		return "/modehome_beepon_corridoron.shtml";
	//modehome_corridoron
	else if(sys_status==HOME&&BEEP==0&&RED_LED==1&&GREEN_LED==0)
		return "/modehome_corridoron.shtml";
	else return "/404.html";
}

//BEEP��CGI���ƾ��
const char *BEEP_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])
{
	uint8_t i=0;
	iIndex = FindCGIParameter("BEEP",pcParam,iNumParams);  //�ҵ�BEEP��������
	if(iIndex != -1) 	//�ҵ�BEEP������
	{
		BEEP=0;  		//�ر�
		for(i = 0;i < iNumParams;i++)
		{
			if(strcmp(pcParam[i],"BEEP") == 0)  //����CGI����
			{
				if(strcmp(pcValue[i],"BEEPON") == 0) //��BEEP
					beep_warning();
				else if(strcmp(pcValue[i],"BEEPOFF") == 0) //�ر�BEEP
					BEEP = 0;
			}
		}
	}
  	//alloff
	if(sys_status==OUT&&BEEP==0&&RED_LED==1&&GREEN_LED==1)
		return "/alloff.shtml";
	//allon
	else if(sys_status==HOME&&BEEP==1&&RED_LED==0&&GREEN_LED==0)
		return "/allon.shtml";
	//bedroomon
	else if(sys_status==OUT&&BEEP==0&&RED_LED==0&&GREEN_LED==1)
		return "/bedroomon.shtml";
	//bedroomon_corridoron
	else if(sys_status==OUT&&BEEP==0&&RED_LED==0&&GREEN_LED==0)
		return "/bedroomon_corridoron.shtml";
	//beepon
	else if(sys_status==OUT&&BEEP==1&&RED_LED==1&&GREEN_LED==1)
		return "/beepon.shtml";
	//beepon_Bedroomon
	else if(sys_status==OUT&&BEEP==1&&RED_LED==0&&GREEN_LED==1)
		return "/beepon_Bedroomon.shtml";
	//beepon_bedroomon_corridoron
	else if(sys_status==OUT&&BEEP==1&&RED_LED==0&&GREEN_LED==0)
		return "/beepon_bedroomon_corridoron.shtml";
	//beepon_corridoron
	else if(sys_status==OUT&&BEEP==1&&RED_LED==1&&GREEN_LED==0)
		return "/beepon_corridoron.shtml";
	//corridoron
	else if(sys_status==OUT&&BEEP==0&&RED_LED==1&&GREEN_LED==0)
		return "/corridoron.shtml";
	//modehome
	else if(sys_status==HOME&&BEEP==0&&RED_LED==1&&GREEN_LED==1)
		return "/modehome.shtml";
	//modehome_bedroomon
	else if(sys_status==HOME&&BEEP==0&&RED_LED==0&&GREEN_LED==1)
		return "/modehome_bedroomon.shtml";
	//modehome_bedroomon_corridoron
	else if(sys_status==HOME&&BEEP==0&&RED_LED==0&&GREEN_LED==0)
		return "/modehome_bedroomon_corridoron.shtml";
	//modehome_beepon
	else if(sys_status==HOME&&BEEP==1&&RED_LED==1&&GREEN_LED==1)
		return "/modehome_beepon.shtml";
	//modehome_beepon_bedroomon
	else if(sys_status==HOME&&BEEP==1&&RED_LED==0&&GREEN_LED==1)
		return "/modehome_beepon_bedroomon.shtml";
	//modehome_beepon_corridoron
	else if(sys_status==HOME&&BEEP==1&&RED_LED==1&&GREEN_LED==0)
		return "/modehome_beepon_corridoron.shtml";
	//modehome_corridoron
	else if(sys_status==HOME&&BEEP==0&&RED_LED==1&&GREEN_LED==0)
		return "/modehome_corridoron.shtml";
	else return "/404.html";
}

//MODE��CGI���ƾ��
const char *MODE_CGI_Handler(int iIndex,int iNumParams,char *pcParam[],char *pcValue[])
{
	uint8_t i=0;
	iIndex = FindCGIParameter("Mode",pcParam,iNumParams);  //�ҵ�Mode��������
	if(iIndex != -1) 	//�ҵ�Mode������
	{
		for(i = 0;i < iNumParams;i++)
		{
			if(strcmp(pcParam[i],"Mode") == 0)  //����CGI����
			{
				if(strcmp(pcValue[i],"Home") == 0) //����Ϊ�ڼ�ģʽ
				{
					sys_status=HOME;
					POINT_COLOR =GREEN;
					LCD_ShowString(lcddev.width-95,58,200,24,24," Home ");
				}
				else if(strcmp(pcValue[i],"Out") == 0) //����Ϊ���ģʽ
				{
					sys_status=OUT;
					POINT_COLOR =RED;
					LCD_ShowString(lcddev.width-95,58,200,24,24," OUT  ");
				}
			}
		}
	}
 //alloff
	if(sys_status==OUT&&BEEP==0&&RED_LED==1&&GREEN_LED==1)
		return "/alloff.shtml";
	//allon
	else if(sys_status==HOME&&BEEP==1&&RED_LED==0&&GREEN_LED==0)
		return "/allon.shtml";
	//bedroomon
	else if(sys_status==OUT&&BEEP==0&&RED_LED==0&&GREEN_LED==1)
		return "/bedroomon.shtml";
	//bedroomon_corridoron
	else if(sys_status==OUT&&BEEP==0&&RED_LED==0&&GREEN_LED==0)
		return "/bedroomon_corridoron.shtml";
	//beepon
	else if(sys_status==OUT&&BEEP==1&&RED_LED==1&&GREEN_LED==1)
		return "/beepon.shtml";
	//beepon_Bedroomon
	else if(sys_status==OUT&&BEEP==1&&RED_LED==0&&GREEN_LED==1)
		return "/beepon_Bedroomon.shtml";
	//beepon_bedroomon_corridoron
	else if(sys_status==OUT&&BEEP==1&&RED_LED==0&&GREEN_LED==0)
		return "/beepon_bedroomon_corridoron.shtml";
	//beepon_corridoron
	else if(sys_status==OUT&&BEEP==1&&RED_LED==1&&GREEN_LED==0)
		return "/beepon_corridoron.shtml";
	//corridoron
	else if(sys_status==OUT&&BEEP==0&&RED_LED==1&&GREEN_LED==0)
		return "/corridoron.shtml";
	//modehome
	else if(sys_status==HOME&&BEEP==0&&RED_LED==1&&GREEN_LED==1)
		return "/modehome.shtml";
	//modehome_bedroomon
	else if(sys_status==HOME&&BEEP==0&&RED_LED==0&&GREEN_LED==1)
		return "/modehome_bedroomon.shtml";
	//modehome_bedroomon_corridoron
	else if(sys_status==HOME&&BEEP==0&&RED_LED==0&&GREEN_LED==0)
		return "/modehome_bedroomon_corridoron.shtml";
	//modehome_beepon
	else if(sys_status==HOME&&BEEP==1&&RED_LED==1&&GREEN_LED==1)
		return "/modehome_beepon.shtml";
	//modehome_beepon_bedroomon
	else if(sys_status==HOME&&BEEP==1&&RED_LED==0&&GREEN_LED==1)
		return "/modehome_beepon_bedroomon.shtml";
	//modehome_beepon_corridoron
	else if(sys_status==HOME&&BEEP==1&&RED_LED==1&&GREEN_LED==0)
		return "/modehome_beepon_corridoron.shtml";
	//modehome_corridoron
	else if(sys_status==HOME&&BEEP==0&&RED_LED==1&&GREEN_LED==0)
		return "/modehome_corridoron.shtml";
	else return "/404.html";
}


//SSI�����ʼ��
void httpd_ssi_init(void)
{  
	//����SSI���
	http_set_ssi_handler(SSIHandler,ppcTAGs,NUM_CONFIG_SSI_TAGS);
}

//CGI�����ʼ��
void httpd_cgi_init(void)
{ 
	//����CGI���
	http_set_cgi_handlers(ppcURLs, NUM_CONFIG_CGI_URIS);
}







