/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：key_led.h
* 摘要: 键盘以及LED刷新程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#ifndef _KEY_LED_H_
#define _KEY_LED_H_
#include <stdbool.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"

#define KEY_UP			0x01
#define KEY_DOWN		0x02
#define KEY_KEEP		0xEE

/*
LE9:	PB0--->PB13
LE10:	PB1--->PB14

PW11:	PB13---->X
PW12:	PB14---->X

*/


#define KEY_X_1				GPIO_Pin_4
#define KEY_X_2				GPIO_Pin_3
#define KEY_X_3				GPIO_Pin_6
#define KEY_X_4				GPIO_Pin_7
#define KEY_X_5				GPIO_Pin_8
#define KEY_X_6				GPIO_Pin_9
#define KEY_X_7				GPIO_Pin_10
#define KEY_X_8				GPIO_Pin_11


#define KEY_X_PORT_1		GPIOC
#define KEY_X_PORT_2		GPIOE
#define KEY_X_PORT_3		GPIOC
#define KEY_X_PORT_4		GPIOC
#define KEY_X_PORT_5		GPIOC
#define KEY_X_PORT_6		GPIOC
#define KEY_X_PORT_7		GPIOC
#define KEY_X_PORT_8		GPIOC

#define LED_X_1				GPIO_Pin_5
#define LED_X_2				GPIO_Pin_5
#define LED_X_3				GPIO_Pin_2
#define LED_X_4				GPIO_Pin_3
#define LED_X_5				GPIO_Pin_4
#define LED_X_6				GPIO_Pin_5
#define LED_X_7				GPIO_Pin_6
#define LED_X_8				GPIO_Pin_7

#define LED_X_PORT_1		GPIOB
#define LED_X_PORT_2		GPIOE
#define LED_X_PORT_3		GPIOA
#define LED_X_PORT_4		GPIOA
#define LED_X_PORT_5		GPIOA
#define LED_X_PORT_6		GPIOA
#define LED_X_PORT_7		GPIOA
#define LED_X_PORT_8		GPIOA


#define KEY_LED_POWER_1		GPIO_Pin_3
#define KEY_LED_POWER_2		GPIO_Pin_4
#define KEY_LED_POWER_3		GPIO_Pin_5
#define KEY_LED_POWER_4		GPIO_Pin_6
#define KEY_LED_POWER_5		GPIO_Pin_7
#define KEY_LED_POWER_6		GPIO_Pin_8
#define KEY_LED_POWER_7		GPIO_Pin_9
#define KEY_LED_POWER_8		GPIO_Pin_12
#define KEY_LED_POWER_9		GPIO_Pin_13
#define KEY_LED_POWER_10	GPIO_Pin_14



#define KEY_LED_POWER_PORT_1		GPIOB
#define KEY_LED_POWER_PORT_2		GPIOB
#define KEY_LED_POWER_PORT_3		GPIOB
#define KEY_LED_POWER_PORT_4		GPIOB
#define KEY_LED_POWER_PORT_5		GPIOB
#define KEY_LED_POWER_PORT_6		GPIOB
#define KEY_LED_POWER_PORT_7		GPIOB
#define KEY_LED_POWER_PORT_8		GPIOB
#define KEY_LED_POWER_PORT_9		GPIOB
#define KEY_LED_POWER_PORT_10		GPIOB



typedef struct _tagStKeyState
{
	u8 u8KeyValue;		/* 值 */
	u8 u8KeyLocation;	/* 位置 */
	u8 u8KeyState;		/* 状态 */
	u8 u8Reserved;
}StKeyState;


typedef struct _tagStKeyValue
{
	u8 u8KeyValue[KEY_Y_CNT];	/* 所有按键的当前值 */
}StKeyValue;

typedef struct _tagStKeyScan
{
	StKeyValue stKeyNow;
	StKeyValue stKeyOld;
	StKeyValue stKeyTmp[KEY_SCAN_CNT]; /* 所有按键的当前值 */
	u32 u32ScanCnt;
	StKeyState	stKeyState[KEY_MIX_MAX];	
}StKeyScan;



#if ((LED_X_CNT <= 32) && (LED_X_CNT > 16))
typedef u32 StLedSize;
#elif ((LED_X_CNT <= 16) && (LED_X_CNT > 8))
typedef u16 StLedSize;
#else
typedef u8 StLedSize;
#endif

typedef struct _tagStLedScan
{
	StLedSize stLedValue[LED_Y_CNT];				/* LED 的值: 高电平点亮 */
	u32 u32ScanCnt;									/* 扫面的列数 */
}StLedScan;



void KeyLedInit(void);
void KeyLedFlush(void);
void ChangeLedState(u32 x, u32 y, bool boIsLight);
void ChangeLedBlinkState(u32 x, u32 y, bool boIsBlink);
void ChangeAllLedState(bool boIsLight);

#endif
