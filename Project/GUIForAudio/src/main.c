
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
	u32 u32DeviceKeepAliveTime = 0;
	u32 u32DeviceGetEchoTime = (u32)(-20 * 1000);

	KeyBufInit();
	GlobalStateInit();

	PeripheralPinClkEnable();
	OpenSpecialGPIO();
	
	ReadSaveData();
	KeyLedInit();
	CodeSwitchInit();
	
	SPIFlashInit();
	
	MessageUartInit();
	MessageUart2Init();
	MessageUart3Init();
	
	ExternPeripheralInit();


	SysTickInit();
	
	/* must after SysTickInit */
	MessageUSBInit();
	
	LCDInit();

	I2CInit();
	GT9147Init();
	
	ChangeLedBlinkState(0, 0, true);
#if 0
	{
		void Test(void);
		Test();
	}
#endif
	
#if USE_LVGL
	LvglInit();
	GUI_Init();
#endif	

	LOGOCoordsInit();
	LOGODrawNoFlush();
	Delay(3 * 1000);


	LoadPowerOffMemoryToDevice();
	
#if USE_LVGL
	{
		ReflushLanguageInit();
		CreateTableInit();
		CreateTableView(~0);
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
			if (!SrceenProtectIsStart())
			{
				LvglFlushTask();
			}
#endif	
		}
#if USE_LVGL
		SrceenProtectFlush();
		ReflushLanguage();
#endif	

		if (SysTimeDiff(u32Time, g_u32SysTickCnt) > 100)
		{
			u32Time = g_u32SysTickCnt;
#if USE_LVGL && 0
			LvglDispMem();
#endif	

		}

#if 1		
		{
			StIOFIFO *pMsgIn = MessageUartFlush(false);
			StIOFIFO *pMsg2In = MessageUart2Flush(false);
			StIOFIFO *pMsg3In = MessageUart3Flush(false);
			void *pMsgUSB = MessageUSBFlush(false);
			
			void *pKeyIn = KeyBufGetBuf();

			if (pKeyIn != NULL)
			{
				KeyProcess(pKeyIn);
			}	
			if (pMsgIn != NULL)
			{
				if (BaseCmdProcess(pMsgIn, &c_stUartIOTCB) == 0)			
				{
					MessageUartRelease(pMsgIn);
				}
				else				
				{
					int32_t res = 0;
					PCEchoProcess(pMsgIn, &c_stUartIOTCB);
					res = MessageUart3Write(pMsgIn->pData, pMsgIn->boNeedFree, 0, 
						pMsgIn->s32Length);
					if (res != 0)
					{
						if (pMsgIn->boNeedFree)
						{
							free(pMsgIn->pData);
						}
					}						
					MessageUartReleaseNoReleaseData(pMsgIn);
				}
				
			}
			
			if (pMsg3In != NULL)
			{
				if (BaseCmdProcess(pMsg3In, NULL) == 0)
				{
					if (SysTimeDiff(u32DeviceGetEchoTime, g_u32SysTickCnt) > 10 * 1000)
					{
						DeviceGetCurState();
					}
					u32DeviceGetEchoTime = g_u32SysTickCnt;
					MessageUart3Release(pMsg3In);
				}
				else
				{
					int32_t res = 0;
					PCEchoProcess(pMsg3In, NULL);
					res = MessageUartWrite(pMsg3In->pData, pMsg3In->boNeedFree, 0, 
							pMsg3In->s32Length);
					if (res != 0)
					{
						if (pMsg3In->boNeedFree)
						{
							free(pMsg3In->pData);
						}
					}						
					MessageUart3ReleaseNoReleaseData(pMsg3In);
				}
				//u32DeviceKeepAliveTime = g_u32SysTickCnt;
			}
			
			if (pMsgUSB != NULL)
			{
				PCEchoProcess(pMsgUSB, NULL);
			}
			
			
			MessageUart2Release(pMsg2In);
			
			KeyBufGetEnd(pKeyIn);
			
			MessageUSBRelease(pMsgUSB);	


			if (SysTimeDiff(u32DeviceKeepAliveTime, g_u32SysTickCnt) >= 5 * 1000)
			{		
				u32DeviceKeepAliveTime = g_u32SysTickCnt;
				DeviceSendKeepAlive();
			}

		}
#endif	
		PowerOffMemoryFlush();	
		{
			void UpgradeFlush(void);
			UpgradeFlush();
		}		
    }
}



