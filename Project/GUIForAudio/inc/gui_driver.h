/******************(C) copyright �����XXXXX���޹�˾ **************************
* All Rights Reserved
* �ļ�����gui_driver.h
* ժҪ: lvgl ��س�ʼ���Լ�����
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2018��04��06��
*******************************************************************************/

#ifndef _GUI_DRIVER_H_
#define _GUI_DRIVER_H_
#include <stdint.h>
#include <stdbool.h>


void SetLvglKey(uint32_t u32Key, bool boIsPress);

void LvglInit(void);

void LvglDispMem(void);

void LvglTickInc(void);

void LvglFlushTask(void);


#endif

