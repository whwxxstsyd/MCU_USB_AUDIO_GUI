/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：I2C.h
* 摘要: 模拟I2C头文件
* 版本：0.0.1
* 作者：许龙杰
* 日期：2017年11月29日
*******************************************************************************/
#ifndef _I2C_H_
#define _I2C_H_

#include <stdbool.h>
#include "stm32f10x_conf.h"

void I2CInit(void);
bool I2CWriteBuffer(uint8_t u8Addr, uint32_t u32Reg, uint8_t u8RegByteCnt, uint8_t u8Len, uint8_t *pData);
int32_t I2CWrite(uint8_t u8Addr, uint32_t u32Reg, uint8_t u8RegByteCnt, uint8_t u8Data);
bool I2CRead(uint8_t u8Addr, uint32_t u32Reg, uint8_t u8RegByteCnt, uint8_t u8Len, uint8_t *pData);
uint16_t I2CGetErrorCounter(void);

#endif
