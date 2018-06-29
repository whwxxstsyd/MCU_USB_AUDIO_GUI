#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "C2D.h"
#include "lvgl/lvgl.h"
#include "gui.h"
#include "logo.h"


#if USE_LVGL

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
		{ 75, 29, 21 },
	},
	{
		{ { 78, 406, },{ 750, 200, },{ 1046, 372 },{ 954, 843 },{ 48, 843 } },
		{ 255, 255, 255 }
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
		{ 255, 255, 255 }
	},
	{
		{ { 472, 610, },{ 534, 280, },{ 595, 260, },{ 535, 614, }, },
		{ 255, 255, 255 }
	},
	{
		{ { 712, 613, },{ 776, 619, },{ 793, 524, },{ 727, 517 }, },
		{ 33, 101, 230 },
	},
};

static StCycle s_stCycle[4] =
{
	{
		290, 535, 75,
		{ 255, 255, 255 }
	},
	{
		500, 679, 75,
		{ 255, 255, 255 }
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
			s_stPentagon[i].stPoint[j].x = (s_stPentagon[i].stPoint[j].x * LV_VER_RES + (1080 / 2)) / 1080;
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

	GUI_SetColor(GUI_MAKE_ARGB(0, 255, 255, 255));
	GUI_FillRect(0, 0, LV_HOR_RES - 1, LV_VER_RES - 1);

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
#endif

