/******************(C) copyright �����XXXXX���޹�˾ **************************
* All Rights Reserved
* �ļ�����user_api.h
* ժҪ: �û���һЩAPI
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��05��
*******************************************************************************/
#ifndef _USER_API_H_
#define _USER_API_H_
#include <stdint.h>
#include "stm32f10x_conf.h"

extern u32 g_u32SysTickCnt;
uint16_t CRC16(const uint8_t *pFrame, uint16_t u16Len);
extern void __NOP(void);
void ErrorHappend(s32 s32ErrorCode);
u32 SysTimeDiff( u32 u32Begin, u32 u32End);
void Delay(u32 u32MilliSecond);
void DelayMicro(u32 u32MicroSecond);



#endif
