/*
*********************************************************************************************************
*                                             uC/GUI V3.98
*                        Universal graphic software for embedded applications
*
*                       (c) Copyright 2002, Micrium Inc., Weston, FL
*                       (c) Copyright 2002, SEGGER Microcontroller Systeme GmbH
*
*              µC/GUI is protected by international copyright laws. Knowledge of the
*              source code may not be used to write a similar product. This file may
*              only be used in accordance with a license and should not be redistributed
*              in any way. We appreciate your understanding and fairness.
*
----------------------------------------------------------------------
File        : GUI2DLib.C
Purpose     : Main part of the 2D graphics library
---------------------------END-OF-HEADER------------------------------
*/

#include "C2D.h"

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
struct GUI_CONTEXT GUI_Context = { 0 };

/*********************************************************************
*
*       GUI_MoveRel
*/
void GUI_MoveRel(int dx, int dy) { /*tbd: GL_LinePos. */
	GUI_LOCK();
	GUI_Context.DrawPosX += dx;
	GUI_Context.DrawPosY += dy;
	GUI_UNLOCK();
}

/*********************************************************************
*
*       GL_MoveTo
*/
void GL_MoveTo(int x, int y) {
	GUI_Context.DrawPosX = x;
	GUI_Context.DrawPosY = y;
}

/*********************************************************************
*
*       GUI_MoveTo
*/
void GUI_MoveTo(int x, int y) {
	GUI_LOCK();
#if (GUI_WINSUPPORT)
	WM_ADDORG(x, y);
#endif
	GL_MoveTo(x, y);
	GUI_UNLOCK();
}

/*********************************************************************
*
*       Rectangle filling / inverting
*
**********************************************************************
*/
/*********************************************************************
*
*       _DrawRect
*/
static void _DrawRect(int x0, int y0, int x1, int y1) {
	LCD_DrawHLine(x0, y0, x1);
	LCD_DrawHLine(x0, y1, x1);
	LCD_DrawVLine(x0, y0 + 1, y1 - 1);
	LCD_DrawVLine(x1, y0 + 1, y1 - 1);
}

/*********************************************************************
*
*       GUI_DrawRect
*/
void GUI_DrawRect(int x0, int y0, int x1, int y1) {
#if (GUI_WINSUPPORT)
	int Off;
	GUI_RECT r;
#endif
	GUI_LOCK();
#if (GUI_WINSUPPORT)
	Off = GUI_Context.PenSize - 1;
	WM_ADDORG(x0, y0);
	WM_ADDORG(x1, y1);
	r.x0 = x0 - Off;
	r.x1 = x1 + Off;
	r.y0 = y0 - Off;
	r.y1 = y1 + Off;
	WM_ITERATE_START(&r); {
#endif
		_DrawRect(x0, y0, x1, y1);
#if (GUI_WINSUPPORT)
	} WM_ITERATE_END();
#endif
	GUI_UNLOCK();
}




U8 GUI_GetPenSize(void) {
	U8 r;
	GUI_LOCK();
	r = GUI_Context.PenSize;
	GUI_UNLOCK();
	return r;
}

/*********************************************************************
*
*       GUI_GetPenShape
*/
U8 GUI_GetPenShape(void) {
	U8 r;
	GUI_LOCK();
	r = GUI_Context.PenShape;
	GUI_UNLOCK();
	return r;
}

/*********************************************************************
*
*       GUI_SetPenSize
*/
U8 GUI_SetPenSize(U8 PenSize) {
	U8 r;
	GUI_LOCK();
	r = GUI_Context.PenSize;
	GUI_Context.PenSize = PenSize;
	GUI_UNLOCK();
	return r;
}

/*********************************************************************
*
*       GUI_SetPenShape
*/
U8 GUI_SetPenShape(U8 PenShape) {
	U8 r;
	GUI_LOCK();
	r = GUI_Context.PenShape;
	GUI_Context.PenShape = PenShape;
	GUI_UNLOCK();
	return r;
}

/*********************************************************************
*
*       GUI_GetLineStyle
*/
U8 GUI_GetLineStyle(void) {
	U8 r;
	GUI_LOCK();
	r = GUI_Context.LineStyle;
	GUI_UNLOCK();
	return r;
}

/*********************************************************************
*
*       GUI_SetLineStyle
*/
U8 GUI_SetLineStyle(U8 LineStyle) {
	U8 r;
	GUI_LOCK();
	r = GUI_Context.LineStyle;
	GUI_Context.LineStyle = LineStyle;
	GUI_UNLOCK();
	return r;
}

/*********************************************************************
*
*       GUI_GetBkColor
*/
GUI_COLOR GUI_GetBkColor(void) {
	GUI_COLOR r;
	GUI_LOCK();
	r = GUI_Context.BkColor;
	GUI_UNLOCK();
	return r;
}


/*********************************************************************
*
*       GUI_GetColor
*/
GUI_COLOR GUI_GetColor(void) {
	GUI_COLOR r;
	GUI_LOCK();
	r = GUI_Context.Color;
	GUI_UNLOCK();
	return r;
}

/*********************************************************************
*
*       GUI_SetBkColor
*/
void GUI_SetBkColor(GUI_COLOR color) {
	GUI_LOCK(); {
		GUI_Context.BkColor = color;
	} GUI_UNLOCK();
}

/*********************************************************************
*
*       GUI_SetColor
*/
void GUI_SetColor(GUI_COLOR color) {
	GUI_LOCK(); {
		GUI_Context.Color = color;
	} GUI_UNLOCK();
}


void GUI_Init(void) {
	struct GUI_CONTEXT* pContext = &GUI_Context;
	/* memset(..,0,..) is not required, as this function is called only at startup of the GUI when data is 0 */
#if GUI_SUPPORT_DEVICES
	pContext->pDeviceAPI = LCD_aAPI[0]; /* &LCD_L0_APIList; */
#endif
	pContext->pClipRect_HL = &GUI_Context.ClipRect;
	pContext->ClipRect.x0 = 0;
	pContext->ClipRect.y0 = 0;
	pContext->ClipRect.x1 = LV_HOR_RES;
	pContext->ClipRect.y1 = LV_VER_RES;
#if GUI_SUPPORT_AA
	pContext->pLCD_HL = &_HL_APIList;
#endif
	pContext->pClipRect_HL = &GUI_Context.ClipRect;
	pContext->PenSize = 1;
	/* Variables in WM module */
#if GUI_WINSUPPORT
	pContext->hAWin = WM_GetDesktopWindow();
#endif
	/* Variables in GUI_AA module */
#if GUI_SUPPORT_AA
	pContext->AA_Factor = 3;
#endif
	pContext->Color = 0xFFFFFF;
	pContext->BkColor = 0;
#if GUI_SUPPORT_UNICODE
	pContext->pUC_API = &GUI_UC_None;
#endif
}



#define RETURN_IF_Y_OUT() \
  if (y < GUI_Context.ClipRect.y0) return;             \
  if (y > GUI_Context.ClipRect.y1) return;

#define RETURN_IF_X_OUT() \
  if (x < GUI_Context.ClipRect.x0) return;             \
  if (x > GUI_Context.ClipRect.x1) return;

#define CLIP_X() \
  if (x0 < GUI_Context.ClipRect.x0) { x0 = GUI_Context.ClipRect.x0; } \
  if (x1 > GUI_Context.ClipRect.x1) { x1 = GUI_Context.ClipRect.x1; }

#define CLIP_Y() \
  if (y0 < GUI_Context.ClipRect.y0) { y0 = GUI_Context.ClipRect.y0; } \
  if (y1 > GUI_Context.ClipRect.y1) { y1 = GUI_Context.ClipRect.y1; }

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/
/*********************************************************************
*
*       LCD_DrawVLine
*/
void LCD_DrawVLine(int x, int y0, int y1) {
	/* Perform clipping and check if there is something to do */
	RETURN_IF_X_OUT();
	CLIP_Y();
	if (y1 < y0) {
		return;
	}
	/* Call driver to draw */
	LCD_HL_DrawVLine(x, y0, y1);
}

/*********************************************************************
*
*       LCD_DrawHLine
*/
void LCD_DrawHLine(int x0, int y, int x1) {
	/* Perform clipping and check if there is something to do */
	RETURN_IF_Y_OUT();
	CLIP_X();
	if (x1 < x0)
		return;
	/* Call driver to draw */
	LCD_HL_DrawHLine(x0, y, x1);
}

/*********************************************************************
*
*       LCD_FillRect
*/
void LCD_FillRect(int x0, int y0, int x1, int y1) {
	/* Perform clipping and check if there is something to do */
	CLIP_X();
	if (x1 < x0)
		return;
	CLIP_Y();
	if (y1 < y0)
		return;
	/* Call driver to draw */
	LCD_HL_FillRect(x0, y0, x1, y1);
}

/*************************** End of file ****************************/

