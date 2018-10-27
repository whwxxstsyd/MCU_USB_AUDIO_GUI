/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����extern_peripheral.h
* ժҪ: �������ͷ�ļ�
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2018��06��29��
*******************************************************************************/
#ifndef _EXTERN_PERIPHERAL_H_
#define _EXTERN_PERIPHERAL_H_

#include <stdint.h>
#include <stdbool.h>

void LDP6803Init(void);
void LDPWrite(uint8_t R, uint8_t G, uint8_t B);
void LPDTest(void);

void KeyboardPowerInit(void);
void KeyboardPowerEnable(bool boIsEnable);

void PCKeyboardPowerInit(void);
void PCKeyboardPowerEnable(bool boIsEnable);

void KeyboardConnectInit(void);
void KeyboardConnectSetMode(uint8_t u8Mode);
void ExternPeripheralInit(void);


#endif
