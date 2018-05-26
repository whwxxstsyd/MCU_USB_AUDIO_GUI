
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
#include "C2D.h"

typedef struct _tagStPentagon
{
	GUI_POINT stPoint[5];
	lv_color24_t stColor;
}StPentagon;

typedef struct _tagStTactangle
{
	GUI_POINT stPoint[4];
	lv_color24_t stColor;
}StTactangle;

typedef struct _tagStCycle
{
	uint16_t x;
	uint16_t y;
	uint16_t r;
	lv_color24_t stColor;
}StCycle;

static StPentagon s_stPentagon[3] =
{
	{
		{ { 52, 395, },{ 750, 180, },{ 1075, 368 },{ 976, 866 },{ 21, 866 } },
		{75, 29, 21},
	},
	{ 
		{ { 78, 406, },{ 750, 200, },{ 1046, 372 },{ 954, 843 },{ 48, 843 } },
		{ 51, 51, 51 }
	},
	{
		{ { 110, 420, },{ 722, 233, },{ 1003, 396, },{ 915, 817, },{ 84, 817 } },
		{ 75, 29, 21 }
	},
};

static StTactangle s_stTactangle[3] =
{
	{
		{ { 234, 817, },{ 255, 600, },{ 316, 600, },{ 297, 817, }, },
		{ 51, 51, 51 }
	},
	{
		{ { 472, 610, },{ 534, 280, },{ 595, 260, },{ 535, 614, }, },
		{ 51, 51, 51 }
	},
	{
		{ { 712, 613, },{ 776, 619, },{ 793, 524, },{ 727, 517 }, },
		{33, 101, 230},
	},
};

static StCycle s_stCycle[4] =
{
	{
		290, 535, 75,
		{ 51, 51, 51 }
	},
	{
		500, 679, 75,
		{ 51, 51, 51 }
	},
	{
		768, 468, 71,
		{ 33, 101, 230 },
	},
	{
		739, 680, 71,
		{ 33, 101, 230 },
	},
};

void LOGOCoordsInit(void)
{
	uint32_t i;
	for (i = 0; i < 3; i++)
	{
		uint32_t j;
		for (j = 0; j < 5; j++)
		{
			s_stPentagon[i].stPoint[j].x = (s_stPentagon[i].stPoint[j].x * LV_VER_RES + (1080 / 2))/ 1080;
			s_stPentagon[i].stPoint[j].y = (s_stPentagon[i].stPoint[j].y * LV_VER_RES + (1080 / 2)) / 1080;
			
			s_stPentagon[i].stPoint[j].x += ((LV_HOR_RES - LV_VER_RES) / 2);
		}
	}

	for (i = 0; i < 3; i++)
	{
		uint32_t j;
		for (j = 0; j < 4; j++)
		{
			s_stTactangle[i].stPoint[j].x = (s_stTactangle[i].stPoint[j].x * LV_VER_RES + (1080 / 2)) / 1080;
			s_stTactangle[i].stPoint[j].y = (s_stTactangle[i].stPoint[j].y * LV_VER_RES + (1080 / 2)) / 1080;

			s_stTactangle[i].stPoint[j].x += ((LV_HOR_RES - LV_VER_RES) / 2);
		}
	}

	for (i = 0; i < 4; i++)
	{
		s_stCycle[i].x = (s_stCycle[i].x * LV_VER_RES + (1080 / 2)) / 1080;
		s_stCycle[i].y = (s_stCycle[i].y * LV_VER_RES + (1080 / 2)) / 1080;
		s_stCycle[i].r = (s_stCycle[i].r * LV_VER_RES + (1080 / 2)) / 1080;

		s_stCycle[i].x += ((LV_HOR_RES - LV_VER_RES) / 2);

	}
}

void LOGODraw(void)
{

	uint32_t i;
	GUI_SetColor(GUI_MAKE_ARGB(0, 51, 51, 51));
	GUI_FillRect(0, 0, LV_HOR_RES - 1, LV_VER_RES - 1) ;

	for (i = 0; i < 3; i++)
	{
		GUI_SetColor(GUI_MAKE_ARGB(0, s_stPentagon[i].stColor.red, s_stPentagon[i].stColor.green, s_stPentagon[i].stColor.blue));
		GUI_FillPolygon(s_stPentagon[i].stPoint, 5, 0, 0);
	}

	for (i = 0; i < 3; i++)
	{
		GUI_SetColor(GUI_MAKE_ARGB(0, s_stTactangle[i].stColor.red, s_stTactangle[i].stColor.green, s_stTactangle[i].stColor.blue));
		GUI_FillPolygon(s_stTactangle[i].stPoint, 4, 0, 0);
	}

	for (i = 0; i < 4; i++)
	{
		GUI_SetColor(GUI_MAKE_ARGB(0, s_stCycle[i].stColor.red, s_stCycle[i].stColor.green, s_stCycle[i].stColor.blue));
		GUI_FillCircle(s_stCycle[i].x, s_stCycle[i].y, s_stCycle[i].r);
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
	u32 u32DeviceKeepAliveTime = 0;
	u32 u32DeviceGetEchoTime = -20 * 1000;

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

	I2CInit();
	GT9147Init();
	
	ChangeLedBlinkState(0, 0, true);
	
#if USE_LVGL
	LvglInit();
	GUI_Init();

	LOGOCoordsInit();
	LOGODraw();
	Delay(3 * 1000);

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
#if USE_LVGL && 0
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
				if (BaseCmdProcess(pMsgIn, &c_stUartIOTCB) == 0)			
				{
					MessageUartRelease(pMsgIn);
				}
				else				
				{
					int32_t res = 0;
					PCEchoProcess(pMsgIn, &c_stUartIOTCB);
					res = MessageUart2Write(pMsgIn->pData, pMsgIn->boNeedFree, 0, pMsgIn->s32Length);
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
			
			if (pMsg2In != NULL)
			{
				if (BaseCmdProcess(pMsg2In, NULL) == 0)
				{
					if (SysTimeDiff(u32DeviceGetEchoTime, g_u32SysTickCnt) > 10 * 1000)
					{
						DeviceGetCurState();
					}
					u32DeviceGetEchoTime = g_u32SysTickCnt;
					MessageUart2Release(pMsg2In);
				}
				else
				{
					int32_t res = 0;
					PCEchoProcess(pMsg2In, NULL);
					res = MessageUartWrite(pMsg2In->pData, pMsg2In->boNeedFree, 0, pMsg2In->s32Length);
					if (res != 0)
					{
						if (pMsg2In->boNeedFree)
						{
							free(pMsg2In->pData);
						}
					}						
					MessageUart2ReleaseNoReleaseData(pMsg2In);
				}
				//u32DeviceKeepAliveTime = g_u32SysTickCnt;
			}
			
			
			KeyBufGetEnd(pKeyIn);

			if (SysTimeDiff(u32DeviceKeepAliveTime, g_u32SysTickCnt) >= 5 * 1000)
			{		
				u32DeviceKeepAliveTime = g_u32SysTickCnt;
				DeviceSendKeepAlive();
			}

		}
#endif		
    }
}



