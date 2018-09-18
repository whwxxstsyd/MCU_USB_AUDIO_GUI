#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_api.h"
#include "I2C.h"
#include "lcd.h"
#include "gt9147.h"

#define GT9147_CTRL_PIN				GPIO_Pin_11
#define GT9147_CTRL_PIN_PORT		GPIOF

#define GT9147_INT_PIN				GPIO_Pin_10
#define GT9147_INT_PIN_PORT			GPIOF

#define GT9147_EXIT_LINE_CONFIG()	GPIO_EXTILineConfig(GPIO_PortSourceGPIOF, GPIO_PinSource10)
#define GT9147_EXIT_LINE			EXTI_Line10
#define GT9147_EXIT_IRQ				EXTI15_10_IRQn
#define GT9147_EXIT_IRQHandler		EXTI15_10_IRQHandler


#define GT9147_CMD_WR		0xBA
#define GT9147_CMD_RD		0xBB

#ifndef GT9147_MAX_TOUCH
#define GT9147_MAX_TOUCH    5		//电容触摸屏最大支持的点数
#endif

#define GT_CTRL_REG 	0X8040   	//GT9147控制寄存器
#define GT_CFGS_REG 	0X8047   	//GT9147配置起始地址寄存器
#define GT_CHECK_REG 	0X80FF   	//GT9147校验和寄存器
#define GT_PID_REG 		0X8140   	//GT9147产品ID寄存器

#define GT_GSTID_REG 	0X814E   	//GT9147当前检测到的触摸情况
#define GT_TP1_REG 		0X8150  	//第一个触摸点数据地址
#define GT_TP2_REG 		0X8158		//第二个触摸点数据地址
#define GT_TP3_REG 		0X8160		//第三个触摸点数据地址
#define GT_TP4_REG 		0X8168		//第四个触摸点数据地址
#define GT_TP5_REG 		0X8170		//第五个触摸点数据地址  

#define GT9147_POINT_BUF_SIZE	(GT9147_MAX_TOUCH * 4)


#define GT9147ReadReg(reg, buf, len)	I2CRead(0x5D, reg, 2, len, buf)
#define GT9147WriteReg(reg, buf, len) 	I2CWriteBuffer(0x5D, reg, 2, len, buf)


const u8 c_GT9147ConfigData[]=
{
	0x60,0xE0,0x01,0x20,0x03,0x05,0x35,0x00,0x02,0x08,
	0x1E,0x08,0x50,0x3C,0x0F,0x05,0x00,0x00,0xFF,0x67,
	0x50,0x00,0x00,0x18,0x1A,0x1E,0x14,0x89,0x28,0x0A,
	0x30,0x2E,0xBB,0x0A,0x03,0x00,0x00,0x02,0x33,0x1D,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x32,0x00,0x00,
	0x2A,0x1C,0x5A,0x94,0xC5,0x02,0x07,0x00,0x00,0x00,
	0xB5,0x1F,0x00,0x90,0x28,0x00,0x77,0x32,0x00,0x62,
	0x3F,0x00,0x52,0x50,0x00,0x52,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,
	0x0F,0x03,0x06,0x10,0x42,0xF8,0x0F,0x14,0x00,0x00,
	0x00,0x00,0x1A,0x18,0x16,0x14,0x12,0x10,0x0E,0x0C,
	0x0A,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x29,0x28,0x24,0x22,0x20,0x1F,0x1E,0x1D,
	0x0E,0x0C,0x0A,0x08,0x06,0x05,0x04,0x02,0x00,0xFF,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,
};	 

const u16 c_u16GT9147PointReg[5] = 
{
	GT_TP1_REG,
	GT_TP2_REG,
	GT_TP3_REG,
	GT_TP4_REG,
	GT_TP5_REG
};


static union
{
	u8 u8Buf[GT9147_MAX_TOUCH][4];
	StPoint stPoint[GT9147_MAX_TOUCH];
}s_unPoint;

static u8 s_u8PointCnt = 0;

//tatic u8 s_u8Buf[GT9147_MAX_TOUCH][4];

u8 GT9147SendCfg(u8 mode)
{
	u8 u8Buf[2];
	u8 i = 0;
	u8Buf[0] = 0;
	u8Buf[1] = mode;	//是否写入到GT9147 FLASH?  即是否掉电保存
	for(i=0; i< sizeof(c_GT9147ConfigData); i++)
	{
		u8Buf[0] += c_GT9147ConfigData[i];//计算校验和
	}
    u8Buf[0]=(~u8Buf[0]) + 1;
	
	GT9147WriteReg(GT_CFGS_REG, (uint8_t *)c_GT9147ConfigData, sizeof(c_GT9147ConfigData));//发送寄存器配置
	GT9147WriteReg(GT_CHECK_REG, u8Buf, 2);//写入校验和,和配置更新标记
	return 0;
} 

bool GT9147Init(void)
{
	char c8ID[8] = {0};
	
	EXTI_InitTypeDef   EXTI_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GT9147_CTRL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GT9147_CTRL_PIN_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GT9147_INT_PIN;
	GPIO_Init(GT9147_INT_PIN_PORT, &GPIO_InitStructure);

	
	GPIO_WriteBit(GT9147_INT_PIN_PORT, GT9147_INT_PIN, Bit_RESET);

	GPIO_WriteBit(GT9147_CTRL_PIN_PORT, GT9147_CTRL_PIN, Bit_RESET);
	Delay(100);
	GPIO_WriteBit(GT9147_CTRL_PIN_PORT, GT9147_CTRL_PIN, Bit_SET);
	Delay(20);
	
	GT9147ReadReg(GT_PID_REG, (uint8_t *)c8ID, 4);
	if(strcmp(c8ID, "9147") ==0 )	//ID==9147
	{
		u8 u8Tmp[2] = {0x02};
		
		GT9147WriteReg(GT_CTRL_REG, u8Tmp, 1);//软复位GT9147
 		GT9147ReadReg(GT_CFGS_REG, u8Tmp, 1);//读取GT_CFGS_REG寄存器
		if(u8Tmp[0] < 0x60)	//默认版本比较低,需要更新flash配置
		{
			GT9147SendCfg(1);//更新并保存配置
		}
		Delay(10);
		u8Tmp[0] = 0x00;	 
		GT9147WriteReg(GT_CTRL_REG,u8Tmp,1);	//结束复位   	
	}
	else 
	{
		return false;
	}

	Delay(100);

	GPIO_InitStructure.GPIO_Pin = GT9147_INT_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GT9147_INT_PIN_PORT, &GPIO_InitStructure);
	
	/* Connect EXTI Line to the pin */
	GT9147_EXIT_LINE_CONFIG();

	/* Configure EXTI0 line */
	EXTI_InitStructure.EXTI_Line = GT9147_EXIT_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	return true;
}

static bool s_boNeedGetPoint = false;

void GT9147_EXIT_IRQHandler(void)
{
	if(EXTI_GetITStatus(GT9147_EXIT_LINE) != RESET)
	{
		s_boNeedGetPoint = true;
		GT9147GetPoint(1);
		/* Clear the  EXTI line 0 pending bit */
		EXTI_ClearITPendingBit(GT9147_EXIT_LINE);
	}
} 

__weak void SrceenProtectReset(void)
{
}

bool GT9147GetPoint(uint8_t u8Mode)
{
	u8 u8Tmp = 0;
	if (!s_boNeedGetPoint)
	{
		return false;
	}
	s_boNeedGetPoint = false;
	if (!GT9147ReadReg(GT_GSTID_REG, &u8Mode, 1))
	{
		return false;
	}
	
	//if((u8Mode & 0x80) && ((u8Mode & 0x0F) < 6))
	{
		u8Tmp = 0;
		GT9147WriteReg(GT_GSTID_REG, &u8Tmp, 1);//清标志 				
	}
	
//	if ((u8Mode & 0x80) == 0)
//	{
//		return false;
//	}
		

	u8Tmp = u8Mode & 0x0F;
	
	if(u8Tmp != 0)
	{
		u32 i;
		for (i = 0; i < GT9147_MAX_TOUCH; i++)
		{
			u16 u16Tmp;
			GT9147ReadReg(c_u16GT9147PointReg[i], s_unPoint.u8Buf[i],4);			//读取XY坐标值
		
			u16Tmp = s_unPoint.stPoint[i].u16X;	
#if 0			
			s_unPoint.stPoint[i].u16X = s_unPoint.stPoint[i].u16Y;
			s_unPoint.stPoint[i].u16Y = LCD_HEIGHT - 1 - u16Tmp;
#else
			s_unPoint.stPoint[i].u16X = LCD_WIDTH - 1 -  s_unPoint.stPoint[i].u16Y;
			s_unPoint.stPoint[i].u16Y = u16Tmp;
#endif
			
		}
		s_u8PointCnt = u8Tmp;
		SrceenProtectReset();
	}	
	else
	{
		s_u8PointCnt = 0;	
	}

	return true;

}


bool GT9147Scan(StPoint stPoint[GT9147_MAX_TOUCH], uint8_t *pCnt)
{

	__disable_irq();
	memcpy(stPoint, s_unPoint.stPoint, sizeof(StPoint) * GT9147_MAX_TOUCH);
	if (pCnt != NULL)
	{
		*pCnt = s_u8PointCnt;
	}
	__enable_irq();
	
	return true;
}

