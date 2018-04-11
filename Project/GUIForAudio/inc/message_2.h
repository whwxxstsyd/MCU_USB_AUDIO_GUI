/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����message.h
* ժҪ: Э���Զ�������
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _MESSAGE_2_H_
#define _MESSAGE_2_H_
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"

#include "IOCtrl.h"


void MessageUart2Init(void);
StIOFIFO *MessageUart2Flush(bool boSendALL);
void MessageUart2Release(StIOFIFO *pFIFO);
void MessageUart2ReleaseNoReleaseData(StIOFIFO *pFIFO);
int32_t MessageUart2Write(void *pData, bool boNeedFree, uint16_t u16ID, uint32_t u32Length);
void MessageUART2Config(USART_InitTypeDef *pConfig);


extern const StIOTCB c_stUart2IOTCB;
#endif
