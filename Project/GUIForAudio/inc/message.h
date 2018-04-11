/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����message.h
* ժҪ: Э���Զ�������
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _MESSAGE_H_
#define _MESSAGE_H_
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"

#include "IOCtrl.h"


void MessageUartInit(void);
StIOFIFO *MessageUartFlush(bool boSendALL);
void MessageUartRelease(StIOFIFO *pFIFO);
void MessageUartReleaseNoReleaseData(StIOFIFO *pFIFO);

int32_t MessageUartWrite(void *pData, bool boNeedFree, uint16_t u16ID, uint32_t u32Length);
void MessageUARTConfig(USART_InitTypeDef *pConfig);

extern const StIOTCB c_stUartIOTCB;

#endif
