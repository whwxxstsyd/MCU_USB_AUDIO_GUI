#include <stddef.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "common.h"
#include "user_api.h"

#include "adc_ctrl.h"

#include "code_switch.h"
#include "key_led_ctrl.h"


#include "protocol.h"
#include "flash_ctrl.h"


void LockApplication(bool boIsLock)
{
	FLASH_Status emFLASHStatus = FLASH_COMPLETE;
	uint32_t u32WRPRValue;
	uint32_t u32ProtectedPages;
	/* Unlock the Flash Program Erase controller */  
	FLASH_Unlock();

	(void)emFLASHStatus;
	
	/* Get pages write protection status */
	u32WRPRValue = FLASH_GetWriteProtectionOptionByte();

	if (boIsLock)
	{
	
		/* Get current write protected pages and the new pages to be protected */
		u32ProtectedPages =  (~u32WRPRValue) | FLASH_PAGES_TO_BE_PROTECTED; 

		/* Check if desired pages are not yet write protected */
		if(((~u32WRPRValue) & FLASH_PAGES_TO_BE_PROTECTED )!= FLASH_PAGES_TO_BE_PROTECTED)
		{
			/* Erase all the option Bytes because if a program operation is 
			performed on a protected page, the Flash memory returns a 
			protection error */
			emFLASHStatus = FLASH_EraseOptionBytes();

			/* Enable the pages write protection */
			emFLASHStatus = FLASH_EnableWriteProtection(u32ProtectedPages);

			/* Generate System Reset to load the new option byte values */
			NVIC_SystemReset();
		}
	}
	else
	{
		/* Get pages already write protected */
		u32ProtectedPages = ~(u32WRPRValue | FLASH_PAGES_TO_BE_PROTECTED);

		/* Check if desired pages are already write protected */
		if((u32WRPRValue | (~FLASH_PAGES_TO_BE_PROTECTED)) != 0xFFFFFFFF )
		{
			/* Erase all the option Bytes */
			emFLASHStatus = FLASH_EraseOptionBytes();

			/* Check if there is write protected pages */
			if(u32ProtectedPages != 0x0)
			{
				/* Restore write protected pages */
				emFLASHStatus = FLASH_EnableWriteProtection(u32ProtectedPages);
			}
			/* Generate System Reset to load the new option byte values */
			NVIC_SystemReset();
		}
	}
	FLASH_Lock();

}

int32_t FlashWritePage( uint32_t u32FlashStartAddress, void *pBuf, uint32_t u32Length)
{
	FLASH_Status emFLASHStatus = FLASH_COMPLETE;
	uint32_t u32NumOfPage;
	uint32_t i;
	uint32_t u32CurWriteAddress;
	uint32_t u32FlashEndAddress;
	uint32_t *pData = (uint32_t *)pBuf;

	if ((u32Length & (~0x00000003)) != u32Length)
	{
		return -1;
	}
#if 0
	u32Length /= FLASH_PAGE_SIZE;
	u32Length *= FLASH_PAGE_SIZE;
	if (u32Length < FLASH_PAGE_SIZE)
	{
		return -1; /* too small */
	}
#endif	
	u32FlashStartAddress /= FLASH_PAGE_SIZE;
	u32FlashStartAddress *= FLASH_PAGE_SIZE;
	u32FlashEndAddress = u32FlashStartAddress + u32Length;
	
	/* Unlock the Flash Program Erase controller */  
	FLASH_Unlock();
	/* Get the number of pages to be erased */
	u32NumOfPage = (u32Length + (FLASH_PAGE_SIZE - 1)) / FLASH_PAGE_SIZE;
	

	/* Clear All pending flags */
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP|FLASH_FLAG_PGERR |FLASH_FLAG_WRPRTERR);	

	/* erase the FLASH pages */
	for(i = 0; (i < u32NumOfPage) && (emFLASHStatus == FLASH_COMPLETE); i++)
	{
		emFLASHStatus = FLASH_ErasePage(u32FlashStartAddress + (FLASH_PAGE_SIZE * i));
	}
	if (emFLASHStatus != FLASH_COMPLETE)
	{
		goto end;
	}
	/* FLASH Half Word program of data 0x1753 at addresses defined by  BANK1_WRITE_START_ADDR and BANK1_WRITE_END_ADDR */
	u32CurWriteAddress = u32FlashStartAddress;

	i = 0;
	while((u32CurWriteAddress < u32FlashEndAddress) && (emFLASHStatus == FLASH_COMPLETE))
	{
		emFLASHStatus = FLASH_ProgramWord(u32CurWriteAddress, pData[i++]);
		u32CurWriteAddress = u32CurWriteAddress + 4;
	}
	if (emFLASHStatus != FLASH_COMPLETE)
	{
		goto end;
	}

	/* Check the correctness of written data */
	u32CurWriteAddress = u32FlashStartAddress;
	emFLASHStatus = FLASH_COMPLETE;
	i = 0;
	while((u32CurWriteAddress < u32FlashEndAddress) && (emFLASHStatus != FLASH_COMPLETE))
	{
		if((*(__IO uint32_t*) u32CurWriteAddress) != pData[i++])
		{
			emFLASHStatus = FLASH_ERROR_PG;
		}
		u32CurWriteAddress += 4;
	}
	FLASH_Unlock();

#if 0	
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASH_EraseOptionBytes();
	FLASH_ProgramOptionByteData(0x1FFFF804,0xB0);
#endif

end:
	if (emFLASHStatus != FLASH_COMPLETE)
	{
		return -1;
	}
	return 0;
}


void GetUID(StUID *pUID)
{
	if (pUID != NULL)
	{
		//*pUID = *((StUID *)UID_BASE_ADDR);
		int32_t s32Length = sizeof(StUID);
		memcpy(pUID, (void *)UID_BASE_ADDR, s32Length);
	}
}


uint32_t AppCRC32(uint32_t u32AppSize)
{
	uint32_t u32CheckSize = 0;
	uint32_t *pAppData = (uint32_t *)APP_START_ADDRESS;
	uint32_t i = 0;
	while (u32CheckSize < u32AppSize)
	{
		if(pAppData[i] == 0xFFFFFFFF)
		{
			if ((pAppData[i + 1] == ~0) 
				&& (pAppData[i + 2] == ~0)
				&& (pAppData[i + 3] == ~0))
			{
				break;
			}
		}
		u32CheckSize += 4;
		i++;
	}
	
	return CRC32Buf((uint8_t *)APP_START_ADDRESS, u32CheckSize);
}

int32_t GetLic(StBteaKey *pKey, StUID *pUID, uint32_t u32AppCRC32, bool boIsRelease)
{
	if((pKey == NULL) || (pUID == NULL))
	{
		return -1;
	}
	else
	{
		StBteaKey stKey;
		
		stKey.stLIC.stUID = *pUID;
		stKey.stLIC.u32CRC32 = u32AppCRC32;
		if (boIsRelease)
		{
			pKey->stLIC = stKey.stLIC;
			
			btea(pKey->s32ReleaseLIC, 4, stKey.s32Key);
		}
		else
		{
			pKey->s32RCLic = LIC_RC_HEADER;
			
			btea(&pKey->s32RCLic, 1, stKey.s32Key);
		}
		
		return 0;
	}
}

int32_t WriteLic(StBteaKey *pKey, bool boIsRelease, uint32_t u32RCCount)
{
	if (pKey == NULL)
	{
		return -1;
	}
	else
	{
		FLASH_Status emFLASHStatus = FLASH_COMPLETE;

		uint32_t i;
		StLICCtrl stCtrl;
		uint32_t *pTmp = (uint32_t *)(&stCtrl);
		
		stCtrl.stBteaKey = *pKey;
		if (boIsRelease)
		{
			stCtrl.u32Header = LIC_RELEASE_HEADER;
		}
		else
		{
			stCtrl.u32Header = LIC_RC_HEADER;
			/* xxtea the counter */
			stCtrl.u64RCCount = u32RCCount;
			btea((int32_t *)(&stCtrl.u64RCCount), 2, stCtrl.stBteaKey.s32Key);
		}
		stCtrl.u32CheckSum = 0;
		for(i = 0; i < offsetof(StLICCtrl, u32CheckSum) / sizeof(uint32_t); i++)
		{
			stCtrl.u32CheckSum ^= pTmp[i];
		}
		
		FLASH_Unlock();
		
		/* Clear All pending flags */
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
		
		emFLASHStatus = FLASH_ErasePage(LIC_ADDRESS);

		pTmp = (uint32_t *)(&stCtrl);
		for (i = 0; i < (sizeof(StLICCtrl) / sizeof(uint32_t)); i++)
		{
			emFLASHStatus = FLASH_ProgramWord(LIC_ADDRESS + i * 4, pTmp[i]);
			if (emFLASHStatus != FLASH_COMPLETE)
			{
				FLASH_Lock();
				return -1;
			}
		}
		
		FLASH_Lock();
		return 0;
	}
}

void CheckLIC(PFUN_CheckLicCB pFunCB, void *pContext)
{
	static EmLicStatus emStatus = _LicStatus_StartUp;
	static uint32_t u32CheckTime = 0;
	static StUID stUID;
	static uint32_t u32CRC32 = ~0;
	static StBteaKey stLicRelease;
	static StBteaKey stLicRC;
	
	StLICCtrl *pCtrl = (StLICCtrl *)LIC_ADDRESS;
	
	if (u32CRC32 == ~0)
	{
		GetUID(&stUID);
		u32CRC32 = AppCRC32(~0);
		GetLic(&stLicRC, &stUID, u32CRC32, false);
		GetLic(&stLicRelease, &stUID, u32CRC32, true);
	}
	//WriteLic(&stLicRC, false, 0);
	
	if (emStatus == _LicStatus_StartUp) /* first startup */
	{
		if ((pCtrl->u32Header == LIC_RC_HEADER) 
			|| (pCtrl->u32Header == LIC_RELEASE_HEADER))
		{
			uint32_t *pTmp = (uint32_t *)LIC_ADDRESS;
			uint32_t u32CheckSum = 0;
			uint32_t i;
			for(i = 0; i < offsetof(StLICCtrl, u32CheckSum) / sizeof(uint32_t); i++)
			{
				u32CheckSum ^= pTmp[i];
			}
			if (u32CheckSum == pCtrl->u32CheckSum)
			{

				if (pCtrl->u32Header == LIC_RC_HEADER)
				{
					/* valid */
					if (stLicRC.s32RCLic == pCtrl->stBteaKey.s32RCLic)
					{
						uint64_t u64RCCount = pCtrl->u64RCCount;
						btea((int32_t *)(&u64RCCount), -2, pCtrl->stBteaKey.s32Key);
						if (u64RCCount > LIC_RC_MAX_COUNT)
						{
							emStatus = _LicStatus_InValid;							
						}
						else
						{	
							StBteaKey stLicTmp = stLicRC;
							WriteLic(&stLicTmp, false, u64RCCount + 1);
							emStatus = _LicStatus_RC;
						}
					}
					else		/* has been modified spitefully */
					{
						StBteaKey stLicTmp = stLicRC;
						WriteLic(&stLicTmp, false, LIC_RC_MAX_COUNT + 1);
						emStatus = _LicStatus_InValid;
					}						
				}
				else
				{
					/* valid */
					if (memcmp(&stLicRelease, &pCtrl->stBteaKey, sizeof(StBteaKey)) == 0)
					{
						emStatus = _LicStatus_Release;
					}
					else/* has been modified spitefully */
					{
						StBteaKey stLicTmp = stLicRC;
						WriteLic(&stLicTmp, false, LIC_RC_MAX_COUNT + 1);
						emStatus = _LicStatus_InValid;
					}			
				}
			}
		}
		else	/* write  the RC LIC */
		{
			StBteaKey stLicTmp = stLicRC;
			WriteLic(&stLicTmp, false, 0);
			emStatus = _LicStatus_RC;
		}
	}
	if (emStatus == _LicStatus_RC)
	{
		if (SysTimeDiff(u32CheckTime, g_u32SysTickCnt) > LIC_RC_TIMEOUT)
		{
			uint64_t u64RCCount = pCtrl->u64RCCount;
			StBteaKey stLicTmp = pCtrl->stBteaKey;
			
			u32CheckTime = g_u32SysTickCnt; 
			
			btea((int32_t *)(&u64RCCount), -2, pCtrl->stBteaKey.s32Key);
			WriteLic(&stLicTmp, false, u64RCCount + 1);
			if (u64RCCount > LIC_RC_MAX_COUNT)
			{
				emStatus = _LicStatus_InValid;
			}
			
			/* call the callback */
			if (pFunCB != NULL)
			{
				pFunCB(_LicStatus_RC_Timeout, pContext);
			}
		}
	}
	
	if (emStatus == _LicStatus_Release)
	{
	
	}
	
	if (emStatus == _LicStatus_InValid)
	{
		if (memcmp(&stLicRelease, &pCtrl->stBteaKey, sizeof(StBteaKey)) == 0)
		{
			emStatus = _LicStatus_Release;
			return;
		}
		/* call the callback */
		if (pFunCB != NULL)
		{
			pFunCB(_LicStatus_InValid, pContext);
		}
		
		if (SysTimeDiff(u32CheckTime, g_u32SysTickCnt) > LIC_INVALID_TIMEOUT)
		{
			u32CheckTime = g_u32SysTickCnt; 
			/* call the callback */
			if (pFunCB != NULL)
			{
				pFunCB(_LicStatus_InValid_Timeout, pContext);
			}
		}
	}
}

int32_t SetOptionByte(uint8_t u8Data)
{
	FLASH_Status emStatus = FLASH_COMPLETE;
	FLASH_Unlock();		
	do
	{
		FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
		emStatus = FLASH_EraseOptionBytes();
		if (emStatus != FLASH_COMPLETE)
		{
			break;
		}
		emStatus = FLASH_ProgramOptionByteData(OPTION_BYTE_ADDRESS, u8Data);
	
	} while (0);
	FLASH_Lock();

	return 	emStatus == FLASH_COMPLETE ? 0 : -1;
}

uint8_t GetOptionByte(void)
{
	return *((uint8_t *)OPTION_BYTE_ADDRESS);
}

#if 0
StSave g_stSave;

#define CHECK_SIZE		((sizeof(StSave) / sizeof(u16)) - 1)

bool WriteSaveData(void)
{
	FLASH_Status FLASHStatus = FLASH_COMPLETE;

	u16 u16CheckSum = 0, *pData = (u16 *)(&g_stSave);
	u32 i, u32Addr = EEPROM_ADDRESS;
	
	g_stSave.u16Head = EEPROM_DATA_HEAD;
	
	
	for (i = 0; i < CHECK_SIZE; i++)
	{
		u16CheckSum += pData[i];
	}
	g_stSave.u16CheckDum = u16CheckSum;

	FLASH_Unlock();
		
	/* Clear All pending flags */
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR); 
	
	FLASHStatus = FLASH_ErasePage(EEPROM_ADDRESS);

	
	for (i = 0; i < (CHECK_SIZE + 1); i++)
	{
		FLASHStatus = FLASH_ProgramHalfWord(u32Addr, pData[i]);
		if (FLASHStatus != FLASH_COMPLETE)
		{
			FLASH_Lock();
			return false;
		}
		u32Addr += sizeof(u16);
	}
	
	FLASH_Lock();
	return true;
}

void ReadSaveData(void)
{
	u16 u16CheckSum = 0, *pTmp = (u16 *)(&g_stSave);
	u32 i;
	
	memcpy(&g_stSave, (void *)EEPROM_ADDRESS, sizeof(StSave));

	if (g_stSave.u16Head != EEPROM_DATA_HEAD)
	{
		goto end;
	}

	for (i = 0; i < CHECK_SIZE; i++)
	{
		u16CheckSum += pTmp[i];
	}

	if (u16CheckSum == g_stSave.u16CheckDum)
	{	
		return;
	}
	
end:

	for (i = 0; i < MAX_MEMORY_CNT + 2; i++)
	{
		SaveMemoryFromDevice((g_stSave.stMemory + i));
	}
	
	WriteSaveData();
}
#endif
