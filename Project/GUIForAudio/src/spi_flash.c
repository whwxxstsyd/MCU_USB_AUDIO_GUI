#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"

#include "spi_flash.h"

#define FLASH_SPI				SPI2
#define SPI_CS_PIN				GPIO_Pin_12
#define SPI_CLK_PIN				GPIO_Pin_13
#define SPI_MISO_PIN			GPIO_Pin_14
#define SPI_MOSI_PIN			GPIO_Pin_15

#define SPI_CS_PORT				GPIOB
#define SPI_CLK_PORT			GPIOB
#define SPI_MISO_PORT			GPIOB
#define SPI_MOSI_PORT			GPIOB

#define SPI_ENABLE()			RCC_APB1PeriphClockCmd(	RCC_APB1Periph_SPI2, ENABLE)


#define SPI_FLASH_CS_ENABLE()	SPI_CS_PORT->BRR = SPI_CS_PIN
#define SPI_FLASH_CS_DISABLE()	SPI_CS_PORT->BSRR = SPI_CS_PIN;//GPIO_SetBits(SPI_CS_PORT, SPI_CS_PIN);


#define W25Q80 	0XEF13
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17

#define W25Q128_SIZE	(8 * 1024 * 1024)


u16 g_u16FlashID = 0;

static void SPIInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	SPI_InitTypeDef  SPI_InitStructure;

	SPI_ENABLE();

	GPIO_InitStructure.GPIO_Pin = SPI_CLK_PIN | SPI_MISO_PIN | SPI_MOSI_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI_CLK_PORT, &GPIO_InitStructure);

	GPIO_SetBits(SPI_CLK_PORT, SPI_CLK_PIN | SPI_MISO_PIN | SPI_MOSI_PIN);

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//����SPI����ģʽ:����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//����ͬ��ʱ�ӵĿ���״̬Ϊ�ߵ�ƽ
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//����ͬ��ʱ�ӵĵڶ��������أ��������½������ݱ�����
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		//���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRCֵ����Ķ���ʽ
	SPI_Init(FLASH_SPI, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���

	SPI_Cmd(FLASH_SPI, ENABLE); //ʹ��SPI����

	SPIFlashReadWriteByte(0xFF);//��������
}

void SPIFlashSpeed(uint16_t u16SPIBaudRatePrescaler)
{
	assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));

	FLASH_SPI->CR1 &= 0XFFC7;
	FLASH_SPI->CR1 |= u16SPIBaudRatePrescaler;
	SPI_Cmd(FLASH_SPI, ENABLE);

}

uint8_t SPIFlashReadWriteByte(uint8_t u8TxData)
{
	uint8_t u8RetryCnt = 0;
	while(SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET)  //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
	{
		u8RetryCnt++;
		if(u8RetryCnt > 200)
			return 0;
	}
	SPI_I2S_SendData(FLASH_SPI, u8TxData); //ͨ������SPIx����һ������

	u8RetryCnt = 0;
	while(SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET)  //���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
	{
		u8RetryCnt++;
		if(u8RetryCnt > 200)
			return 0;
	}
	return SPI_I2S_ReceiveData(FLASH_SPI); //����ͨ��SPIx������յ�����
}

void SPIFlashInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;


	GPIO_InitStructure.GPIO_Pin = SPI_CS_PIN;  			// PB12 ����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  	//�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI_CS_PORT, &GPIO_InitStructure);

	SPI_FLASH_CS_DISABLE();


	SPIInit();		   					//��ʼ��SPI
	g_u16FlashID = SPIFlashReadID();	//��ȡFLASH ID.
}


#define W25X_WriteEnable		0x06
#define W25X_WriteDisable		0x04
#define W25X_ReadStatusReg		0x05
#define W25X_WriteStatusReg		0x01
#define W25X_ReadData			0x03
#define W25X_FastReadData		0x0B
#define W25X_FastReadDual		0x3B
#define W25X_PageProgram		0x02
#define W25X_BlockErase			0xD8
#define W25X_SectorErase		0x20
#define W25X_ChipErase			0xC7
#define W25X_PowerDown			0xB9
#define W25X_ReleasePowerDown	0xAB
#define W25X_DeviceID			0xAB
#define W25X_ManufactDeviceID	0x90
#define W25X_JedecDeviceID		0x9F



//��ȡSPI_FLASH��״̬�Ĵ���
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
uint8_t SPIFlashReadSR(void)
{
	uint8_t byte = 0;
	SPI_FLASH_CS_ENABLE();                            
	SPIFlashReadWriteByte(W25X_ReadStatusReg);    //���Ͷ�ȡ״̬�Ĵ�������
	byte = SPIFlashReadWriteByte(0Xff);           //��ȡһ���ֽ�
	SPI_FLASH_CS_DISABLE();                            
	return byte;
}
//дSPI_FLASH״̬�Ĵ���
//ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
void SPIFlashWriteSR(uint8_t u8SR)
{
	SPI_FLASH_CS_ENABLE();                            
	SPIFlashReadWriteByte(W25X_WriteStatusReg);   //����дȡ״̬�Ĵ�������
	SPIFlashReadWriteByte(u8SR);               //д��һ���ֽ�
	SPI_FLASH_CS_DISABLE();                            
}

void SPIFlashWriteEnable(void)
{
	SPI_FLASH_CS_ENABLE();                            
	SPIFlashReadWriteByte(W25X_WriteEnable);
	SPI_FLASH_CS_DISABLE();                            
}

void SPIFlashWriteDisable(void)
{
	SPI_FLASH_CS_ENABLE();                            
	SPIFlashReadWriteByte(W25X_WriteDisable);
	SPI_FLASH_CS_DISABLE();                            
}

uint16_t SPIFlashReadID(void)
{
	uint16_t u16Data = 0;
	SPI_FLASH_CS_ENABLE();
	SPIFlashReadWriteByte(0x90);//���Ͷ�ȡID����
	SPIFlashReadWriteByte(0x00);
	SPIFlashReadWriteByte(0x00);
	SPIFlashReadWriteByte(0x00);
	u16Data |= SPIFlashReadWriteByte(0xFF) << 8;
	u16Data |= SPIFlashReadWriteByte(0xFF);
	SPI_FLASH_CS_DISABLE();
	return u16Data;
}

int32_t SPIFlashRead(uint8_t *pBuffer, uint32_t u32ReadAddr, uint32_t u32NumByteToRead)
{
	uint32_t i;
	
	if (SPIFlashIsBusy())
	{
		return -1;
	}

	SPI_FLASH_CS_ENABLE();                            
	SPIFlashReadWriteByte(W25X_ReadData);         //���Ͷ�ȡ����
	SPIFlashReadWriteByte((uint8_t)((u32ReadAddr) >> 16)); 
	SPIFlashReadWriteByte((uint8_t)((u32ReadAddr) >> 8));
	SPIFlashReadWriteByte((uint8_t)u32ReadAddr);
	for(i = 0; i < u32NumByteToRead; i++)
	{
		pBuffer[i] = SPIFlashReadWriteByte(0XFF); //ѭ������
	}
	SPI_FLASH_CS_DISABLE(); 

	return 0;
}

//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!
int32_t SPIFlashWritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint16_t i;
	if (SPIFlashIsBusy())
	{
		return -1;
	}

	SPIFlashWriteEnable();                  			
	SPI_FLASH_CS_ENABLE();								
	SPIFlashReadWriteByte(W25X_PageProgram);      		//����дҳ����
	SPIFlashReadWriteByte((uint8_t)((WriteAddr) >> 16)); 
	SPIFlashReadWriteByte((uint8_t)((WriteAddr) >> 8));
	SPIFlashReadWriteByte((uint8_t)WriteAddr);
	for(i = 0; i < NumByteToWrite; i++)
		SPIFlashReadWriteByte(pBuffer[i]);				//ѭ��д��
		
	SPI_FLASH_CS_DISABLE();								
	
	return 0;
}

//�޼���дSPI FLASH
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ����
int32_t SPIFlashWriteNoCheck(uint8_t* pBuffer, uint32_t u32WriteAddr, uint32_t u32NumByteToWrite)
{
	uint32_t u32PageRemain;
	
	if (SPIFlashIsBusy())
	{
		return -1;
	}
	
	u32PageRemain = 256 - u32WriteAddr % 256; //��ҳʣ����ֽ���
	if(u32NumByteToWrite <= u32PageRemain)
		u32PageRemain = u32NumByteToWrite; //������256���ֽ�
		
	while(1)
	{
		int32_t ret = SPIFlashWritePage(pBuffer, u32WriteAddr, u32PageRemain);
		if (ret != 0)
		{
			continue;
		}
		if(u32NumByteToWrite == u32PageRemain)
			break; //д�������
		else //NumByteToWrite>pageremain
		{
			pBuffer += u32PageRemain;
			u32WriteAddr += u32PageRemain;

			u32NumByteToWrite -= u32PageRemain;			 //��ȥ�Ѿ�д���˵��ֽ���
			if(u32NumByteToWrite > 256)
				u32PageRemain = 256; //һ�ο���д��256���ֽ�
			else 
				u32PageRemain = u32NumByteToWrite; 	 //����256���ֽ���
		}
	};
	
	return 0;
}
#if 0
//дSPI FLASH
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//pBuffer:���ݴ洢��
//WriteAddr:��ʼд��ĵ�ַ(24bit)
//NumByteToWrite:Ҫд����ֽ���(���65535)
uint8_t SPI_FLASH_BUF[4096];
void SPIFlashWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;
	uint16_t i;

	secpos = WriteAddr / 4096; //������ַ 0~511 for w25x16
	secoff = WriteAddr % 4096; //�������ڵ�ƫ��
	secremain = 4096 - secoff; //����ʣ��ռ��С

	if(NumByteToWrite <= secremain)secremain = NumByteToWrite; //������4096���ֽ�
	while(1)
	{
		SPIFlashRead(SPI_FLASH_BUF, secpos * 4096, 4096); //������������������
		for(i = 0; i < secremain; i++) //У������
		{
			if(SPI_FLASH_BUF[secoff + i] != 0XFF)break; //��Ҫ����
		}
		if(i < secremain) //��Ҫ����
		{
			SPIFlashEraseSector(secpos);//�����������
			for(i = 0; i < secremain; i++)	 //����
			{
				SPI_FLASH_BUF[i + secoff] = pBuffer[i];
			}
			SPIFlashWriteNoCheck(SPI_FLASH_BUF, secpos * 4096, 4096); //д����������

		}
		else SPIFlashWrite_NoCheck(pBuffer, WriteAddr, secremain); //д�Ѿ������˵�,ֱ��д������ʣ������.
		if(NumByteToWrite == secremain)break; //д�������
		else//д��δ����
		{
			secpos++;//������ַ��1
			secoff = 0; //ƫ��λ��Ϊ0

			pBuffer += secremain; //ָ��ƫ��
			WriteAddr += secremain; //д��ַƫ��
			NumByteToWrite -= secremain;				//�ֽ����ݼ�
			if(NumByteToWrite > 4096)secremain = 4096;	//��һ����������д����
			else secremain = NumByteToWrite;			//��һ����������д����
		}
	};
}
#endif

//��������оƬ
//��Ƭ����ʱ��:
//W25X16:25s
//W25X32:40s
//W25X64:40s
//�ȴ�ʱ�䳬��...
int32_t SPIFlashEraseChip(void)
{
	if (SPIFlashIsBusy())
	{
		return -1;
	}
	SPIFlashWriteEnable();
	SPIFlashWaitBusy();
	SPI_FLASH_CS_ENABLE();
	SPIFlashReadWriteByte(W25X_ChipErase);        
	SPI_FLASH_CS_DISABLE(); 
	
	return 0;
}

int32_t SPIFlashEraseSector(uint32_t u32Sector)
{
	uint32_t u32DestAddr = u32Sector * 4096;
	if (SPIFlashIsBusy())
	{
		return -1;
	}

	SPIFlashWriteEnable();                  
	SPIFlashWaitBusy();
	SPI_FLASH_CS_ENABLE();                        
	SPIFlashReadWriteByte(W25X_SectorErase);
	SPIFlashReadWriteByte((uint8_t)((u32DestAddr) >> 16)); 
	SPIFlashReadWriteByte((uint8_t)((u32DestAddr) >> 8));
	SPIFlashReadWriteByte((uint8_t)u32DestAddr);
	SPI_FLASH_CS_DISABLE();                            

	return 0;
}

void SPIFlashWaitBusy(void)
{
	while((SPIFlashReadSR() & 0x01) == 0x01); // �ȴ�BUSYλ���
}

bool SPIFlashIsBusy(void)
{
	return ((SPIFlashReadSR() & 0x01) == 0x01);
}

int32_t SPIFlashPowerDown(void)
{
	if (SPIFlashIsBusy())
	{
		return -1;
	}
	SPI_FLASH_CS_ENABLE();
	SPIFlashReadWriteByte(W25X_PowerDown);
	SPI_FLASH_CS_DISABLE();
	
	return 0;
}

void SPIFlashWakeUp(void)
{
	SPI_FLASH_CS_ENABLE();                            
	SPIFlashReadWriteByte(W25X_ReleasePowerDown);
	SPI_FLASH_CS_DISABLE();                            
}

bool boIsFlashCanUse(void)
{
	return g_u16FlashID == W25Q128;
}


int32_t SPIFlashBigDataReadWriteBegin(StFlashBigDataCtrl *pCtrl, 
	uint32_t u32StartAddr, uint32_t u32TotalSize)
{	
	if (pCtrl == NULL)
	{
		return -1;
	}
	
	if ((u32StartAddr & (~0xFFF)) != u32StartAddr)
	{
		return -1;
	}
	
	if (u32TotalSize > W25Q128_SIZE || 
		(u32StartAddr + u32TotalSize) > W25Q128_SIZE)
	{
		u32TotalSize = W25Q128_SIZE - u32StartAddr;
	}
	
	memset(pCtrl, 0, sizeof(StFlashBigDataCtrl));
	
	
	pCtrl->u32StartAddr = u32StartAddr;
	pCtrl->u32CurWRAddr = u32StartAddr;
	pCtrl->u32TotalSize = u32TotalSize;
	pCtrl->u32State = _Flash_BigData_Start;
	return 0;
}
/* return real write count */
int32_t SPIFlashBigDataWrite(StFlashBigDataCtrl *pCtrl, 
	uint8_t *pData, int32_t s32Size)
{
	if (pCtrl == NULL || pData == NULL || s32Size <= 0)
	{
		return -1;
	}
	if (pCtrl->u32State < _Flash_BigData_Start)
	{
		return -1;
	}
	if(SPIFlashIsBusy())
	{
		return -2;
	}
	
	if (((pCtrl->u32CurWRAddr) & (~0xFFF)) == pCtrl->u32CurWRAddr)
	{
		if (pCtrl->u32State == _Flash_BigData_Erasing)	/* has erased */
		{
			pCtrl->u32State = _Flash_BigData_Continues;
		}
		else
		{
			int32_t ret = SPIFlashEraseSector(pCtrl->u32CurWRAddr / 4096);
			if (ret != 0)
			{
				return ret;
			}
			else
			{
				pCtrl->u32State = _Flash_BigData_Erasing;
				return -2;
			}
		}
	}
	
	{
		int32_t s32NextEraseAddr = ((pCtrl->u32CurWRAddr + 4096) & (~0xFFF));
		int32_t s32NeedWrite = s32Size;
		int32_t ret = 0;
		if ((pCtrl->u32CurWRAddr + s32Size) > s32NextEraseAddr)
		{
			s32NeedWrite = s32NextEraseAddr - pCtrl->u32CurWRAddr;
		}
		ret = SPIFlashWriteNoCheck(pData, pCtrl->u32CurWRAddr, s32NeedWrite);
		if (ret == 0)
		{
			pCtrl->u32CurWRAddr += s32NeedWrite;
			return s32NeedWrite;
		}
		else
		{
			return ret;
		}		
	}
	
}

int32_t SPIFlashBigDataWriteNoBreak(StFlashBigDataCtrl *pCtrl, 
	uint8_t *pData, int32_t s32Size)
{
	int32_t s32Count = 0;
	uint8_t *u8Write = pData;
	
	
	while(s32Count < s32Size)
	{
		int32_t ret = SPIFlashBigDataWrite(pCtrl, u8Write + s32Count,
			s32Size - s32Count);
		if (ret == -1)
		{
			break;
		}
		else if (ret == -2)
		{
			continue;
		}
		else
		{
			s32Count += ret;
		}
	}
	
	return s32Count;
}


/* return real read count */
int32_t SPIFlashBigDataRead(StFlashBigDataCtrl *pCtrl, 
	uint8_t *pData, int32_t s32Size)
{
	int32_t ret = 0;
	if (pCtrl == NULL || pData == NULL || s32Size <= 0)
	{
		return -1;
	}
	if (pCtrl->u32State < _Flash_BigData_Start)
	{
		return -1;
	}

	while (1)
	{
		ret = SPIFlashRead(pData, pCtrl->u32CurWRAddr, s32Size);
		if (ret == 0)
		{
			pCtrl->u32State = _Flash_BigData_Continues;
			pCtrl->u32CurWRAddr += s32Size;
			return s32Size;
		}	
	}
	return ret;

}

