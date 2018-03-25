/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：user_init.c
* 摘要: 一些简单的初始化
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_init.h"


/*
 * 使能所有外设引脚时钟
 * 输入: 无
 * 输出: 无
 */
void PeripheralClkEnable(void)
{
	/* enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	/* Enable all the GPIO and AFIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | 
	                         RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF |
							 RCC_APB2Periph_GPIOG |RCC_APB2Periph_AFIO, ENABLE);
}

/* jtag and osc32 */
void OpenSpecialGPIO(void)
{
	/* the main function of PC13, PC14, PC15 is GPIO */

	/* JTAG-DP Disabled and SW-DP Enabled */
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
}



void SysTickInit(void) /* 1ms */
{
	SysTick_Config(SystemCoreClock / 1000);
}

