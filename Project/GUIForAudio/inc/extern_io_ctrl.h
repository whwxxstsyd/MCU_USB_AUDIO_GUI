/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：extern_io_ctrl.h
* 摘要: 键盘以及LED刷新程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
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
