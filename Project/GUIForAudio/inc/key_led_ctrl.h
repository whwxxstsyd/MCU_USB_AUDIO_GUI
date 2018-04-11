/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����key_led_ctrl.h
* ժҪ: �����Լ�LEDˢ�³���
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _KEY_LED_CTRL_H_
#define _KEY_LED_CTRL_H_

#include <stdbool.h>
#include "stm32f10x_conf.h"

#include "user_conf.h"
#include "app_port.h"
#include "IOCtrl.h"

#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"

typedef enum _tagEmKeyType
{
	_Key_Push_Rod = 0x00,
	_Key_Board,
	_Key_Rock,
	_Key_Volume,
	_Key_CodeSwitch,
	_Key_Reserved,
}EmKeyType;


typedef union _tagUnKeyMixIn
{
	StKeyState stKeyState[KEY_MIX_MAX];		/* ɨ��İ�����״̬ */
	StCodeSwitchState stCodeSwitchState;	
}UnKeyMixIn;

typedef struct _tagStKeyMixIn
{
	EmKeyType emKeyType;					/* ��ʾunion����ʲô���� */
	u32 u32Cnt;								/* ���ڰ�����ʱ������ */
	UnKeyMixIn unKeyMixIn;					/* ���ֵ */
}StKeyMixIn;


void KeyBufInit(void);

StIOFIFO *KeyBufGetBuf(void);

void KeyBufGetEnd(StIOFIFO *pFIFO);

void KeyBufWrite(StKeyMixIn *pKey);

#endif
