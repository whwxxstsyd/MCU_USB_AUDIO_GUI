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
void PeripheralPinClkEnable(void)
{
	/* enable the PWR clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
	/* Enable all the GPIO and AFIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | 
	                         RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOF |
							 RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO, ENABLE);
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


void Timer2InitTo1us(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	/* Compute the prescaler value */
	uint16_t u16PrescalerValue = (uint16_t) (SystemCoreClock / 1000000) - 1; /* 1 us */
	
	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 65535;
	TIM_TimeBaseStructure.TIM_Prescaler = u16PrescalerValue;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
	/* TIM2 enable counter */
	TIM_Cmd(TIM2, ENABLE);

}


