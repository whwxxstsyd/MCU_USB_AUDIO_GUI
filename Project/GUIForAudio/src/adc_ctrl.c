/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：adc_ctrl.c
* 摘要: 键盘以及LED刷新程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"
#include "adc_ctrl.h"


static u16 s_vu16ADC1Tab[ADC1_CHANNEL_CNT * ADC_SAMPLE_CNT];


const StADCCtrlSource c_stADC1CtrlSource[ADC1_CHANNEL_CNT] = 
{
	{{GPIOA, GPIO_Pin_6}, ADC_Channel_6},
	{{GPIOA, GPIO_Pin_7}, ADC_Channel_7},
	{{GPIOC, GPIO_Pin_4}, ADC_Channel_14},
	{{GPIOC, GPIO_Pin_5}, ADC_Channel_15},
	{{GPIOB, GPIO_Pin_0}, ADC_Channel_8},
	{{GPIOB, GPIO_Pin_1}, ADC_Channel_9},
	{{GPIOC, GPIO_Pin_0}, ADC_Channel_10},
	{{GPIOC, GPIO_Pin_1}, ADC_Channel_11},
	{{GPIOC, GPIO_Pin_2}, ADC_Channel_12},
	{{GPIOC, GPIO_Pin_3}, ADC_Channel_13},
	{{GPIOA, GPIO_Pin_4}, ADC_Channel_4},
	{{GPIOA, GPIO_Pin_5}, ADC_Channel_5},
};


/* ADC?????,  */
static void ADC1GPIOInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	u32 i;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	for (i = 0; i < ADC1_CHANNEL_CNT; i++)
	{
		GPIO_InitStructure.GPIO_Pin = c_stADC1CtrlSource[i].stPin.u16Pin;
		GPIO_Init(c_stADC1CtrlSource[i].stPin.pPort, &GPIO_InitStructure);	
	}

}

/* ADC???,  */
static void ADC1Init(void)
{
	ADC_InitTypeDef   ADC_InitStructure;
	DMA_InitTypeDef   DMA_InitStructure;
	u32 i;
	
	
	ADC_StructInit(&ADC_InitStructure);
	DMA_StructInit(&DMA_InitStructure);
	
	RCC_ADCCLKConfig(RCC_PCLK2_Div6); 
	
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	/* DMA1 channel1 configuration ----------------------------------------------*/
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)(&(ADC1->DR));
	DMA_InitStructure.DMA_MemoryBaseAddr = (u32)s_vu16ADC1Tab;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = ADC1_CHANNEL_CNT * ADC_SAMPLE_CNT;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
	
	/* ADC1 configuration ------------------------------------------------------*/
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = ADC1_CHANNEL_CNT;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	/* ADC1 regular channels configuration */ 
	for (i = 0; i < ADC1_CHANNEL_CNT; i++)
	{
		ADC_RegularChannelConfig(ADC1, c_stADC1CtrlSource[i].u32Channel, 
			i + 1, ADC_SampleTime_1Cycles5);
	}
	
#if 0
	/* Regular discontinuous mode channel number configuration */
	ADC_DiscModeChannelCountConfig(ADC1, ADC1_CHANNEL_CNT);
	/* Enable regular discontinuous mode */
	ADC_DiscModeCmd(ADC1, ENABLE);
#endif

#if 0	
	/* Enable ADC1 external trigger conversion */ 
	ADC_ExternalTrigConvCmd(ADC1, ENABLE);
#endif
	
#if 0
	/* Configure high and low analog watchdog thresholds */
	ADC_AnalogWatchdogThresholdsConfig(ADC1, 0x0834, 0x07D0);		//2000~2100
	/* Enable analog watchdog on one regular channel */
	ADC_AnalogWatchdogCmd(ADC1, ADC_AnalogWatchdog_AllRegEnable);
#endif
	/* Enable ADC1 DMA */
	ADC_DMACmd(ADC1, ENABLE);
	
	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);  
	
	/* Enable ADC1 reset calibaration register */   
	ADC_ResetCalibration(ADC1);
	/* Check the end of ADC1 reset calibration register */
	while(ADC_GetResetCalibrationStatus(ADC1));
	
	/* Start ADC1 calibaration */
	ADC_StartCalibration(ADC1);
	/* Check the end of ADC1 calibration */
	while(ADC_GetCalibrationStatus(ADC1));
	
#if 0
	ADCTimerInit();
	
	TIM_Cmd(TIM2,ENABLE);
	
	TIM_CtrlPWMOutputs(TIM2, ENABLE);
#endif

	/* Start ADC1 Software Conversion */ 
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

static u16 ADC1GetAverage(u8 u8Channel) 
{
	u32 u32Sum = 0;
	u32 i;
	u8Channel %= ADC1_CHANNEL_CNT;
	for (i = u8Channel; i < ADC1_CHANNEL_CNT * ADC_SAMPLE_CNT; i += ADC1_CHANNEL_CNT)
	{
		u32Sum += s_vu16ADC1Tab[i];
	}
	u32Sum /= ADC_SAMPLE_CNT;
	return (u16)u32Sum;
}


void AudioSrcInit(void)
{
	ADC1GPIOInit();
	ADC1Init();
}

u16 GetAudioVoltage(u16 u16Channel)
{
	if (u16Channel > ADC_ALL_CHANNEL_TOTAL)
	{
		return ~0;
	}
	
	if (u16Channel < ADC1_CHANNEL_CNT)
	{
		return ADC1GetAverage(u16Channel);
	}
	
	return ~0;
}

