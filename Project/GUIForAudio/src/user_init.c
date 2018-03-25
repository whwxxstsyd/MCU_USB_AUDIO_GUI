/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����user_init.c
* ժҪ: һЩ�򵥵ĳ�ʼ��
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_init.h"


/*
 * ʹ��������������ʱ��
 * ����: ��
 * ���: ��
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

