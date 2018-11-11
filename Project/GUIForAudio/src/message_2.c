/******************(C) copyright �����XXXXX���޹�˾ *************************
* All Rights Reserved
* �ļ�����message.c
* ժҪ: Э���Զ�������
* �汾��0.0.1
* ���ߣ�������
* ���ڣ�2013��01��25��
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"
#include "app_port.h"

#include "protocol.h"
#include "message_2.h"

#define MSG_RX_PIN 					GPIO_Pin_3
#define MSG_TX_PIN					GPIO_Pin_2
#define MSG_PORT					GPIOA
#define MSG_UART					USART2


#define ENABLE_MSG_UART()			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

#define MSG_UART_IRQ_CHANNEL		USART2_IRQn
#define MSG_UART_IRQ				USART2_IRQHandler

#define ENABLE_UART_DMA()			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
#define MSG_Tx_DMA		       		DMA1
#define MSG_Tx_DMA_Channel    		DMA1_Channel7
#define MSG_Tx_DMA_FLAG       		DMA1_FLAG_TC7
#define MSG_Tx_DMA_CLEAR_FLAG		DMA1_FLAG_GL7


#ifndef MAX_IO_FIFO_CNT
#define MAX_IO_FIFO_CNT 32
#endif

#ifndef LEVEL_ONE_CACHE_CNT
#define LEVEL_ONE_CACHE_CNT 384
#endif

#ifndef CYCLE_BUF_LENGTH
#define CYCLE_BUF_LENGTH		(512 * 2)
#endif


static StIOFIFOList s_stIOFIFOList[MAX_IO_FIFO_CNT];
static StIOFIFOCtrl s_stIOFIFOCtrl;

static char s_c8LevelOneCache[LEVEL_ONE_CACHE_CNT * 2];
static StLevelOneCache s_stLevelOneCache;

static char s_c8CycleBuf[CYCLE_BUF_LENGTH];
static StCycleBuf s_stCycleBuf;


static void UARTInit(void)
{

	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	ENABLE_MSG_UART();

	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = 115200;
	USART_Init(MSG_UART, &USART_InitStructure);
	USART_Cmd(MSG_UART, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = MSG_TX_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(MSG_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = MSG_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(MSG_PORT, &GPIO_InitStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = MSG_UART_IRQ_CHANNEL; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	USART_ITConfig(MSG_UART, USART_IT_RXNE, ENABLE);
	
	
	/* DMA clock enable */
	ENABLE_UART_DMA();

	/* Configure the DMA */
	MSG_Tx_DMA_Channel->CCR = 0x00003090;
	/*--------------------------- DMAy Channelx CPAR Configuration ----------------*/
	MSG_Tx_DMA_Channel->CPAR = (uint32_t)(&(MSG_UART->DR));

	/*--------------------------- DMAy Channelx CNDTR Configuration ---------------*/
	MSG_Tx_DMA_Channel->CNDTR = 0;

	/*--------------------------- DMAy Channelx CMAR Configuration ----------------*/
	MSG_Tx_DMA_Channel->CMAR = 0;

	  
	/* Enable USARTy DMA TX request */
	USART_DMACmd(MSG_UART, USART_DMAReq_Tx, ENABLE);
}

void MSG_UART_IRQ(void)
{
	u16 u16SR = MSG_UART->SR;
	if (((u16SR & USART_FLAG_RXNE) != 0) || ((u16SR & USART_FLAG_ORE) != 0))
	{
		u8 u8RxTmp = (MSG_UART->DR & (uint16_t)0x00FF);
		LOCWriteSomeData(&s_stLevelOneCache, &u8RxTmp, 1);
		//MSG_UART->DR = (u8RxTmp & (uint16_t)0x00FF);
	}
	
	if ((u16SR & USART_FLAG_TXE) != 0)
	{   
	}   

}


void MessageUart2Init(void)
{
	/* for uart send and get a protocol message */
	IOFIFOInit(&s_stIOFIFOCtrl, s_stIOFIFOList, MAX_IO_FIFO_CNT, _IO_Reserved);
	
	/* level one cache for protocol */
	LOCInit(&s_stLevelOneCache, s_c8LevelOneCache, LEVEL_ONE_CACHE_CNT * 2);
	
	/* for protocol analyse */
	CycleMsgInit(&s_stCycleBuf, s_c8CycleBuf, CYCLE_BUF_LENGTH);
	
	UARTInit();
}

StIOFIFO *MessageUart2Flush(bool boSendALL)
{
	/* for data IN */ 
	do
	{
		void *pData = NULL;
		uint32_t u32Length = 0;
		pData = LOCCheckDataCanRead(&s_stLevelOneCache, &u32Length);
		
		if (pData == NULL)
		{
			/* no new data */
			break;
		}
		
		while(1)
		{
			void *pMsg = NULL;
			uint32_t u32GetMsgLength = 0;
			int32_t s32Protocol = 0;
			StIOFIFOList *pFIFO = NULL;
			pMsg = CycleGetOneMsg(&s_stCycleBuf, pData, u32Length, &u32GetMsgLength, 
					&s32Protocol, NULL);
			if (pMsg == NULL)
			{
				break;
			}
			/* I get some message */
			pFIFO = GetAUnusedFIFO(&s_stIOFIFOCtrl);
			if (pFIFO == NULL)
			{
				/* no buffer for this message */
				free(pMsg);
				break;
			}
			pFIFO->pData = pMsg;
			pFIFO->s32Length = u32GetMsgLength;
			pFIFO->boNeedFree = true;
			pFIFO->u8ProtocolType = s32Protocol;
			InsertIntoTheRWFIFO(&s_stIOFIFOCtrl, pFIFO, true);

			u32Length = 0;
		}
	} while (0);
	
	
	/* check message for send */
	do
	{
		static bool boHasSendAMessage = false;
		static StIOFIFOList stLastFIFO;
		StIOFIFOList *pFIFO = NULL;
		if (boHasSendAMessage)
		{
			if ((MSG_Tx_DMA->ISR & MSG_Tx_DMA_FLAG) == 0)
			{
				if (boSendALL)
				{
					continue;	/* wait to finish to send this message */
				}
				else
				{
					break;
				}
			}	
			/* 
			 * In transmission mode, once the DMA has written all the data to be transmitted (the TCIF flag
			 * is set in the DMA_ISR register), the TC flag can be monitored to make sure that the USART
			 * communication is complete. This is required to avoid corrupting the last transmission before
			 * disabling the USART or entering the Stop mode. The software must wait until TC=1. The TC
			 * flag remains cleared during all data transfers and it is set by hardware at the last frame's end
			 * of transmission. 
			 */
			if ((MSG_UART->SR & USART_FLAG_TC) == 0)
			{
				if (boSendALL)
				{
					continue;	/* wait to finish to send this message */
				}
				else
				{
					break;
				}
			}
			MSG_Tx_DMA->IFCR = MSG_Tx_DMA_CLEAR_FLAG;
			
			if (stLastFIFO.boNeedFree)
			{
				free(stLastFIFO.pData);
			}
			boHasSendAMessage = false;
		}
		pFIFO = GetAListFromRWFIFO(&s_stIOFIFOCtrl, false);
		if (pFIFO != NULL)
		{
			if ((pFIFO->s32Length <= 0) || (pFIFO->pData == NULL))
			{
				if ((pFIFO->boNeedFree) && (pFIFO->pData != NULL))
				{
					free(pFIFO->pData);
				}
				if (boSendALL)
				{
					continue;	/* wait to finish to send this message */
				}
				else
				{
					break;
				}				
			}
			MSG_Tx_DMA_Channel->CCR &= ~DMA_CCR1_EN;
			
			/*--------------------------- DMAy Channelx CNDTR Configuration ---------------*/
			MSG_Tx_DMA_Channel->CNDTR = pFIFO->s32Length;

			/*--------------------------- DMAy Channelx CMAR Configuration ----------------*/
			MSG_Tx_DMA_Channel->CMAR = (uint32_t)pFIFO->pData;

			MSG_Tx_DMA_Channel->CCR |= DMA_CCR1_EN;
			boHasSendAMessage = true;
			
			stLastFIFO = *pFIFO;
			
			ReleaseAUsedFIFO(&s_stIOFIFOCtrl, pFIFO);
			if (boSendALL)
			{
				continue;	/* send this message */
			}
		}
		break;
	} while (1);
	
	/* get message for read */
	do
	{
		StIOFIFOList *pFIFO = NULL;
		pFIFO = GetAListFromRWFIFO(&s_stIOFIFOCtrl, true);
		if (pFIFO != NULL)
		{
			return (StIOFIFO *)pFIFO;
		}

	} while (0);
	
	return NULL;
}


void MessageUart2Release(StIOFIFO *pFIFO)
{
	if (pFIFO != NULL)
	{
		if (pFIFO->boNeedFree)
		{
			free(pFIFO->pData);
		}
		ReleaseAUsedFIFO(&s_stIOFIFOCtrl, (StIOFIFOList *)pFIFO);
	}
}

void MessageUart2ReleaseNoReleaseData(StIOFIFO *pFIFO)
{
	if (pFIFO != NULL)
	{
		ReleaseAUsedFIFO(&s_stIOFIFOCtrl, (StIOFIFOList *)pFIFO);
	}
}
int32_t GetMessageUart2BufLength(void)
{
	return CYCLE_BUF_LENGTH;
}


int32_t MessageUart2Write(void *pData, bool boNeedFree, uint16_t u16ID, uint32_t u32Length)
{
	StIOFIFOList *pFIFO = NULL;
	if (pData == NULL)
	{
		return -1;
	}
	pFIFO = GetAUnusedFIFO(&s_stIOFIFOCtrl);
	if (pFIFO == NULL)
	{
		/* no buffer for this message */
		return -1;
	}
	pFIFO->pData = pData;
	pFIFO->s32Length = u32Length;
	pFIFO->boNeedFree = boNeedFree;
	InsertIntoTheRWFIFO(&s_stIOFIFOCtrl, pFIFO, false);
	
	return 0;
}

/*
 * �ⲿ��Ϣ��������: ������, ֹͣλ, У�鷽ʽ��
 * ����: pConfig: USART_InitTypeDef��ָ��
 * ���: ��
 */
void MessageUART2Config(USART_InitTypeDef *pConfig)
{
	if(pConfig == NULL)
	{
		return;
	}
	pConfig->USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	pConfig->USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(MSG_UART, pConfig);
}


const StIOTCB c_stUart2IOTCB = 
{
	MessageUart2Init,
	MessageUart2Flush,
	MessageUart2Release,
	MessageUart2ReleaseNoReleaseData,
	GetMessageUart2BufLength,
	MessageUart2Write,
};

