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

void PeripheralPinClkEnable(void);
void OpenSpecialGPIO(void);
void SysTickInit(void);
void Timer2InitTo1us(void);
#endif
