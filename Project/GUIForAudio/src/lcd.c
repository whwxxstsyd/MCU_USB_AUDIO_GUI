#include <stdlib.h>
#include <stdbool.h>
#include "stm32f10x_conf.h"
#include "user_api.h"
#include "lcd.h"

#define LCD_DMA_CLOCK			RCC_AHBPeriph_DMA1		
#define LCD_DMA					DMA1_Channel1
#define LCD_DMA_FINISH_FLAG		DMA1_FLAG_TC1

u16 s_u16LCDSysReg = 0;		//PREF寄存器，设置当前操作页，显示页和背光等（ATK-7' TFTLCD暂不支持横竖屏设置，统一为横屏）。	
bool g_boIsLCDCanUse = false;

//读LCD数据
//返回值:读到的值
u16 LCDReadData(void)
{										    	   
	return LCD->LCDRam;		 
}					   
//写寄存器
//LCDReg:寄存器地址
//LCDRegValue:要写入的数据
void LCDWriteReg(u16 u16Cmd, u16 u16RegValue)
{	
	LCD->LCDReg = u16Cmd;		//写入要写的寄存器序号	 
	LCD->LCDRam = u16RegValue;	//写入数据	    		 
}


//读寄存器
//LCDReg:寄存器地址
//返回值:读到的数据
u16 LCDReadReg(u16 u16Cmd)
{										   
	LCD->LCDReg = u16Cmd;		//写入要写的寄存器序号	 
	return LCD->LCDRam;
}  

#if USE_7
//读取个某点的颜色值	 
//x,y:坐标
//返回值:此点的颜色
u16 LCDReadPoint(u16 x, u16 y)
{
 	u16 t = 0;
 	LCDSetCursor(x, y);	    
	while (t < 0X1FFF)
	{	
		if (LCD->LCDReg & 0x0001)
		{
			break; 
		}
		t++;		   
	}					   
	return LCD->LCDRam; 	
}
//LCD背光设置
//pwm:背光等级,0~63.越大越亮.
void LCDSetBackLight(u8 u8Light)
{	
	s_u16LCDSysReg &= ~0x3F;
	s_u16LCDSysReg |= (u8Light & 0x3F);
	LCDWriteReg(LCD_PREF, s_u16LCDSysReg);
}
//设置当前显示层
//layer:当前显示层 
void LCDSetDisplayLayer(u16 u16Layer)
{	 
	s_u16LCDSysReg &= ~0x0E00;
	s_u16LCDSysReg |= ((u16Layer & 0x07) << 9);
	LCDWriteReg(LCD_PREF, s_u16LCDSysReg);
} 
//设置当前操作层
//layer:当前显示层 
void LCDSetOperateLayer(u16 u16Layer)
{	 
	s_u16LCDSysReg &= ~0x7000;
	s_u16LCDSysReg |= ((u16Layer & 0x07) << 12);
	LCDWriteReg(LCD_PREF, s_u16LCDSysReg);
} 
#endif
//设置光标位置
//Xpos:横坐标
//Ypos:纵坐标
void LCDSetCursor(u16 Xpos, u16 Ypos)
{	
#if !USE_7
	LCDWriteReg(0x2A00, Xpos >> 8);
	LCDWriteReg(0x2A01, Xpos & 0xFF);
	
	LCDWriteReg(0x2B00, Ypos >> 8);
	LCDWriteReg(0x2B01, Ypos & 0xFF);
 	
#else
	LCDWriteReg(LCD_CUR_X, Xpos);
	LCDWriteReg(LCD_CUR_Y, Ypos);
#endif	
	
} 	

#if !USE_7

void LCDSetXEnd(u16 Xend)
{
	LCDWriteReg(0x2A02, Xend >> 8);
	LCDWriteReg(0x2A03, Xend & 0xFF);	
}
void LCDSetYEnd(u16 Yend)
{
	LCDWriteReg(0x2B02, Yend >> 8);
	LCDWriteReg(0x2B03, Yend & 0xFF);	
}
#endif


#if USE_7

//设置LCD的自动扫描方向
//注意:我们的驱动器,只支持左右扫描设置,不支持上下扫描设置   	   
void LCDSetScanDir(u8 u8Dir)
{			   
    switch(u8Dir)
	{
		case L2R_U2D:	//从左到右,从上到下
			LCDWriteReg(LCD_MIRROR,1);	//写LCD_PREF寄存器
			break;
		case L2R_D2U:	//从左到右,从下到上
			LCDWriteReg(LCD_MIRROR,3);	//写LCD_PREF寄存器
			break;
		case R2L_U2D:	//从右到左,从上到下
			LCDWriteReg(LCD_MIRROR,0);	//写LCD_PREF寄存器
			break;
		case R2L_D2U:	//从右到左,从下到上
			LCDWriteReg(LCD_MIRROR,2);	//写LCD_PREF寄存器
			break;	
		default:		//其他,默认从左到右,从上到下
			LCDWriteReg(LCD_MIRROR,1);	//写LCD_PREF寄存器
			break;	 
	}
}
#endif

#if USE_7
//画点
//x,y:坐标
//POINT_COLOR:此点的颜色
void LCDDrawPoint(u16 x,u16 y, u16 u16Color)
{
	if ((x >= LCD_WIDTH) || (y  >= LCD_HEIGHT))
	{
		return;
	}
	LCDSetCursor(x, y);		//设置光标位置 
	LCDWriteRAMPrepare();	//开始写入GRAM
	LCD->LCDRam = u16Color; 
}
//快速画点
//x,y:坐标
//color:颜色
void LCDFastDrawPoint(u16 x, u16 y, u16 u16Color)
{	  		 		   
	if ((x >= LCD_WIDTH) || (y  >= LCD_HEIGHT))
	{
		return;
	}
	LCD->LCDReg = LCD_CUR_Y;
	LCD->LCDRam = y; 
 	LCD->LCDReg = LCD_CUR_X; 
	LCD->LCDRam = x; 
	LCD->LCDReg = LCD_PIXELS; 
	LCD->LCDRam = u16Color; 
}

#elif USE_4_3

void LCDDrawPoint(u16 x,u16 y, u16 u16Color)
{
	if ((x >= LCD_WIDTH) || (y  >= LCD_HEIGHT))
	{
		return;
	}
	LCDSetCursor(x, y);		//设置光标位置 
	LCDWriteRAMPrepare();	//开始写入GRAM
	LCD->LCDRam = u16Color; 
}
void LCDFastDrawPoint(u16 x, u16 y, u16 u16Color)
{	  		 		   
	LCDDrawPoint(x, y, u16Color);
}


#endif

//设置LCD显示方向（7寸屏,不能简单的修改为横屏显示）
//dir:0,竖屏；1,横屏
void LCDDisplayDir(u8 dir)
{
	 							    	  
}

void NT35510Init(void)
{
	LCDWriteReg(0xF000,0x55);
	LCDWriteReg(0xF001,0xAA);
	LCDWriteReg(0xF002,0x52);
	LCDWriteReg(0xF003,0x08);
	LCDWriteReg(0xF004,0x01);
	//AVDD Set AVDD 5.2V
	LCDWriteReg(0xB000,0x0D);
	LCDWriteReg(0xB001,0x0D);
	LCDWriteReg(0xB002,0x0D);
	//AVDD ratio
	LCDWriteReg(0xB600,0x34);
	LCDWriteReg(0xB601,0x34);
	LCDWriteReg(0xB602,0x34);
	//AVEE -5.2V
	LCDWriteReg(0xB100,0x0D);
	LCDWriteReg(0xB101,0x0D);
	LCDWriteReg(0xB102,0x0D);
	//AVEE ratio
	LCDWriteReg(0xB700,0x34);
	LCDWriteReg(0xB701,0x34);
	LCDWriteReg(0xB702,0x34);
	//VCL -2.5V
	LCDWriteReg(0xB200,0x00);
	LCDWriteReg(0xB201,0x00);
	LCDWriteReg(0xB202,0x00);
	//VCL ratio
	LCDWriteReg(0xB800,0x24);
	LCDWriteReg(0xB801,0x24);
	LCDWriteReg(0xB802,0x24);
	//VGH 15V (Free pump)
	LCDWriteReg(0xBF00,0x01);
	LCDWriteReg(0xB300,0x0F);
	LCDWriteReg(0xB301,0x0F);
	LCDWriteReg(0xB302,0x0F);
	//VGH ratio
	LCDWriteReg(0xB900,0x34);
	LCDWriteReg(0xB901,0x34);
	LCDWriteReg(0xB902,0x34);
	//VGL_REG -10V
	LCDWriteReg(0xB500,0x08);
	LCDWriteReg(0xB501,0x08);
	LCDWriteReg(0xB502,0x08);
	LCDWriteReg(0xC200,0x03);
	//VGLX ratio
	LCDWriteReg(0xBA00,0x24);
	LCDWriteReg(0xBA01,0x24);
	LCDWriteReg(0xBA02,0x24);
	//VGMP/VGSP 4.5V/0V
	LCDWriteReg(0xBC00,0x00);
	LCDWriteReg(0xBC01,0x78);
	LCDWriteReg(0xBC02,0x00);
	//VGMN/VGSN -4.5V/0V
	LCDWriteReg(0xBD00,0x00);
	LCDWriteReg(0xBD01,0x78);
	LCDWriteReg(0xBD02,0x00);
	//VCOM
	LCDWriteReg(0xBE00,0x00);
	LCDWriteReg(0xBE01,0x64);
	//Gamma Setting
	LCDWriteReg(0xD100,0x00);
	LCDWriteReg(0xD101,0x33);
	LCDWriteReg(0xD102,0x00);
	LCDWriteReg(0xD103,0x34);
	LCDWriteReg(0xD104,0x00);
	LCDWriteReg(0xD105,0x3A);
	LCDWriteReg(0xD106,0x00);
	LCDWriteReg(0xD107,0x4A);
	LCDWriteReg(0xD108,0x00);
	LCDWriteReg(0xD109,0x5C);
	LCDWriteReg(0xD10A,0x00);
	LCDWriteReg(0xD10B,0x81);
	LCDWriteReg(0xD10C,0x00);
	LCDWriteReg(0xD10D,0xA6);
	LCDWriteReg(0xD10E,0x00);
	LCDWriteReg(0xD10F,0xE5);
	LCDWriteReg(0xD110,0x01);
	LCDWriteReg(0xD111,0x13);
	LCDWriteReg(0xD112,0x01);
	LCDWriteReg(0xD113,0x54);
	LCDWriteReg(0xD114,0x01);
	LCDWriteReg(0xD115,0x82);
	LCDWriteReg(0xD116,0x01);
	LCDWriteReg(0xD117,0xCA);
	LCDWriteReg(0xD118,0x02);
	LCDWriteReg(0xD119,0x00);
	LCDWriteReg(0xD11A,0x02);
	LCDWriteReg(0xD11B,0x01);
	LCDWriteReg(0xD11C,0x02);
	LCDWriteReg(0xD11D,0x34);
	LCDWriteReg(0xD11E,0x02);
	LCDWriteReg(0xD11F,0x67);
	LCDWriteReg(0xD120,0x02);
	LCDWriteReg(0xD121,0x84);
	LCDWriteReg(0xD122,0x02);
	LCDWriteReg(0xD123,0xA4);
	LCDWriteReg(0xD124,0x02);
	LCDWriteReg(0xD125,0xB7);
	LCDWriteReg(0xD126,0x02);
	LCDWriteReg(0xD127,0xCF);
	LCDWriteReg(0xD128,0x02);
	LCDWriteReg(0xD129,0xDE);
	LCDWriteReg(0xD12A,0x02);
	LCDWriteReg(0xD12B,0xF2);
	LCDWriteReg(0xD12C,0x02);
	LCDWriteReg(0xD12D,0xFE);
	LCDWriteReg(0xD12E,0x03);
	LCDWriteReg(0xD12F,0x10);
	LCDWriteReg(0xD130,0x03);
	LCDWriteReg(0xD131,0x33);
	LCDWriteReg(0xD132,0x03);
	LCDWriteReg(0xD133,0x6D);
	LCDWriteReg(0xD200,0x00);
	LCDWriteReg(0xD201,0x33);
	LCDWriteReg(0xD202,0x00);
	LCDWriteReg(0xD203,0x34);
	LCDWriteReg(0xD204,0x00);
	LCDWriteReg(0xD205,0x3A);
	LCDWriteReg(0xD206,0x00);
	LCDWriteReg(0xD207,0x4A);
	LCDWriteReg(0xD208,0x00);
	LCDWriteReg(0xD209,0x5C);
	LCDWriteReg(0xD20A,0x00);

	LCDWriteReg(0xD20B,0x81);
	LCDWriteReg(0xD20C,0x00);
	LCDWriteReg(0xD20D,0xA6);
	LCDWriteReg(0xD20E,0x00);
	LCDWriteReg(0xD20F,0xE5);
	LCDWriteReg(0xD210,0x01);
	LCDWriteReg(0xD211,0x13);
	LCDWriteReg(0xD212,0x01);
	LCDWriteReg(0xD213,0x54);
	LCDWriteReg(0xD214,0x01);
	LCDWriteReg(0xD215,0x82);
	LCDWriteReg(0xD216,0x01);
	LCDWriteReg(0xD217,0xCA);
	LCDWriteReg(0xD218,0x02);
	LCDWriteReg(0xD219,0x00);
	LCDWriteReg(0xD21A,0x02);
	LCDWriteReg(0xD21B,0x01);
	LCDWriteReg(0xD21C,0x02);
	LCDWriteReg(0xD21D,0x34);
	LCDWriteReg(0xD21E,0x02);
	LCDWriteReg(0xD21F,0x67);
	LCDWriteReg(0xD220,0x02);
	LCDWriteReg(0xD221,0x84);
	LCDWriteReg(0xD222,0x02);
	LCDWriteReg(0xD223,0xA4);
	LCDWriteReg(0xD224,0x02);
	LCDWriteReg(0xD225,0xB7);
	LCDWriteReg(0xD226,0x02);
	LCDWriteReg(0xD227,0xCF);
	LCDWriteReg(0xD228,0x02);
	LCDWriteReg(0xD229,0xDE);
	LCDWriteReg(0xD22A,0x02);
	LCDWriteReg(0xD22B,0xF2);
	LCDWriteReg(0xD22C,0x02);
	LCDWriteReg(0xD22D,0xFE);
	LCDWriteReg(0xD22E,0x03);
	LCDWriteReg(0xD22F,0x10);
	LCDWriteReg(0xD230,0x03);
	LCDWriteReg(0xD231,0x33);
	LCDWriteReg(0xD232,0x03);
	LCDWriteReg(0xD233,0x6D);
	LCDWriteReg(0xD300,0x00);
	LCDWriteReg(0xD301,0x33);
	LCDWriteReg(0xD302,0x00);
	LCDWriteReg(0xD303,0x34);
	LCDWriteReg(0xD304,0x00);
	LCDWriteReg(0xD305,0x3A);
	LCDWriteReg(0xD306,0x00);
	LCDWriteReg(0xD307,0x4A);
	LCDWriteReg(0xD308,0x00);
	LCDWriteReg(0xD309,0x5C);
	LCDWriteReg(0xD30A,0x00);

	LCDWriteReg(0xD30B,0x81);
	LCDWriteReg(0xD30C,0x00);
	LCDWriteReg(0xD30D,0xA6);
	LCDWriteReg(0xD30E,0x00);
	LCDWriteReg(0xD30F,0xE5);
	LCDWriteReg(0xD310,0x01);
	LCDWriteReg(0xD311,0x13);
	LCDWriteReg(0xD312,0x01);
	LCDWriteReg(0xD313,0x54);
	LCDWriteReg(0xD314,0x01);
	LCDWriteReg(0xD315,0x82);
	LCDWriteReg(0xD316,0x01);
	LCDWriteReg(0xD317,0xCA);
	LCDWriteReg(0xD318,0x02);
	LCDWriteReg(0xD319,0x00);
	LCDWriteReg(0xD31A,0x02);
	LCDWriteReg(0xD31B,0x01);
	LCDWriteReg(0xD31C,0x02);
	LCDWriteReg(0xD31D,0x34);
	LCDWriteReg(0xD31E,0x02);
	LCDWriteReg(0xD31F,0x67);
	LCDWriteReg(0xD320,0x02);
	LCDWriteReg(0xD321,0x84);
	LCDWriteReg(0xD322,0x02);
	LCDWriteReg(0xD323,0xA4);
	LCDWriteReg(0xD324,0x02);
	LCDWriteReg(0xD325,0xB7);
	LCDWriteReg(0xD326,0x02);
	LCDWriteReg(0xD327,0xCF);
	LCDWriteReg(0xD328,0x02);
	LCDWriteReg(0xD329,0xDE);
	LCDWriteReg(0xD32A,0x02);
	LCDWriteReg(0xD32B,0xF2);
	LCDWriteReg(0xD32C,0x02);
	LCDWriteReg(0xD32D,0xFE);
	LCDWriteReg(0xD32E,0x03);
	LCDWriteReg(0xD32F,0x10);
	LCDWriteReg(0xD330,0x03);
	LCDWriteReg(0xD331,0x33);
	LCDWriteReg(0xD332,0x03);
	LCDWriteReg(0xD333,0x6D);
	LCDWriteReg(0xD400,0x00);
	LCDWriteReg(0xD401,0x33);
	LCDWriteReg(0xD402,0x00);
	LCDWriteReg(0xD403,0x34);
	LCDWriteReg(0xD404,0x00);
	LCDWriteReg(0xD405,0x3A);
	LCDWriteReg(0xD406,0x00);
	LCDWriteReg(0xD407,0x4A);
	LCDWriteReg(0xD408,0x00);
	LCDWriteReg(0xD409,0x5C);
	LCDWriteReg(0xD40A,0x00);
	LCDWriteReg(0xD40B,0x81);

	LCDWriteReg(0xD40C,0x00);
	LCDWriteReg(0xD40D,0xA6);
	LCDWriteReg(0xD40E,0x00);
	LCDWriteReg(0xD40F,0xE5);
	LCDWriteReg(0xD410,0x01);
	LCDWriteReg(0xD411,0x13);
	LCDWriteReg(0xD412,0x01);
	LCDWriteReg(0xD413,0x54);
	LCDWriteReg(0xD414,0x01);
	LCDWriteReg(0xD415,0x82);
	LCDWriteReg(0xD416,0x01);
	LCDWriteReg(0xD417,0xCA);
	LCDWriteReg(0xD418,0x02);
	LCDWriteReg(0xD419,0x00);
	LCDWriteReg(0xD41A,0x02);
	LCDWriteReg(0xD41B,0x01);
	LCDWriteReg(0xD41C,0x02);
	LCDWriteReg(0xD41D,0x34);
	LCDWriteReg(0xD41E,0x02);
	LCDWriteReg(0xD41F,0x67);
	LCDWriteReg(0xD420,0x02);
	LCDWriteReg(0xD421,0x84);
	LCDWriteReg(0xD422,0x02);
	LCDWriteReg(0xD423,0xA4);
	LCDWriteReg(0xD424,0x02);
	LCDWriteReg(0xD425,0xB7);
	LCDWriteReg(0xD426,0x02);
	LCDWriteReg(0xD427,0xCF);
	LCDWriteReg(0xD428,0x02);
	LCDWriteReg(0xD429,0xDE);
	LCDWriteReg(0xD42A,0x02);
	LCDWriteReg(0xD42B,0xF2);
	LCDWriteReg(0xD42C,0x02);
	LCDWriteReg(0xD42D,0xFE);
	LCDWriteReg(0xD42E,0x03);
	LCDWriteReg(0xD42F,0x10);
	LCDWriteReg(0xD430,0x03);
	LCDWriteReg(0xD431,0x33);
	LCDWriteReg(0xD432,0x03);
	LCDWriteReg(0xD433,0x6D);
	LCDWriteReg(0xD500,0x00);
	LCDWriteReg(0xD501,0x33);
	LCDWriteReg(0xD502,0x00);
	LCDWriteReg(0xD503,0x34);
	LCDWriteReg(0xD504,0x00);
	LCDWriteReg(0xD505,0x3A);
	LCDWriteReg(0xD506,0x00);
	LCDWriteReg(0xD507,0x4A);
	LCDWriteReg(0xD508,0x00);
	LCDWriteReg(0xD509,0x5C);
	LCDWriteReg(0xD50A,0x00);
	LCDWriteReg(0xD50B,0x81);

	LCDWriteReg(0xD50C,0x00);
	LCDWriteReg(0xD50D,0xA6);
	LCDWriteReg(0xD50E,0x00);
	LCDWriteReg(0xD50F,0xE5);
	LCDWriteReg(0xD510,0x01);
	LCDWriteReg(0xD511,0x13);
	LCDWriteReg(0xD512,0x01);
	LCDWriteReg(0xD513,0x54);
	LCDWriteReg(0xD514,0x01);
	LCDWriteReg(0xD515,0x82);
	LCDWriteReg(0xD516,0x01);
	LCDWriteReg(0xD517,0xCA);
	LCDWriteReg(0xD518,0x02);
	LCDWriteReg(0xD519,0x00);
	LCDWriteReg(0xD51A,0x02);
	LCDWriteReg(0xD51B,0x01);
	LCDWriteReg(0xD51C,0x02);
	LCDWriteReg(0xD51D,0x34);
	LCDWriteReg(0xD51E,0x02);
	LCDWriteReg(0xD51F,0x67);
	LCDWriteReg(0xD520,0x02);
	LCDWriteReg(0xD521,0x84);
	LCDWriteReg(0xD522,0x02);
	LCDWriteReg(0xD523,0xA4);
	LCDWriteReg(0xD524,0x02);
	LCDWriteReg(0xD525,0xB7);
	LCDWriteReg(0xD526,0x02);
	LCDWriteReg(0xD527,0xCF);
	LCDWriteReg(0xD528,0x02);
	LCDWriteReg(0xD529,0xDE);
	LCDWriteReg(0xD52A,0x02);
	LCDWriteReg(0xD52B,0xF2);
	LCDWriteReg(0xD52C,0x02);
	LCDWriteReg(0xD52D,0xFE);
	LCDWriteReg(0xD52E,0x03);
	LCDWriteReg(0xD52F,0x10);
	LCDWriteReg(0xD530,0x03);
	LCDWriteReg(0xD531,0x33);
	LCDWriteReg(0xD532,0x03);
	LCDWriteReg(0xD533,0x6D);
	LCDWriteReg(0xD600,0x00);
	LCDWriteReg(0xD601,0x33);
	LCDWriteReg(0xD602,0x00);
	LCDWriteReg(0xD603,0x34);
	LCDWriteReg(0xD604,0x00);
	LCDWriteReg(0xD605,0x3A);
	LCDWriteReg(0xD606,0x00);
	LCDWriteReg(0xD607,0x4A);
	LCDWriteReg(0xD608,0x00);
	LCDWriteReg(0xD609,0x5C);
	LCDWriteReg(0xD60A,0x00);
	LCDWriteReg(0xD60B,0x81);

	LCDWriteReg(0xD60C,0x00);
	LCDWriteReg(0xD60D,0xA6);
	LCDWriteReg(0xD60E,0x00);
	LCDWriteReg(0xD60F,0xE5);
	LCDWriteReg(0xD610,0x01);
	LCDWriteReg(0xD611,0x13);
	LCDWriteReg(0xD612,0x01);
	LCDWriteReg(0xD613,0x54);
	LCDWriteReg(0xD614,0x01);
	LCDWriteReg(0xD615,0x82);
	LCDWriteReg(0xD616,0x01);
	LCDWriteReg(0xD617,0xCA);
	LCDWriteReg(0xD618,0x02);
	LCDWriteReg(0xD619,0x00);
	LCDWriteReg(0xD61A,0x02);
	LCDWriteReg(0xD61B,0x01);
	LCDWriteReg(0xD61C,0x02);
	LCDWriteReg(0xD61D,0x34);
	LCDWriteReg(0xD61E,0x02);
	LCDWriteReg(0xD61F,0x67);
	LCDWriteReg(0xD620,0x02);
	LCDWriteReg(0xD621,0x84);
	LCDWriteReg(0xD622,0x02);
	LCDWriteReg(0xD623,0xA4);
	LCDWriteReg(0xD624,0x02);
	LCDWriteReg(0xD625,0xB7);
	LCDWriteReg(0xD626,0x02);
	LCDWriteReg(0xD627,0xCF);
	LCDWriteReg(0xD628,0x02);
	LCDWriteReg(0xD629,0xDE);
	LCDWriteReg(0xD62A,0x02);
	LCDWriteReg(0xD62B,0xF2);
	LCDWriteReg(0xD62C,0x02);
	LCDWriteReg(0xD62D,0xFE);
	LCDWriteReg(0xD62E,0x03);
	LCDWriteReg(0xD62F,0x10);
	LCDWriteReg(0xD630,0x03);
	LCDWriteReg(0xD631,0x33);
	LCDWriteReg(0xD632,0x03);
	LCDWriteReg(0xD633,0x6D);
	//LV2 Page 0 enable
	LCDWriteReg(0xF000,0x55);
	LCDWriteReg(0xF001,0xAA);
	LCDWriteReg(0xF002,0x52);
	LCDWriteReg(0xF003,0x08);
	LCDWriteReg(0xF004,0x00);
	//Display control
	LCDWriteReg(0xB100, 0xCC);
	LCDWriteReg(0xB101, 0x00);
	//Source hold time
	LCDWriteReg(0xB600,0x05);
	//Gate EQ control
	LCDWriteReg(0xB700,0x70);
	LCDWriteReg(0xB701,0x70);
	//Source EQ control (Mode 2)
	LCDWriteReg(0xB800,0x01);
	LCDWriteReg(0xB801,0x03);
	LCDWriteReg(0xB802,0x03);
	LCDWriteReg(0xB803,0x03);
	//Inversion mode (2-dot)
	LCDWriteReg(0xBC00,0x02);
	LCDWriteReg(0xBC01,0x00);
	LCDWriteReg(0xBC02,0x00);
	//Timing control 4H w/ 4-delay
	LCDWriteReg(0xC900,0xD0);
	LCDWriteReg(0xC901,0x02);
	LCDWriteReg(0xC902,0x50);
	LCDWriteReg(0xC903,0x50);
	LCDWriteReg(0xC904,0x50);
	LCDWriteReg(0x3500,0x00);
	LCDWriteReg(0x3A00,0x55);  //16-bit/pixel
	
	LCDWriteCmd(0x1100);
	Delay(100);
	LCDWriteCmd(0x2900);
	
	/* Horizontal, NT35510, 5.9.3 */
	//LCDWriteReg(0x3600, ((0 << 7) | (1 << 6) | 1 << 5));	/* dir: Y, X, V */
	LCDWriteReg(0x3600, ((1 << 7) | (0 << 6) | 1 << 5));	/* dir: Y, X, V */
}

//初始化lcd
//该初始化函数可以初始化各种ILI93XX液晶,但是其他函数是基于ILI9320的!!!
//在其他型号的驱动芯片上没有测试! 
void LCDInit(void)
{ 	

	u16 i;
	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef pRead, pWrite;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* Enable DMA clock */
	RCC_AHBPeriphClockCmd(LCD_DMA_CLOCK, ENABLE);
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_FSMC, ENABLE);
	
	/* LCD reset */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	/* FSMC_D2, D3, NOE, NWE, D13, D14, D15, A16, A17, A18, D0, D1 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | 
								GPIO_Pin_4 | GPIO_Pin_5 |
								GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | 
								GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | 
								GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOD, &GPIO_InitStructure);

	/* FSMC_NBL0, NBL1, FSMC_D4, D5, D16, D17, D8, D9, D10, D11, D12, D13 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | 
								GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
								GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
								GPIO_Pin_15;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
	/* FSMC_A0, A1, A2, A3, A4, A5, A6, A7, A8, A9 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |  GPIO_Pin_1 |  GPIO_Pin_2 |  GPIO_Pin_3 |
						GPIO_Pin_4 | GPIO_Pin_5 | 
						GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	

	/* FSMC_A10, A11, A12, A13, A14, A15, NE3, NE4 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |  GPIO_Pin_1 |  GPIO_Pin_2 |  GPIO_Pin_3 |
						GPIO_Pin_4 | GPIO_Pin_5 | 
						GPIO_Pin_10 | GPIO_Pin_12;
	GPIO_Init(GPIOG, &GPIO_InitStructure);

/*-- FSMC Configuration ------------------------------------------------------*/

	/*----------------------- SRAM Bank 3 ----------------------------------------*/
	pRead.FSMC_AddressSetupTime = 0;
	pRead.FSMC_AddressHoldTime = 0;
	pRead.FSMC_DataSetupTime = 3;
	pRead.FSMC_BusTurnAroundDuration = 0;
	pRead.FSMC_CLKDivision = 0;
	pRead.FSMC_DataLatency = 0;
	pRead.FSMC_AccessMode = FSMC_AccessMode_A;
	pWrite = pRead;

    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM3;//  这里我们使用NE3 ，也就对应BTCR[4],[5]。
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable; 
    FSMC_NORSRAMInitStructure.FSMC_MemoryType =FSMC_MemoryType_SRAM;// FSMC_MemoryType_SRAM;  //SRAM   
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;//存储器数据宽度为16bit  
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode =FSMC_BurstAccessMode_Disable;// FSMC_BurstAccessMode_Disable; 
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait=FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;   
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;  
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;	//存储器写使能 
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;  
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable; // 读写使用相同的时序
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;  
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &pRead;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &pWrite; //读写同样时序

    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  //初始化FSMC配置

   	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM3, ENABLE);  // 使能BANK3										  


	/*----------------------- SRAM Bank 4 ----------------------------------------*/
	/* FSMC_Bank1_NORSRAM4 configuration */
#if !USE_7
	pRead.FSMC_AddressSetupTime = 1;
	pRead.FSMC_AddressHoldTime = 0;
	pRead.FSMC_DataSetupTime = 4;
	pRead.FSMC_BusTurnAroundDuration = 0;
	pRead.FSMC_CLKDivision = 0;
	pRead.FSMC_DataLatency = 0;
	pRead.FSMC_AccessMode = FSMC_AccessMode_A;
	pWrite = pRead;
#else
	pRead.FSMC_AddressSetupTime = 1;
	pRead.FSMC_AddressHoldTime = 0;
	pRead.FSMC_DataSetupTime = 4;
	pRead.FSMC_BusTurnAroundDuration = 0;
	pRead.FSMC_CLKDivision = 0;
	pRead.FSMC_DataLatency = 0;
	pRead.FSMC_AccessMode = FSMC_AccessMode_A;
	pWrite = pRead;

#endif
	//pWrite.FSMC_DataSetupTime = 4;
	/* Color LCD configuration ------------------------------------
	 LCD configured as follow:
		- Data/Address MUX = Disable
		- Memory Type = SRAM
		- Data Width = 16bit
		- Write Operation = Enable
		- Extended Mode = Enable
		- Asynchronous Wait = Disable */
	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM4;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;  
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &pRead;
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &pWrite;
	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);  
	/* BANK 4 (of NOR/SRAM Bank 1~4) is enabled */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM4, ENABLE);
	Delay(100);

	//return ;
#if USE_7

	GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_RESET);
	Delay(100);
	GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);
	while (i < 0X1FFF) 
	{
		if (LCDReadData()&0x0001)
		{
			break;//等待控制器准备好
		}
		i++;
  	} 
	LCDSetScanDir(L2R_U2D);		//设置默认扫描方向.	 
	LCDSetDisplayLayer(0);	 	//显示层为0
	LCDSetOperateLayer(0);		//操作层也为0
	LCDSetXEnd(LCD_WIDTH - 1);
	LCDSetBackLight(63);		//背光设置为最亮
	LCDClear(RGB(0xBF, 0xBF, 0xBF));			//清屏
#else
	while (i < 0X1FFF) 
	{
		u16 u16Value = LCDReadReg(0xDB00);
		if (u16Value != 0x0000)
		{
			break;//等待控制器准备好
		}
		i++;
  	} 
	
	NT35510Init();
	
	GPIO_WriteBit(GPIOB, GPIO_Pin_0, Bit_SET);

	LCDSetCursor(0, 0);
	LCDSetXEnd(LCD_WIDTH - 1);
	LCDSetYEnd(LCD_HEIGHT - 1);
	
	LCDClear(RGB(0xBF, 0xBF, 0xBF));

#endif	
	g_boIsLCDCanUse = true;
}  


/* write some data to the LCD by using DMA */
void LCDDMAWrite(const u16 *pBuf, u32 u32Len)	
{
#if 1
	/* peripheral --> memory */
	while (u32Len != 0)
	{
		u32 u32Tmp = (u32Len > 65535) ? 65535: u32Len;
		u32Len -= u32Tmp;
		
		LCD_DMA->CPAR = (u32)pBuf;
		pBuf = (const u16 *)((u32)pBuf + u32Tmp * 2); 
		LCD_DMA->CMAR = (u32)(&(LCD->LCDRam));
		
		LCD_DMA->CNDTR = u32Tmp;
		
		LCD_DMA->CCR = DMA_PeripheralInc_Enable | DMA_PeripheralDataSize_HalfWord |
						DMA_MemoryDataSize_HalfWord | DMA_Priority_High | DMA_M2M_Enable;
		/* enable */
		LCD_DMA->CCR |= DMA_CCR1_EN;
		
		/* Check if DMA channel transfer is finished */
		while(!DMA_GetFlagStatus(LCD_DMA_FINISH_FLAG));

		/* Clear DMA channel transfer complete flag bit */
		DMA_ClearFlag(LCD_DMA_FINISH_FLAG);

		/* disable */
		LCD_DMA->CCR = 0;	
	}
#else
	/* peripheral --> memory */
	LCD_DMA->CPAR = (u32)pBuf;
	LCD_DMA->CMAR = (u32)(&(LCD->LCDRam));
	
	LCD_DMA->CNDTR = u32Len;
	
	LCD_DMA->CCR = DMA_PeripheralInc_Enable | DMA_PeripheralDataSize_HalfWord |
					DMA_MemoryDataSize_HalfWord | DMA_Priority_High | DMA_M2M_Enable;
	/* enable */
	LCD_DMA->CCR |= DMA_CCR1_EN;
	
	/* Check if DMA channel transfer is finished */
	while(!DMA_GetFlagStatus(LCD_DMA_FINISH_FLAG));

	/* Clear DMA channel transfer complete flag bit */
	DMA_ClearFlag(LCD_DMA_FINISH_FLAG);

	/* disable */
	LCD_DMA->CCR = 0;
#endif	
}

static u16 s_u16Value = ~0; /* when DMA transfer the value must not be changed by int */ 
/* write some data to the LCD by using DMA */
void LCDDMAWriteSameValue(u16 u16Value, u32 u32Len)	
{
	/* peripheral --> memory */
	s_u16Value = u16Value;
	while (u32Len != 0)
	{
		u16 u16Tmp = (u32Len > 65535) ? 65535: u32Len;
		u32Len -= u16Tmp;
		LCD_DMA->CPAR = (u32)(&s_u16Value);
		LCD_DMA->CMAR = (u32)(&(LCD->LCDRam));
		
		LCD_DMA->CNDTR = u16Tmp;
		
		LCD_DMA->CCR = DMA_PeripheralDataSize_HalfWord |
						DMA_MemoryDataSize_HalfWord | DMA_Priority_High | DMA_M2M_Enable;
		/* enable */
		LCD_DMA->CCR |= DMA_CCR1_EN;
		
		/* Check if DMA channel transfer is finished */
		while(!DMA_GetFlagStatus(LCD_DMA_FINISH_FLAG));

		/* Clear DMA channel transfer complete flag bit */
		DMA_ClearFlag(LCD_DMA_FINISH_FLAG);

		/* disable */
		LCD_DMA->CCR = 0;	
	}
}

//清屏函数
//color:要清屏的填充色
void LCDClear(u16 u16Color)
{
#if 0
	u32 i=0;      

	LCDSetCursor(0,0);	//设置光标位置 
	LCDWriteRAMPrepare();     //开始写入GRAM	 	  
	for(i = 0; i < (LCD_WIDTH * LCD_HEIGHT); i++)
	{
		LCD->LCDRam = u16Color;	   
	}
#else
	LCDSetCursor(0,0);	//设置光标位置 
	LCDSetXEnd(LCD_WIDTH - 1);
	LCDWriteRAMPrepare();     //开始写入GRAM	 	  
	LCDDMAWriteSameValue(u16Color, LCD_WIDTH * LCD_HEIGHT);
#endif
}  



