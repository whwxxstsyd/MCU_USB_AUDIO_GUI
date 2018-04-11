/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����io_buf_ctrl.c
* ժҪ: �����������������
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/

#include <stddef.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "app_port.h"
#include "io_buf_ctrl.h"

/*
 * ��ʼ��IO����
 * ����: pHandle  ָ�򻺳���
 * 	   : pIOBufAddr �������ͷ��ַ
 *     : u32IOBufCnt ָʾ�ж��ٸ��������ͷ
 *     : pBuf ��������ַ
 *     : u32BufSize �������Ĵ�С
 * ����: û�д��󷵻�0
 */
s32 IOBufInit(StIOBufHandle *pHandle, 
				StIOBuf *pIOBufAddr, u32 u32IOBufCnt, 
				void *pBuf, u32 u32BufSize)
{
	u32 i;
	u32 u32EachBufSize;
	if(pHandle == NULL)
	{
		return -1;
	}
	memset(pHandle, 0, sizeof(StIOBufHandle));

	memset(pIOBufAddr, 0, sizeof(StIOBuf) * u32IOBufCnt);

	u32EachBufSize = u32BufSize / u32IOBufCnt;
	for (i = 0; i < u32IOBufCnt; i++) /* ����ַ���ӵ�����ͷ�� */
	{
		pIOBufAddr[i].pBuf = pBuf;
		pBuf = (void *)((u32)pBuf + u32EachBufSize);
	}

	pHandle->pWrite = pHandle->pNeedSend = pHandle->pIOBufAddr = pIOBufAddr;

	for (i = 0; i < (u32IOBufCnt - 1); i++) /* ��ÿһ���������һ���� */
	{
		pHandle->pIOBufAddr[i].pNext = &pHandle->pIOBufAddr[i + 1];
	}
	pHandle->pIOBufAddr[i].pNext = pHandle->pIOBufAddr;
	pHandle->u32IOBufCnt = u32IOBufCnt;
	return 0;
}

/*
 * �õ�����ָ�룬���ָ��ָ��Ҫ���������
 * �뺯�� ReadBufEnd �ɶԳ���
 * ����: pHandle ָ�򻺳���
 * ����: NULL��û�еõ�ָ�룬���ܻ�����û������
 */
StIOBuf *ReadBufBegin(StIOBufHandle *pHandle)
{	
	StIOBuf *pIOBuf = NULL;

	if(pHandle == NULL)
	{
		return NULL;
	}
	

	if(pHandle->u32Used == 0)
	{
		goto end;
	}
	pIOBuf = pHandle->pNeedSend;
end:

	return pIOBuf;
}

/*
 * ֪ͨ���������Ѿ���������һ������
 * �뺯�� ReadBufBegin�ɶԳ���
 * ����: pHandle ָ�򻺳���
 * ����: û�з���
 */
void ReadBufEnd(StIOBufHandle *pHandle)
{
	StIOBuf *pTmp;

	if(pHandle == NULL)
	{
		return;
	}

	if(pHandle->u32Used == 0)
	{
		return;
	}

	pTmp = pHandle->pNeedSend;		/* ��λ���� */
	pTmp->u32Read = 0;
	pTmp->u32Used = 0;
	pHandle->pNeedSend = pHandle->pNeedSend->pNext;
	pHandle->u32Used--;
}

/*
 * �õ�����ָ�룬���ǿ��Ծ�����д�����ָ��ָ���λ�� 
 * �뺯�� WriteBufEnd �ɶԳ���
 * ����:  pHandle ָ�򻺳���
 * ����: NULL��û�еõ�ָ�룬���ܻ�����û���㹻�Ŀռ����д����
 */
StIOBuf *WriteBufBegin(StIOBufHandle *pHandle)
{	
	StIOBuf *pTmp = NULL;

	if(pHandle == NULL)
	{
		return NULL;
	}


	if(pHandle->u32Used >= pHandle->u32IOBufCnt)
	{
		goto end;
	}
	pTmp = pHandle->pWrite;
end:
	return pTmp;
}

/*
 * ֪ͨ���壬�����Ѿ��ڻ����������һ������
 * �뺯�� WriteBufBegin �ɶԳ���
 * ����:  pHandleָ�򻺳���
 * ����: û�з���
 */
void WriteBufEnd(StIOBufHandle *pHandle)
{

	if(pHandle == NULL)
	{
		return;
	}


	if(pHandle->u32Used >= pHandle->u32IOBufCnt)
	{
		goto end;
	}
	pHandle->pWrite = pHandle->pWrite->pNext;
	pHandle->u32Used++;
end:
	;
}
