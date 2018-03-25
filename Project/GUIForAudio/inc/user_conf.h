/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����user_conf.h
* ժҪ: Э���Զ�������
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _USER_CONF_H_
#define _USER_CONF_H_
#include <stdint.h>

#define KEY_UP			0x01
#define KEY_DOWN		0x02
#define KEY_KEEP		0xEE
#define KEY_MOVE		0xDD

#define KEY_MIX_MAX				8				/* �����ϼ����� */




#define MSG_BUF_MAX					8	/* ��Ϣ�������� */

#define BUF_CHECK_SIZE				32	/* ��Ϣ���� buffer ��С */

#define KEY_BUF_MAX					8	/* ������Ϣ�������� */


//#define SYNC_MAX_TIME	(KEY_SCAN_CNT * 2 * KEY_BUF_MAX) /* ���ͬ��ʱ�� ��λ ms */
#define SYNC_MAX_TIME	(10 * 1000)	/* 10s */


#define DATA_SAVE_ADDR  		(0x0807D000)
#define AUDIO_DATA_SAVE_ADDR	(0x0807E000)
#define DATA_SAVE_HEAD			(0xA5A5)

typedef struct _tagStKeyState
{
	uint8_t u8KeyValue;		/* ֵ */
	uint8_t u8KeyLocation;	/* λ��/ ID */
	uint8_t u8KeyState;		/* ״̬ */
	uint8_t u8Reserved;
}StKeyState;


#endif

