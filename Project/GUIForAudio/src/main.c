
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


    while(1) 
	{
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
		if (SysTimeDiff(u32LvglHandlerTime, g_u32SysTickCnt) >= 2)
		{		
#if USE_LVGL
			LvglFlushTask();
#endif	
			u32LvglHandlerTime = g_u32SysTickCnt;
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



