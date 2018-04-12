/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����code_switch.h
* ժҪ: ���뿪��ͷ�ļ�
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _CODE_SWITCH_H_
#define _CODE_SWITCH_H_

#include <stdbool.h>
#include "stm32f10x_conf.h"

#define CODE_SWITCH1_PIN_A				GPIO_Pin_1
#define CODE_SWITCH1_PIN_A_PORT			GPIOF
#define CODE_SWITCH1_PIN_B				GPIO_Pin_5
#define CODE_SWITCH1_PIN_B_PORT			GPIOF

#define CODE_SWITCH1_INT_SRC			GPIO_PinSource1
#define CODE_SWITCH1_INT_SRC_PORT		GPIO_PortSourceGPIOF
#define CODE_SWITCH1_INT_LINE			EXTI_Line1
#define CODE_SWITCH1_INT_CHANNEL		EXTI1_IRQn



#define CODE_SWITCH2_PIN_A				GPIO_Pin_3
#define CODE_SWITCH2_PIN_A_PORT			GPIOF
#define CODE_SWITCH2_PIN_B				GPIO_Pin_7
#define CODE_SWITCH2_PIN_B_PORT			GPIOF

#define CODE_SWITCH2_INT_SRC			GPIO_PinSource3
#define CODE_SWITCH2_INT_SRC_PORT		GPIO_PortSourceGPIOF
#define CODE_SWITCH2_INT_LINE			EXTI_Line3
#define CODE_SWITCH2_INT_CHANNEL		EXTI3_IRQn


typedef struct _tagStCodeSwitchState
{
	u16 u16Index;
	
	u16 u16Dir;
	u16 u16Speed;
	
	u16 u16Cnt;
	u16 u16OldCnt;
	u32 u32TriggerTime;
}StCodeSwitchState;

void CodeSwitchInit(void);
u16 CodeSwitchPluse(u16 u16Index);

u16 CodeSwitchGetValue(u16 u16Index);
u16 CodeSwitchSetValue(u16 u16Index, u16 u16Value);
void CodeSwitchFlush(void);
#endif
