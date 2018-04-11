/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����io_buf_ctrl.h
* ժҪ: �����������������ͷ�ļ�
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _IO_BUF_CTRL_H_
#define _IO_BUF_CTRL_H_
#include "stm32f10x_conf.h"

typedef struct _tagStIOBuf
{
	void *pBuf;
	u32 u32Read;				/* for user */
	u32 u32Used;				/* for user */
	struct _tagStIOBuf *pNext;
}StIOBuf;

typedef struct _tagStIOBufHandle
{
	StIOBuf *pIOBufAddr;
	StIOBuf *pWrite;			/* point to the position which we can write at */
	StIOBuf *pNeedSend;			/* point to the position which we can read/send at */
	u32 u32Used;				/* point out how many buffers we have used*/
	u32 u32IOBufCnt;
}StIOBufHandle;


s32 IOBufInit(StIOBufHandle *pHandle, StIOBuf *pIOBufAddr, u32 u32IOBufSize, void *pBuf, u32 u32BufSize);
StIOBuf *ReadBufBegin(StIOBufHandle *pHandle);
void ReadBufEnd(StIOBufHandle *pHandle);
StIOBuf *WriteBufBegin(StIOBufHandle *pHandle);
void WriteBufEnd(StIOBufHandle *pHandle);

#endif
