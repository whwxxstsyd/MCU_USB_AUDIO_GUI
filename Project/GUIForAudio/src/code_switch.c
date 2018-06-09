/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：code_swtich.c
* 摘要: 键盘以及LED刷新程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"
#include "io_buf_ctrl.h"
#include "app_port.h"

#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "key_led_ctrl.h"

#define CANCLE_TIME			(1)

#define CODE_SWITCH1_PIN_A				GPIO_Pin_0
#define CODE_SWITCH1_PIN_A_PORT			GPIOC
#define CODE_SWITCH1_PIN_B				GPIO_Pin_13
#define CODE_SWITCH1_PIN_B_PORT			GPIOC

#define CODE_SWITCH1_INT_SRC			GPIO_PinSource0
#define CODE_SWITCH1_INT_SRC_PORT		GPIO_PortSourceGPIOC
#define CODE_SWITCH1_INT_LINE			EXTI_Line0
#define CODE_SWITCH1_INT_CHANNEL		EXTI0_IRQn



#define CODE_SWITCH2_PIN_A				GPIO_Pin_1
#define CODE_SWITCH2_PIN_A_PORT			GPIOC
#define CODE_SWITCH2_PIN_B				GPIO_Pin_14
#define CODE_SWITCH2_PIN_B_PORT			GPIOC

#define CODE_SWITCH2_INT_SRC			GPIO_PinSource1
#define CODE_SWITCH2_INT_SRC_PORT		GPIO_PortSourceGPIOC
#define CODE_SWITCH2_INT_LINE			EXTI_Line1
#define CODE_SWITCH2_INT_CHANNEL		EXTI1_IRQn


static uint8_t s_u8SwitchMode[CODE_SWITCH_MAX] = {0};
#define s_u8SwitchMode1		s_u8SwitchMode[0]
#define s_u8SwitchMode2		s_u8SwitchMode[1]
#define s_u8SwitchMode3		s_u8SwitchMode[2]
#define s_u8SwitchMode4		s_u8SwitchMode[3]


static StCodeSwitchState 	s_stCodeSwitch[CODE_SWITCH_MAX];
#define s_stCodeSwitch1		s_stCodeSwitch[0]
#define s_stCodeSwitch2		s_stCodeSwitch[1]
#define s_stCodeSwitch3		s_stCodeSwitch[2]
#define s_stCodeSwitch4		s_stCodeSwitch[3]

const u16 c_u16CodeSwitchMaxValue[CODE_SWITCH_MAX] = 
{
	CODE_SWITCH1_MAX_VALUE,
	CODE_SWITCH2_MAX_VALUE,
};

static void CodeSwitchPinInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed =   GPIO_Speed_2MHz;
	
	/* switch1 */
	GPIO_InitStructure.GPIO_Pin = CODE_SWITCH1_PIN_A;
	GPIO_Init(CODE_SWITCH1_PIN_A_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = CODE_SWITCH1_PIN_B;
	GPIO_Init(CODE_SWITCH1_PIN_B_PORT, &GPIO_InitStructure);


	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	
	/* Connect EXTI for switch 1 */
	GPIO_EXTILineConfig(CODE_SWITCH1_INT_SRC_PORT, CODE_SWITCH1_INT_SRC);

	EXTI_InitStructure.EXTI_Line = CODE_SWITCH1_INT_LINE;
	EXTI_Init(&EXTI_InitStructure);


	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	/* Enable and set switch 1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = CODE_SWITCH1_INT_CHANNEL;
	NVIC_Init(&NVIC_InitStructure);


	/* switch2 */
	GPIO_InitStructure.GPIO_Pin = CODE_SWITCH2_PIN_A;
	GPIO_Init(CODE_SWITCH2_PIN_A_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = CODE_SWITCH2_PIN_B;
	GPIO_Init(CODE_SWITCH2_PIN_B_PORT, &GPIO_InitStructure);


	/* Connect EXTI for switch 1 */
	GPIO_EXTILineConfig(CODE_SWITCH2_INT_SRC_PORT, CODE_SWITCH2_INT_SRC);

	EXTI_InitStructure.EXTI_Line = CODE_SWITCH2_INT_LINE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set switch 1 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = CODE_SWITCH2_INT_CHANNEL;
	NVIC_Init(&NVIC_InitStructure);

}

void ChangeCodeSwitch1(bool boIsCW)
{
	if (boIsCW)
	{
		s_stCodeSwitch1.u16Cnt++;
	}
	else
	{
		s_stCodeSwitch1.u16Cnt--;
	}
	s_stCodeSwitch1.u16Dir = boIsCW;
	
	if (s_stCodeSwitch1.u16Cnt > CODE_SWITCH1_MAX_VALUE)
	{
		if (boIsCW)
		{
			s_stCodeSwitch1.u16Cnt = 0;
		}
		else
		{
			s_stCodeSwitch1.u16Cnt = CODE_SWITCH1_MAX_VALUE;
		}
	}
}

void FlushCodeSwitch1(void)
{
	bool boIsCW;
	u16 u16PinA, u16PinB;

	u16PinB = CODE_SWITCH1_PIN_B_PORT->IDR;
	u16PinA = CODE_SWITCH1_PIN_A_PORT->IDR;

	u16PinA &= CODE_SWITCH1_PIN_A;
	u16PinB &= CODE_SWITCH1_PIN_B;

	if (u16PinA == 0)
	{
		if (u16PinB == 0)
		{
			boIsCW = true;
		}
		else
		{
			boIsCW = false;
		}	
	}
	else
	{
		if (u16PinB == 0)
		{
			boIsCW = false;
		}
		else
		{
			boIsCW = true;
		}	
	}
#if CODE_SWITCH1_REVERSE
	boIsCW = !boIsCW;
#endif
	ChangeCodeSwitch1(boIsCW);
	
	{
		u32 u32Time = s_stCodeSwitch1.u32TriggerTime;
		s_stCodeSwitch1.u32TriggerTime = g_u32SysTickCnt;
		u32Time  = SysTimeDiff(u32Time, g_u32SysTickCnt);
		if (u32Time > 250)
		{
			s_stCodeSwitch1.u16Speed = 1;
		}
		else
		{
			if (u32Time != 0)
			{
				s_stCodeSwitch1.u16Speed = 500 / u32Time;				
			}
			if (s_stCodeSwitch1.u16Speed > 80)
			{
				s_stCodeSwitch1.u16Speed = 80;
			}
		}
		
	}
	
}


void EXTI0_IRQHandler(void)
{
	if(EXTI_GetITStatus(CODE_SWITCH1_INT_LINE) != RESET)
	{	
		USE_CRITICAL();
		ENTER_CRITICAL();
		s_u8SwitchMode1 = 1;
		EXIT_CRITICAL();
		/* Clear the  pending bit */
		EXTI_ClearITPendingBit(CODE_SWITCH1_INT_LINE);
	}

}

void ChangeCodeSwitch2(bool boIsCW)
{
	if (boIsCW)
	{
		s_stCodeSwitch2.u16Cnt++;
	}
	else
	{
		s_stCodeSwitch2.u16Cnt--;
	}
	s_stCodeSwitch2.u16Dir = boIsCW;
	if (s_stCodeSwitch2.u16Cnt > CODE_SWITCH2_MAX_VALUE)
	{
		if (boIsCW)
		{
			s_stCodeSwitch2.u16Cnt = 0;
		}
		else
		{
			s_stCodeSwitch2.u16Cnt = CODE_SWITCH2_MAX_VALUE;
		}
	}

}

void FlushCodeSwitch2(void)
{
	bool boIsCW;
	u16 u16PinA, u16PinB;
	{
		int32_t i;
		for (i = 0; i < 120; i++);
	}

	u16PinB = CODE_SWITCH2_PIN_B_PORT->IDR;
	u16PinA = CODE_SWITCH2_PIN_A_PORT->IDR;

	u16PinA &= CODE_SWITCH2_PIN_A;
	u16PinB &= CODE_SWITCH2_PIN_B;

	if (u16PinA == 0)
	{
		if (u16PinB == 0)
		{
			boIsCW = true;
		}
		else
		{
			boIsCW = false;
		}	
	}
	else
	{
		if (u16PinB == 0)
		{
			boIsCW = false;
		}
		else
		{
			boIsCW = true;
		}	
	}
#if CODE_SWITCH2_REVERSE
	boIsCW = !boIsCW;
#endif
	ChangeCodeSwitch2(boIsCW);

	{
		u32 u32Time = s_stCodeSwitch2.u32TriggerTime;
		s_stCodeSwitch2.u32TriggerTime = g_u32SysTickCnt;
		u32Time  = SysTimeDiff(u32Time, g_u32SysTickCnt);
		if (u32Time > 250)
		{
			s_stCodeSwitch2.u16Speed = 1;
		}
		else
		{
			if (u32Time != 0)
			{
				s_stCodeSwitch2.u16Speed = 500 / u32Time;				
			}
			if (s_stCodeSwitch2.u16Speed > 80)
			{
				s_stCodeSwitch2.u16Speed = 80;
			}
		}
		
	}

}

void EXTI1_IRQHandler(void)
{
	if(EXTI_GetITStatus(CODE_SWITCH2_INT_LINE) != RESET)
	{	
		USE_CRITICAL();
		ENTER_CRITICAL();
		s_u8SwitchMode2 = 1;
		EXIT_CRITICAL();
		/* Clear the  pending bit */
		EXTI_ClearITPendingBit(CODE_SWITCH2_INT_LINE);
	}

}
/* 编码开关初始化,  */
void CodeSwitchInit(void)
{
	u32 i;
	CodeSwitchPinInit();

	for (i = 0; i < CODE_SWITCH_MAX; i++)
	{
		s_stCodeSwitch[i].u16Index = i;
		s_stCodeSwitch[i].u16Cnt = 0;
		s_stCodeSwitch[i].u16OldCnt = 0;		
	}
}



static bool CodeSwitchGetValueInner(StCodeSwitchState *pState)
{
	if (pState->u16Cnt != pState->u16OldCnt)
	{
		pState->u16OldCnt = pState->u16Cnt;
		return true;
	}
	return false;
}

u16 CodeSwitchPlus(u16 u16Index)
{
	USE_CRITICAL();
	u16 u16Cnt;
	if (u16Index > CODE_SWITCH_MAX)
	{
		return ~0;
	}

	u16Cnt = s_stCodeSwitch[u16Index].u16Cnt + 1;

	ENTER_CRITICAL();
	if (u16Cnt > c_u16CodeSwitchMaxValue[u16Index])
	{
		u16Cnt = 0;
	}
	s_stCodeSwitch[u16Index].u16Cnt = s_stCodeSwitch[u16Index].u16OldCnt = u16Cnt;
	EXIT_CRITICAL();
	return u16Cnt;
}


u16 CodeSwitchGetValue(u16 u16Index)
{
	return s_stCodeSwitch[u16Index].u16OldCnt;
}

u16 CodeSwitchSetValue(u16 u16Index, u16 u16Value)
{
	USE_CRITICAL();
	if (u16Index > CODE_SWITCH_MAX)
	{
		return ~0;
	}

	if (u16Value > c_u16CodeSwitchMaxValue[u16Index])
	{
		u16Value = c_u16CodeSwitchMaxValue[u16Index];
	}

	ENTER_CRITICAL();
	s_stCodeSwitch[u16Index].u16Cnt = s_stCodeSwitch[u16Index].u16OldCnt = u16Value;
	EXIT_CRITICAL();
	return u16Value;
	
}

void CodeSwitchFlush(void)
{
	{
		u8 u8SwitchMode[CODE_SWITCH_MAX] = {0};
		u8 i;
		USE_CRITICAL();
		ENTER_CRITICAL();
		
		for (i = 0; i < CODE_SWITCH_MAX; i++)
		{
			if (s_u8SwitchMode[i] != 0)
			{
				s_u8SwitchMode[i]++;
			}
			if (s_u8SwitchMode[i] == 3)
			{
				u8SwitchMode[i] = 1;
			}
			else if (s_u8SwitchMode[i] == (3 + 25))
			{
				s_u8SwitchMode[i] = 0;
				u8SwitchMode[i] = 2;				
			}
		}
		
		EXIT_CRITICAL();
		if (u8SwitchMode[0] == 1)
		{
			FlushCodeSwitch1();
			//s_stCodeSwitch1.u16Cnt = 1;
			//s_stCodeSwitch1.u16OldCnt = 0;
		}
		else if (u8SwitchMode[0] == 2)
		{
			ChangeCodeSwitch1(s_stCodeSwitch1.u16Dir);
			s_stCodeSwitch1.u16Cnt &= 0xFFFE;
			s_stCodeSwitch1.u16OldCnt = s_stCodeSwitch1.u16Cnt - 1;
		}
		
		if (u8SwitchMode[1] == 1)
		{
			FlushCodeSwitch2();
			//s_stCodeSwitch2.u16Cnt = 1;
			//s_stCodeSwitch2.u16OldCnt = 0;
		}
		else if (u8SwitchMode[1] == 2)
		{
			ChangeCodeSwitch2(s_stCodeSwitch2.u16Dir);
			s_stCodeSwitch2.u16Cnt &= 0xFFFE;
			s_stCodeSwitch2.u16OldCnt = s_stCodeSwitch2.u16Cnt - 1;
		}
		

	}
	u32 i;
	for (i = 0; i < CODE_SWITCH_MAX; i++)
	{
		if (CodeSwitchGetValueInner(s_stCodeSwitch + i))
		{
			StKeyMixIn stKey;
			stKey.emKeyType = _Key_CodeSwitch;
			memcpy(&(stKey.unKeyMixIn.stCodeSwitchState), s_stCodeSwitch + i, 
					sizeof(StCodeSwitchState));
			KeyBufWrite(&stKey);

		}
	}
}


