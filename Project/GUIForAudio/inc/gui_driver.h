/******************(C) copyright 天津市XXXXX有限公司 **************************
* All Rights Reserved
* 文件名：gui_driver.h
* 摘要: lvgl 相关初始化以及驱动
* 版本：0.0.1
* 作者：许龙杰
* 日期：2018年04月06日
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

