/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����user_init.h
* ժҪ: һЩ�򵥵ĳ�ʼ��
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/

#ifndef _USER_INIT_H_
#define _USER_INIT_H_

#define USB_POWER_PIN			GPIO_Pin_0
#define USB_POWER_PIN_PORT		GPIOB

void PeripheralClkEnable(void);
void OpenSpecialGPIO(void);
void USBPowerPinInit(void);
void PVDInit(void);
void SysTickInit(void);

#endif
