/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����key_led_ctrl.c
* ժҪ: �����Լ�LEDˢ�³���
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/

#include <stddef.h>
#include <string.h>

#include "key_led_ctrl.h"
#include "protocol.h"

#ifndef MAX_IO_FIFO_CNT
#define MAX_IO_FIFO_CNT 8
#endif


static StKeyMixIn s_stKeyMixIn[MAX_IO_FIFO_CNT];
static StIOFIFOList s_stIOFIFOList[MAX_IO_FIFO_CNT];
static StIOFIFOCtrl s_stIOFIFOCtrl;

void KeyBufInit(void)
{
	u32 i;
	IOFIFOInit(&s_stIOFIFOCtrl, s_stIOFIFOList, MAX_IO_FIFO_CNT, _IO_Reserved); 
	for (i = 0; i < MAX_IO_FIFO_CNT; i++)
	{
		s_stIOFIFOList[i].boNeedFree = false;
		s_stIOFIFOList[i].pData = s_stKeyMixIn + i;
		s_stIOFIFOList[i].s32Length = sizeof(StKeyMixIn);
	}
}

/* ������ʹ��, ��ѯ�Ƿ����¼�, �з���StIOFIFO.pData�� StKeyMixIn ����ָ��(��ת��) */
StIOFIFO *KeyBufGetBuf(void)
{
	return (StIOFIFO *)(GetAListFromRWFIFO(&s_stIOFIFOCtrl, true));
}
/* ������ʹ��, KeyBufGetBuf����ֵ��ΪNULL��ʱ��, �ô˺�������ʹ�� */
void KeyBufGetEnd(StIOFIFO *pFIFO)
{
	ReleaseAUsedFIFO(&s_stIOFIFOCtrl, (StIOFIFOList *)pFIFO);
}


void KeyBufWrite(StKeyMixIn *pKey)
{
	StIOFIFOList *pList = NULL;
	
	if (pKey == NULL)
	{
		return;
	}

	pList = GetAUnusedFIFO(&s_stIOFIFOCtrl);
	
	if (pList != NULL)
	{
		memcpy(pList->pData, pKey, sizeof(StKeyMixIn));
		if (InsertIntoTheRWFIFO(&s_stIOFIFOCtrl, pList, true) != 0)
		{
			ReleaseAUsedFIFO(&s_stIOFIFOCtrl, pList);
		}
	}
}

