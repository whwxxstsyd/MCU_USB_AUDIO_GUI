/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����message.h
* ժҪ: Э���Զ�������
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _MESSAGE_USB_H_
#define _MESSAGE_USB_H_
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "IOCtrl.h"


void MessageUSBInit(void);
StIOFIFO *MessageUSBFlush(bool boSendALL);
void MessageUSBRelease(StIOFIFO *pFIFO);
int32_t MessageUSBWrite(void *pData, bool boNeedFree, uint16_t u16ID, uint32_t u32Length);
extern const StIOTCB c_stUSBIOTCB;

void MessageUSBWriteInDate(void *pData, uint32_t u32Length);
bool IsUSBDeviceConnect(void);
void CopyToUSBMessage(void *pData, uint32_t u32Length, uint16_t u16ID);

#endif
