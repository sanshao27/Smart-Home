#ifndef __LCD_H
#define __LCD_H	
#include "stdint.h"

//LCD重要参数集
typedef struct  
{		 	 
	uint16_t width;			//LCD 宽度
	uint16_t height;			//LCD 高度
	uint16_t id;				//LCD ID
	uint8_t  dir;			//横屏还是竖屏控制：0，竖屏；1，横屏。	
	uint16_t	wramcmd;		//开始写gram指令
	uint16_t setxcmd;		//设置x坐标指令
	uint16_t setycmd;		//设置y坐标指令 
}_lcd_dev; 	  

//LCD参数
extern _lcd_dev lcddev;	//管理LCD重要参数
//LCD的画笔颜色和背景色	   
extern uint32_t  POINT_COLOR;//默认红色    
extern uint32_t  BACK_COLOR; //背景颜色.默认为白色


//////////////////////////////////////////////////////////////////////////////////	 
//-----------------MCU屏 LCD端口定义---------------- 
#define	LCD_LED PBout(0) 	//LCD背光	PB0    
//LCD地址结构体
typedef struct
{
	volatile uint16_t LCD_REG;
	volatile uint16_t LCD_RAM;
} LCD_TypeDef;
//使用NOR/SRAM的 Bank1.sector4,地址位HADDR[27,26]=11 A10作为数据命令区分线 
//注意设置时STM32内部会右移一位对其! 			    
#define LCD_BASE        ((uint32_t)(0x6C000000 | 0x000007FE))
#define LCD             ((LCD_TypeDef *) LCD_BASE)
//////////////////////////////////////////////////////////////////////////////////
	 
//扫描方向定义
#define L2R_U2D  0 		//从左到右,从上到下
#define L2R_D2U  1 		//从左到右,从下到上
#define R2L_U2D  2 		//从右到左,从上到下
#define R2L_D2U  3 		//从右到左,从下到上

#define U2D_L2R  4 		//从上到下,从左到右
#define U2D_R2L  5 		//从上到下,从右到左
#define D2U_L2R  6 		//从下到上,从左到右
#define D2U_R2L  7		//从下到上,从右到左	 

#define DFT_SCAN_DIR  L2R_U2D  //默认的扫描方向

//画笔颜色
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
#define BROWN 			 0XBC40 //棕色
#define BRRED 			 0XFC07 //棕红色
#define GRAY  			 0X8430 //灰色
//GUI颜色

#define DARKBLUE      	 0X01CF	//深蓝色
#define LIGHTBLUE      	 0X7D7C	//浅蓝色  
#define GRAYBLUE       	 0X5458 //灰蓝色
//以上三色为PANEL的颜色 
 
#define LIGHTGREEN     	 0X841F //浅绿色
//#define LIGHTGRAY        0XEF5B //浅灰色(PANNEL)
#define LGRAY 			 0XC618 //浅灰色(PANNEL),窗体背景色

#define LGRAYBLUE        0XA651 //浅灰蓝色(中间层颜色)
#define LBBLUE           0X2B12 //浅棕蓝色(选择条目的反色)

//定义触摸屏功能按键的触摸区域
//以下数值可通过调用tp_dev.scan(0)后由串口打印tp_dev.x[0]，tp_dev.y[0]得到
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
//设置时间触控位置
#define SET_YEAR           (((tp_dev.x[0]>58)&&(tp_dev.x[0]<100))&&((tp_dev.y[0]>48)&&(tp_dev.y[0]<61)))
#define SET_MON			   (((tp_dev.x[0]>119)&&(tp_dev.x[0]<139))&&((tp_dev.y[0]>48)&&(tp_dev.y[0]<61)))
#define SET_DAY			   (((tp_dev.x[0]>155)&&(tp_dev.x[0]<177))&&((tp_dev.y[0]>48)&&(tp_dev.y[0]<61)))
#define SET_HOUR		   (((tp_dev.x[0]>64)&&(tp_dev.x[0]<83))&&((tp_dev.y[0]>75)&&(tp_dev.y[0]<88)))
#define SET_MIN            (((tp_dev.x[0]>100)&&(tp_dev.x[0]<123))&&((tp_dev.y[0]>75)&&(tp_dev.y[0]<88)))
#define SET_SEC            (((tp_dev.x[0]>140)&&(tp_dev.x[0]<159))&&((tp_dev.y[0]>75)&&(tp_dev.y[0]<88)))
#define SET_WEEK           (((tp_dev.x[0]>78)&&(tp_dev.x[0]<148))&&((tp_dev.y[0]>102)&&(tp_dev.y[0]<118)))
//功能触控位置
#define TIME                (((tp_dev.x[0]>x1_time)&&(tp_dev.x[0]<x2_time))&&((tp_dev.y[0]>y1_time)&&(tp_dev.y[0]<y2_time)))
#define MODE                ((tp_dev.x[0]>120&&tp_dev.x[0]<239)&&(tp_dev.y[0]<Y_Mode&&tp_dev.y[0]>Y_Mode-52)) 
#define Bedroom_Light       (tp_dev.x[0]>(lcddev.width-60)&&(tp_dev.x[0]<(lcddev.width-60+16))&&((tp_dev.y[0]>Y_Bedroom_Light)&&(tp_dev.y[0]<(Y_Bedroom_Light+16))))
#define Livingroom_Light    (tp_dev.x[0]>(lcddev.width-60)&&(tp_dev.x[0]<(lcddev.width-60+16))&&((tp_dev.y[0]>Y_Livingroom_Light)&&(tp_dev.y[0]<(Y_Livingroom_Light+16)))) 
#define Corridor_Light      (tp_dev.x[0]>(lcddev.width-60)&&(tp_dev.x[0]<(lcddev.width-60+16))&&((tp_dev.y[0]>Y_Corridor_Light)&&(tp_dev.y[0]<(Y_Corridor_Light+16))))    
#define Curtain_CLOSE       (((tp_dev.x[0]>x3_curtain_close)&&(tp_dev.x[0]<x4_curtain_close))&&((tp_dev.y[0]>Y_Curtain)&&(tp_dev.y[0]<Y_Curtain+16)))
#define Curtain_OPEN        (((tp_dev.x[0]>x1_curtain_open)&&(tp_dev.x[0]<x2_curtain_open))&&((tp_dev.y[0]>Y_Curtain)&&(tp_dev.y[0]<Y_Curtain+16)))
#define Curtain_KT          (((tp_dev.x[0]>x5_curtain_kt)&&(tp_dev.x[0]<x6_curtain_kt))&&((tp_dev.y[0]>Y_Curtain)&&(tp_dev.y[0]<Y_Curtain+16)))
//数字键盘触控位置
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
//函数声明
int LCD_Init(void);													   	//初始化
void LCD_DisplayOn(void);													//开显示
void LCD_DisplayOff(void);													//关显示
void LCD_Clear(uint32_t Color);	 												//清屏
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);										//设置光标
void LCD_DrawPoint(uint16_t x,uint16_t y);											//画点
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint32_t color);								//快速画点
uint32_t  LCD_ReadPoint(uint16_t x,uint16_t y); 											//读点 
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r);						 			//画圆
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);							//画线
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);		   				//画矩形
void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint32_t color);		   				//填充单色
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color);				//填充指定颜色
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode);						//显示一个字符
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size);  						//显示一个数字
void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode);				//显示 数字
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,char *p);		//显示一个字符串,12/16字体
int Draw_window(void);
void Draw_Number_Pad(void);
void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue);
uint16_t LCD_ReadReg(uint16_t LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(uint16_t RGB_Code);
void LCD_SSD_BackLightSet(uint8_t pwm);							//SSD1963 背光控制
void LCD_Scan_Dir(uint8_t dir);									//设置屏扫描方向
void LCD_Display_Dir(uint8_t dir);								//设置屏幕显示方向
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height);	//设置窗口					   						   																			 
//LCD分辨率设置
#define SSD_HOR_RESOLUTION		800		//LCD水平分辨率
#define SSD_VER_RESOLUTION		480		//LCD垂直分辨率
//LCD驱动参数设置
#define SSD_HOR_PULSE_WIDTH		1		//水平脉宽
#define SSD_HOR_BACK_PORCH		46		//水平前廊
#define SSD_HOR_FRONT_PORCH		210		//水平后廊

#define SSD_VER_PULSE_WIDTH		1		//垂直脉宽
#define SSD_VER_BACK_PORCH		23		//垂直前廊
#define SSD_VER_FRONT_PORCH		22		//垂直前廊
//如下几个参数，自动计算
#define SSD_HT	(SSD_HOR_RESOLUTION+SSD_HOR_BACK_PORCH+SSD_HOR_FRONT_PORCH)
#define SSD_HPS	(SSD_HOR_BACK_PORCH)
#define SSD_VT 	(SSD_VER_RESOLUTION+SSD_VER_BACK_PORCH+SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)

#endif  
	 
	 



