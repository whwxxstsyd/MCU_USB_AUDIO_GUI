/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：user_init.h
* 摘要: 一些简单的初始化
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
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
