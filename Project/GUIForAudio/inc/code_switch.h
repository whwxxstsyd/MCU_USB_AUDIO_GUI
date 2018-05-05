/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：code_switch.h
* 摘要: 编码开关头文件
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
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
