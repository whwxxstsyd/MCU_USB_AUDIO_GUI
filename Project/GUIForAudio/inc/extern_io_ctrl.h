/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����extern_io_ctrl.h
* ժҪ: �����Լ�LEDˢ�³���
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#ifndef _EXTERN_IO_CTRL_H_
#define _EXTERN_IO_CTRL_H_

#define SHIFT_CLOCK_PIN			GPIO_Pin_15
#define SHIFT_CLOCK_PORT		GPIOB

#define SHIFT_CLEAR_PIN			GPIO_Pin_12
#define SHIFT_CLEAR_PORT		GPIOD

#define SHIFT_DATA_PIN			GPIO_Pin_13
#define SHIFT_DATA_PORT			GPIOD

#define IO_CNT			16

void ExternIOInit(void);
void ExternIOClear(void);
void ExternIOCtrl(u8 u8Index, BitAction emAction);
#endif
