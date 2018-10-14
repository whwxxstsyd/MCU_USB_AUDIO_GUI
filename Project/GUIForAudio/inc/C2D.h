#ifndef _C2D_H_
#define _C2D_H_
#include <stddef.h>           /* needed for definition of NULL */
#include <stdint.h> 
#include "lvgl\lvgl.h"
typedef int8_t		I8;
typedef uint8_t		U8;

typedef int16_t		I16;
typedef uint16_t	U16;
typedef int32_t		I32;
typedef uint32_t	U32;

typedef U32			LCD_COLOR;

typedef LCD_COLOR	GUI_COLOR;

#define GUI_MAKE_ARGB(a, r, g, b)	(((a << 24) & 0xFF000000) | ((r << 16) & 0x00FF0000) | ((g << 8) & 0x0000FF00) |((b << 0) & 0x000000FF))


#define I16P		I16              /*   signed 16 bits OR MORE ! */


#define GUI_XMIN -4095
#define GUI_XMAX  4095
#define GUI_YMIN -4095
#define GUI_YMAX  4095

#define GUI_LOCK()
#define GUI_UNLOCK()


/**********************************
*
*      Line styles
*
***********************************
*/

#define GUI_LS_SOLID        (0)
#define GUI_LS_DASH         (1)
#define GUI_LS_DOT          (2)
#define GUI_LS_DASHDOT      (3)
#define GUI_LS_DASHDOTDOT   (4)


/**********************************
*
*      Pen shapes
*
***********************************
*/

#define GUI_PS_ROUND        (0)
#define GUI_PS_FLAT         (1)
#define GUI_PS_SQUARE       (2)


#define GUI_45DEG			512
#define GUI_90DEG			(2*GUI_45DEG)
#define GUI_180DEG			(4*GUI_45DEG)
#define GUI_360DEG			(8*GUI_45DEG)


typedef union {
	U8  aColorIndex8[2];
	U16 aColorIndex16[2];
	U32 aColorIndex32[2];
} LCD_COLORINDEX_UNION;

typedef struct { I16P x, y; } GUI_POINT;
typedef struct { I16 x0, y0, x1, y1; } LCD_RECT;

typedef LCD_RECT        GUI_RECT;

struct GUI_CONTEXT {
	/* Variables in LCD module */
	LCD_COLORINDEX_UNION Lcd;
	LCD_RECT       ClipRect;
	U8             DrawMode;
	U8             SelLayer;
	U8             TextStyle;
	/* Variables in GL module */
	GUI_RECT* pClipRect_HL;                /* High level clip rectangle ... Speed optimization so drawing routines can optimize */
	U8        PenSize;
	U8        PenShape;
	U8        LineStyle;
	U8        FillStyle;

#if GUI_SUPPORT_UNICODE
	const GUI_UC_ENC_APILIST * pUC_API;    /* Unicode encoding API */
#endif
	I16P LBorder;
	I16P DispPosX, DispPosY;
	I16P DrawPosX, DrawPosY;
	I16P TextMode, TextAlign;
	GUI_COLOR Color, BkColor;           /* Required only when changing devices and for speed opt (caching) */
										/* Variables in WM module */
#if GUI_WINSUPPORT
	const GUI_RECT* WM__pUserClipRect;
	GUI_HWIN hAWin;
	int xOff, yOff;
#endif
	/* Variables in MEMDEV module (with memory devices only) */
#if GUI_SUPPORT_DEVICES
	const tLCDDEV_APIList* pDeviceAPI;  /* function pointers only */
	GUI_HMEM    hDevData;
	GUI_RECT    ClipRectPrev;
#endif
	/* Variables in Anitaliasing module */
#if GUI_SUPPORT_AA
	const tLCD_HL_APIList* pLCD_HL;     /* Required to reroute drawing (HLine & Pixel) to the AA module */
	U8 AA_Factor;
	U8 AA_HiResEnable;
#endif
};

void LCD_HL_DrawPixel(int x0, int y0);
void LCD_HL_DrawHLine(int x0, int y0, int x1);
void LCD_HL_DrawVLine(int x0, int y0, int y1);
void LCD_HL_FillRect(int x0, int y0, int x1, int y1);


void LCD_DrawVLine(int x, int y0, int y1);
void LCD_DrawHLine(int x, int y0, int y1);
void LCD_FillRect(int x0, int y0, int x1, int y1);


extern struct GUI_CONTEXT GUI_Context;

void GUI_Init(void);

GUI_COLOR GUI_GetBkColor(void);
GUI_COLOR GUI_GetColor(void);

U8 GUI_GetPenSize(void);
U8 GUI_GetPenShape(void);
U8 GUI_GetLineStyle(void);
   
U8 GUI_SetPenSize(U8 Size);
U8 GUI_SetPenShape(U8 Shape);
U8 GUI_SetLineStyle(U8 Style);
void GUI_SetBkColor(GUI_COLOR);
void GUI_SetColor(GUI_COLOR);

void GUI_DrawCircle(int x0, int y0, int r);
void GUI_DrawEllipse(int x0, int y0, int rx, int ry);
void GUI_DrawGraph(I16 *pay, int NumPoints, int x0, int y0);
void GUI_DrawGraphEx(I16 *pay, int NumPoints, int x0, int y0, int Numerator, int Denominator);
void GUI_DrawHLine(int y0, int x0, int x1);
void GUI_DrawLine(int x0, int y0, int x1, int y1);
void GUI_DrawLineRel(int dx, int dy);
void GUI_DrawLineTo(int x, int y);
void GUI_DrawPie(int x0, int y0, int r, int a0, int a1, int Type);
void GUI_DrawPixel(int x, int y);
void GUI_DrawPoint(int x, int y);
void GUI_DrawPolygon(const GUI_POINT* pPoints, int NumPoints, int x0, int y0);
void GUI_DrawPolyLine(const GUI_POINT* pPoints, int NumPoints, int x0, int y0);
void GUI_DrawFocusRect(const GUI_RECT *pRect, int Dist);
void GUI_DrawRect(int x0, int y0, int x1, int y1);
void GUI_DrawRectEx(const GUI_RECT *pRect);
void GUI_DrawVLine(int x0, int y0, int y1);
void GUI_FillCircle(int x0, int y0, int r);
void GUI_FillEllipse(int x0, int y0, int rx, int ry);
void GUI_FillPolygon(const GUI_POINT* pPoints, int NumPoints, int x0, int y0);
void GUI_FillRect(int x0, int y0, int x1, int y1);
void GUI_FillRectEx(const GUI_RECT* pRect);
void GUI_MoveRel(int dx, int dy);
void GUI_MoveTo(int x, int y);




void GL_DrawArc(int x0, int y0, int rx, int ry, int a0, int a1);
void GL_DrawCircle(int x0, int y0, int r);
void GL_DrawEllipse(int x0, int y0, int rx, int ry);
void GL_DrawHLine(int y0, int x0, int x1);
void GL_DrawPolygon(const GUI_POINT* pPoints, int NumPoints, int x0, int y0);
void GL_DrawPoint(int x, int y);
void GL_DrawLine1(int x0, int y0, int x1, int y1);
void GL_DrawLine1Ex(int x0, int y0, int x1, int y1, unsigned * pPixelCnt);
void GL_DrawLineRel(int dx, int dy);
void GL_DrawLineTo(int x, int y);
void GL_DrawLineToEx(int x, int y, unsigned * pPixelCnt);
void GL_DrawLine(int x0, int y0, int x1, int y1);
void GL_DrawLineEx(int x0, int y0, int x1, int y1, unsigned * pPixelCnt);
void GL_MoveTo(int x, int y);
void GL_FillCircle(int x0, int y0, int r);
void GL_FillCircleAA(int x0, int y0, int r);
void GL_FillEllipse(int x0, int y0, int rx, int ry);
void GL_FillPolygon(const GUI_POINT* pPoints, int NumPoints, int x0, int y0);

#endif // 
