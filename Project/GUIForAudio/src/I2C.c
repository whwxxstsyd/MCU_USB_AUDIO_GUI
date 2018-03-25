/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：I2C.c
* 摘要: I2C源文件
* 版本：0.0.1
* 作者：许龙杰
* 日期：2017年11月29日
*******************************************************************************/

#include "I2C.h"

#define SCL_PORT		GPIOB
#define SCL_PIN			GPIO_Pin_1

#define SDA_PORT		GPIOF
#define SDA_PIN			GPIO_Pin_9

#define SCL_H         SCL_PORT->BSRR = SCL_PIN
#define SCL_L         SCL_PORT->BRR  = SCL_PIN

#define SDA_H         SDA_PORT->BSRR = SDA_PIN
#define SDA_L         SDA_PORT->BRR  = SDA_PIN

#define SCL_Read      SCL_PORT->IDR & SCL_PIN
#define SDA_Read      SDA_PORT->IDR & SDA_PIN

static void I2CDelay(void)
{
	volatile int i = 7;

	while (i)
	{
		i--;
	}

}

static bool I2C_Start(void)
{
	SDA_H;
	SCL_H;
	I2CDelay();
	if (!SDA_Read)
		return false;
	SDA_L;
	I2CDelay();
	if (SDA_Read)
		return false;
	SDA_L;
	I2CDelay();
	return true;
}

static void I2C_Stop(void)
{
	SCL_L;
	I2CDelay();
	SDA_L;
	I2CDelay();
	SCL_H;
	I2CDelay();
	SDA_H;
	I2CDelay();
}

static void I2C_Ack(void)
{
	SCL_L;
	I2CDelay();
	SDA_L;
	I2CDelay();
	SCL_H;
	I2CDelay();
	SCL_L;
	I2CDelay();
}

static void I2C_NoAck(void)
{
	SCL_L;
	I2CDelay();
	SDA_H;
	I2CDelay();
	SCL_H;
	I2CDelay();
	SCL_L;
	I2CDelay();
}

static bool I2C_WaitAck(void)
{
	SCL_L;
	I2CDelay();
	SDA_H;
	I2CDelay();
	SCL_H;
	I2CDelay();
	if (SDA_Read)
	{
		SCL_L;
		return false;
	}
	SCL_L;
	return true;
}

static void I2C_SendByte(uint8_t u8Byte)
{
	uint8_t i = 8;
	while (i--)
	{
		SCL_L;
		I2CDelay();
		if (u8Byte & 0x80)
			SDA_H;
		else
			SDA_L;
		u8Byte <<= 1;
		I2CDelay();
		SCL_H;
		I2CDelay();
	}
	SCL_L;
}

static uint8_t I2C_ReceiveByte(void)
{
	uint8_t i = 8;
	uint8_t u8Byte = 0;

	SDA_H;
	while (i--)
	{
		u8Byte <<= 1;
		SCL_L;
		I2CDelay();
		SCL_H;
		I2CDelay();
		if (SDA_Read)
		{
			u8Byte |= 0x01;
		}
	}
	SCL_L;
	return u8Byte;
}

void I2CInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;

	GPIO_InitStructure.GPIO_Pin = SCL_PIN;
	GPIO_Init(SCL_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = SDA_PIN;
	GPIO_Init(SDA_PORT, &GPIO_InitStructure);
}

void I2CReleaseBus(void)
{
	


}


void I2CSendRegAddr(uint32_t u32Reg, uint8_t u8RegByteCnt)
{
	uint8_t j, u8Reg;
	
	u8RegByteCnt -= 1;
	u8RegByteCnt &= 0x03;
	u8RegByteCnt += 1;
	u32Reg <<= ((4 - u8RegByteCnt) * 8);
	for (j = 0; j < u8RegByteCnt; j++)
	{
		u8Reg = (u32Reg >> 24);
		u32Reg <<= 8;
		I2C_SendByte(u8Reg);
		I2C_WaitAck();			
	}			
}

bool I2CWriteBuffer(uint8_t u8Addr, uint32_t u32Reg, uint8_t u8RegByteCnt, uint8_t u8Len, uint8_t *pData)
{
	int32_t i;
	if (!I2C_Start())
		return false;
	I2C_SendByte(u8Addr << 1 | I2C_Direction_Transmitter);
	if (!I2C_WaitAck())
	{
		I2C_Stop();
		return false;
	}
	
	I2CSendRegAddr(u32Reg, u8RegByteCnt);
	
	for (i = 0; i < u8Len; i++)
	{
		I2C_SendByte(pData[i]);
		if (!I2C_WaitAck())
		{
			I2C_Stop();
			return false;
		}
	}
	I2C_Stop();
	return true;
}

int32_t I2CWrite(uint8_t u8Addr, uint32_t u32Reg, uint8_t u8RegByteCnt, uint8_t u8Data)
{
	if (!I2C_Start())
		return -1;
	I2C_SendByte(u8Addr << 1 | I2C_Direction_Transmitter);
	if (!I2C_WaitAck())
	{
		I2C_Stop();
		return -2;
	}

	I2CSendRegAddr(u32Reg, u8RegByteCnt);

	I2C_SendByte(u8Data);
	I2C_WaitAck();
	I2C_Stop();
	return 0;
}

bool I2CRead(uint8_t u8Addr, uint32_t u32Reg, uint8_t u8RegByteCnt, uint8_t u8Len, uint8_t *pData)
{
	if (!I2C_Start())
		return false;
	I2C_SendByte(u8Addr << 1 | I2C_Direction_Transmitter);
	if (!I2C_WaitAck())
	{
		I2C_Stop();
		return false;
	}

	I2CSendRegAddr(u32Reg, u8RegByteCnt);

	I2C_Start();
	I2C_SendByte(u8Addr << 1 | I2C_Direction_Receiver);
	I2C_WaitAck();
	while (u8Len != 0)
	{
		pData[0] = I2C_ReceiveByte();
		if (u8Len == 1)
			I2C_NoAck();
		else
			I2C_Ack();
		pData++;
		u8Len--;
	}
	I2C_Stop();
	return true;
}

uint16_t I2CGetErrorCounter(void)
{
	// TODO maybe fix this, but since this is test code, doesn't matter.
	return 0;
}
