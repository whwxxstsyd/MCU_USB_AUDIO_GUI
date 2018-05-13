#include "C2D.h"

void LCD_HL_DrawHLine(int x0, int y0, int x1)
{
	GUI_COLOR stColor = GUI_GetColor();
	
	lv_disp_fill(x0, y0, x1, y0, LV_COLOR_MAKE(((stColor >> 16) & 0xFF), 
		((stColor >> 8) & 0xFF), ((stColor >> 0) & 0xFF)));
}
void LCD_HL_DrawPixel(int x0, int y0)
{
	GUI_COLOR stColor = GUI_GetColor();
	lv_disp_fill(x0, y0, x0, y0, LV_COLOR_MAKE(((stColor >> 16) & 0xFF),
		((stColor >> 8) & 0xFF), ((stColor >> 0) & 0xFF)));
}

void LCD_HL_DrawVLine(int x0, int y0, int y1)
{
	GUI_COLOR stColor = GUI_GetColor();
	lv_disp_fill(x0, y0, x0, y1, LV_COLOR_MAKE(((stColor >> 16) & 0xFF),
		((stColor >> 8) & 0xFF), ((stColor >> 0) & 0xFF)));
}

void LCD_HL_FillRect(int x0, int y0, int x1, int y1)
{
	GUI_COLOR stColor = GUI_GetColor();
	lv_disp_fill(x0, y0, x1, y1, LV_COLOR_MAKE(((stColor >> 16) & 0xFF),
		((stColor >> 8) & 0xFF), ((stColor >> 0) & 0xFF)));
}

