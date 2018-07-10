/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����extern_peripheral.c
* ժҪ: �������ͷ
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2018��06��29��
*******************************************************************************/
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "stm32f10x_conf.h"

#include "extern_peripheral.h"

void LDP6803Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed =   GPIO_Speed_2MHz;
	
	/* GMODE 0: ��٤��256�����Ե���*/
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOA, GPIO_Pin_6, Bit_RESET);

	/* OMODE 1:�ں���/��ѹģʽ */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOA, GPIO_Pin_5, Bit_SET);

	/* CMODE 1:�ڲ����� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
	
	
	/* DIN */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_RESET);
	
	/* DCLK */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_RESET);
}

#define CLR_CLK		GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_RESET)
#define SET_CLK		GPIO_WriteBit(GPIOA, GPIO_Pin_7, Bit_SET)
#define CLR_D0		GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_RESET)
#define SET_D0		GPIO_WriteBit(GPIOA, GPIO_Pin_8, Bit_SET)

void LDPWrite(uint8_t R, uint8_t G, uint8_t B)
{
	uint8_t i, j, n;
	uint8_t u8Arr[3];
	
	u8Arr[0] = G;
	u8Arr[1] = R;
	u8Arr[2] = B;
	
	CLR_CLK;
	CLR_D0;
	
	for (i = 0; i < 32; i++)
	{
		SET_CLK;
		CLR_CLK;
	}
	
	for (n = 0; n < 2; n++)
	{
		SET_D0;
		SET_CLK;
		CLR_CLK;			
		
		for (j = 0; j < 3; j++)
		{
			uint8_t u8Tmp = u8Arr[j];
			uint8_t u8Mask = 0x10;
			
			
			for (i = 0; i < 5; i++)
			{
				if ((u8Tmp & u8Mask) == 0)
					CLR_D0;
				else
					SET_D0;
				SET_CLK;
				CLR_CLK;
				u8Mask >>= 1;
			}
		}
	}
	
	CLR_D0;
	for (n = 0; n < 2; n++)
	{
		SET_CLK;
		CLR_CLK;
	}
	
}

void LDPTest(void)
{
	static u8 u8Tmp = 0, R, G, B;
	u8 u8RGB = (u8Tmp >> 5) & 0x03;
	
	R = G = B = 0;
	if (u8RGB == 0)
	{
		R = u8Tmp & 0x1F;
	}
	else if (u8RGB == 0x01)
	{
		G = u8Tmp & 0x1F;
	}
	else if (u8RGB == 0x02)
	{
		B = u8Tmp & 0x1F;
	}
	else
	{
		u8Tmp = 0;
		return;
	}
	
	LDPWrite(R, G, B);
	u8Tmp++;
}

void KeyboardPowerInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed =   GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOE, &GPIO_InitStructure);
	
}

void KeyboardPowerEnable(bool boIsEnable)
{
	GPIO_WriteBit(GPIOE, GPIO_Pin_2, boIsEnable ? Bit_RESET : Bit_SET);	
}

void KeyboardConnectInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed =   GPIO_Speed_50MHz;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void KeyboardConnectSetMode(uint8_t u8Mode)
{
	GPIO_WriteBit(GPIOA, GPIO_Pin_15, u8Mode != 0 ? Bit_SET : Bit_RESET);		
}


void ExternPeripheralInit(void)
{
	LDP6803Init();
	LDPWrite(0, 0, 0);
	
	KeyboardPowerInit();
	KeyboardPowerEnable(true);
	
	KeyboardConnectInit();
	KeyboardConnectSetMode(0);
}

