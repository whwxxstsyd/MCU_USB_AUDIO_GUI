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
