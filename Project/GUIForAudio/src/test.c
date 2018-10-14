
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>


#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "key_led_ctrl.h"

#include "protocol.h"
#include "message.h"
#include "message_2.h"
#include "message_3.h"
#include "message_USB.h"


#include "user_init.h"
#include "user_api.h"
#include "lcd.h"
#include "I2C.h"
#include "gt9147.h"
#include "gui_driver.h"
#include "gui.h"
#include "C2D.h"
#include "logo.h"
#include "screen_protect.h"

#include "extern_peripheral.h"

#include "spi_flash.h"

#include "flash_bmp.h"


int32_t BMPLoadCallback(int32_t s32Type, void *pData, uint32_t u32Len, void *pContext)
{
	if (s32Type == _BMP_Load_Line)
	{
		StBMPLineInfo *pInfo = (StBMPLineInfo *)pData;

		int32_t s32Length = pInfo->u16LineEnd - pInfo->u16LineBegin;

		LCDSetCursor(pInfo->u16LineBegin, pInfo->u16LineIndex);	//设置光标位置 
		LCDSetXEnd(pInfo->u16LineEnd);
		LCDWriteRAMPrepare();     //开始写入GRAM	 	  
		LCDDMAWrite((const uint16_t *)pInfo->pLine, s32Length);	
		
	}
	
	return 0;
}

void Test(void)
{
#if 1
	{
		StSPIFlashBMPLoadCtrl stCtrl;
		int32_t s32FlushCnt = 0;
		uint32_t u32StartTime = g_u32SysTickCnt;
		SPIFlashBMPLoadStart(&stCtrl, 0, BMPLoadCallback, NULL);
		uint32_t u32EndTime = g_u32SysTickCnt;
		while (SPIFlashBMPLoadFlush(&stCtrl) != _BMP_Load_End)
		{
			s32FlushCnt++;
		}
		u32EndTime = g_u32SysTickCnt - u32StartTime;
		u32EndTime = u32EndTime;
	}
#endif
	
#if 0
	{
		uint32_t u32StartTime = g_u32SysTickCnt;
		uint32_t u32EndTime = g_u32SysTickCnt;
		SPIFlashBMPLoad(0, BMPLoadCallback, NULL);
		u32EndTime = g_u32SysTickCnt - u32StartTime;
		u32EndTime = u32EndTime;
	}
#endif
		
#if 0
	{
		SPIFlashBMPLoad(0, NULL, NULL);
	}
#endif
	
#if 0
	{
		BITMAPFILEHEADER stBmpHeader = { 0 };
		BITMAPINFOHEADER stInfoHeader = { 0 };
		
		int32_t s32Width = 800;
		int32_t s32Height = 480;
		bool boIsYP = false;

		static uint8_t u8Line[800 * 2];
		int32_t s32Count;
		StFlashBigDataCtrl stCtrl;
		uint8_t *u8Write = NULL;

		
		stBmpHeader.bfType = 0x4D42;

		stBmpHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		stInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
		stInfoHeader.biWidth = s32Width;
		if (!boIsYP)
		{
			stInfoHeader.biHeight = s32Height;
		}
		else
		{
			stInfoHeader.biHeight = 0 - s32Height;
		}
		stInfoHeader.biPlanes = 1;
		stInfoHeader.biBitCount = 16;
		stInfoHeader.biCompression = 0;
		stInfoHeader.biSizeImage = (((s32Width * 2 + 3) / 4) * 4) * s32Height + 2;/* 54 ----> 56 */

		stInfoHeader.biXPelsPerMeter = stInfoHeader.biYPelsPerMeter = 0x1E40;


		SPIFlashBigDataReadWriteBegin(&stCtrl, 0, 1024);
		
		s32Count = 0;
		u8Write = (uint8_t *)(&stBmpHeader);
		while(s32Count < sizeof(BITMAPFILEHEADER))
		{
			int32_t ret = SPIFlashBigDataWrite(&stCtrl, u8Write + s32Count,
				sizeof(BITMAPFILEHEADER) - s32Count);
			if (ret == -1)
			{
				break;
			}
			else if (ret == -2)
			{
				continue;
			}
			else
			{
				s32Count += ret;
			}
		}
		
		s32Count = 0;
		u8Write = (uint8_t *)(&stInfoHeader);
		while(s32Count < sizeof(BITMAPINFOHEADER))
		{
			int32_t ret = SPIFlashBigDataWrite(&stCtrl, u8Write + s32Count,
				sizeof(BITMAPINFOHEADER) - s32Count);
			if (ret == -1)
			{
				break;
			}
			else if (ret == -2)
			{
				continue;
			}
			else
			{
				s32Count += ret;
			}
		}
		
		{
			int32_t i;
			for (i = 0; i < s32Height; i++)
			{
				if (i == 0)
				{
					uint16_t *pTmp = (uint16_t *)u8Line;
					int32_t j;
					for (j = 0; j < 800; j++)
					{
						pTmp[j] = 0x001F;
					}
				}
				else if (i == 160)
				{
					uint16_t *pTmp = (uint16_t *)u8Line;
					int32_t j;
					for (j = 0; j < 800; j++)
					{
						pTmp[j] = 0x07E0;
					}
				}
				else if (i == 320)
				{
					uint16_t *pTmp = (uint16_t *)u8Line;
					int32_t j;
					for (j = 0; j < 800; j++)
					{
						pTmp[j] = 0xF800;
					}
				}
				
				
				s32Count = 0;
				u8Write = u8Line;
				while(s32Count < 800 * 2)
				{
					int32_t ret = SPIFlashBigDataWrite(&stCtrl, u8Write + s32Count,
						800 * 2 - s32Count);
					if (ret == -1)
					{
						break;
					}
					else if (ret == -2)
					{
						continue;
					}
					else
					{
						s32Count += ret;
					}
				}
			}
		}

	}
#endif	
#if 0
	{
	#define TEST_CNT	(9 * 1024)
		static uint8_t u8Write[TEST_CNT];
		static uint8_t u8Read[TEST_CNT];
		uint32_t i;
		int32_t s32Count;
		StFlashBigDataCtrl stCtrl;
		
		for (i = 0; i < TEST_CNT; i++)
		{
			u8Write[i] = i;
			u8Read[i] = 0;
		}
		
		SPIFlashBigDataReadWriteBegin(&stCtrl, 0, TEST_CNT);
#if 0		
		s32Count = 0;
		while(s32Count < TEST_CNT)
		{
			int32_t ret = SPIFlashBigDataWrite(&stCtrl, u8Write + s32Count,TEST_CNT - s32Count);
			if (ret == -1)
			{
				break;
			}
			else if (ret == -2)
			{
				continue;
			}
			else
			{
				s32Count += ret;
			}
		}
		s32Count = s32Count;
#endif		
		SPIFlashBigDataReadWriteBegin(&stCtrl, 0, TEST_CNT);
		s32Count = SPIFlashBigDataRead(&stCtrl, u8Read, TEST_CNT); 
		if (memcmp(u8Read, u8Write, TEST_CNT) == 0)
		{
			s32Count = s32Count;			
		}		
		s32Count = s32Count;
		
		
	}
#endif
#if 0
	{
		uint8_t u8Data[64] = { 0 };
		{
			uint8_t i;
			for (i = 0; i < 64; i++)
			{
				u8Data[i] = i;
			}
		}
		SPIFlashEraseSector(4096);
		while (SPIFlashWriteNoCheck(u8Data, 4096, 64) != 0)
		{
		
		}
		
		{
			uint8_t i;
			for (i = 0; i < 64; i++)
			{
				u8Data[i] = 63 - i;
			}
		}
		while (SPIFlashRead(u8Data, 4096, 64) != 0)
		{
		
		}
		u8Data[0] = u8Data[0];		
	}
#endif
}

