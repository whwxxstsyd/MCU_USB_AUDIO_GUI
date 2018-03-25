#ifndef __BLCD_H
#define __BLCD_H		
#include <stdint.h>
#include <stdbool.h>
#include "stm32f10x_conf.h"


#define LCD_WIDTH		(800)
#define LCD_HEIGHT		(480)

#define USE_7			0

#if !USE_7
#define USE_4_3			1
#endif

//-----------------LCD�˿ڶ���---------------- 
//LCD��ַ�ṹ��
typedef struct _tagStLCDCtrl
{
	u16 LCDReg;
	u16 LCDRam;
}StLCDCtrl;

//ʹ��NOR/SRAM�� Bank1.sector4,��ַλHADDR[27,26]=11 A10��Ϊ�������������� 
//ע������ʱSTM32�ڲ�������һλ����! 111110=0X3E			    
#define LCD_BASE        ((u32)(0x6C000000 | 0x000007FE))
#define LCD             ((StLCDCtrl *) LCD_BASE)
//////////////////////////////////////////////////////////////////////////////////
//LCD ָ���
#define LCD_CUR_X  		0x01
#define LCD_END_X 		0x03

#define LCD_CUR_Y  		0x00
#define LCD_PIXELS 		0x02
#define LCD_PREF 		0x05
#define LCD_MIRROR 		0x07


//////////////////////////////////////////////////////////////////////////////////
	 
//ɨ�跽����
#define L2R_U2D  0 //������,���ϵ���
#define L2R_D2U  1 //������,���µ���
#define R2L_U2D  2 //���ҵ���,���ϵ���
#define R2L_D2U  3 //���ҵ���,���µ���

#define U2D_L2R  4 //���ϵ���,������
#define U2D_R2L  5 //���ϵ���,���ҵ���
#define D2U_L2R  6 //���µ���,������
#define D2U_R2L  7 //���µ���,���ҵ���	 

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

#define RGBA(r, g, b) ((int)((r<<16) | (g<<8) | b))
#define RGB(r, g, b) ((unsigned short)((((r >> 3)& 0x1F) << 11) | (((g >> 2) & 0x3F) << 5) | ((b >> 3) & 0x1F)))

extern bool g_boIsLCDCanUse;


#define LCDWriteCmd(u16Cmd) 	LCD->LCDReg = u16Cmd
#define LCDWriteRam(u16Data)	LCD->LCDRam = u16Data

#if USE_7
#define LCDSetXEnd(x)			LCDWriteReg(LCD_END_X, x)
#define LCDWriteRAMPrepare()	LCD->LCDReg = LCD_PIXELS
#else

//#define LCDSetXEnd(x)			LCDWriteReg(LCD_END_X, x)
#define LCDWriteRAMPrepare()	LCD->LCDReg = 0x2C00

#endif

#define LCDDisplayOn()			LCDWriteReg(LCD_PREF, s_u16LCDSysReg);
#define LCDDisplayOff()			LCDWriteReg(LCD_PREF,0)


u16 LCDReadData(void);
void LCDWriteReg(u16 u16Cmd, u16 u16RegValue);
u16 LCDReadReg(u16 u16Cmd);
u16 LCDReadPoint(u16 x, u16 y);
void LCDSetBackLight(u8 u8Light);
void LCDSetDisplayLayer(u16 u16Layer);
void LCDSetOperateLayer(u16 u16Layer);
void LCDSetCursor(u16 Xpos, u16 Ypos);

#if !USE_7
void LCDSetXEnd(u16 Xend);
#endif

void LCDSetScanDir(u8 u8Dir);
void LCDDrawPoint(u16 x,u16 y, u16 u16Color);
void LCDFastDrawPoint(u16 x, u16 y, u16 u16Color);
void LCDInit(void);

void LCDDMAWrite(const u16 *pBuf, u32 u32Len);
void LCDDMAWriteSameValue(u16 u16Value, u32 u32Len);

void LCDClear(u16 u16Color);
#if 0
void LCDFill(u16 u16MinX, u16 u16MinY, u16 u16MaxX, u16 u16MaxY, u16 u16Color);
void LCDFillWithRam(u16 u16MinX, u16 u16MinY, u16 u16MaxX, u16 u16MaxY,  u16 *pRam); 
void LCDDrawLine(u16 x1, u16 y1, u16 x2, u16 y2, u16 u16Color);
void LCDDrawRectangle(u16 u16MinX, u16 u16MinY, u16 u16MaxX, u16 u16MaxY, u16 u16Color);
void LCDDrawCircle(u16 u16XPos, u16 u16YPos, u16 u16Radius, u16 u16Color);

#endif

#endif  
	 
	 



