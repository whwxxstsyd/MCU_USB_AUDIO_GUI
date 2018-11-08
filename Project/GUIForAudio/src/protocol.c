/******************(C) copyright 天津市XXXXX有限公司 *************************
* All Rights Reserved
* 文件名：protocol.c
* 摘要: 协议控制程序
* 版本：0.0.1
* 作者：许龙杰
* 日期：2013年01月25日
*******************************************************************************/
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"
#include "io_buf_ctrl.h"
#include "app_port.h"
	
	
#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "key_led_ctrl.h"
	
#include "buzzer.h"
	
#include "user_init.h"
#include "user_api.h"

#include "key_led_table.h"

#include "message.h"
#include "message_2.h"
#include "message_3.h"
#include "message_usb.h"
#include "flash_ctrl.h"
#include "extern_io_ctrl.h"

#include "common.h"

#include "gui.h"
#include "gui_driver.h"
#include "screen_protect.h"

#include "protocol.h"
#include "extern_peripheral.h"
#include "spi_flash.h"

//#include "I2C.h"
//#include "wm8776.h"


#define APP_VERSOIN	"YA_GUI_AUDIO_181108"

u8 g_u8CamAddr = 0;

EmProtocol g_emProtocol = _Protocol_YNA;
u32 g_u32BoolIsEncode = 0;
u8 g_u8MIDIChannel = 0;


int32_t SendAudioCtrlModeCmdEx(uint16_t u16Channel, EmAudioCtrlMode emMode, uint32_t u32Flag);
int32_t SendAudioVolumeCmdEx(uint16_t u16Channel, StVolume stVolume, uint32_t u32Flag);
int32_t SendInputEnableStateCmdEx(uint8_t u8Index, uint8_t u8NewState, uint32_t u32Flag);
int32_t SendOutputEnableStateCmdEx(uint8_t u8Index, uint8_t u8NewState, uint32_t u32Flag);
int32_t SendPhantomPowerStateCmdEx(uint16_t u16Channel, bool boIsEnable, uint32_t u32Flag);



int32_t CycleMsgInit(StCycleBuf *pCycleBuf, void *pBuf, uint32_t u32Length)
{
	if ((pCycleBuf == NULL) || (pBuf == NULL) || (u32Length == 0))
	{
		return -1;
	}
	memset(pCycleBuf, 0, sizeof(StCycleBuf));
	pCycleBuf->pBuf = pBuf;
	pCycleBuf->u32TotalLength = u32Length;
	
	return 0;
}

void *CycleGetOneMsg(StCycleBuf *pCycleBuf, const char *pData, 
	uint32_t u32DataLength, uint32_t *pLength, int32_t *pProtocolType, int32_t *pErr)
{
	char *pBuf = NULL;
	int32_t s32Err = 0;
	if ((pCycleBuf == NULL) || (pLength == NULL))
	{
		s32Err = -1;
		goto end;
	}
	if (((pCycleBuf->u32TotalLength - pCycleBuf->u32Using) < u32DataLength)
		/*|| (u32DataLength == 0)*/)
	{
		PRINT_MFC("data too long\n");
		s32Err = -1;
	}
	if (u32DataLength != 0)
	{
		if (pData == NULL)
		{
			s32Err = -1;
			goto end;
		}
		else	/* copy data */
		{
			uint32_t u32Tmp = pCycleBuf->u32Write + u32DataLength;
			if (u32Tmp > pCycleBuf->u32TotalLength)
			{
				uint32_t u32CopyLength = pCycleBuf->u32TotalLength - pCycleBuf->u32Write;
				memcpy(pCycleBuf->pBuf + pCycleBuf->u32Write, pData, u32CopyLength);
				memcpy(pCycleBuf->pBuf, pData + u32CopyLength, u32DataLength - u32CopyLength);
				pCycleBuf->u32Write = u32DataLength - u32CopyLength;
			}
			else
			{
				memcpy(pCycleBuf->pBuf + pCycleBuf->u32Write, pData, u32DataLength);
				pCycleBuf->u32Write += u32DataLength;
			}
			pCycleBuf->u32Using += u32DataLength;

		}
	}

	do
	{
		uint32_t i;
		bool boIsBreak = false;

		for (i = 0; i < pCycleBuf->u32Using; i++)
		{
			uint32_t u32ReadIndex = i + pCycleBuf->u32Read;
			char c8FirstByte;
			u32ReadIndex %= pCycleBuf->u32TotalLength;
			c8FirstByte = pCycleBuf->pBuf[u32ReadIndex];
			if (c8FirstByte == ((char)0xAA))
			{
				#define YNA_NORMAL_CMD		0
				#define YNA_VARIABLE_CMD	1 /*big than PROTOCOL_YNA_DECODE_LENGTH */
				uint32_t u32MSB = 0;
				uint32_t u32LSB = 0;
				int32_t s32RemainLength = pCycleBuf->u32Using - i;
				
				/* check whether it's a variable length command */
				if (s32RemainLength >= PROTOCOL_YNA_DECODE_LENGTH - 1)
				{
					if (pCycleBuf->u32Flag != YNA_NORMAL_CMD)
					{
						u32MSB = ((pCycleBuf->u32Flag >> 8) & 0xFF);
						u32LSB = ((pCycleBuf->u32Flag >> 0) & 0xFF);
					}
					else
					{
						uint32_t u32Start = i + pCycleBuf->u32Read;
						char *pTmp = pCycleBuf->pBuf;
						if ((pTmp[(u32Start + _YNA_Mix) % pCycleBuf->u32TotalLength] == 0x04)
							&& (pTmp[(u32Start + _YNA_Cmd) % pCycleBuf->u32TotalLength] == 0x00))
						{
							u32MSB = pTmp[(u32Start + _YNA_Data2) % pCycleBuf->u32TotalLength];
							u32LSB = pTmp[(u32Start + _YNA_Data3) % pCycleBuf->u32TotalLength];
							if (s32RemainLength >= PROTOCOL_YNA_DECODE_LENGTH)
							{
								uint32_t u32Start = i + pCycleBuf->u32Read;
								uint32_t u32End = PROTOCOL_YNA_DECODE_LENGTH - 1 + i + pCycleBuf->u32Read;
								char c8CheckSum = 0;
								uint32_t j;
								char c8Tmp;
								for (j = u32Start; j < u32End; j++)
								{
									c8CheckSum ^= pCycleBuf->pBuf[j % pCycleBuf->u32TotalLength];
								}
								c8Tmp = pCycleBuf->pBuf[u32End % pCycleBuf->u32TotalLength];
								if (c8CheckSum != c8Tmp) /* wrong message */
								{
									PRINT_MFC("get a wrong command: %d\n", u32MSB);
									pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
									pCycleBuf->u32Read += (i + 1);
									pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
									break;
								}
								
								u32MSB &= 0xFF;
								u32LSB &= 0xFF;
								
								pCycleBuf->u32Flag = ((u32MSB << 8) + u32LSB);
							}
						}
					}
				}
				u32MSB &= 0xFF;
				u32LSB &= 0xFF;
				u32MSB = (u32MSB << 8) + u32LSB;
				u32MSB += PROTOCOL_YNA_DECODE_LENGTH;
				PRINT_MFC("the data length is: %d\n", u32MSB);
				if (u32MSB > (pCycleBuf->u32TotalLength / 2))	/* maybe the message is wrong */
				{
					pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
					pCycleBuf->u32Read += (i + 1);
					pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
					pCycleBuf->u32Flag = 0;
				}
				else if (((int32_t)(u32MSB)) <= s32RemainLength) /* good, I may got a message */
				{
					if (u32MSB == PROTOCOL_YNA_DECODE_LENGTH)
					{
						char c8CheckSum = 0, *pBufTmp, c8Tmp;
						uint32_t j, u32Start, u32End;
						uint32_t u32CmdLength = u32MSB;
						pBuf = (char *)malloc(u32CmdLength);
						if (pBuf == NULL)
						{
							s32Err = -1; /* big problem */
							goto end;
						}
						pBufTmp = pBuf;
						u32Start = i + pCycleBuf->u32Read;

						u32End = u32MSB - 1 + i + pCycleBuf->u32Read;
						PRINT_MFC("start: %d, end: %d\n", u32Start, u32End);
						for (j = u32Start; j < u32End; j++)
						{
							c8Tmp = pCycleBuf->pBuf[j % pCycleBuf->u32TotalLength];
							c8CheckSum ^= c8Tmp;
							*pBufTmp++ = c8Tmp;
						}
						c8Tmp = pCycleBuf->pBuf[u32End % pCycleBuf->u32TotalLength];
						if (c8CheckSum == c8Tmp) /* good message */
						{
							boIsBreak = true;
							*pBufTmp = c8Tmp;

							pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + u32CmdLength));
							pCycleBuf->u32Read = i + pCycleBuf->u32Read + u32CmdLength;
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
							PRINT_MFC("get a command: %d\n", u32MSB);
							PRINT_MFC("u32Using: %d, u32Read: %d, u32Write: %d\n", pCycleBuf->u32Using, pCycleBuf->u32Read, pCycleBuf->u32Write);
							*pLength = u32CmdLength;
							if (pProtocolType != NULL)
							{
								*pProtocolType = _Protocol_YNA;
							}
						}
						else
						{
							free(pBuf);
							pBuf = NULL;
							PRINT_MFC("get a wrong command: %d\n", u32MSB);
							pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
							pCycleBuf->u32Read += (i + 1);
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
							pCycleBuf->u32Flag = 0;
						}
					}
					else /* variable length */
					{
						uint32_t u32Start, u32End;
						uint32_t u32CmdLength = u32MSB;
						uint16_t u16CRCModBus;
						uint16_t u16CRCBuf;
						pBuf = (char *)malloc(u32CmdLength);
						if (pBuf == NULL)
						{
							s32Err = -1; /* big problem */
							goto end;
						}
						u32Start = (i + pCycleBuf->u32Read) % pCycleBuf->u32TotalLength;
						u32End = (u32MSB + i + pCycleBuf->u32Read) % pCycleBuf->u32TotalLength;
						PRINT_MFC("start: %d, end: %d\n", u32Start, u32End);
						if (u32End > u32Start)
						{
							memcpy(pBuf, pCycleBuf->pBuf + u32Start, u32MSB);
						}
						else
						{
							uint32_t u32FirstCopy = pCycleBuf->u32TotalLength - u32Start;
							memcpy(pBuf, pCycleBuf->pBuf + u32Start, u32FirstCopy);
							memcpy(pBuf + u32FirstCopy, pCycleBuf->pBuf, u32MSB - u32FirstCopy);
						}

						pCycleBuf->u32Flag = YNA_NORMAL_CMD;
						
						/* we need not check the head's check sum,
						 * just check the CRC16-MODBUS
						 */
						u16CRCModBus = CRC16((const uint8_t *)pBuf + PROTOCOL_YNA_DECODE_LENGTH, 
							u32MSB - PROTOCOL_YNA_DECODE_LENGTH - 2);
						u16CRCBuf = 0;

						LittleAndBigEndianTransfer((char *)(&u16CRCBuf), pBuf + u32MSB - 2, 2);
						if (u16CRCBuf == u16CRCModBus) /* good message */
						{
							boIsBreak = true;

							pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + u32CmdLength));
							pCycleBuf->u32Read = i + pCycleBuf->u32Read + u32CmdLength;
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
							PRINT_MFC("get a command: %d\n", u32MSB);
							PRINT_MFC("u32Using: %d, u32Read: %d, u32Write: %d\n", pCycleBuf->u32Using, pCycleBuf->u32Read, pCycleBuf->u32Write);
							*pLength = u32CmdLength;
							if (pProtocolType != NULL)
							{
								*pProtocolType = _Protocol_YNA;
							}
						}
						else
						{
							free(pBuf);
							pBuf = NULL;
							PRINT_MFC("get a wrong command: %d\n", u32MSB);
							pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
							pCycleBuf->u32Read += (i + 1);
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
						}
					}
				}
				else	/* message not enough long */
				{
					pCycleBuf->u32Using = (pCycleBuf->u32Using - i);
					pCycleBuf->u32Read += i;
					pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
					boIsBreak = true;
				}
				break;
			}
			else if(c8FirstByte == ((char)0xFA))
			{
				int32_t s32RemainLength = pCycleBuf->u32Using - i;
				if (s32RemainLength >= PROTOCOL_RQ_LENGTH)
				{
					volatile char c8CheckSum = 0, *pBufTmp, c8Tmp;
					volatile uint32_t j, u32Start, u32End;
					volatile uint32_t u32CmdLength = PROTOCOL_RQ_LENGTH;
					pBuf = (char *)malloc(u32CmdLength);
					if (pBuf == NULL)
					{
						s32Err = -1; /* big problem */
						goto end;
					}
					pBufTmp = pBuf;
					u32Start = i + pCycleBuf->u32Read;

					u32End = PROTOCOL_RQ_LENGTH - 1 + i + pCycleBuf->u32Read;
					PRINT_MFC("start: %d, end: %d\n", u32Start, u32End);
					for (j = u32Start; j < u32End; j++)
					{
						c8Tmp = pCycleBuf->pBuf[j % pCycleBuf->u32TotalLength];
						*pBufTmp++ = c8Tmp;
					}
					
					for (j = 2; j < PROTOCOL_RQ_LENGTH - 1; j++)
					{
						c8CheckSum += pBuf[j];
					}
					
					
					c8Tmp = pCycleBuf->pBuf[u32End % pCycleBuf->u32TotalLength];
					if (c8CheckSum == c8Tmp) /* good message */
					{
						boIsBreak = true;
						*pBufTmp = c8Tmp;

						pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + u32CmdLength));
						pCycleBuf->u32Read = i + pCycleBuf->u32Read + u32CmdLength;
						pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
						PRINT_MFC("get a command: %d\n", u32MSB);
						PRINT_MFC("u32Using: %d, u32Read: %d, u32Write: %d\n", pCycleBuf->u32Using, pCycleBuf->u32Read, pCycleBuf->u32Write);
						*pLength = u32CmdLength;
						if (pProtocolType != NULL)
						{
							*pProtocolType = _Protocol_RQ;
						}
					}
					else
					{
						free(pBuf);
						pBuf = NULL;
						PRINT_MFC("get a wrong command: %d\n", u32MSB);
						pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
						pCycleBuf->u32Read += (i + 1);
						pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
					}
				}
				else	/* message not enough long */
				{
					pCycleBuf->u32Using = (pCycleBuf->u32Using - i);
					pCycleBuf->u32Read += i;
					pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
					boIsBreak = true;
				}
				break;
			}
			
			else if((c8FirstByte & 0xF0) == ((char)0x80))
			{
				int32_t s32RemainLength = pCycleBuf->u32Using - i;
				if (s32RemainLength >= PROTOCOL_VISCA_MIN_LENGTH)
				{
					u32 j;
					u32 u32Start = i + pCycleBuf->u32Read;
					u32 u32End = pCycleBuf->u32Using + pCycleBuf->u32Read;
					char c8Tmp = 0;
					for (j = u32Start + PROTOCOL_VISCA_MIN_LENGTH - 1; j < u32End; j++)
					{
						c8Tmp = pCycleBuf->pBuf[j % pCycleBuf->u32TotalLength];
						if (c8Tmp == (char)0xFF)
						{
							u32End = j;
							break;
						}
					}
					if (c8Tmp == (char)0xFF)
					{
						/* wrong message */
						if ((u32End - u32Start + 1) > PROTOCOL_VISCA_MAX_LENGTH)
						{
							pBuf = NULL;
							PRINT_MFC("get a wrong command: %d\n", u32MSB);
							pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
							pCycleBuf->u32Read += (i + 1);
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;							
						}
						else
						{
							char *pBufTmp;
							uint32_t u32CmdLength = u32End - u32Start + 1;
							pBuf = (char *)malloc(u32CmdLength);
							if (pBuf == NULL)
							{
								s32Err = -1; /* big problem */
								goto end;
							}	
							pBufTmp = pBuf;
							boIsBreak = true;
							
							PRINT_MFC("start: %d, end: %d\n", u32Start, u32End);
							for (j = u32Start; j <= u32End; j++)
							{
								*pBufTmp++ = pCycleBuf->pBuf[j % pCycleBuf->u32TotalLength];
							}
							
							pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + u32CmdLength));
							pCycleBuf->u32Read = i + pCycleBuf->u32Read + u32CmdLength;
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
							PRINT_MFC("get a command: %d\n", u32MSB);
							PRINT_MFC("u32Using: %d, u32Read: %d, u32Write: %d\n", pCycleBuf->u32Using, pCycleBuf->u32Read, pCycleBuf->u32Write);
							*pLength = u32CmdLength;
							if (pProtocolType != NULL)
							{
								*pProtocolType = _Protocol_VISCA;
							}

						}
					}
					else
					{
						/* wrong message */
						if ((u32End - u32Start) >= PROTOCOL_VISCA_MAX_LENGTH)
						{
							pBuf = NULL;
							PRINT_MFC("get a wrong command: %d\n", u32MSB);
							pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
							pCycleBuf->u32Read += (i + 1);
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;							
						}
						else
						{
							pCycleBuf->u32Using = (pCycleBuf->u32Using - i);
							pCycleBuf->u32Read += i;
							pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
							boIsBreak = true;						
						}
					}
				}
				else	/* message not enough long */
				{
					pCycleBuf->u32Using = (pCycleBuf->u32Using - i);
					pCycleBuf->u32Read += i;
					pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
					boIsBreak = true;
				}
				break;
			}
			else if(c8FirstByte == ((char)0xA5))
			{
				int32_t s32RemainLength = pCycleBuf->u32Using - i;
				if (s32RemainLength >= PROTOCOL_SB_LENGTH)
				{
					volatile char c8CheckSum = 0, *pBufTmp, c8Tmp;
					volatile uint32_t j, u32Start, u32End;
					volatile uint32_t u32CmdLength = PROTOCOL_SB_LENGTH;
					pBuf = (char *)malloc(u32CmdLength);
					if (pBuf == NULL)
					{
						s32Err = -1; /* big problem */
						goto end;
					}
					pBufTmp = pBuf;
					u32Start = i + pCycleBuf->u32Read;

					u32End = PROTOCOL_SB_LENGTH - 1 + i + pCycleBuf->u32Read;
					PRINT_MFC("start: %d, end: %d\n", u32Start, u32End);
					for (j = u32Start; j < u32End; j++)
					{
						c8Tmp = pCycleBuf->pBuf[j % pCycleBuf->u32TotalLength];
						*pBufTmp++ = c8Tmp;
					}
					
					for (j = 1; j < PROTOCOL_SB_LENGTH - 1; j++)
					{
						c8CheckSum += pBuf[j];
					}
					
					
					c8Tmp = pCycleBuf->pBuf[u32End % pCycleBuf->u32TotalLength];
					if (c8CheckSum == c8Tmp) /* good message */
					{
						boIsBreak = true;
						*pBufTmp = c8Tmp;

						pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + u32CmdLength));
						pCycleBuf->u32Read = i + pCycleBuf->u32Read + u32CmdLength;
						pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
						PRINT_MFC("get a command: %d\n", u32MSB);
						PRINT_MFC("u32Using: %d, u32Read: %d, u32Write: %d\n", pCycleBuf->u32Using, pCycleBuf->u32Read, pCycleBuf->u32Write);
						*pLength = u32CmdLength;
						if (pProtocolType != NULL)
						{
							*pProtocolType = _Protocol_SB;
						}
					}
					else
					{
						free(pBuf);
						pBuf = NULL;
						PRINT_MFC("get a wrong command: %d\n", u32MSB);
						pCycleBuf->u32Using = (pCycleBuf->u32Using - (i + 1));
						pCycleBuf->u32Read += (i + 1);
						pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
					}
				}
				else	/* message not enough long */
				{
					pCycleBuf->u32Using = (pCycleBuf->u32Using - i);
					pCycleBuf->u32Read += i;
					pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
					boIsBreak = true;
				}
				break;
			}
		}
		if ((i == pCycleBuf->u32Using) && (!boIsBreak))
		{
			PRINT_MFC("cannot find AA, i = %d\n", pCycleBuf->u32Using);
			pCycleBuf->u32Using = 0;
			pCycleBuf->u32Read += i;
			pCycleBuf->u32Read %= pCycleBuf->u32TotalLength;
			pCycleBuf->u32Flag = 0;
		}

		if (boIsBreak)
		{
			break;
		}
	} while (((int32_t)pCycleBuf->u32Using) > 0);

	//if (pCycleBuf->u32Write + u32DataLength)

end:
	if (pErr != NULL)
	{
		*pErr = s32Err;
	}
	return pBuf;
}
void *YNAMakeAnArrayVarialbleCmd(uint16_t u16Cmd, void *pData,
	uint32_t u32Count, uint32_t u32Length, uint32_t *pCmdLength)
{
	uint32_t u32CmdLength;
	uint32_t u32DataLength;
	uint32_t u32Tmp;
	uint8_t *pCmd = NULL;
	uint8_t *pVarialbleCmd;
	if (pData == NULL)
	{
		return NULL;
	}
	
	u32DataLength = u32Count * u32Length;
	
	/*  */
	u32CmdLength = PROTOCOL_YNA_DECODE_LENGTH + 6 + u32DataLength + 2;
	pCmd = malloc(u32CmdLength);
	if (pCmd == NULL)
	{
		return NULL;
	}
	memset(pCmd, 0, u32CmdLength);
	pCmd[_YNA_Sync] = 0xAA;
	pCmd[_YNA_Mix] = 0x04;
	pCmd[_YNA_Cmd] = 0x00;
	
	/* total length */
	u32Tmp = u32CmdLength - PROTOCOL_YNA_DECODE_LENGTH;
	LittleAndBigEndianTransfer((char *)pCmd + _YNA_Data2, (const char *)(&u32Tmp), 2);
	
	YNAGetCheckSum(pCmd);
	
	pVarialbleCmd = pCmd + PROTOCOL_YNA_DECODE_LENGTH;
	
	/* command serial */
	LittleAndBigEndianTransfer((char *)pVarialbleCmd, (const char *)(&u16Cmd), 2);

	/* command count */
	LittleAndBigEndianTransfer((char *)pVarialbleCmd + 2, (const char *)(&u32Count), 2);

	/* Varialble data length */
	LittleAndBigEndianTransfer((char *)pVarialbleCmd + 4, (const char *)(&u32Length), 2);
	
	/* copy the data */
	memcpy(pVarialbleCmd + 6, pData, u32DataLength);

	/* get the CRC16 of the variable command */
	u32Tmp = CRC16(pVarialbleCmd, 6 + u32DataLength);
	
	LittleAndBigEndianTransfer((char *)pVarialbleCmd + 6 + u32DataLength, 
		(const char *)(&u32Tmp), 2);

	if (pCmdLength != NULL)
	{
		*pCmdLength = u32CmdLength;
	}
	
	return pCmd;
}

void *YNAMakeASimpleVarialbleCmd(uint16_t u16Cmd, void *pData, 
	uint32_t u32DataLength, uint32_t *pCmdLength)
{
	return YNAMakeAnArrayVarialbleCmd(u16Cmd, pData, 1, u32DataLength, pCmdLength);
}

void CopyToUart1Message(void *pData, u32 u32Length)
{
	if ((pData != NULL) && (u32Length != 0))
	{
		void *pBuf = malloc(u32Length);
		if (pBuf != NULL)
		{
			memcpy(pBuf, pData, u32Length);
			if (MessageUartWrite(pBuf, true, _IO_Reserved, u32Length) != 0)
			{
				free (pBuf);
			}	
		}
	}

}
void CopyToUart2Message(void *pData, u32 u32Length)
{
}

void CopyToUart3Message(void *pData, u32 u32Length)
{
	if ((pData != NULL) && (u32Length != 0))
	{
		void *pBuf = malloc(u32Length);
		if (pBuf != NULL)
		{
			memcpy(pBuf, pData, u32Length);
			if (MessageUart3Write(pBuf, true, _IO_Reserved, u32Length) != 0)
			{
				free (pBuf);
			}	
		}
	}
}


void CopyToUartMessage(const StIOTCB *pIOTCB, void *pData, u32 u32Length)
{
	if ((pIOTCB != NULL) && (pData != NULL) && (u32Length != 0))
	{
		void *pBuf = malloc(u32Length);
		if (pBuf != NULL)
		{
			memcpy(pBuf, pData, u32Length);
			if (pIOTCB->pFunMsgWrite(pBuf, true, _IO_Reserved, u32Length) != 0)
			{
				free (pBuf);
			}	
		}
	}
}



typedef enum
{
	_UG_Invalide,
	_UG_BeginUpgrade,
	_UG_PrepareUpgrade,
	_UG_GetBufferCapacity,
	_UG_UpgradePosition,
	_UG_FileInfo,
	_UG_GetPacket,
	_UG_GetFileComplete,
	_UG_GetLIC,
	_UG_NeedReset,
	_UG_Complete,
}EMUGStatus;

typedef struct _tagStUGFileInfo
{
	uint32_t u32FileLength;
	uint32_t u32CRC32;
}StUGFileInfo;

typedef struct _tagStUGPacketInfo
{
	uint32_t u32FileOffset;
	uint32_t u32PacketLength;	/* must not bigger than 65535 */
}StUGPacketInfo;


typedef struct _tagStSPIFlashUpgrade
{
	EMUGStatus emStatus;
	uint32_t u32CurGetOffset;
	uint32_t u32OrgCmdTime;
	const StIOTCB *pIOTCB;
	StFlashBigDataCtrl stFlashCtrl;
}StSPIFlashUpgrade;

static StSPIFlashUpgrade s_stSPIFlashUpgrade;

#define SPI_FLASH_GET_SIZE		256
#define SPI_FLASH_UPGRADE_TIME	3000

int32_t SPIFlashUpgradeSetPosition(StSPIFlashUpgrade *pCtrl, uint32_t u32Position)
{
	memset(pCtrl, 0, sizeof(StSPIFlashUpgrade));
	return SPIFlashBigDataReadWriteBegin(&pCtrl->stFlashCtrl, u32Position, 1000);
}

int32_t SPIFlashUpgradeSetTotalSize(StSPIFlashUpgrade *pCtrl, uint32_t u32Size)
{
	SPIFlashBigDataSetTotalSize(&pCtrl->stFlashCtrl, u32Size);
	pCtrl->emStatus = _UG_GetPacket;
	return 0;
}

int32_t SPIFlashUpgradeSetIOCtrl(StSPIFlashUpgrade *pCtrl, const StIOTCB *pIOTCB)
{
	pCtrl->pIOTCB = pIOTCB;
	return 0;
}

int32_t SPIFlashUpgradeSetOffset(StSPIFlashUpgrade *pCtrl, uint32_t u32Offset)
{
	pCtrl->u32CurGetOffset = u32Offset;
	
	return 0;
}

int32_t SPIFlashUpgradeSetState(StSPIFlashUpgrade *pCtrl, EMUGStatus emState)
{
	pCtrl->emStatus = emState;
	
	return 0;
}

EMUGStatus SPIFlashUpgradeGetState(StSPIFlashUpgrade *pCtrl)
{
	return pCtrl->emStatus;
}

int32_t SPIFlashUpgradeWrite(StSPIFlashUpgrade *pCtrl, void *pData, uint32_t u32Len)
{
	return SPIFlashBigDataWriteNoBreak(&pCtrl->stFlashCtrl, pData, u32Len);
}


int32_t SPIFlashUpgradeFlushBegin(StSPIFlashUpgrade *pCtrl)
{
	pCtrl->u32OrgCmdTime = g_u32SysTickCnt - 2 * SPI_FLASH_UPGRADE_TIME;
	return 0;
}



int32_t SPIFlashUpgradeFlush(StSPIFlashUpgrade *pCtrl)
{
	if (pCtrl->emStatus == _UG_GetPacket)
	{
		if (SysTimeDiff(pCtrl->u32OrgCmdTime, g_u32SysTickCnt) >= SPI_FLASH_UPGRADE_TIME)
		{
			StUGPacketInfo stInfo;
			void *pCmd = NULL;
			uint32_t u32CmdLength = 0;
			stInfo.u32FileOffset = pCtrl->u32CurGetOffset;
			stInfo.u32PacketLength = SPI_FLASH_GET_SIZE;
			if ((pCtrl->stFlashCtrl.u32TotalSize - stInfo.u32FileOffset) < SPI_FLASH_GET_SIZE)
			{
				stInfo.u32PacketLength = 
					pCtrl->stFlashCtrl.u32TotalSize - stInfo.u32FileOffset;
			}
			
			pCmd = YNAMakeASimpleVarialbleCmd(0x8007, 
					&stInfo, sizeof(StUGPacketInfo), &u32CmdLength);
					
			if (pCmd != NULL)
			{
				if (pCtrl->pIOTCB != NULL)
				{
					if(pCtrl->pIOTCB->pFunMsgWrite(pCmd, true, _IO_Reserved, u32CmdLength) != 0)
					{
						free(pCmd);
					}
				}
				else
				{
					free(pCmd);
				}
				pCtrl->u32OrgCmdTime = g_u32SysTickCnt;
			}
			
		}
	}
	
	return 0;
}

void UpgradeFlush(void)
{
	SPIFlashUpgradeFlush(&s_stSPIFlashUpgrade);
}


u8 u8YNABuf[PROTOCOL_YNA_ENCODE_LENGTH];

int32_t BaseCmdProcess(StIOFIFO *pFIFO, const StIOTCB *pIOTCB)
{
	uint8_t *pMsg;
	bool boGetVaildBaseCmd = true;
	if (pFIFO == NULL)
	{
		return -1;
	}
	pMsg = (uint8_t *)pFIFO->pData;
	
	if (pMsg[_YNA_Sync] != 0xAA)
	{
		return -1;
	}
	
	if (pMsg[_YNA_Mix] == 0x0C)
	{
		if (pMsg[_YNA_Cmd] == 0x80)
		{
			uint8_t u8EchoBase[PROTOCOL_YNA_DECODE_LENGTH] = {0};
			uint8_t *pEcho = NULL;
			uint32_t u32EchoLength = 0;
			bool boHasEcho = true;
			bool boNeedCopy = true;
			bool boNeedReset = false;
			u8EchoBase[_YNA_Sync] = 0xAA;
			u8EchoBase[_YNA_Mix] = 0x0C;
			u8EchoBase[_YNA_Cmd] = 0x80;
			u8EchoBase[_YNA_Data1] = 0x01;
			switch(pMsg[_YNA_Data3])
			{
				case 0x01:	/* just echo the same command */
				{
					//SetOptionByte(OPTION_UPGRADE_DATA);
					//boNeedReset = true;
					
					if (pMsg[_YNA_Data1] == 0x10)
					{
						SPIFlashUpgradeSetState(&s_stSPIFlashUpgrade, _UG_BeginUpgrade);
					}
					
					u8EchoBase[_YNA_Data3] = pMsg[_YNA_Data3];
					pEcho = (uint8_t *)malloc(PROTOCOL_YNA_DECODE_LENGTH);
					if (pEcho == NULL)
					{
						boHasEcho = false;
						break;
					}
					u32EchoLength = PROTOCOL_YNA_DECODE_LENGTH;						
					break;
				}
				case 0x02:	/* just echo the same command */
				{
					u8EchoBase[_YNA_Data3] = pMsg[_YNA_Data3];
					pEcho = (uint8_t *)malloc(PROTOCOL_YNA_DECODE_LENGTH);
					if (pEcho == NULL)
					{
						boHasEcho = false;
						break;
					}
					u32EchoLength = PROTOCOL_YNA_DECODE_LENGTH;						
					break;
				}
				case 0x03:	/* return the UUID */
				{
					StUID stUID;
					GetUID(&stUID);
					boNeedCopy = false;
					pEcho = YNAMakeASimpleVarialbleCmd(0x8003, 
							&stUID, sizeof(StUID), &u32EchoLength);
					break;
				}
				case 0x04:	/* just echo the same command */
				{
					u8EchoBase[_YNA_Data3] = pMsg[_YNA_Data3];
					pEcho = (uint8_t *)malloc(PROTOCOL_YNA_DECODE_LENGTH);
					if (pEcho == NULL)
					{
						boHasEcho = false;
						break;
					}
					u32EchoLength = PROTOCOL_YNA_DECODE_LENGTH;						
					break;
				}
				case 0x05:	/* return the BufLength */
				{
					uint16_t u16BufLength = 0;
					if (pIOTCB != NULL
						 && pIOTCB->pFunGetMsgBufLength != 0)
					{
						u16BufLength = pIOTCB->pFunGetMsgBufLength();
					}
					boNeedCopy = false;
					pEcho = YNAMakeASimpleVarialbleCmd(0x8005, 
							&u16BufLength, sizeof(uint16_t), &u32EchoLength);
					break;
				}
				case 0x09:	/* RESET the MCU */
				{
					NVIC_SystemReset();
					boHasEcho = false;
					break;
				}
				case 0x0B:	/* Get the application's CRC32 */
				{
					uint32_t u32CRC32 = ~0;
					u32CRC32 = AppCRC32(~0);
					boNeedCopy = false;
					pEcho = YNAMakeASimpleVarialbleCmd(0x800B, 
							&u32CRC32, sizeof(uint32_t), &u32EchoLength);
					break;
				}
				case 0x0C:	/* get the version */
				{
					const char *pVersion = APP_VERSOIN;
					boNeedCopy = false;
					pEcho = YNAMakeASimpleVarialbleCmd(0x800C, 
							(void *)pVersion, strlen(pVersion) + 1, &u32EchoLength);
					break;
				}
				
				default:
					boHasEcho = false;
					boGetVaildBaseCmd = false;
					break;
			}
			if (boHasEcho && pEcho != NULL)
			{
				if (boNeedCopy)
				{
					YNAGetCheckSum(u8EchoBase);
					memcpy(pEcho, u8EchoBase, PROTOCOL_YNA_DECODE_LENGTH);
				}
				if (pIOTCB == NULL)
				{
					free(pEcho);
				}
				else if (pIOTCB->pFunMsgWrite == NULL)
				{
					free(pEcho);			
				}
				else if(pIOTCB->pFunMsgWrite(pEcho, true, _IO_Reserved, u32EchoLength) != 0)
				{
					free(pEcho);
				}
			}
			
			/* send all the command in the buffer */
			if (boNeedReset)
			{
				//MessageUartFlush(true); 
				NVIC_SystemReset();
			}
		}
	}
	else if (pMsg[_YNA_Mix] == 0x04)	/* variable command */
	{
		uint32_t u32TotalLength = 0;
		uint32_t u32ReadLength = 0;
		uint8_t *pVariableCmd;
		
		boGetVaildBaseCmd = false;
		
		/* get the total command length */
		LittleAndBigEndianTransfer((char *)(&u32TotalLength), (const char *)pMsg + _YNA_Data2, 2);
		pVariableCmd = pMsg + PROTOCOL_YNA_DECODE_LENGTH;
		u32TotalLength -= 2; /* CRC16 */
		while (u32ReadLength < u32TotalLength)
		{
			uint8_t u8EchoBase[PROTOCOL_YNA_DECODE_LENGTH] = {0};
			uint8_t *pEcho = NULL;
			uint32_t u32EchoLength = 0;
			bool boHasEcho = true;
			bool boNeedCopy = false;
			
			uint16_t u16Command = 0, u16Count = 0, u16Length = 0;
			
			LittleAndBigEndianTransfer((char *)(&u16Command),
				(char *)pVariableCmd, 2);
			LittleAndBigEndianTransfer((char *)(&u16Count),
				(char *)pVariableCmd + 2, 2);
			LittleAndBigEndianTransfer((char *)(&u16Length),
				(char *)pVariableCmd + 4, 2);

			u8EchoBase[_YNA_Sync] = 0xAA;
			u8EchoBase[_YNA_Mix] = 0x0C;
			u8EchoBase[_YNA_Cmd] = 0x80;
			u8EchoBase[_YNA_Data1] = 0x01;

			switch (u16Command)
			{
				case 0x8006:
				{
					StUGFileInfo stInfo;
					memcpy(&stInfo, pVariableCmd + 6, sizeof(StUGFileInfo));

					SPIFlashUpgradeSetTotalSize(&s_stSPIFlashUpgrade, stInfo.u32FileLength);
					SPIFlashUpgradeSetIOCtrl(&s_stSPIFlashUpgrade, pIOTCB);
					SPIFlashUpgradeSetState(&s_stSPIFlashUpgrade, _UG_GetPacket);
					
					u8EchoBase[_YNA_Data3] = 0x06;
					boNeedCopy = true;
					pEcho = (uint8_t *)malloc(PROTOCOL_YNA_DECODE_LENGTH);
					break;
				}
				case 0x8007:
				{
					StUGPacketInfo stInfo;
					void *pData = (pVariableCmd + 6 + sizeof(StUGPacketInfo));
					
					if (SPIFlashUpgradeGetState(&s_stSPIFlashUpgrade) != _UG_GetPacket)
					{
						break;
					}
				
					memcpy(&stInfo, pVariableCmd + 6, sizeof(StUGPacketInfo));

					
					if (stInfo.u32PacketLength > SPI_FLASH_GET_SIZE)
					{
						break;
					}
					/* <TODO> write flash */
					SPIFlashUpgradeWrite(&s_stSPIFlashUpgrade, pData, stInfo.u32PacketLength);
					
					SPIFlashUpgradeSetOffset(&s_stSPIFlashUpgrade, stInfo.u32FileOffset
						+ stInfo.u32PacketLength);
					SPIFlashUpgradeFlushBegin(&s_stSPIFlashUpgrade);
					
					u8EchoBase[_YNA_Data3] = 0x07;
					
					if ((stInfo.u32FileOffset + stInfo.u32PacketLength) >=
						s_stSPIFlashUpgrade.stFlashCtrl.u32TotalSize)
					{
						u8EchoBase[_YNA_Data3] = 0x08;
						SPIFlashUpgradeSetState(&s_stSPIFlashUpgrade, _UG_Invalide);				
					}						

					boNeedCopy = true;
					pEcho = (uint8_t *)malloc(PROTOCOL_YNA_DECODE_LENGTH);
					
					break;
				}
				case 0x800A:
				{
					/* check the crc32 and UUID, and BTEA and check the number */
					int32_t s32Err;
					char *pData = (char *)pVariableCmd + 6;
					StBteaKey stLic;
					StUID stUID;
					uint32_t u32CRC32;
					
					GetUID(&stUID);
					u32CRC32 = AppCRC32(~0);
					GetLic(&stLic, &stUID, u32CRC32, true);
					
					if (memcmp(&stLic, pData, sizeof(StBteaKey)) == 0)
					{
						s32Err = 0;
						
						WriteLic(&stLic, true, 0);
					}
					else
					{
						s32Err = -1;
					}
					pEcho = YNAMakeASimpleVarialbleCmd(0x800A, &s32Err, 4, &u32EchoLength);
					boGetVaildBaseCmd = true;
					break;
				}
				case 0x800D:
				{
					uint32_t *pData = (uint32_t *)(pVariableCmd + 6);

					SPIFlashUpgradeSetPosition(&s_stSPIFlashUpgrade, *pData);
					
					u8EchoBase[_YNA_Data3] = 0x0D;
					boNeedCopy = true;
					pEcho = (uint8_t *)malloc(PROTOCOL_YNA_DECODE_LENGTH);
					break;					
				}

				default:
					break;
			}
			
			if (boHasEcho && pEcho != NULL)
			{
				if (boNeedCopy)
				{
					YNAGetCheckSum(u8EchoBase);
					memcpy(pEcho, u8EchoBase, PROTOCOL_YNA_DECODE_LENGTH);
					u32EchoLength = PROTOCOL_YNA_DECODE_LENGTH;
				}

				if (pIOTCB == NULL)
				{
					free(pEcho);
				}
				else if (pIOTCB->pFunMsgWrite == NULL)
				{
					free(pEcho);
				}
				else if(pIOTCB->pFunMsgWrite(pEcho, true, _IO_Reserved, u32EchoLength) != 0)
				{
					free(pEcho);
				}
			}
			
			
			u32ReadLength += (6 + (uint32_t)u16Count * u16Length);
			pVariableCmd = pMsg + PROTOCOL_YNA_DECODE_LENGTH + u32ReadLength;
		}
	}
	else
	{
		boGetVaildBaseCmd = false;
	}

	return boGetVaildBaseCmd ? 0: -1;
}


void GlobalStateInit(void)
{
	g_u8CamAddr = 0;
}


void ChangeEncodeState(void)
{

}

void YNADecode(u8 *pBuf)
{
	if (g_u32BoolIsEncode)
	{
			
	}
	else
	{
		
	}

}
void YNAEncodeAndGetCheckSum(u8 *pBuf)
{
	if (g_u32BoolIsEncode)
	{
			
	}
	else
	{
		
	}
}


void YNAGetCheckSum(u8 *pBuf)
{
	s32 i, s32End;
	u8 u8Sum = pBuf[0];

	if (g_u32BoolIsEncode)
	{
		s32End = PROTOCOL_YNA_ENCODE_LENGTH - 1;	
	}
	else
	{
		s32End = PROTOCOL_YNA_DECODE_LENGTH - 1;
	}
	for (i = 1; i < s32End; i++)
	{
		u8Sum ^= pBuf[i];
	}
	pBuf[i] = u8Sum;
}

void PelcoDGetCheckSum(u8 *pBuf)
{
	s32 i;
	u8 u8Sum = 0;
	for (i = 1; i < 6; i++)
	{
		u8Sum += pBuf[i];
	}
	pBuf[i] = u8Sum;
}


static bool KeyBoardProcess(StKeyMixIn *pKeyIn)
{
	u32 i;
	for (i = 0; i < pKeyIn->u32Cnt; i++)
	{
		u8 *pBuf;
		StKeyState *pKeyState = pKeyIn->unKeyMixIn.stKeyState + i;
		u8 u8KeyValue;
		if (pKeyState->u8KeyState == KEY_KEEP)
		{
			continue;
		}

		u8KeyValue = pKeyState->u8KeyValue;

		pBuf = u8YNABuf;

		memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

		pBuf[_YNA_Sync] = 0xAA;
		pBuf[_YNA_Addr] = g_u8CamAddr;
		pBuf[_YNA_Mix] = 0x07;
		if (pKeyState->u8KeyState == KEY_UP)
		{
			pBuf[_YNA_Data1] = 0x01;
		}

		/* 处理按键 */
		switch (u8KeyValue)
		{
			case _Key_Switch_2:
			{
				SetKeyValue(LV_GROUP_KEY_ENTER, pKeyState->u8KeyState == KEY_DOWN);
				SetLvglKey(LV_GROUP_KEY_ENTER, pKeyState->u8KeyState == KEY_DOWN);
				break;
			}
			case _Key_Switch_1:
			{
				if (pKeyState->u8KeyState == KEY_DOWN)
				{
					if (SrceenProtectIsStart())
					{
						SrceenProtectForceStop();
					}
					else
					{
						SrceenProtectForceStart();
					}
				}
				break;
			}
			default:
				continue;
		}

		SrceenProtectReset();

		pBuf[_YNA_Data2] = u8KeyValue;
		
		YNAGetCheckSum(pBuf);
		CopyToUart1Message(pBuf, PROTOCOL_YNA_DECODE_LENGTH);	
	}
	return true;
}


static bool CodeSwitchProcess(StKeyMixIn *pKeyIn)
{
	u8 *pBuf;

	pBuf = u8YNABuf;
	if (pBuf == NULL)
	{
		return false;
	}

	memset(pBuf, 0, PROTOCOL_YNA_ENCODE_LENGTH);

	pBuf[_YNA_Sync] = 0xAA;
	pBuf[_YNA_Addr] = g_u8CamAddr;
	pBuf[_YNA_Mix] = 0x08;
	
	pBuf[_YNA_Data1] = pKeyIn->unKeyMixIn.stCodeSwitchState.u16Index;
	pBuf[_YNA_Data2] = pKeyIn->unKeyMixIn.stCodeSwitchState.u16Cnt;
	
	YNAGetCheckSum(pBuf);
	CopyToUart1Message(pBuf, PROTOCOL_YNA_DECODE_LENGTH);

{
	bool boIsPress = ((pKeyIn->unKeyMixIn.stCodeSwitchState.u16Cnt & 0x0001) == 0x0001);
	if (pKeyIn->unKeyMixIn.stCodeSwitchState.u16Index == 0)
	{
		if (pKeyIn->unKeyMixIn.stCodeSwitchState.u16Dir)
		{
			SetLvglKey(LV_GROUP_KEY_NEXT, boIsPress);
		}
		else
		{
			SetLvglKey(LV_GROUP_KEY_PREV, boIsPress);			
		}
	}
	else if (pKeyIn->unKeyMixIn.stCodeSwitchState.u16Index == 1)
	{
		SetKeySpeek(pKeyIn->unKeyMixIn.stCodeSwitchState.u16Speed);
		if (pKeyIn->unKeyMixIn.stCodeSwitchState.u16Dir)
		{
			SetLvglKey(LV_GROUP_KEY_RIGHT, boIsPress);
		}
		else
		{
			SetLvglKey(LV_GROUP_KEY_LEFT, boIsPress);			
		}	
	}

}	
	SrceenProtectReset();

	
	return true;
	
}

static PFun_KeyProcess s_KeyProcessArr[_Key_Reserved] = 
{
	NULL, KeyBoardProcess, NULL,
	NULL, CodeSwitchProcess, 
};


bool KeyProcess(StIOFIFO *pFIFO)
{
	StKeyMixIn *pKeyIn = pFIFO->pData;
	
	if (pKeyIn->emKeyType >= _Key_Reserved)
	{
		return false;
	}
	if (s_KeyProcessArr[pKeyIn->emKeyType] != NULL)
	{
		return s_KeyProcessArr[pKeyIn->emKeyType](pKeyIn);	
	}
	return false;
}

bool PCEchoProcessYNA(StIOFIFO *pFIFO, const StIOTCB *pIOTCB)
{
#if 1
	uint8_t *pMsg;
	if (pFIFO == NULL)
	{
		return false;
	}
	pMsg = (uint8_t *)pFIFO->pData;
	
	if (pMsg[_YNA_Mix] == 0x07)
	{
		switch(pMsg[_YNA_Cmd])
		{
			case 0x50:
			{
				LDPWrite(pMsg[_YNA_Data1] >> 3,
					 pMsg[_YNA_Data2] >> 3,
					 pMsg[_YNA_Data3] >> 3);
				break;
			}

			default:
				break;
		}		
	}
	else if (pMsg[_YNA_Mix] == 0x06)
	{
		uint8_t *u8EchoBase = u8YNABuf;
		uint8_t *pEcho = NULL;
		uint32_t u32EchoLength = 0;
		bool boHasEcho = true;
		bool boNeedCopy = true;
		u8EchoBase[_YNA_Sync] = 0xAA;
		u8EchoBase[_YNA_Mix] = 0x06;
		u8EchoBase[_YNA_Cmd] = 0x80;
		u8EchoBase[_YNA_Data1] = 0x01;
		switch(pMsg[_YNA_Cmd])
		{
			case 0x40:	/* get audio control mode */
			{
				StMixAudioCtrlMode *pMode = malloc(TOTAL_MODE_CTRL * sizeof(StMixAudioCtrlMode));
				boNeedCopy = false;
				if (pMode != NULL)
				{
					u16 i; 
					for (i = 0; i < TOTAL_MODE_CTRL_IN; i++)
					{
						EmAudioCtrlMode emMode;
						GetAudioCtrlMode(i, &emMode);

						pMode[i].u8Index = i;
						pMode[i].u8Mode = emMode;
					}
					pEcho = YNAMakeAnArrayVarialbleCmd(0x0640, pMode, 
						TOTAL_MODE_CTRL_IN, sizeof(StMixAudioCtrlMode), &u32EchoLength);
					free(pMode);
				}
				break;
			}
			case 0x41:	/* get out ctrl mode */
			{
				StMixAudioCtrlMode *pMode = malloc(TOTAL_MODE_CTRL_OUT * sizeof(StMixAudioCtrlMode));
				boNeedCopy = false;
				if (pMode != NULL)
				{
					u16 i; 
					for (i = 0; i < TOTAL_MODE_CTRL_OUT; i++)
					{
						EmAudioCtrlMode emMode;
						GetAudioCtrlMode(i + TOTAL_MODE_CTRL_IN, &emMode);

						pMode[i].u8Index = i + TOTAL_MODE_CTRL_IN;
						pMode[i].u8Mode = emMode;
					}
					pEcho = YNAMakeAnArrayVarialbleCmd(0x0641, pMode, 
						TOTAL_MODE_CTRL_OUT, sizeof(StMixAudioCtrlMode), &u32EchoLength);
					free(pMode);
				}
				break;
			}
			case 0x43:
			{
				boHasEcho = false;
				break;
			}
			case 0x44:
			{
				boHasEcho = false;
				
				break;
			}
			case 0x46:
			{	
				if (pMsg[_YNA_Data3] == 0xFF)
				{
					bool boState[PHANTOM_POWER_CTRL] = {0};
					StMixAudioPhantomPowerMode *pMode = malloc(PHANTOM_POWER_CTRL * 
						sizeof(StMixAudioPhantomPowerMode));
					boNeedCopy = false;
					if (pMode != NULL)
					{
						s32 i;
						GetAllPhantomPowerState(boState);
						for (i = 0; i < PHANTOM_POWER_CTRL; i++)
						{
							pMode[i].u8Index = i;
							pMode[i].u8Enable = boState[i];
						}
						pEcho = YNAMakeAnArrayVarialbleCmd(0x0646, pMode, 
							PHANTOM_POWER_CTRL, sizeof(StMixAudioVolume), &u32EchoLength);
						free(pMode);
					}
				}
				else
				{
					u8 u8Array = pMsg[_YNA_Data1] >> 4;
					bool boIsEnable = pMsg[_YNA_Data2] == 0x00 ? true : false;
					SetPhantomPowerState(u8Array, boIsEnable);
					ReflushActiveTable(_Fun_PhantomPower, u8Array);

					SendPhantomPowerStateCmdEx(u8Array, boIsEnable, FLAG_IO_USB_MIDI);
				}
				break;
			}
			case 0x50:
			{
				if ((pMsg[_YNA_Data1] == 0x00) || (pMsg[_YNA_Data1] == 0x81))
				{
					SetInputEnableState(~0, pMsg[_YNA_Data2]);
					SetOutputEnableState(~0, pMsg[_YNA_Data3]);
					ReflushActiveTable(_Fun_InputEnable, 0);
					SendInputEnableStateCmdEx(~0, pMsg[_YNA_Data2], FLAG_IO_USB_MIDI);
					SendOutputEnableStateCmdEx(~0, pMsg[_YNA_Data3], FLAG_IO_USB_MIDI);
					boNeedCopy = false;
				}
				else if (pMsg[_YNA_Data1] == 0x01)
				{
					u8EchoBase[_YNA_Cmd] = 0x50;
					u8EchoBase[_YNA_Data1] = 0x81;
					u8EchoBase[_YNA_Data2] = GetInputEnableState();
					u8EchoBase[_YNA_Data3] = GetOutputEnableState();
				}
					
				break;
			}

			case 0x80:	
			{
				StMixAudioVolume *pVolume = malloc(TOTAL_VOLUME_CHANNEL * sizeof(StMixAudioVolume));
				boNeedCopy = false;
				if (pVolume != NULL)
				{
					u16 i; 
					
					for (i = 0; i < TOTAL_VOLUME_CHANNEL; i++)
					{
						StVolume stVolume;
						GetAudioVolume(i, &stVolume);
						pVolume[i].u8Index = i;
						pVolume[i].u8Left = stVolume.u8Channel1;
						pVolume[i].u8Right = stVolume.u8Channel2;
					}
					pEcho = YNAMakeAnArrayVarialbleCmd(0x0680, pVolume, 
						TOTAL_VOLUME_CHANNEL, sizeof(StMixAudioVolume), &u32EchoLength);
					free(pVolume);
				}
				break;
			}

			case 0x81:	
			{
#if 0			
				const u8 u8ChannelMap[AUDIO_VOLTAGE_CNT + 1] = 
				{
					_Channel_AIN_1,
					_Channel_AIN_2,
					_Channel_AIN_3,
					_Channel_AIN_4,
					_Channel_AIN_5,
					_Channel_InnerSpeaker,
					_Channel_NormalOut,
				};				
				StMixAudioVoltage *pVoltage = malloc((AUDIO_VOLTAGE_CNT + 1) * sizeof(StMixAudioVoltage));
				boNeedCopy = false;
				if (pVoltage != NULL)
				{
					u16 i; 
					for (i = 0; i < AUDIO_VOLTAGE_CNT; i++)
					{
						pVoltage[i].u8Index = u8ChannelMap[i];
						pVoltage[i].u16Left = rand() % 4096;//GetAudioVoltage(i * 2 + 0);
						pVoltage[i].u16Right = rand() % 4096;//GetAudioVoltage(i * 2 + 1);
					}
					memcpy(pVoltage + i, pVoltage + i - 1, sizeof(StMixAudioVoltage));
					pVoltage[i].u8Index = u8ChannelMap[i];
					
					pEcho = YNAMakeAnArrayVarialbleCmd(0x0681, pVoltage, 
						AUDIO_VOLTAGE_CNT + 1, sizeof(StMixAudioVoltage), &u32EchoLength);
					free(pVoltage);
				}
#else
				boHasEcho = false;
				boNeedCopy = false;
#endif				
				break;
			}
			case 0xC1:
			{
				switch (pMsg[_YNA_Data1])
				{
					case 0x00:
					{
						uint16_t u16Status = 0;
						u16Status = pMsg[_YNA_Data2];
						u16Status <<= 8;
						u16Status |= pMsg[_YNA_Data3];
												
						SetAllVolumeUniformState(u16Status);					
						ReflushCurrentActiveTable(~0);

						boHasEcho = false;
						boNeedCopy = false;
						
						break;
					}
					case 0x01:
					{
						uint16_t u16Status = 0;
												
						GetAllVolumeUniformState(&u16Status);					
						u8EchoBase[_YNA_Cmd] = 0xC1;
						u8EchoBase[_YNA_Data1] = 0x00;
						u8EchoBase[_YNA_Data2] = u16Status >> 8;
						u8EchoBase[_YNA_Data3] = u16Status;

						break;
					}
					case (0x10 + _Channel_AIN_Mux):
					{
						StVolumeCtrlEnable stEnable;
						memcpy(&stEnable, pMsg + _YNA_Data3, sizeof(StVolumeCtrlEnable));
						
						SetVolumeCtrlState(_Channel_AIN_Mux, &stEnable);
						ReflushCurrentActiveTable(~0);

						boHasEcho = false;
						boNeedCopy = false;
						break;
					}
					case 0x20:
					{
						SetAudioDeviceListIndex(_Channel_PC_Ctrl_Play, pMsg[_YNA_Data2]);
						SetAudioDeviceListIndex(_Channel_PC_Ctrl_Record, pMsg[_YNA_Data3]);
						ReflushActiveTable(_Fun_AudioVolume, _Channel_PC_Ctrl_Record);
						
						boHasEcho = false;
						boNeedCopy = false;
						break;
					}
					default:
						boHasEcho = false;
						boNeedCopy = false;
						break;
				}
				break;
			}
			default:
				boHasEcho = false;
				boNeedCopy = false;
				break;
		}
		if (boNeedCopy)
		{
			YNAGetCheckSum(u8EchoBase);
			CopyToUartMessage(pIOTCB, u8EchoBase, PROTOCOL_YNA_DECODE_LENGTH);
		}
		if (boHasEcho && pEcho != NULL)
		{
			if (pIOTCB == NULL)
			{
				free(pEcho);
			}
			else if(pIOTCB->pFunMsgWrite == NULL)
			{
				free(pEcho);
			}
			else if (pIOTCB->pFunMsgWrite(pEcho, true, _IO_Reserved, u32EchoLength) != 0)
			{
				free(pEcho);
			}
		}
		
	}
	else if (pMsg[_YNA_Mix] == 0x04)	/* variable command */
	{
		uint32_t u32TotalLength = 0;
		uint32_t u32ReadLength = 0;
		uint8_t *pVariableCmd;
		/* get the total command length */
		LittleAndBigEndianTransfer((char *)(&u32TotalLength), (const char *)pMsg + _YNA_Data2, 2);
		pVariableCmd = pMsg + PROTOCOL_YNA_DECODE_LENGTH;
		u32TotalLength -= 2; /* CRC16 */
		while (u32ReadLength < u32TotalLength)
		{
			uint8_t *pEcho = NULL;
			uint32_t u32EchoLength = 0;
			bool boHasEcho = true;
			uint16_t u16Command = 0, u16Count = 0, u16Length = 0;
			LittleAndBigEndianTransfer((char *)(&u16Command),
				(char *)pVariableCmd, 2);
			LittleAndBigEndianTransfer((char *)(&u16Count),
				(char *)pVariableCmd + 2, 2);
			LittleAndBigEndianTransfer((char *)(&u16Length),
				(char *)pVariableCmd + 4, 2);

			switch (u16Command)
			{
				case 0x0640:
				case 0x0641:
				case 0x0642:
				{
					StMixAudioCtrlMode *pMode = (StMixAudioCtrlMode *)(pVariableCmd + 6);
					u16 i;
					for (i = 0; i < u16Count; i++)
					{
						SetAudioCtrlMode(pMode[i].u8Index, (EmAudioCtrlMode)(pMode[i].u8Mode));
						ReflushActiveTable(_Fun_AudioMode, pMode[i].u8Index);
			
						SendAudioCtrlModeCmdEx(pMode[i].u8Index, 
							(EmAudioCtrlMode)(pMode[i].u8Mode), FLAG_IO_USB_MIDI);
					}
					break;
				}
				case 0x0646:
				{
					StMixAudioPhantomPowerMode *pMode = (StMixAudioPhantomPowerMode *)(pVariableCmd + 6);
					u16 i;
					for (i = 0; i < u16Count; i++)
					{
						SetPhantomPowerState(pMode[i].u8Index, pMode[i].u8Enable);
						ReflushActiveTable(_Fun_PhantomPower, pMode[i].u8Index);
						
						SendPhantomPowerStateCmdEx(pMode[i].u8Index, 
									pMode[i].u8Enable, FLAG_IO_USB_MIDI);
					}
					break;
				}
				case 0x0680:
				{
					StMixAudioVolume *pVolume = (StMixAudioVolume *)(pVariableCmd + 6);
					u16 i;
					for (i = 0; i < u16Count; i++)
					{
						StVolume stVolume;
						
//						if (pVolume[i].u8Index == _Channel_AIN_Mux)
//						{
//							continue;
//						}
						stVolume.u8Channel1 = pVolume[i].u8Left;
						stVolume.u8Channel2 = pVolume[i].u8Right;
						SetAudioVolume(pVolume[i].u8Index, stVolume);
						ReflushActiveTable(_Fun_AudioVolume, pVolume[i].u8Index);
						
						SendAudioVolumeCmdEx(pVolume[i].u8Index, stVolume,
							FLAG_IO_USB_MIDI);
					}
					break;
				}
				case 0x06C1:
				{
					uint8_t *pCmd = pVariableCmd + 6;
					uint16_t u16Offset = 0;
					while (u16Offset < u16Length)
					{
						uint8_t u8Len = pCmd[1];
						if (pCmd[0] == 0x25)
						{
							SetAudioDeviceList(_Channel_PC_Ctrl_Play, (const char *)pCmd + 2, u8Len);
						}
						else if (pCmd[0] == 0x26)
						{
							SetAudioDeviceList(_Channel_PC_Ctrl_Record, (const char *)pCmd + 2, u8Len);
							
						}
						u16Offset += (u8Len + 2);
						pCmd += (u8Len + 2);
					}
					break;
				}
				default:
					break;
			}
			if (boHasEcho && pEcho != NULL)
			{
				if (pIOTCB == NULL)
				{
					free(pEcho);
				}
				else if(pIOTCB->pFunMsgWrite == NULL)
				{
					free(pEcho);
				}
				else if (pIOTCB->pFunMsgWrite(pEcho, true, _IO_Reserved, u32EchoLength) != 0)
				{
					free(pEcho);
				}
			}
			
			
			u32ReadLength += (6 + (uint32_t)u16Count * u16Length);
			pVariableCmd = pMsg + PROTOCOL_YNA_DECODE_LENGTH + u32ReadLength;
		}
	}
#endif
	return true;
}

bool PCEchoProcessForMIDI(StIOFIFO *pFIFO, const StIOTCB *pIOTCB)
{
	u8 *pMsg;
	u8 u8Cmd;
	u8 u8Key;
	bool boIsLightON = true;

	if (pFIFO == NULL)
	{
		return false;
	}

	pMsg = (u8 *)pFIFO->pData;
	
	if ((pMsg[1] & 0x0F) != g_u8MIDIChannel)
	{
		return false;
	}
	
	u8Cmd = (pMsg[1] & 0xF0);

	if ( u8Cmd == 0x80)
	{
		boIsLightON = false;
	}
	else if ((u8Cmd == 0x90) || (u8Cmd == 0xB0))
	{
		if ((pMsg[3] & 0x7F) == 0)
		{
			boIsLightON = false;
		}
	}
	else
	{
		return false;
	}
	
	u8Key = pMsg[2];
	
	if (u8Key >= 0x04 && u8Key <= 0x0F)
	{
		u16 u16Channel = RealToChannel(u8Key - 0x04);
		StVolume stVolume = {0};
		s32 ret = GetAudioVolume(u16Channel, &stVolume);
		if (ret != 0)
		{
			return false;
		}
		else
		{
			u32 u32Tmp = stVolume.u8Channel1 + stVolume.u8Channel2;
			u32Tmp >>= 2;
			if (((u8)u32Tmp) != pMsg[3])
			{
				u32Tmp = pMsg[3];
				u32Tmp <<= 1;
				u32Tmp &= 0xFF;
				if (u32Tmp == 0xFE)
				{
					u32Tmp = 0xFF;
				}
				
				stVolume.u8Channel1 = 
				stVolume.u8Channel2 = u32Tmp;
				
				SetAudioVolume(u16Channel, stVolume);
				
				ReflushActiveTable(_Fun_AudioVolume, u16Channel);
				
				SendAudioVolumeCmdEx(u16Channel, stVolume,
					FLAG_IO_UART1 | FLAG_IO_UART3);
			}
		}
	}
	else if (u8Key >= 0x20 && u8Key < 0x50)
	{
		u16 u16Channel = (u8Key - 0x20) >> 2;
		EmAudioCtrlMode emMode = (EmAudioCtrlMode)((u8Key - 0x20) & 0x03);
		EmAudioCtrlMode emModeOrg = _Audio_Ctrl_Mode_Normal;
		
		u16Channel = RealToChannel(u16Channel);
		if (emMode == _Audio_Ctrl_Mode_Normal)
		{
			if (!boIsLightON)
			{
				emMode = _Audio_Ctrl_Mode_ShieldLeftAndRight;
			}
		}
		else if (emMode == _Audio_Ctrl_Mode_ShieldLeft ||
				emMode == _Audio_Ctrl_Mode_ShieldRight)
		{
			if (u16Channel != _Channel_AIN_Mux && u16Channel != _Channel_PC)
			{
				return false;
			}		
		}
		
		GetAudioCtrlMode(u16Channel, &emModeOrg);
		if (emModeOrg != emMode)
		{
			SetAudioCtrlMode(u16Channel, emMode);
			ReflushActiveTable(_Fun_AudioMode, u16Channel);

			SendAudioCtrlModeCmdEx(u16Channel, emMode, 
					FLAG_IO_UART1 | FLAG_IO_UART3);
		}
		else
		{
			return false;
		}		
	}
	else if (u8Key >= 0x50 && u8Key <= 0x54)
	{
		u8 u8State = GetInputEnableState();
		u8 u8Channel = u8Key - 0x50;
		bool boCurState = !!(u8State & (1 << u8Channel));
		
		if (boCurState != boIsLightON)
		{
			SetInputEnableState(u8Channel, boIsLightON);
			
			ReflushActiveTable(_Fun_InputEnable, 0);
			SendInputEnableStateCmdEx(~0, 0, FLAG_IO_UART1 | FLAG_IO_UART2);
		}
		else
		{
			return false;
		}		
		
	}
	else if (u8Key >= 0x58 && u8Key <= 0x5A)
	{
		u8 u8State = GetOutputEnableState();
		u8 u8Channel = u8Key - 0x58;
		bool boCurState = !!(u8State & (1 << u8Channel));
		
		if (boCurState != boIsLightON)
		{
			SetOutputEnableState(u8Channel, boIsLightON);
			
			ReflushActiveTable(_Fun_OutputEnable, 0);
			SendOutputEnableStateCmdEx(~0, 0, FLAG_IO_UART1 | FLAG_IO_UART2);
		}
		else
		{
			return false;
		}				
	}
	else if (u8Key >= 0x60 && u8Key <= 0x61)
	{
		u8 u8Channel = u8Key - 0x60;
		bool boCurState = false;
		
		GetPhantomPowerState(u8Channel, &boCurState);
	
		if (boCurState != boIsLightON)
		{
			SetPhantomPowerState(u8Channel, boIsLightON);
			
			ReflushActiveTable(_Fun_PhantomPower, boIsLightON);
			
			SendPhantomPowerStateCmdEx(u8Channel, boIsLightON, FLAG_IO_UART1 | FLAG_IO_UART3);
		}
		else
		{
			return false;
		}				
	}
	
	return true;
}


bool PCEchoProcess(StIOFIFO *pFIFO, const StIOTCB *pIOTCB)
{
	if (pFIFO->u8ProtocolType == _Protocol_YNA)
	{
		return PCEchoProcessYNA(pFIFO, pIOTCB);
	}
	else if (pFIFO->u8ProtocolType == _Protocol_MIDI)
	{
		return PCEchoProcessForMIDI(pFIFO, pIOTCB);
	}
	
	return false;
}

void DeviceSendKeepAlive(void)
{
	uint8_t u8Base[PROTOCOL_YNA_DECODE_LENGTH] = {0};

	u8Base[_YNA_Sync] = 0xAA;
	u8Base[_YNA_Mix] = 0x0C;
	u8Base[_YNA_Cmd] = 0x80;
	u8Base[_YNA_Data1] = 0x00;
	u8Base[_YNA_Data3] = 0x02;
	YNAGetCheckSum(u8Base);
	
	CopyToUart3Message(u8Base, PROTOCOL_YNA_DECODE_LENGTH);
}

void DeviceGetCurState(void)
{
	unsigned char u8Cmd[PROTOCOL_YNA_DECODE_LENGTH] = { 0 };
	u8Cmd[_YNA_Sync] = 0xAA;
	u8Cmd[_YNA_Mix] = 0x06;

	u8Cmd[_YNA_Cmd] = 0x40;	/* 输入源 */
	YNAGetCheckSum(u8Cmd);
	CopyToUart3Message(u8Cmd, PROTOCOL_YNA_DECODE_LENGTH);



	u8Cmd[_YNA_Cmd] = 0x41;	/* 输出源 */
	YNAGetCheckSum(u8Cmd);
	CopyToUart3Message(u8Cmd, PROTOCOL_YNA_DECODE_LENGTH);

	u8Cmd[_YNA_Cmd] = 0x80;	/* 音量 */
	YNAGetCheckSum(u8Cmd);
	CopyToUart3Message(u8Cmd, PROTOCOL_YNA_DECODE_LENGTH);

	u8Cmd[_YNA_Cmd] = 0x50;	/* 输入输出通道 */
	u8Cmd[_YNA_Data1] = 0x01;
	YNAGetCheckSum(u8Cmd);
	CopyToUart3Message(u8Cmd, PROTOCOL_YNA_DECODE_LENGTH);

	u8Cmd[_YNA_Data1] = 0x00;

	u8Cmd[_YNA_Cmd] = 0x46;	/* 输入输出通道 */
	u8Cmd[_YNA_Data3] = 0xFF;
	YNAGetCheckSum(u8Cmd);
	CopyToUart3Message(u8Cmd, PROTOCOL_YNA_DECODE_LENGTH);
}

int32_t SendAudioCtrlModeCmdEx(uint16_t u16Channel, EmAudioCtrlMode emMode, uint32_t u32Flag)
{
	u16 u16RealChannel = ChannelToReal(u16Channel);

	if ((u16RealChannel >= TOTAL_CHANNEL))
	{
		return -1;
	}

	if ((u32Flag & (FLAG_IO_UART1 | FLAG_IO_UART3)) != 0)
	{
		StMixAudioCtrlMode stValue = {u16Channel, emMode};
		void *pCmd;
		uint32_t u32CmdLen = 0;
		uint16_t u16Cmd;
		if (u16Channel < TOTAL_MODE_CTRL_IN)
		{
			u16Cmd = 0x0640;
		}
		else if (u16Channel < TOTAL_MODE_CTRL)
		{
			u16Cmd = 0x0641;		
		}
		else if (u16Channel <= _Channel_PC_Ctrl_Record && u16Channel >= _Channel_PC_Ctrl_Play)
		{
			u16Cmd = 0x0642;
		}
		else 
		{
			return -1;
		}
		pCmd = YNAMakeAnArrayVarialbleCmd(u16Cmd, &stValue, 
					1, sizeof(StMixAudioCtrlMode), &u32CmdLen);
		
		if (pCmd != NULL)
		{
			if ((u32Flag & FLAG_IO_UART1) != 0)
			{
				CopyToUart1Message(pCmd, u32CmdLen);
			}
			
			if ((u32Flag & FLAG_IO_UART3) != 0)
			{
				CopyToUart3Message(pCmd, u32CmdLen);
			}
			
			{
				free(pCmd);
			}
		}
	}
	
	if ((u32Flag & FLAG_IO_USB_MIDI) != 0)
	{
		u8 u8Midi[4] = {0x09, 0x90};
		u8 u8KK = 0;
		u8 u8VV = 0x7F;
		
		if (emMode > _Audio_Ctrl_Mode_ShieldLeftAndRight)
		{
			return -1;
		}
		if (emMode == _Audio_Ctrl_Mode_ShieldLeft || 
				emMode == _Audio_Ctrl_Mode_ShieldRight)
		{
			if (u16Channel != _Channel_AIN_Mux && u16Channel != _Channel_PC)
			{
				return -1;
			}
			
			u8KK = emMode - _Audio_Ctrl_Mode_Normal; 
		}
		else if (emMode == _Audio_Ctrl_Mode_ShieldLeftAndRight)
		{
			u8Midi[0] = 0x08;
			u8Midi[1] = 0x80;
			
			u8VV = 0;
		}
		
		/* 0x20 + 12 * 4 = 0x50 */
		u8KK = 0x20 + u16RealChannel * 4 + u8KK;
	
		u8Midi[1] |= (g_u8MIDIChannel & 0x0F);
		u8Midi[2] = u8KK;
		u8Midi[3] = u8VV;
		
		CopyToUSBMessage(u8Midi, 4, _IO_USB_ENDP1);		
	}	
	
	return 0;
}

int32_t SendAudioVolumeCmdEx(uint16_t u16Channel, StVolume stVolume, uint32_t u32Flag)
{

	u16 u16RealChannel = ChannelToReal(u16Channel);

	if ((u16RealChannel >= TOTAL_CHANNEL))
	{
		return -1;
	}
	
	if ((u32Flag & (FLAG_IO_UART1 | FLAG_IO_UART3)) != 0)
	{
	
		StMixAudioVolume stValue = {u16Channel, stVolume.u8Channel1, stVolume.u8Channel2};
		void *pCmd;
		uint32_t u32CmdLen = 0;
		pCmd = YNAMakeAnArrayVarialbleCmd(0x0680, &stValue, 
					1, sizeof(StMixAudioVolume), &u32CmdLen);
		
		if (pCmd != NULL)
		{
			if ((u32Flag & FLAG_IO_UART1) != 0)
			{
				CopyToUart1Message(pCmd, u32CmdLen);
			}
			
			if ((u32Flag & FLAG_IO_UART3) != 0)
			{
				CopyToUart3Message(pCmd, u32CmdLen);
			}
			
			{
				free(pCmd);
			}

		}
	}
	
	if ((u32Flag & FLAG_IO_USB_MIDI) != 0)
	{
		u8 u8Midi[4] = {0x0B, 0xB0};
		u8 u8CC = 0x04 + u16RealChannel;

		u8Midi[1] |= (g_u8MIDIChannel & 0x0F);
		u8Midi[2] = u8CC;
		u8Midi[3] = ((stVolume.u8Channel1 + stVolume.u8Channel2) / 4) & 0x7F;	/* 2 * 2 = 4 */
		
		CopyToUSBMessage(u8Midi, 4, _IO_USB_ENDP1);		
	}
	
	return 0;	
}

int32_t SendInputEnableStateCmdEx(uint8_t u8Index, uint8_t u8NewState, uint32_t u32Flag)
{

	if ((u32Flag & (FLAG_IO_UART1 | FLAG_IO_UART3)) != 0)
	{
		uint8_t u8Cmd[PROTOCOL_YNA_DECODE_LENGTH];
		
		u8Cmd[_YNA_Sync] = 0xAA;
		u8Cmd[_YNA_Mix] = 0x06;
		
		u8Cmd[_YNA_Cmd] = 0x50;
		u8Cmd[_YNA_Data1] = 0x81;
		u8Cmd[_YNA_Data2] = GetInputEnableState();
		u8Cmd[_YNA_Data3] = GetOutputEnableState();

		YNAGetCheckSum(u8Cmd);
		if ((u32Flag & FLAG_IO_UART1) != 0)
		{
			CopyToUart1Message(u8Cmd, PROTOCOL_YNA_DECODE_LENGTH);
		}
		
		u8Cmd[_YNA_Data1] = 0x00;
		YNAGetCheckSum(u8Cmd);
		if ((u32Flag & FLAG_IO_UART1) != 0)
		{
			CopyToUart3Message(u8Cmd, PROTOCOL_YNA_DECODE_LENGTH);
		}
	}
	
	if ((u32Flag & FLAG_IO_USB_MIDI) != 0)
	{
		u8 u8MidiOn[4] = {0x09, 0x90};
		u8 u8MidiOff[4] = {0x08, 0x80};
		int32_t i, b = 0, e = ENABLE_INPUT_CTRL;
		if (u8Index < ENABLE_INPUT_CTRL)
		{
			b = u8Index;
			e = u8Index + 1;
		}
		u8MidiOn[1] |= (g_u8MIDIChannel & 0x0F);
		u8MidiOff[1] |= (g_u8MIDIChannel & 0x0F);
		
		for (i = b; i < e; i++)
		{
			if ((u8NewState & (1 << i)) != 0)
			{
				u8MidiOn[2] = 0x50 + i;
				u8MidiOn[3] = 0x7F;
				CopyToUSBMessage(u8MidiOn, 4, _IO_USB_ENDP1);					
			}
			else
			{
				u8MidiOff[2] = 0x50 + i;
				u8MidiOff[3] = 0x00;
				CopyToUSBMessage(u8MidiOff, 4, _IO_USB_ENDP1);					
				
			}
		}
	}

	return 0;
}


int32_t SendOutputEnableStateCmdEx(uint8_t u8Index, uint8_t u8NewState, uint32_t u32Flag)
{

	if ((u32Flag & (FLAG_IO_UART1 | FLAG_IO_UART3)) != 0)
	{
		uint8_t u8Cmd[PROTOCOL_YNA_DECODE_LENGTH];
		
		u8Cmd[_YNA_Sync] = 0xAA;
		u8Cmd[_YNA_Mix] = 0x06;
		
		u8Cmd[_YNA_Cmd] = 0x50;
		u8Cmd[_YNA_Data1] = 0x81;
		u8Cmd[_YNA_Data2] = GetInputEnableState();
		u8Cmd[_YNA_Data3] = GetOutputEnableState();

		YNAGetCheckSum(u8Cmd);
		if ((u32Flag & FLAG_IO_UART1) != 0)
		{
			CopyToUart1Message(u8Cmd, PROTOCOL_YNA_DECODE_LENGTH);
		}
		
		u8Cmd[_YNA_Data1] = 0x00;
		YNAGetCheckSum(u8Cmd);
		if ((u32Flag & FLAG_IO_UART1) != 0)
		{
			CopyToUart3Message(u8Cmd, PROTOCOL_YNA_DECODE_LENGTH);
		}
	}
	
	if ((u32Flag & FLAG_IO_USB_MIDI) != 0)
	{
		u8 u8MidiOn[4] = {0x09, 0x90};
		u8 u8MidiOff[4] = {0x08, 0x80};
		int32_t i, b = 0, e = ENABLE_OUTPUT_CTRL;
		if (u8Index < ENABLE_OUTPUT_CTRL)
		{
			b = u8Index;
			e = u8Index + 1;
		}
		u8MidiOn[1] |= (g_u8MIDIChannel & 0x0F);
		u8MidiOff[1] |= (g_u8MIDIChannel & 0x0F);
		
		for (i = b; i < e; i++)
		{
			if ((u8NewState & (1 << i)) != 0)
			{
				u8MidiOn[2] = 0x58 + i;
				u8MidiOn[3] = 0x7F;
				CopyToUSBMessage(u8MidiOn, 4, _IO_USB_ENDP1);					
			}
			else
			{
				u8MidiOff[2] = 0x58 + i;
				u8MidiOff[3] = 0x00;
				CopyToUSBMessage(u8MidiOff, 4, _IO_USB_ENDP1);					
				
			}
		}
	}

	return 0;
}

int32_t SendPhantomPowerStateCmdEx(uint16_t u16Channel, bool boIsEnable, uint32_t u32Flag)
{
	if ((u16Channel >= PHANTOM_POWER_CTRL))
	{
		return -1;
	}

	if ((u32Flag & (FLAG_IO_UART1 | FLAG_IO_UART3)) != 0)
	{

		StMixAudioPhantomPowerMode stValue = {u16Channel, boIsEnable};
		void *pCmd;
		uint32_t u32CmdLen = 0;
		pCmd = YNAMakeAnArrayVarialbleCmd(0x0646, &stValue, 
					1, sizeof(StMixAudioPhantomPowerMode), &u32CmdLen);
		
		if (pCmd != NULL)
		{
			if ((u32Flag & FLAG_IO_UART1) != 0)
			{
				CopyToUart1Message(pCmd, u32CmdLen);
			}
			
			if ((u32Flag & FLAG_IO_UART3) != 0)
			{
				CopyToUart3Message(pCmd, u32CmdLen);
			}
			
			{
				free(pCmd);
			}
		}
	}
	if ((u32Flag & FLAG_IO_USB_MIDI) != 0)
	{
		u8 u8MidiOn[4] = {0x09, 0x90};
		u8 u8MidiOff[4] = {0x08, 0x80};
		
		u8MidiOn[1] |= (g_u8MIDIChannel & 0x0F);
		u8MidiOff[1] |= (g_u8MIDIChannel & 0x0F);
		
		if (boIsEnable)
		{
			u8MidiOn[2] = 0x60 + u16Channel;
			u8MidiOn[3] = 0x7F;
			CopyToUSBMessage(u8MidiOn, 4, _IO_USB_ENDP1);					
		}
		else
		{
			u8MidiOff[2] = 0x60 + u16Channel;
			u8MidiOff[3] = 0x00;
			CopyToUSBMessage(u8MidiOff, 4, _IO_USB_ENDP1);					
			
		}
	}
	return 0;		
}

/* GUI 命令发送 */

int32_t SendAudioCtrlModeCmd(uint16_t u16Channel, EmAudioCtrlMode emMode)
{
#if 1
	return SendAudioCtrlModeCmdEx(u16Channel, emMode, 
				FLAG_IO_UART1 | FLAG_IO_UART3 | FLAG_IO_USB_MIDI);
#else
	StMixAudioCtrlMode stValue = {u16Channel, emMode};
	void *pCmd;
	uint32_t u32CmdLen = 0;
	uint16_t u16Cmd;
	if (u16Channel < TOTAL_MODE_CTRL_IN)
	{
		u16Cmd = 0x0640;
	}
	else if (u16Channel < TOTAL_MODE_CTRL)
	{
		u16Cmd = 0x0641;		
	}
	else
	{
		u16Cmd = 0x0642;
	}
	pCmd = YNAMakeAnArrayVarialbleCmd(u16Cmd, &stValue, 
				1, sizeof(StMixAudioCtrlMode), &u32CmdLen);
	
	if (pCmd != NULL)
	{
		uint32_t res = MessageUartWrite(pCmd, true, 0, u32CmdLen);
		CopyToUart3Message(pCmd, u32CmdLen);
		if (res != 0)
		{
			free(pCmd);
			return -1;
		}
		return 0;
	}
	return -1;
#endif	
}

int32_t SendUniformCheckState(uint16_t u16Channel, uint16_t u16State)
{
	uint8_t u8Cmd[PROTOCOL_YNA_DECODE_LENGTH];
	
	uint16_t u16Status = 0;
							
	GetAllVolumeUniformState(&u16Status);
	
	u8Cmd[_YNA_Sync] = 0xAA;
	u8Cmd[_YNA_Mix] = 0x06;
	
	u8Cmd[_YNA_Cmd] = 0xC1;
	u8Cmd[_YNA_Data1] = 0x00;
	u8Cmd[_YNA_Data2] = u16Status >> 8;
	u8Cmd[_YNA_Data3] = u16Status;

	YNAGetCheckSum(u8Cmd);
	CopyToUart1Message(u8Cmd, PROTOCOL_YNA_DECODE_LENGTH);

	return 0;
}

int32_t SendAudioVolumeCmd(uint16_t u16Channel, StVolume stVolume)
{
#if 1
	return SendAudioVolumeCmdEx(u16Channel, stVolume, 
			FLAG_IO_UART1 | FLAG_IO_UART3 | FLAG_IO_USB_MIDI);
#else
	StMixAudioVolume stValue = {u16Channel, stVolume.u8Channel1, stVolume.u8Channel2};
	void *pCmd;
	uint32_t u32CmdLen = 0;
	pCmd = YNAMakeAnArrayVarialbleCmd(0x0680, &stValue, 
				1, sizeof(StMixAudioVolume), &u32CmdLen);
	
	if (pCmd != NULL)
	{
		uint32_t res = MessageUartWrite(pCmd, true, 0, u32CmdLen);
		CopyToUart3Message(pCmd, u32CmdLen);
		if (res != 0)
		{
			free(pCmd);
			return -1;
		}
		return 0;
	}
	return -1;	
#endif
}
int32_t SendPhantomPowerStateCmd(uint16_t u16Channel, bool boIsEnable)
{
#if 1
	return SendPhantomPowerStateCmdEx(u16Channel, boIsEnable,
			FLAG_IO_UART1 | FLAG_IO_UART3 | FLAG_IO_USB_MIDI);
#else

	StMixAudioPhantomPowerMode stValue = {u16Channel, boIsEnable};
	void *pCmd;
	uint32_t u32CmdLen = 0;
	pCmd = YNAMakeAnArrayVarialbleCmd(0x0646, &stValue, 
				1, sizeof(StMixAudioPhantomPowerMode), &u32CmdLen);
	
	if (pCmd != NULL)
	{
		uint32_t res = MessageUartWrite(pCmd, true, 0, u32CmdLen);
		CopyToUart3Message(pCmd, u32CmdLen);
		if (res != 0)
		{
			free(pCmd);
			return -1;
		}
		return 0;
	}
	return -1;	
	
#endif	
}

int32_t SendInputEnableStateCmd(uint8_t u8Index, uint8_t u8NewState)
{
#if 1
	return SendInputEnableStateCmdEx(u8Index, u8NewState,
			FLAG_IO_UART1 | FLAG_IO_UART3 | FLAG_IO_USB_MIDI);
#else

	uint8_t u8Cmd[PROTOCOL_YNA_DECODE_LENGTH];
	
	u8Cmd[_YNA_Sync] = 0xAA;
	u8Cmd[_YNA_Mix] = 0x06;
	
	u8Cmd[_YNA_Cmd] = 0x50;
	u8Cmd[_YNA_Data1] = 0x81;
	u8Cmd[_YNA_Data2] = GetInputEnableState();
	u8Cmd[_YNA_Data3] = GetOutputEnableState();

	YNAGetCheckSum(u8Cmd);
	CopyToUart1Message(u8Cmd, PROTOCOL_YNA_DECODE_LENGTH);
	
	u8Cmd[_YNA_Data1] = 0x00;
	YNAGetCheckSum(u8Cmd);
	CopyToUart3Message(u8Cmd, PROTOCOL_YNA_DECODE_LENGTH);

	return 0;
#endif
}
int32_t SendOutputEnableStateCmd(uint8_t u8Index, uint8_t u8NewState)
{
#if 1
	return SendOutputEnableStateCmdEx(u8Index, u8NewState,
			FLAG_IO_UART1 | FLAG_IO_UART3 | FLAG_IO_USB_MIDI);
#else
	return SendInputEnableStateCmd(u8Index, u8NewState);
#endif
}

int32_t SendPCAudioDeviceSelectCmd(uint16_t u16Channel, uint8_t u8Select)
{
	uint8_t u8Cmd[PROTOCOL_YNA_DECODE_LENGTH];
	
	u8Cmd[_YNA_Sync] = 0xAA;
	u8Cmd[_YNA_Mix] = 0x06;
	
	u8Cmd[_YNA_Cmd] = 0xC1;
	u8Cmd[_YNA_Data1] = 0x20;
	
	GetAudioDeviceListIndex(_Channel_PC_Ctrl_Play, u8Cmd + _YNA_Data2);
	GetAudioDeviceListIndex(_Channel_PC_Ctrl_Record, u8Cmd + _YNA_Data3);
	
	YNAGetCheckSum(u8Cmd);
	CopyToUart1Message(u8Cmd, PROTOCOL_YNA_DECODE_LENGTH);
	return 0;
}

int32_t SendMemeoryCtrlCmd(uint16_t u16Channel, bool boIsSave)
{
	uint8_t u8Cmd[PROTOCOL_YNA_DECODE_LENGTH];
	
	u8Cmd[_YNA_Sync] = 0xAA;
	u8Cmd[_YNA_Addr] = 0x00;
	u8Cmd[_YNA_Mix] = 0x06;
	
	if (boIsSave)
	{
		u8Cmd[_YNA_Cmd] = 0x43;	
	}
	else
	{
		u8Cmd[_YNA_Cmd] = 0x44;			
	}
	u8Cmd[_YNA_Data1] = 0x00;
	u8Cmd[_YNA_Data2] = (uint8_t)u16Channel;
	u8Cmd[_YNA_Data3] = 0x00;

	YNAGetCheckSum(u8Cmd);
	CopyToUart3Message(u8Cmd, PROTOCOL_YNA_DECODE_LENGTH);
	
	if (!boIsSave)
	{
		DeviceGetCurState();
	}
	
	return 0;
}
int32_t SendFactoryCtrlCmd(void)
{
	return SendMemeoryCtrlCmd(0xFF, false);
}

int32_t SendLogoColorCtrlCmd(lv_color24_t stColor)
{
	LDPWrite(stColor.red, stColor.green, stColor.blue);
	
	return 0;
}

int32_t SendKeyboardPowerCmd(bool boIsPowerOn)
{
	KeyboardPowerEnable(boIsPowerOn);
	return 0;
}

int32_t SendKeyboardConnectCmd(uint8_t u8CurConnect)
{
	KeyboardConnectSetMode(u8CurConnect);
	return 0;
}

int32_t SendPCKeyboardPowerCmd(uint8_t u8CurConnect)
{
	PCKeyboardPowerEnable(u8CurConnect);
	return 0;
}

int32_t SendMIDIChannelCmd(uint8_t u8Index)
{
	if (u8Index < 16)
	{
		g_u8MIDIChannel = u8Index;
	}
	
	return 0;
}


void SaveMemoryFromDevice(StPowerOffMemory *pMem)
{
	int32_t i;
	for (i = _Channel_AIN_Mux; i <= _Channel_NormalOut; i++)
	{
		GetUnionVolumeValue(i, pMem->boUnionVolume + i - _Channel_AIN_Mux);			
	}
	GetLogoColor(&(pMem->stLogoColor));
	GetKeyboardPowerValue(&(pMem->boIsKeyboardPowerOn));
	GetKeyboardConnectMode(&(pMem->u8KeyboardConnectMode));	
	GetScreenProtectTimeIndex(&(pMem->u8ScreenProtectTimeIndex));
	GetMIDIChannelIndex(&(pMem->u8MIDIChannelIndex));
}

void LoadMemoryToDevice(StPowerOffMemory *pMem)
{
	int32_t i;
	//StPowerOffMemory *pMem = &(g_stSave.stMemory);
	for (i = _Channel_AIN_Mux; i <= _Channel_NormalOut; i++)
	{
		SetUnionVolumeValue(i, pMem->boUnionVolume[i - _Channel_AIN_Mux]);			
	}
	SetLogoColor(pMem->stLogoColor);
	SetKeyboardPowerValue(pMem->boIsKeyboardPowerOn);
	SetKeyboardConnectMode(pMem->u8KeyboardConnectMode);	
	SetScreenProtectTimeIndex(pMem->u8ScreenProtectTimeIndex);
	SetScreenProtectModeIndex(pMem->u8ScreenProtectModeIndex);
	SetMIDIChannelIndex(pMem->u8MIDIChannelIndex);
}

void LoadPowerOffMemoryToDevice(void)
{
	LoadMemoryToDevice(&(g_stSave.stMemory));
}

/* GUI 命令发送 */


void PowerOffMemoryFlush(void)	/* 100ms flush */
{
	static StPowerOffMemory StMemMonitor = {0};
	static StPowerOffMemory StMemCur = {0};
	static bool boIsGetMonitor = false;
	static u32 u32GetMonitorTime = 0;
	static u32 u32MonitorTimeInterval = 0;
	
	if (SysTimeDiff(u32MonitorTimeInterval, g_u32SysTickCnt) < 100)
	{
		return;
	}
	u32MonitorTimeInterval = g_u32SysTickCnt;

	memset(&StMemCur, 0, sizeof(StPowerOffMemory));
	SaveMemoryFromDevice(&StMemCur);
	if (memcmp(&StMemMonitor, &StMemCur, sizeof(StPowerOffMemory)) != 0)
	{
		/* begin to monitor */
		memcpy(&StMemMonitor, &StMemCur, sizeof(StPowerOffMemory));
		boIsGetMonitor = true;
		u32GetMonitorTime = g_u32SysTickCnt;
	}
	if (boIsGetMonitor)
	{
		if (SysTimeDiff(u32GetMonitorTime, g_u32SysTickCnt) > 15 * 1000)	/* 15S unchanged */
		{
			/* prevent to write flash after power on */
			if (memcmp(&StMemMonitor, &g_stSave.stMemory, sizeof(StPowerOffMemory)) != 0)
			{
				memcpy(&(g_stSave.stMemory), &StMemMonitor, sizeof(StPowerOffMemory));
				WriteSaveData();
			}
			boIsGetMonitor = false;
		}
	}
	

}



__weak void SetKeySpeek(uint16_t u16Speed)
{
	
}

__weak void SetLvglKey(uint32_t u32Key, bool boIsPress)
{

}

__weak void SetKeyValue(uint32_t u32Key, bool boIsPress)
{

}

__weak int32_t ReflushActiveTable(uint32_t u32Fun, uint32_t u32Channel)
{
	return 0;
}

__weak int32_t ChannelToReal(uint16_t u16Channel)
{
	return 0;
}
__weak int32_t RealToChannel(uint16_t u16Channel)
{
	return 0;
}


__weak int32_t GetAudioCtrlMode(uint16_t u16Channel, EmAudioCtrlMode *pMode)
{
	return 0;
}

__weak int32_t SetAudioCtrlMode(uint16_t u16Channel, EmAudioCtrlMode emMode)
{
	return 0;
}

__weak void GetAllAudioCtrlMode(EmAudioCtrlMode emAudioCtrlMode[TOTAL_MODE_CTRL])
{
	return;
}

__weak void SetAllAudioCtrlMode(EmAudioCtrlMode emAudioCtrlMode[TOTAL_MODE_CTRL])
{
	return;
}


__weak int32_t SetAudioVolume(uint16_t u16Channel, StVolume stVolume)
{
	return 0;
}

__weak int32_t GetAudioVolume(uint16_t u16Channel, StVolume *pVolume)
{
	return 0;
}

__weak void GetAllAudioVolume(StVolume stVolume[TOTAL_VOLUME_CHANNEL])
{
	return;
}

__weak void SetAllAudioVolume(StVolume stVolume[TOTAL_VOLUME_CHANNEL])
{
	return;
}


__weak int32_t GetPhantomPowerState(uint16_t u16Channel, bool *pState)
{
	return 0;
}

__weak int32_t SetPhantomPowerState(uint16_t u16Channel, bool boIsEnable)
{
	return 0;
}

__weak void GetAllPhantomPowerState(bool boState[PHANTOM_POWER_CTRL])
{
	return;
}

__weak void SetAllPhantomPowerState(bool boState[PHANTOM_POWER_CTRL])
{
	return;
}


__weak uint8_t GetInputEnableState(void)
{
	return 0;
}

__weak int32_t SetInputEnableState(uint8_t u8Index, uint8_t u8NewState)
{
	return 0;
}

__weak uint8_t GetOutputEnableState(void)
{
	return 0;
}

__weak int32_t SetOutputEnableState(uint8_t u8Index, uint8_t u8NewState)
{
	return 0;
}

__weak int32_t SetAudioDeviceList(uint8_t u8Channel, const char *pAudioDeviceList, int32_t s32Length)
{
	return 0;
}
__weak int32_t SetAudioDeviceListIndex(uint8_t u8Channel, uint8_t u8Index)
{
	return 0;
}
__weak int32_t GetAudioDeviceListIndex(uint8_t u8Channel, uint8_t *pIndex)
{
	return 0;
}


__weak int32_t GetUnionVolumeValue(uint16_t u16Channel, bool *pValue)
{
	return -1;
}

__weak int32_t GetLogoColor(lv_color24_t *pValue)
{
	return -1;
}

__weak int32_t GetKeyboardPowerValue(bool *pIsPowerOn)
{
	return -1;
}

__weak int32_t GetKeyboardConnectMode(uint8_t *pCurConnect)
{
	return -1;
}


__weak int32_t SetUnionVolumeValue(uint16_t u16Channel, bool boValue)
{
	return -1;
}

__weak int32_t SetLogoColor(lv_color24_t stValue)
{
	return -1;
}

__weak int32_t SetKeyboardPowerValue(bool boIsPowerOn)
{
	return -1;
}

__weak int32_t SetKeyboardConnectMode(uint8_t u8CurConnect)
{
	return -1;
}


__weak int32_t GetScreenProtectTimeIndex(uint8_t *pIndex)
{
	return -1;
}

__weak int32_t GetScreenProtectModeIndex(uint8_t *pIndex)
{
	return -1;
}

__weak int32_t SetScreenProtectTimeIndex(uint8_t u8Index)
{
	return -1;
}

__weak int32_t SetScreenProtectModeIndex(uint8_t u8Index)
{
	return -1;
}

__weak int32_t GetMIDIChannelIndex(uint8_t *pIndex)
{
	return 0;
}
__weak int32_t SetMIDIChannelIndex(uint8_t u8Index)
{
	return -1;
}

__weak bool SrceenProtectIsStart(void)
{
	return false;
}

__weak void SrceenProtectReset(void)
{
}

__weak void SrceenProtectForceStart(void)
{
}

__weak void SrceenProtectForceStop(void)
{
}





