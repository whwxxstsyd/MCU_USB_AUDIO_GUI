/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����key_led.c
* ժҪ: �����Լ�LEDˢ�³���
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/

#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "io_buf_ctrl.h"
#include "app_port.h"
#include "user_api.h"

#include "key_led.h"
#include "adc_ctrl.h"
#include "code_switch.h"
#include "key_led_ctrl.h"

#include "key_led_table.h"

static StKeyScan s_stKeyScan;
static StLedScan s_stLedScan;
static StLedSize s_stLedBlinkState[LED_Y_CNT];

static StLedSize s_stLedStateBackup[LED_Y_CNT];
static StLedSize s_stLedBlinkStateBackup[LED_Y_CNT];


GPIO_TypeDef *const c_pKeyInPort[KEY_X_CNT] = 
{
	KEY_X_PORT_1,		
	KEY_X_PORT_2,		
	KEY_X_PORT_3,		
};

const u16 c_u16KeyInPin[KEY_X_CNT] = 
{
	KEY_X_1,		
	KEY_X_2,		
	KEY_X_3,		
};

GPIO_TypeDef *const c_pKeyLedPowerPort[KEY_Y_CNT] = 
{
	KEY_LED_POWER_PORT_1,
    KEY_LED_POWER_PORT_2,
    KEY_LED_POWER_PORT_3,
    KEY_LED_POWER_PORT_4,
};

const u16 c_u16KeyLedPowerPin[KEY_Y_CNT] = 
{
	KEY_LED_POWER_1,
    KEY_LED_POWER_2,
    KEY_LED_POWER_3,
    KEY_LED_POWER_4,

};

GPIO_TypeDef *const c_pLedInPort[LED_X_CNT] = 
{
	LED_X_PORT_1,		
};

const u16 c_u16LedInPin[LED_X_CNT] = 
{
	LED_X_1,		
};


const u8 c_u8KeyPowerUsedMap[KEY_Y_CNT] = 
{
	0, 1, 2, 3
};

#define POWER_PIN_HAS_NULL		1
#define KEY_PIN_HAS_NULL		0
#define LED_PIN_HAS_NULL		0


/* ɨ����������һ�� */
/* power ΪY ��������ΪX ���� */
static void KeyScanOnce(StKeyScan *pKey)
{
	u32 i;
	u32 u32Real;
	u32 u32Cnt = pKey->u32ScanCnt % KEY_SCAN_CNT;
	
	u32 j;
	for (j = 0; j < KEY_Y_CNT; j++)
	{
		u32Real = c_u8KeyPowerUsedMap[j];
#if POWER_PIN_HAS_NULL
		if (c_pKeyLedPowerPort[u32Real] != NULL)
#endif
		{
			c_pKeyLedPowerPort[u32Real]->BSRR = c_u16KeyLedPowerPin[u32Real];
		}
	}
	
	for (i = 0; i < KEY_Y_CNT; i++)
	{
		u16 u16KeyValue, u16Tmp;
		u32Real = c_u8KeyPowerUsedMap[i];
#if POWER_PIN_HAS_NULL
		if (c_pKeyLedPowerPort[u32Real] != NULL)
#endif
		{
			c_pKeyLedPowerPort[u32Real]->BRR = c_u16KeyLedPowerPin[u32Real];
		}

		for (j = 0; j < 40; j++);
		__NOP();
		__NOP();
		__NOP();
		
		u16KeyValue = 0;
		for (j = 0; j < KEY_X_CNT; j++)
		{
#if KEY_PIN_HAS_NULL
			if (c_pKeyInPort[j] != NULL)
#endif
			{
				u16Tmp = c_pKeyInPort[j]->IDR & c_u16KeyInPin[j];
			}
			u16Tmp = !!u16Tmp;

			u16KeyValue |= (u16Tmp << j);
		}
		
#if POWER_PIN_HAS_NULL
		if (c_pKeyLedPowerPort[u32Real] != NULL)
#endif
		{
			c_pKeyLedPowerPort[u32Real]->BSRR = c_u16KeyLedPowerPin[u32Real];
		}

		pKey->stKeyTmp[u32Cnt].u8KeyValue[i] = u16KeyValue & 0xFF;
	}
	pKey->u32ScanCnt++;	
}

/* ����, ��ȷ����true, ����pKeyOut����д����, ���򷵻�false */
static bool KeyCheckValue(StKeyValue *pKeyTmp, StKeyValue *pKeyOut)
{
	s32 i, j, s32NotSameCnt;
	s32 s32ValidRow = 0xFF;
	s32NotSameCnt = 0;
	for (i = 0; i < (KEY_SCAN_CNT - 1); i++)
	{
		for (j = 0; j < KEY_Y_CNT; j++)
		{
			if(pKeyTmp[i].u8KeyValue[j] != pKeyTmp[i + 1].u8KeyValue[j])
			{
				s32NotSameCnt++;
				break;
			}			
		}
		if (j == KEY_Y_CNT)
		{
			s32ValidRow = i;			/* ȡ������ͬ������ */
		}
	}
	if (s32NotSameCnt > ((KEY_SCAN_CNT >> 1) + 1))		
	{
		return false; /* �������ϲ�һ�� */
	}
	if (s32ValidRow >= KEY_SCAN_CNT)
	{
		return false;
	}		

	*pKeyOut = pKeyTmp[s32ValidRow];

	return true;
}

/*У���λ��ͻ*/
static bool KeyClashCheck(StKeyState *pKeyState, u8 u8Cnt)
{
	u8 a,b,c;

	for (a = 0; a < (u8Cnt - 1); a++)
	{
	 	for( b = (a + 1); b < u8Cnt; b++)
		{
			/* Y ���� ��ֵһ�� */
			if((pKeyState[a].u8KeyLocation & 0xF0) == (pKeyState[b].u8KeyLocation & 0xF0))
			{
				
				/* ��� a ���� */
				for(c = 0; c < a; c++)
				{			
					/* X ���� ��ֵһ�� */
					if ((pKeyState[a].u8KeyLocation & 0x0F) == (pKeyState[c].u8KeyLocation & 0x0F))
					{
						return true;	
					}			
				}

				for(c = (a + 1); c < u8Cnt; c++)
				{
					if ((pKeyState[a].u8KeyLocation & 0x0F) == (pKeyState[c].u8KeyLocation & 0x0F))
					{
						return true;	
					}			
				}

				/* ��� b ���� */
				for(c = 0; c < b; c++)
				{
					if ((pKeyState[b].u8KeyLocation & 0x0F) == (pKeyState[c].u8KeyLocation & 0x0F))
					{
						return true;	
					}			
				}

				for(c = (b + 1); c < u8Cnt; c++)
				{
					if ((pKeyState[b].u8KeyLocation & 0x0F) == (pKeyState[c].u8KeyLocation & 0x0F))
					{
						return true;	
					}			
				}
			}
		}
	}
	return false;

}


/* �õ�ɨ�赽����Ч��ֵ, ����ɨ�赽������ */
static u8 KeyGetValid(StKeyScan *pKey)
{
	if (((pKey->u32ScanCnt) & (KEY_SCAN_CNT - 1)) == (KEY_SCAN_CNT - 1))
	{
		s32 i, j;
		bool boHasKey = KeyCheckValue(pKey->stKeyTmp, &(pKey->stKeyNow));
		u8 *pNow, *pOld;
		StKeyState *pKeyState;
		u8 u8KeyCnt;
		if (!boHasKey)
		{
			return 0;
		}
		
		pNow = pKey->stKeyNow.u8KeyValue;
		pOld = pKey->stKeyOld.u8KeyValue;
		pKeyState = pKey->stKeyState;
		u8KeyCnt = 0;


		for (i = 0; i < KEY_Y_CNT; i++)
		{
			u8 u8NotSame = 0;
			u8 u8PressKeep = 0;
			
			u8NotSame =  pNow[i] ^ pOld[i];
			u8PressKeep = pNow[i] & pOld[i];

			if (u8NotSame != 0)
			{
				for (j = 0; j < KEY_X_CNT; j++)
				{
					if ((u8NotSame & (1 << j)) != 0)
					{
						if (u8KeyCnt >= KEY_MIX_MAX)
						{
							goto end;
						}
						pKeyState[u8KeyCnt].u8KeyLocation = ((i << 4) & 0xF0) | (j & 0x0F);
						/* <TODO:> ��д��ֵ */
						pKeyState[u8KeyCnt].u8KeyValue = g_u8KeyTable[i][j];
						if ((pNow[i] & (1 << j)) != 0)
						{
							pKeyState[u8KeyCnt].u8KeyState = KEY_DOWN;						
						}
						else
						{
							pKeyState[u8KeyCnt].u8KeyState = KEY_UP;
						}
						u8KeyCnt++;
					}
				}	
			}

			if (u8PressKeep != 0)
			{
				for (j = 0; j < KEY_X_CNT; j++)
				{
					if ((u8PressKeep & (1 << j)) != 0)
					{
						if (u8KeyCnt >= KEY_MIX_MAX)
						{
							goto end;
						}
						pKeyState[u8KeyCnt].u8KeyLocation = ((i << 4) & 0xF0) | (j & 0x0F);
						/* <TODO:> ��д��ֵ */
						pKeyState[u8KeyCnt].u8KeyValue = g_u8KeyTable[i][j];
						pKeyState[u8KeyCnt].u8KeyState = KEY_KEEP;						
						u8KeyCnt++;
					}
				}	
			}
		}
end:
		memcpy(pOld, pNow, KEY_Y_CNT);
		if(u8KeyCnt >= 4)
		{
			if (KeyClashCheck(pKey->stKeyState, u8KeyCnt))
			{
				return 0;
			}
		}
		return u8KeyCnt;
	}
	else
	{
		return 0;
	}
}

/* ���� X �������ų�ʼ�� */
static void KeyXPinInit(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	u32 i;
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	for (i = 0; i < KEY_X_CNT; i++)
	{
		GPIO_InitStructure.GPIO_Pin = c_u16KeyInPin[i];
		GPIO_Init((GPIO_TypeDef *)c_pKeyInPort[i], &GPIO_InitStructure);
		c_pKeyInPort[i]->BRR = c_u16KeyInPin[i];
	}
	i = i;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	for (i = 0; i < KEY_X_CNT; i++)
	{
		GPIO_InitStructure.GPIO_Pin = c_u16KeyInPin[i];
		GPIO_Init((GPIO_TypeDef *)c_pKeyInPort[i], &GPIO_InitStructure);
	}
	
	i = i;
}
/* LED X �������ų�ʼ�� */
static void LEDXPinInit(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	u32 i;
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	for (i = 0; i < LED_X_CNT; i++)
	{
		GPIO_InitStructure.GPIO_Pin = c_u16LedInPin[i];
		GPIO_Init((GPIO_TypeDef *)c_pLedInPort[i], &GPIO_InitStructure);
	}

}
/* LED �� KEY ��Դ�������ų�ʼ�� */
static void PowerPinInit(void)
{	
	GPIO_InitTypeDef GPIO_InitStructure;
	u32 i;
	
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;

	for (i = 0; i < KEY_Y_CNT; i++)
	{
		GPIO_InitStructure.GPIO_Pin = c_u16KeyLedPowerPin[i];
#if POWER_PIN_HAS_NULL
		if (c_pKeyLedPowerPort[i] != NULL)
#endif			
		{
			GPIO_Init((GPIO_TypeDef *)c_pKeyLedPowerPort[i], &GPIO_InitStructure);			
		}
	}
	for (i = 0; i < LED_Y_CNT; i++)
	{
		GPIO_InitStructure.GPIO_Pin = c_u16KeyLedPowerPin[i];
#if POWER_PIN_HAS_NULL
		if (c_pKeyLedPowerPort[i] != NULL)
#endif			
		{
			GPIO_Init((GPIO_TypeDef *)c_pKeyLedPowerPort[i], &GPIO_InitStructure);			
		}
	}

}

/* �ر�����LED */
static void LedCloseAll(void)
{
	/* �ر�����LED���� */
	u32 i;
	for (i = 0; i < LED_X_CNT; i++)
	{
		c_pLedInPort[i]->BSRR = c_u16LedInPin[i];
	}

}
/* LED��Դ����ر� */
static void LedPowerCloseAll(void)
{
	u32 i;
	for (i = 0; i < LED_Y_CNT; i++)
	{
#if POWER_PIN_HAS_NULL
		if (c_pKeyLedPowerPort[i] != NULL)
#endif			
		{
			c_pKeyLedPowerPort[i]->BSRR = c_u16KeyLedPowerPin[i];
		}
	}
}
/* LED �� KEY ��Դ����ر� */
static void KeyPowerCloseAll(void)
{
	u32 i;
	for (i = 0; i < KEY_Y_CNT; i++)
	{
#if POWER_PIN_HAS_NULL
		if (c_pKeyLedPowerPort[i] != NULL)
#endif			
		{
			c_pKeyLedPowerPort[i]->BSRR = c_u16KeyLedPowerPin[i];
		}
	}

}
/* LED �� KEY ��Դ����ر� */
static void PowerCloseAll(void)
{
	/* �ر�����Power���� */
	KeyPowerCloseAll();
	LedPowerCloseAll();
}
/* �ָ�LED �ĵ��� */
/* powerΪY��LED ΪX ���� */
static void LedResume(StLedScan *pLenScan)
{
	u32 u32Cnt = pLenScan->u32ScanCnt % LED_Y_CNT;
	StLedSize stLedValue = pLenScan->stLedValue[u32Cnt];
	u32 i;
	LedCloseAll();
	LedPowerCloseAll();
	
	/* ������Ӧ���� */
	for (i = 0; i < LED_X_CNT; i++)
	{
		if ((stLedValue & (0x01 << i)) != 0)
		{
			c_pLedInPort[i]->BRR = c_u16LedInPin[i];
		}
	}
#if POWER_PIN_HAS_NULL
	if (c_pKeyLedPowerPort[u32Cnt] != NULL)
#endif			
	{
		c_pKeyLedPowerPort[u32Cnt]->BRR = c_u16KeyLedPowerPin[u32Cnt];
	}	
	pLenScan->u32ScanCnt = u32Cnt + 1;
}

void KeyLedInit(void)
{
	KeyXPinInit();
	LEDXPinInit();
	PowerPinInit();

	PowerCloseAll();

	memset(&s_stKeyScan, 0, sizeof(StKeyScan));
	memset(&s_stLedScan, 0, sizeof(StLedScan));
	memset(&s_stLedBlinkState, 0, sizeof(StLedSize) * LED_Y_CNT);
	
	memset(&s_stLedStateBackup, 0, sizeof(StLedSize) * LED_Y_CNT);
	memset(&s_stLedBlinkStateBackup, 0, sizeof(StLedSize) * LED_Y_CNT);

	
}


void ChangeLedState(u32 x, u32 y, bool boIsLight)
{
	StLedSize *pLedValue;
	x %= LED_X_CNT;
	y %= LED_Y_CNT;
	pLedValue = &(s_stLedScan.stLedValue[y]);

	if (boIsLight)/* turn on */
	{
		*pLedValue |= (0x01 << x);
	}
	else
	{
		*pLedValue &= (~(0x01 << x));
	}
	
}

bool GetLedState(u32 x, u32 y)
{
	StLedSize stLedValue;
	x %= LED_X_CNT;
	y %= LED_Y_CNT;
	stLedValue = s_stLedScan.stLedValue[y];

	if ((stLedValue & (0x01 << x)) != 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void ChangeLedBlinkState(u32 x, u32 y, bool boIsBlink)
{
	StLedSize *pLedValue;
	x %= LED_X_CNT;
	y %= LED_Y_CNT;
	pLedValue = s_stLedBlinkState + y;

	if (boIsBlink)/* turn on */
	{
		*pLedValue |= (0x01 << x);
	}
	else
	{
		*pLedValue &= (~(0x01 << x));
	}
}

static void FlushLedBlink(void)
{
	static u32 u32Time = 0;
	static bool boIsTurnOn = false;
	if (SysTimeDiff(u32Time, g_u32SysTickCnt) > 500)
	{
		StLedSize *pLedValue, stLedBlink;
		u32 i;
		for (i = 0; i < LED_Y_CNT; i++)
		{
			pLedValue = s_stLedScan.stLedValue + i;
			stLedBlink = s_stLedBlinkState[i];
			if (stLedBlink != 0)
			{
				if (boIsTurnOn)
				{
					*pLedValue |= stLedBlink;
				}
				else
				{
					*pLedValue &= (~stLedBlink);
				}
			}
			
		}
		u32Time = g_u32SysTickCnt;	
		boIsTurnOn = !boIsTurnOn;
	}
}

void ChangeAllLedState(bool boIsLight)
{
	u8 u8Value = 0;
	if (boIsLight)
	{
		u8Value = 0xFF;
	}
	memset(&s_stLedBlinkState, 0, sizeof(StLedSize) * LED_Y_CNT);
	memset(s_stLedScan.stLedValue, u8Value, sizeof(StLedSize) * LED_Y_CNT);
}

void BackupLedState(void)
{
	memcpy(&s_stLedBlinkStateBackup, &s_stLedBlinkState, sizeof(StLedSize) * LED_Y_CNT);
	memcpy(&s_stLedStateBackup, &s_stLedScan.stLedValue, sizeof(StLedSize) * LED_Y_CNT);
}

void RecoverLedState(void)
{
	memcpy(&s_stLedBlinkState, &s_stLedBlinkStateBackup, sizeof(StLedSize) * LED_Y_CNT);
	memcpy(&s_stLedScan.stLedValue, &s_stLedStateBackup, sizeof(StLedSize) * LED_Y_CNT);
}

														
void KeyLedFlush(void)
{
	if ((g_u32SysTickCnt & 0x01) == 0x00) /* even */	
	{
		u8 u8KeyCnt = 0;

		LedCloseAll();

		KeyScanOnce(&s_stKeyScan);
		u8KeyCnt = KeyGetValid(&s_stKeyScan);
		if (u8KeyCnt > 0)
		{
			StKeyMixIn stKey;

			stKey.u32Cnt = u8KeyCnt;
			stKey.emKeyType = _Key_Board;
			memcpy(stKey.unKeyMixIn.stKeyState, s_stKeyScan.stKeyState, 
				sizeof(StKeyState) * u8KeyCnt);
			KeyBufWrite(&stKey);
		}
	}
	FlushLedBlink();
	LedResume(&s_stLedScan);
}





