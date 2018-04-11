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

#define MSG_BUF_MAX					8	/* ��Ϣ�������� */

#define BUF_CHECK_SIZE				32	/* ��Ϣ���� buffer ��С */

#define GET_UID_CNT(Byte)		(96 / (Byte * 8))
#define UID_BASE_ADDR			(0x1FFFF7E8)

#define UID_CHECK_ADDR			(0x0800F800)
#define DATA_SAVE_ADDR  		(0x0800F000)
#define DATA_SAVE_HEAD 			(0xA5A5)

/*
 * 8 * 8����
 */

#define KEY_X_CNT		2				/* ����ɨ���� */
#define KEY_Y_CNT		1				/* ����ɨ���� power */
#define KEY_SCAN_CNT	8				/* ����ɨ����� */
#define KEY_MIX_MAX		8				/* ����֧�ֵ����ɨ����� */

#define KEY_BUF_MAX		8				/* ������Ϣ�������� */

#define LED_X_CNT		2				/* LEDɨ���� */
#define LED_Y_CNT		1				/* LEDɨ���� power */


#define CAM_ADDR_MAX						8

#define CODE_SWITCH_MAX						2
#define CODE_SWITCH1_MAX_VALUE				63
#define CODE_SWITCH2_MAX_VALUE				63
#define CODE_SWITCH3_MAX_VALUE				63
#define CODE_SWITCH4_MAX_VALUE				63

#define CODE_SWITCH1_REVERSE	0
#define CODE_SWITCH2_REVERSE	0
#define CODE_SWITCH3_REVERSE	0
#define CODE_SWITCH4_REVERSE	0


#endif

