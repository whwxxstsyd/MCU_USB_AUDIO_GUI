
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
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


#include "user_init.h"
#include "user_api.h"
#include "lcd.h"
#include "I2C.h"
#include "gt9147.h"
#include "gui_driver.h"
#include "gui.h"

#define _GUI_Dir_Horizontal 0

void GUIDrawLine(uint16_t u16Xpos, uint16_t u16Ypos, uint16_t u16Length, 
	uint16_t u16Direction, uint16_t u16Color)
{
	uint16_t i;
	uint16_t *pTmp = (uint16_t *)(SRAM3_ADDR);
	pTmp = pTmp + u16Ypos * LV_HOR_RES + u16Xpos;
	for (i = 0; i < u16Length; i++)
	{
		pTmp[i] |= u16Color;
	}
}

void GUIDrawFullCircle(uint16_t u16XPos, uint16_t u16YPos, uint16_t u16Radius, 
	uint16_t u16Color)
{
	int16_t  D;    /* Decision Variable */ 
	uint16_t  CurX;/* Current X Value */
	uint16_t  CurY;/* Current Y Value */ 

	D = 3 - (u16Radius << 1);

	CurX = 0;
	CurY = u16Radius;

	while (CurX <= CurY)
	{

		GUIDrawLine(u16XPos - CurX, u16YPos + CurY, 2 * CurX, _GUI_Dir_Horizontal, u16Color);
		GUIDrawLine(u16XPos - CurX, u16YPos - CurY, 2 * CurX, _GUI_Dir_Horizontal, u16Color);

		GUIDrawLine(u16XPos - CurY, u16YPos + CurX, 2 * CurY, _GUI_Dir_Horizontal, u16Color);
		GUIDrawLine(u16XPos - CurY, u16YPos - CurX, 2 * CurY, _GUI_Dir_Horizontal, u16Color);

		if (D < 0)
		{ 
			D += ((CurX << 2) + 6);
		}
		else
		{
			D += (((CurX - CurY) << 2) + 10);
			CurY--;
		}
		CurX++;
	}
}

void EnableWatchDog(void)
{
	/* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency
	 dispersion) */
	/* Enable write access to IWDG_PR and IWDG_RLR registers */
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

	/* IWDG counter clock: LSI/32 */
	IWDG_SetPrescaler(IWDG_Prescaler_32);

	/* Set counter reload value to obtain 250ms IWDG TimeOut.
	 Counter Reload Value = 250ms/IWDG counter clock period
						  = 250ms / (LSI/32)
						  = 0.25s / (LsiFreq/32)
						  = LsiFreq/(32 * 4)
						  = LsiFreq/128
	*/
	IWDG_SetReload(40000/128);

	/* Reload IWDG counter */
	IWDG_ReloadCounter();

	/* Enable IWDG (the LSI oscillator will be enabled by hardware) */
	IWDG_Enable();	
}

int main (void)
{
	u32 u32Time = 0;
	u32 u32LvglHandlerTime = 0;

	KeyBufInit();
	GlobalStateInit();

	PeripheralPinClkEnable();
	OpenSpecialGPIO();
	
	//ReadSaveData();
	KeyLedInit();
	CodeSwitchInit();
	
	MessageUartInit();
	MessageUart2Init();

	SysTickInit();
	
	
	LCDInit();
	LCDClear(RGB(255, 0, 0));
	{
		int32_t i = 0;
		uint32_t *pTmp = (uint32_t *)SRAM3_ADDR;
		for (i = 0; i < 512 * 1024 / 2; i++)
		{
			*pTmp++ = 0;
		}

//		for (i = 0; i < 512 * 1024; i++)
//		{
//			uint8_t u8Gray = i;
//			*pTmp1++ = RGB(u8Gray, u8Gray, u8Gray);
//		}
		
		GUIDrawFullCircle(400, 150, 270 - 120, RGB(255, 0, 0));
		GUIDrawFullCircle(400 + 104, 270 + 60, 150, RGB(0, 255, 0));
		GUIDrawFullCircle(400 - 104, 270 + 60, 150, RGB(0, 0, 255));
		
		LCDDMAWrite((const uint16_t *)SRAM3_ADDR, 384000);	

		i = i;
	}

	I2CInit();
	GT9147Init();
	
	ChangeLedBlinkState(0, 0, true);
	
#if USE_LVGL
	LvglInit();
#endif	
	
#if USE_LVGL
	{
		CreateTableView();
	}
#endif	
	//EnableWatchDog();


    while(1) 
	{
		{
			//IWDG_ReloadCounter();
		}
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
		if (SysTimeDiff(u32LvglHandlerTime, g_u32SysTickCnt) != 0)
		{		
			u32LvglHandlerTime = g_u32SysTickCnt;
#if USE_LVGL
			LvglFlushTask();
#endif	
		}

		if (SysTimeDiff(u32Time, g_u32SysTickCnt) > 100)
		{
			u32Time = g_u32SysTickCnt;
#if USE_LVGL
			LvglDispMem();
#endif	
		}

#if 1		
		{
			StIOFIFO *pMsgIn = MessageUartFlush(false);
			StIOFIFO *pMsg2In = MessageUart2Flush(false);
			void *pKeyIn = KeyBufGetBuf();

			if (pKeyIn != NULL)
			{
				KeyProcess(pKeyIn);
			}	
			if (pMsgIn != NULL)
			{
				if (BaseCmdProcess(pMsgIn, &c_stUartIOTCB) != 0)			
				{
					PCEchoProcess(pMsgIn);
				}
				
				{
					int32_t res = MessageUart2Write(pMsgIn->pData, pMsgIn->boNeedFree, 0, pMsgIn->s32Length);
					if (res != 0)
					{
						if (pMsgIn->boNeedFree)
						{
							free(pMsgIn->pData);
						}
					}						
				}
				
				MessageUartReleaseNoReleaseData(pMsgIn);
			}
			
			if (pMsg2In != NULL)
			{
				
				{
					int32_t res = MessageUartWrite(pMsg2In->pData, pMsg2In->boNeedFree, 0, pMsg2In->s32Length);
					if (res != 0)
					{
						if (pMsg2In->boNeedFree)
						{
							free(pMsg2In->pData);
						}
					}						
				}
				
				MessageUart2ReleaseNoReleaseData(pMsg2In);
			}
			
			
			KeyBufGetEnd(pKeyIn);				

		}
#endif		
    }
}



