/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����I2C.h
* ժҪ: ģ��I2Cͷ�ļ�
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2017��11��29��
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
