/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：extern_peripheral.h
* 摘要: 外设控制头文件
* 版本：0.0.1
* 作者：许龙杰
* 日期：2018年06月29日
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
