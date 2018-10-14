#ifndef FLASH_BMP_H_
#define FLASH_BMP_H_
#include <stdint.h>
#include <stdbool.h>

#include "spi_flash.h"

#pragma pack(2)
typedef int16_t WORD;
typedef int32_t DWORD;
typedef int32_t LONG;

typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;
typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;

#pragma pack()


typedef struct _tagStBMPLineInfo
{
	uint16_t u16LineIndex;
	uint16_t u16LineBegin;
	uint16_t u16LineEnd;
	void *pLine;
}StBMPLineInfo;


enum
{
	_BMP_Load_Invalid = -1,
	_BMP_Load_FileHeader = 0,	/* BITMAPFILEHEADER */
	_BMP_Load_InfoHeader,		/* BITMAPINFOHEADER */
	_BMP_Load_Line,				/* StBMPLineInfo */
	_BMP_Load_End,				/* StBMPLineInfo */
};



typedef int32_t (*PFUN_BMPLoadCallback)(int32_t s32Type, void *pData, uint32_t u32Len, void *pContext);

int32_t SPIFlashBMPLoad(uint32_t u32Addr, PFUN_BMPLoadCallback pFCB, void *pContext);

typedef struct _tagStSPIFlashBMPLoadCtrl
{
	StFlashBigDataCtrl stCtrl;
	int32_t ret;
	int32_t s32Count;
	int32_t s32BmpOffset;
	int32_t s32Width;
	int32_t s32Height;
	int32_t s32BitDepth;
	int32_t s32Pitch;
	int32_t s32CtrlType;
	bool boIsYP;
	uint8_t *pData;
	
	StBMPLineInfo stLineInfo;
	
	uint32_t u32Addr;
	PFUN_BMPLoadCallback pFCB;
	void *pContext;
}StSPIFlashBMPLoadCtrl;

int32_t SPIFlashBMPLoadStart(StSPIFlashBMPLoadCtrl *pCtrl, 
	uint32_t u32Addr, PFUN_BMPLoadCallback pFCB, void *pContext);
int32_t SPIFlashBMPLoadFlush(StSPIFlashBMPLoadCtrl *pCtrl);

#endif
