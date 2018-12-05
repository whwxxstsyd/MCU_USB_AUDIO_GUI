#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"

#include "spi_flash.h"
#include "flash_bmp.h"


#define BMP_READ_BUF	512


int32_t SPIFlashBMPLoadStart(StSPIFlashBMPLoadCtrl *pCtrl, 
	uint32_t u32Addr, PFUN_BMPLoadCallback pFCB, void *pContext)
{
	if (pCtrl == NULL)
	{
		return -1;
	}
	
	memset(pCtrl, 0, sizeof(StSPIFlashBMPLoadCtrl));
	
	pCtrl->s32CtrlType = _BMP_Load_Invalid;
	
	if (!boIsFlashCanUse())
	{
		return -1;
	}

	SPIFlashBigDataReadWriteBegin(&(pCtrl->stCtrl), u32Addr, BMP_READ_BUF);

	pCtrl->pData = (uint8_t *)malloc(BMP_READ_BUF);
	if (pCtrl->pData == NULL)
	{
		return -1;
	}

	
	pCtrl->s32CtrlType = _BMP_Load_FileHeader;
	
	pCtrl->u32Addr = u32Addr;
	pCtrl->pFCB = pFCB;
	pCtrl->pContext = pContext;
	
	
	return 0;
}

int32_t SPIFlashBMPLoadFlush(StSPIFlashBMPLoadCtrl *pCtrl)
{
	if (pCtrl == NULL || 
		(pCtrl->s32CtrlType < _BMP_Load_FileHeader))
	{
		return -1;
	}
	
	if (pCtrl->s32CtrlType >= _BMP_Load_End)
	{
		if (pCtrl->pData != NULL)
		{
			free (pCtrl->pData);
			pCtrl->pData = NULL;
		}
		return pCtrl->s32CtrlType;
	}
	
	if (pCtrl->s32CtrlType == _BMP_Load_FileHeader)
	{
		BITMAPFILEHEADER *pHeader = (BITMAPFILEHEADER *)pCtrl->pData;
		
		pCtrl->s32Count = SPIFlashBigDataRead(&pCtrl->stCtrl, pCtrl->pData, sizeof(BITMAPFILEHEADER)); 
		if (pCtrl->s32Count < 0)
		{	
			pCtrl->ret	= pCtrl->s32Count;
			goto end;
		}
		
		if (pHeader->bfType != 0x4D42)
		{
			pCtrl->ret = -1;
			goto end;
		}
		if (pCtrl->pFCB != NULL)
		{
			pCtrl->ret = pCtrl->pFCB(_BMP_Load_FileHeader, 
				pCtrl->pData, sizeof(BITMAPFILEHEADER), 
				pCtrl->pContext);
		}

		if (pCtrl->ret != 0)
		{
			goto end;
		}
		
		pCtrl->s32BmpOffset = pHeader->bfOffBits;	
		pCtrl->s32CtrlType = _BMP_Load_InfoHeader;
	}	
	else if (pCtrl->s32CtrlType == _BMP_Load_InfoHeader)
	{
		BITMAPINFOHEADERWithMask *pHeader = (BITMAPINFOHEADERWithMask *)pCtrl->pData;
		
		pCtrl->s32Count = SPIFlashBigDataRead(&pCtrl->stCtrl, pCtrl->pData, sizeof(BITMAPINFOHEADERWithMask)); 
		if (pCtrl->s32Count < 0)
		{	
			pCtrl->ret	= pCtrl->s32Count;
			goto end;
		}
		
		if (pCtrl->pFCB != NULL)
		{
			pCtrl->ret = pCtrl->pFCB(_BMP_Load_InfoHeader, 
				pCtrl->pData, sizeof(BITMAPINFOHEADER), 
				pCtrl->pContext);
		}
		if (pCtrl->ret != 0)
		{
			goto end;
		}
		
		pCtrl->s32Width = pHeader->biWidth;
		pCtrl->s32Height= pHeader->biHeight;
		if (pCtrl->s32Height < 0)
		{
			pCtrl->s32Height = 0 - pCtrl->s32Height;
			pCtrl->boIsYP = true;
		}	
			
		pCtrl->s32BitDepth = pHeader->biBitCount / 8;
		pCtrl->s32Pitch = (pCtrl->s32BitDepth * pCtrl->s32Width + 3) & (~0x03);

		pCtrl->s32CtrlType = _BMP_Load_Line;
	}
	else if (pCtrl->s32CtrlType == _BMP_Load_Line)
	{
		uint32_t u32ReadAddr;
		StBMPLineInfo *pLineInfo = &pCtrl->stLineInfo;
		if (pLineInfo->u16LineBegin >= pCtrl->s32Width)
		{
			pLineInfo->u16LineBegin = 
			pLineInfo->u16LineEnd = 0;
			pLineInfo->u16LineIndex++;
		}
		if ((pLineInfo->u16LineIndex >= pCtrl->s32Height))
		{
			SPIFlashBMPLoadEnd(pCtrl);
			goto end;
		}
		if (!pCtrl->boIsYP)
		{
			u32ReadAddr = (pCtrl->u32Addr + 
					pCtrl->s32BmpOffset + 
					pCtrl->s32Pitch * 
					(pCtrl->s32Height - pLineInfo->u16LineIndex - 1));
		}
		else
		{
			u32ReadAddr = (pCtrl->u32Addr + 
					pCtrl->s32BmpOffset + 
					pCtrl->s32Pitch * pLineInfo->u16LineIndex);
			
		}
		{
			int32_t j = (pCtrl->s32Width - pLineInfo->u16LineBegin) * pCtrl->s32BitDepth;
		
			pCtrl->stLineInfo.pLine = pCtrl->pData;
			
			pCtrl->stCtrl.u32CurWRAddr = u32ReadAddr + 
				pLineInfo->u16LineBegin * pCtrl->s32BitDepth;
			
			{
				int32_t s32NeedRead = j;
				if (j > BMP_READ_BUF)
				{
					s32NeedRead = BMP_READ_BUF;
				}
				s32NeedRead = (s32NeedRead / pCtrl->s32BitDepth) * pCtrl->s32BitDepth;
				pCtrl->s32Count = SPIFlashBigDataRead(&pCtrl->stCtrl, pCtrl->pData, s32NeedRead); 
				if (pCtrl->s32Count < 0)
				{	
					pCtrl->ret	= pCtrl->s32Count;
					goto end;
				}
				pLineInfo->u16LineEnd = pLineInfo->u16LineBegin + pCtrl->s32Count / pCtrl->s32BitDepth;
				
				if (pCtrl->pFCB != NULL)
				{
					pCtrl->ret = pCtrl->pFCB(_BMP_Load_Line, pLineInfo, sizeof(StBMPLineInfo), pCtrl->pContext);
				}
				if (pCtrl->ret != 0)
				{
					goto end;
				}
				
				pLineInfo->u16LineBegin = pLineInfo->u16LineEnd;			
			}
#if 0
			{
				if (pCtrl->stLineInfo.u16LineIndex == 1)
				{
					pCtrl->stLineInfo.u16LineIndex = pCtrl->stLineInfo.u16LineIndex;
				}
				else if (pCtrl->stLineInfo.u16LineIndex == 161)
				{
					pCtrl->stLineInfo.u16LineIndex = pCtrl->stLineInfo.u16LineIndex;
				}

				else if (pCtrl->stLineInfo.u16LineIndex == 321)
				{
					pCtrl->stLineInfo.u16LineIndex = pCtrl->stLineInfo.u16LineIndex;
				}

			}
#endif
		}
	}
end:
	return pCtrl->ret;
} 

int32_t SPIFlashBMPLoadEnd(StSPIFlashBMPLoadCtrl *pCtrl)
{
	if (pCtrl != NULL)
	{
		if (pCtrl->pData != NULL)
		{
			free (pCtrl->pData);
			pCtrl->pData = NULL;
		}
		pCtrl->s32CtrlType = _BMP_Load_Invalid;
	}
	
	return 0;
}



int32_t SPIFlashBMPLoad(uint32_t u32Addr, PFUN_BMPLoadCallback pFCB, void *pContext)
{
	StFlashBigDataCtrl stCtrl;
	int32_t ret = 0;
	int32_t s32Count;
	int32_t s32BmpOffset;
	int32_t s32Width;
	int32_t s32Height;
	int32_t s32BitDepth;
	int32_t s32Pitch;
	bool boIsYP = false;
	
	uint8_t *pData = NULL;
	
	
	SPIFlashBigDataReadWriteBegin(&stCtrl, u32Addr, BMP_READ_BUF);

	pData = (uint8_t *)malloc(BMP_READ_BUF);
	if (pData == NULL)
	{
		ret = -1;
		goto end;
	}
	
	/* file header */
	{
		BITMAPFILEHEADER *pHeader = (BITMAPFILEHEADER *)pData;
		
		s32Count = SPIFlashBigDataRead(&stCtrl, pData, sizeof(BITMAPFILEHEADER)); 
		if (s32Count < 0)
		{	
			ret	= s32Count;
			goto end;
		}
		
		if (pHeader->bfType != 0x4D42)
		{
			ret = -1;
			goto end;
		}
		if (pFCB != NULL)
		{
			ret = pFCB(_BMP_Load_FileHeader, pData, sizeof(BITMAPFILEHEADER), pContext);
		}

		if (ret != 0)
		{
			goto end;
		}
		
		s32BmpOffset = pHeader->bfOffBits;
		
	}
	
	/* bmp info header */
	{
		BITMAPINFOHEADER *pHeader = (BITMAPINFOHEADER *)pData;
		
		s32Count = SPIFlashBigDataRead(&stCtrl, pData, sizeof(BITMAPINFOHEADER)); 
		if (s32Count < 0)
		{	
			ret	= s32Count;
			goto end;
		}
		
		if (pFCB != NULL)
		{
			ret = pFCB(_BMP_Load_InfoHeader, pData, sizeof(BITMAPINFOHEADER), pContext);
		}
		if (ret != 0)
		{
			goto end;
		}
		
		s32Width = pHeader->biWidth;
		s32Height= pHeader->biHeight;
		if (s32Height < 0)
		{
			s32Height = 0 - s32Height;
			boIsYP = true;
		}	
			
		s32BitDepth = pHeader->biBitCount / 8;
		s32Pitch = (s32BitDepth * s32Width + 3) & (~0x03);
	}
	
	/* line */
	{
		int32_t i;
		uint32_t u32ReadAddr = 0;
		if (!boIsYP)
		{
			u32ReadAddr = (u32Addr + s32BmpOffset + s32Pitch * (s32Height - 1));
		}
		for (i = 0; i < s32Height; i++)
		{
			int32_t j = s32Width * s32BitDepth;
			StBMPLineInfo stLineInfo;
			
			stLineInfo.pLine = pData;
			stLineInfo.u16LineIndex = i;
			stLineInfo.u16LineBegin = 0;
			
			stCtrl.u32CurWRAddr = u32ReadAddr;
			
			while (j > 0)
			{
				int32_t s32NeedRead = j;
				if (j > BMP_READ_BUF)
				{
					s32NeedRead = BMP_READ_BUF;
				}
				s32BitDepth = (s32BitDepth / s32BitDepth) * s32BitDepth;
				s32Count = SPIFlashBigDataRead(&stCtrl, pData, s32NeedRead); 
				if (s32Count < 0)
				{	
					ret	= s32Count;
					goto end;
				}
				stLineInfo.u16LineEnd = stLineInfo.u16LineBegin + s32Count / s32BitDepth;
				
				if (pFCB != NULL)
				{
					ret = pFCB(_BMP_Load_Line, &stLineInfo, sizeof(StBMPLineInfo), pContext);
				}
				if (ret != 0)
				{
					goto end;
				}
				
				stLineInfo.u16LineBegin += stLineInfo.u16LineEnd;
				j -= s32Count;				
			}
			{
				if (i == 1)
				{
					i = i;
				}
				else if (i == 161)
				{
					i = i;
				}

				else if (i == 321)
				{
					i = i;
				}

			}
			if (!boIsYP)
			{
				u32ReadAddr -= s32Pitch;
			}
			else
			{
				u32ReadAddr += s32Pitch;
			}

		}	
	
	}

	
end:
	if (pData != NULL)
	{
		free(pData);
	}

	return ret;
}



