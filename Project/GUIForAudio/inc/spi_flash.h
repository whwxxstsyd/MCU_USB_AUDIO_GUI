#ifndef SPI_FLASH_H_
#define SPI_FLASH_H_
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"

uint8_t SPIFlashReadWriteByte(uint8_t uint8_tTxData);

void SPIFlashInit(void);
uint16_t SPIFlashReadID(void);
uint8_t SPIFlashReadSR(void);
void SPIFlashWriteSR(uint8_t uint8_tSR);
void SPIFlashWriteEnable(void);
void SPIFlashWriteDisable(void);
int32_t SPIFlashWriteNoCheck(uint8_t *pBuffer, uint32_t u32WriteAddr, uint32_t u32NumByteToWrite);
int32_t SPIFlashRead(uint8_t *pBuffer, uint32_t u32WriteAddr, uint32_t u32NumByteToRead);
int32_t SPIFlashWrite(uint8_t *pBuffer, uint32_t u32WriteAddr, uint16_t u16NumByteToWrite);
int32_t SPIFlashEraseChip(void);
int32_t SPIFlashEraseSector(uint32_t u32Sector);
void SPIFlashWaitBusy(void);
bool SPIFlashIsBusy(void);
int32_t SPIFlashPowerDown(void);
void SPIFlashWakeUp(void);
bool boIsFlashCanUse(void);



enum
{
	_Flash_BigData_Undefined = 0,
	_Flash_BigData_Start,
	_Flash_BigData_Continues,
	_Flash_BigData_Erasing,
	_Flash_BigData_End,
};

typedef struct _tagStFlashBigDataCtrl
{
	uint32_t u32StartAddr;
	uint32_t u32TotalSize;
	uint32_t u32CurWRAddr;
	uint32_t u32State;	
}StFlashBigDataCtrl;

int32_t SPIFlashBigDataReadWriteBegin(StFlashBigDataCtrl *pCtrl, 
	uint32_t u32StartAddr, uint32_t u32TotalSize);
int32_t SPIFlashBigDataWrite(StFlashBigDataCtrl *pCtrl, 
	uint8_t *pData, int32_t s32Size);
int32_t SPIFlashBigDataRead(StFlashBigDataCtrl *pCtrl, 
	uint8_t *pData, int32_t s32Size);
int32_t SPIFlashBigDataWriteNoBreak(StFlashBigDataCtrl *pCtrl, 
	uint8_t *pData, int32_t s32Size);

#endif
