/******************(C) copyright 天津市XXXXX有限公司 **************************
* All Rights Reserved
* 文件名：user_api.c
* 摘要: 用户的一些API
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月05日
*******************************************************************************/
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stm32f10x_conf.h"

#include "user_conf.h"
#include "app_port.h"

#include "user_api.h"
#include "gui_driver.h"

u32 g_u32SysTickCnt = 0;



/*
 * 说明: 空指令
 * 输入: 无
 * 输出: 无
 */
__asm void __NOP(void)
{
  nop;
}

/*
 * 说明: 错误处理函数
 * 输入: 错误码
 * 输出: 无
 */
void ErrorHappend(s32 s32ErrorCode)
{
	/* we can do something using the error code */

	while(1)
	{
	
	}
}

u32 SysTimeDiff( u32 u32Begin, u32 u32End)
{
	if (u32End >= u32Begin)
	{
		return (u32End - u32Begin);
	}
	else
	{
		return ((u32)(~0)) - u32Begin + u32End;
	}
}

void Delay(u32 u32MilliSecond)
{
	u32 u32SysTimeBegin = g_u32SysTickCnt;
	while(SysTimeDiff(u32SysTimeBegin, g_u32SysTickCnt) < u32MilliSecond);
}

void DelayMicro(u32 u32MicroSecond)
{
	u32 i;
	for (i = 0; i < u32MicroSecond; i++)
	{
		u32 j;
		for (j = 0; j < 10; j++);
	}
}

__weak void LvglTickInc(void)
{

}
__weak void KeyLedFlush(void)
{

}

__weak void CodeSwitchFlush(void)
{
	
}
/* 滴答时钟终端 */
void SysTick_Handler(void)
{
	g_u32SysTickCnt++;
	LvglTickInc();
	//KeyLedFlush();
	//CodeSwitchFlush();
}


