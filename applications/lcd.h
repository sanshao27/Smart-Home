#ifndef __LCD_H
#define __LCD_H	
#include "stdint.h"

//LCD��Ҫ������
typedef struct  
{		 	 
	uint16_t width;			//LCD ���
	uint16_t height;			//LCD �߶�
	uint16_t id;				//LCD ID
	uint8_t  dir;			//���������������ƣ�0��������1��������	
	uint16_t	wramcmd;		//��ʼдgramָ��
	uint16_t setxcmd;		//����x����ָ��
	uint16_t setycmd;		//����y����ָ�� 
}_lcd_dev; 	  

//LCD����
extern _lcd_dev lcddev;	//����LCD��Ҫ����
//LCD�Ļ�����ɫ�ͱ���ɫ	   
extern uint32_t  POINT_COLOR;//Ĭ�Ϻ�ɫ    
extern uint32_t  BACK_COLOR; //������ɫ.Ĭ��Ϊ��ɫ


//////////////////////////////////////////////////////////////////////////////////	 
//-----------------MCU�� LCD�˿ڶ���---------------- 
#define	LCD_LED PBout(0) 	//LCD����	PB0    
//LCD��ַ�ṹ��
typedef struct
{
	volatile uint16_t LCD_REG;
	volatile uint16_t LCD_RAM;
} LCD_TypeDef;
//ʹ��NOR/SRAM�� Bank1.sector4,��ַλHADDR[27,26]=11 A10��Ϊ�������������� 
//ע������ʱSTM32�ڲ�������һλ����! 			    
#define LCD_BASE        ((uint32_t)(0x6C000000 | 0x000007FE))
#define LCD             ((LCD_TypeDef *) LCD_BASE)
//////////////////////////////////////////////////////////////////////////////////
	 
//ɨ�跽����
#define L2R_U2D  0 		//������,���ϵ���
#define L2R_D2U  1 		//������,���µ���
#define R2L_U2D  2 		//���ҵ���,���ϵ���
#define R2L_D2U  3 		//���ҵ���,���µ���

#define U2D_L2R  4 		//���ϵ���,������
#define U2D_R2L  5 		//���ϵ���,���ҵ���
#define D2U_L2R  6 		//���µ���,������
#define D2U_R2L  7		//���µ���,���ҵ���	 

#define DFT_SCAN_DIR  L2R_U2D  //Ĭ�ϵ�ɨ�跽��

//������ɫ
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //��ɫ
#define BRRED 			 0XFC07 //�غ�ɫ
#define GRAY  			 0X8430 //��ɫ
//GUI��ɫ

#define DARKBLUE      	 0X01CF	//����ɫ
#define LIGHTBLUE      	 0X7D7C	//ǳ��ɫ  
#define GRAYBLUE       	 0X5458 //����ɫ
//������ɫΪPANEL����ɫ 
 
#define LIGHTGREEN     	 0X841F //ǳ��ɫ
//#define LIGHTGRAY        0XEF5B //ǳ��ɫ(PANNEL)
#define LGRAY 			 0XC618 //ǳ��ɫ(PANNEL),���屳��ɫ

#define LGRAYBLUE        0XA651 //ǳ����ɫ(�м����ɫ)
#define LBBLUE           0X2B12 //ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)

//���崥�������ܰ����Ĵ�������
//������ֵ��ͨ������tp_dev.scan(0)���ɴ��ڴ�ӡtp_dev.x[0]��tp_dev.y[0]�õ�
#define Y_Bedroom_Light 236
#define Y_Livingroom_Light 254
#define Y_Corridor_Light 272
#define Y_Curtain 290
#define x1_curtain_open 80
#define x2_curtain_open 111
#define x3_curtain_close 128
#define x4_curtain_close 171
#define x5_curtain_kt 184
#define x6_curtain_kt 208
#define Y_Mode  95
#define x1_time 1
#define x2_time 120
#define y1_time 43
#define y2_time 92
#define x1_mode 120
#define x2_mode 239
#define y1_mode 43
#define y2_mode 92
//����ʱ�䴥��λ��
#define SET_YEAR           (((tp_dev.x[0]>58)&&(tp_dev.x[0]<100))&&((tp_dev.y[0]>48)&&(tp_dev.y[0]<61)))
#define SET_MON			   (((tp_dev.x[0]>119)&&(tp_dev.x[0]<139))&&((tp_dev.y[0]>48)&&(tp_dev.y[0]<61)))
#define SET_DAY			   (((tp_dev.x[0]>155)&&(tp_dev.x[0]<177))&&((tp_dev.y[0]>48)&&(tp_dev.y[0]<61)))
#define SET_HOUR		   (((tp_dev.x[0]>64)&&(tp_dev.x[0]<83))&&((tp_dev.y[0]>75)&&(tp_dev.y[0]<88)))
#define SET_MIN            (((tp_dev.x[0]>100)&&(tp_dev.x[0]<123))&&((tp_dev.y[0]>75)&&(tp_dev.y[0]<88)))
#define SET_SEC            (((tp_dev.x[0]>140)&&(tp_dev.x[0]<159))&&((tp_dev.y[0]>75)&&(tp_dev.y[0]<88)))
#define SET_WEEK           (((tp_dev.x[0]>78)&&(tp_dev.x[0]<148))&&((tp_dev.y[0]>102)&&(tp_dev.y[0]<118)))
//���ܴ���λ��
#define TIME                (((tp_dev.x[0]>x1_time)&&(tp_dev.x[0]<x2_time))&&((tp_dev.y[0]>y1_time)&&(tp_dev.y[0]<y2_time)))
#define MODE                ((tp_dev.x[0]>120&&tp_dev.x[0]<239)&&(tp_dev.y[0]<Y_Mode&&tp_dev.y[0]>Y_Mode-52)) 
#define Bedroom_Light       (tp_dev.x[0]>(lcddev.width-60)&&(tp_dev.x[0]<(lcddev.width-60+16))&&((tp_dev.y[0]>Y_Bedroom_Light)&&(tp_dev.y[0]<(Y_Bedroom_Light+16))))
#define Livingroom_Light    (tp_dev.x[0]>(lcddev.width-60)&&(tp_dev.x[0]<(lcddev.width-60+16))&&((tp_dev.y[0]>Y_Livingroom_Light)&&(tp_dev.y[0]<(Y_Livingroom_Light+16)))) 
#define Corridor_Light      (tp_dev.x[0]>(lcddev.width-60)&&(tp_dev.x[0]<(lcddev.width-60+16))&&((tp_dev.y[0]>Y_Corridor_Light)&&(tp_dev.y[0]<(Y_Corridor_Light+16))))    
#define Curtain_CLOSE       (((tp_dev.x[0]>x3_curtain_close)&&(tp_dev.x[0]<x4_curtain_close))&&((tp_dev.y[0]>Y_Curtain)&&(tp_dev.y[0]<Y_Curtain+16)))
#define Curtain_OPEN        (((tp_dev.x[0]>x1_curtain_open)&&(tp_dev.x[0]<x2_curtain_open))&&((tp_dev.y[0]>Y_Curtain)&&(tp_dev.y[0]<Y_Curtain+16)))
#define Curtain_KT          (((tp_dev.x[0]>x5_curtain_kt)&&(tp_dev.x[0]<x6_curtain_kt))&&((tp_dev.y[0]>Y_Curtain)&&(tp_dev.y[0]<Y_Curtain+16)))
//���ּ��̴���λ��
#define ZERO                (((tp_dev.x[0]>0)&&(tp_dev.x[0]<77))&&((tp_dev.y[0]>162)&&(tp_dev.y[0]<199)))
#define ONE                 (((tp_dev.x[0]>78)&&(tp_dev.x[0]<166))&&((tp_dev.y[0]>162)&&(tp_dev.y[0]<199)))
#define TWO                 (((tp_dev.x[0]>167)&&(tp_dev.x[0]<239))&&((tp_dev.y[0]>162)&&(tp_dev.y[0]<199)))
#define THREE               (((tp_dev.x[0]>0)&&(tp_dev.x[0]<77))&&((tp_dev.y[0]>201)&&(tp_dev.y[0]<236)))
#define FOUR                (((tp_dev.x[0]>78)&&(tp_dev.x[0]<166))&&((tp_dev.y[0]>201)&&(tp_dev.y[0]<236)))
#define FIVE                (((tp_dev.x[0]>167)&&(tp_dev.x[0]<239))&&((tp_dev.y[0]>201)&&(tp_dev.y[0]<236)))
#define SIX                 (((tp_dev.x[0]>0)&&(tp_dev.x[0]<77))&&((tp_dev.y[0]>238)&&(tp_dev.y[0]<279)))
#define SEVEN               (((tp_dev.x[0]>78)&&(tp_dev.x[0]<166))&&((tp_dev.y[0]>238)&&(tp_dev.y[0]<279)))
#define EIGHT               (((tp_dev.x[0]>167)&&(tp_dev.x[0]<239))&&((tp_dev.y[0]>238)&&(tp_dev.y[0]<279)))
#define NINE                (((tp_dev.x[0]>0)&&(tp_dev.x[0]<77))&&((tp_dev.y[0]>282)&&(tp_dev.y[0]<320)))
#define CANCLE              (((tp_dev.x[0]>78)&&(tp_dev.x[0]<166))&&((tp_dev.y[0]>282)&&(tp_dev.y[0]<320)))
#define OK                  (((tp_dev.x[0]>167)&&(tp_dev.x[0]<239))&&((tp_dev.y[0]>282)&&(tp_dev.y[0]<320)))
//��������
int LCD_Init(void);													   	//��ʼ��
void LCD_DisplayOn(void);													//����ʾ
void LCD_DisplayOff(void);													//����ʾ
void LCD_Clear(uint32_t Color);	 												//����
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);										//���ù��
void LCD_DrawPoint(uint16_t x,uint16_t y);											//����
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint32_t color);								//���ٻ���
uint32_t  LCD_ReadPoint(uint16_t x,uint16_t y); 											//���� 
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r);						 			//��Բ
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);							//����
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);		   				//������
void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint32_t color);		   				//��䵥ɫ
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color);				//���ָ����ɫ
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode);						//��ʾһ���ַ�
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size);  						//��ʾһ������
void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode);				//��ʾ ����
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,char *p);		//��ʾһ���ַ���,12/16����
int Draw_window(void);
void Draw_Number_Pad(void);
void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue);
uint16_t LCD_ReadReg(uint16_t LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(uint16_t RGB_Code);
void LCD_SSD_BackLightSet(uint8_t pwm);							//SSD1963 �������
void LCD_Scan_Dir(uint8_t dir);									//������ɨ�跽��
void LCD_Display_Dir(uint8_t dir);								//������Ļ��ʾ����
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height);	//���ô���					   						   																			 
//LCD�ֱ�������
#define SSD_HOR_RESOLUTION		800		//LCDˮƽ�ֱ���
#define SSD_VER_RESOLUTION		480		//LCD��ֱ�ֱ���
//LCD������������
#define SSD_HOR_PULSE_WIDTH		1		//ˮƽ����
#define SSD_HOR_BACK_PORCH		46		//ˮƽǰ��
#define SSD_HOR_FRONT_PORCH		210		//ˮƽ����

#define SSD_VER_PULSE_WIDTH		1		//��ֱ����
#define SSD_VER_BACK_PORCH		23		//��ֱǰ��
#define SSD_VER_FRONT_PORCH		22		//��ֱǰ��
//���¼����������Զ�����
#define SSD_HT	(SSD_HOR_RESOLUTION+SSD_HOR_BACK_PORCH+SSD_HOR_FRONT_PORCH)
#define SSD_HPS	(SSD_HOR_BACK_PORCH)
#define SSD_VT 	(SSD_VER_RESOLUTION+SSD_VER_BACK_PORCH+SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)

#endif  
	 
	 



