/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����adc_ctrl.h
* ժҪ: �����Լ�LEDˢ�³���
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _ADC_CTRL_H_
#define _ADC_CTRL_H_
#include <stdbool.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"

#define ADC_SAMPLE_CNT					1

#define ADC1_CHANNEL_CNT				12
#define ADC3_CHANNEL_CNT				0

#define ADC_ALL_CHANNEL_TOTAL			(ADC1_CHANNEL_CNT + ADC3_CHANNEL_CNT)
#define AUDIO_VOLTAGE_CNT				(ADC_ALL_CHANNEL_TOTAL / 2)

typedef struct _tagStADCCtrlSource
{
	StPinSource stPin;
	u32 u32Channel;
}StADCCtrlSource;

void AudioSrcInit(void);
u16 GetAudioVoltage(u16 u16Channel);

#endif
