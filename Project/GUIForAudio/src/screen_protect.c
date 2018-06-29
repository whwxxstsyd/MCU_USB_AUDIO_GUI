#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "C2D.h"
#include "lvgl/lvgl.h"
#include "gui.h"
#include "logo.h"
#include "screen_protect.h"

#if USE_LVGL

#ifndef _WIN32
#include "user_api.h"
#define printf(x, ...)
#endif

StScreenProtect s_stScreenProtect = { 0 };


const uint32_t c_u32ScreenProtectTime[_ScreenProtect_Reserved] =
{
	15 * 1000, 30 * 1000, 60 * 1000,  2 * 60 * 1000,
	5 * 60 * 1000,  10 * 60 * 1000,  ~0,
};


__weak uint32_t SysTimeDiff(uint32_t u32Begin, uint32_t u32End)
{
	if (u32End >= u32Begin)
	{
		return (u32End - u32Begin);
	}
	else
	{
		return ((uint32_t)(~0)) - u32Begin + u32End;
	}
}


void SrceenProtectInit(void)
{
	s_stScreenProtect.u8ProtectMode = _ScreenProtect_Mode_Logo;
	s_stScreenProtect.u8ProtectTimeIndex = _ScreenProtect_15S;
}

bool SrceenProtectIsStart(void)
{
	return s_stScreenProtect.boCurState;
}

void SrceenProtectReset(void)
{
	s_stScreenProtect.u32LastKeyTime = lv_tick_get();
}

void SrceenProtectForceStart(void)
{
	s_stScreenProtect.u8ForceProtect = _ScreenProtect_Force_Start;
}

void SrceenProtectForceStop(void)
{
	s_stScreenProtect.u8ForceProtect = _ScreenProtect_Force_Stop;
}

__weak void LCDSetBackLight(uint8_t u8Light)
{
}

static void SrceenProtectStart(void)
{
	if (s_stScreenProtect.boCurState)
	{
		return;
	}
	if (s_stScreenProtect.u8ProtectMode == _ScreenProtect_Mode_Logo)
	{
		LOGODraw();
	}
	else
	{
		/* TODO, close background light power */
		LCDSetBackLight(0);
	}
	s_stScreenProtect.boCurState = true;
	printf("%s\n", __FUNCTION__);
}

static void SrceenProtectStop(void)
{
	if (!s_stScreenProtect.boCurState)
	{
		return;
	}
	
	LCDSetBackLight(1);

	SrceenProtectReset();
	s_stScreenProtect.boCurState = false;
	lv_obj_invalidate(lv_scr_act());
	printf("%s\n", __FUNCTION__);
}

void SrceenProtectFlush(void)
{
	if (s_stScreenProtect.u8ProtectTimeIndex < _ScreenProtect_Close)
	{
		uint32_t u32ProtectTime = c_u32ScreenProtectTime[s_stScreenProtect.u8ProtectTimeIndex];
		uint32_t u32TimeDiff = 0;
		
#ifndef _WIN32
		__disable_irq();
#endif
		{
			uint32_t u32Cur = lv_tick_get();
			u32TimeDiff = SysTimeDiff(s_stScreenProtect.u32LastKeyTime, u32Cur);
		}
#ifndef _WIN32
		__enable_irq();
#endif
		
		if (!s_stScreenProtect.boCurState)
		{
			if (u32TimeDiff >= u32ProtectTime)
			{
				SrceenProtectStart();
			}
		}
		
	}
	
	if (s_stScreenProtect.u8ForceProtect == _ScreenProtect_Force_Start)
	{
		SrceenProtectStart();
		s_stScreenProtect.u8ForceProtect = 0;
	}
	else if (s_stScreenProtect.u8ForceProtect == _ScreenProtect_Force_Stop)
	{
		SrceenProtectStop();
		s_stScreenProtect.u8ForceProtect = 0;
	}
}


int32_t SrceenProtectSetTime(uint8_t u8TimeIndex)
{
	if (u8TimeIndex >= _ScreenProtect_Reserved)
	{
		return -1;
	}

	s_stScreenProtect.u8ProtectTimeIndex = u8TimeIndex;

	return 0;
}

int32_t SrceenProtectSetMode(uint8_t u8ModeIndex)
{
	if (u8ModeIndex >= _ScreenProtect_Mode_Reserved)
	{
		return -1;
	}

	s_stScreenProtect.u8ProtectMode = u8ModeIndex;
	
	return 0;
}


uint8_t SrceenProtectGetTime(void)
{
	return s_stScreenProtect.u8ProtectTimeIndex;
}

uint8_t SrceenProtectGetMode(void)
{
	return s_stScreenProtect.u8ProtectMode;
}

#endif
