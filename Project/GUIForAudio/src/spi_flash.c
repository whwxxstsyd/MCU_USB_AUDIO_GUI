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

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;		//设置SPI的数据大小:SPI发送接收8位帧结构
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	//串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		//NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;		//定义波特率预分频的值:波特率预分频值为256
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(FLASH_SPI, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

	SPI_Cmd(FLASH_SPI, ENABLE); //使能SPI外设

	SPIFlashReadWriteByte(0xFF);//启动传输
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
	while(SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET)  //检查指定的SPI标志位设置与否:发送缓存空标志位
	{
		u8RetryCnt++;
		if(u8RetryCnt > 200)
			return 0;
	}
	SPI_I2S_SendData(FLASH_SPI, u8TxData); //通过外设SPIx发送一个数据

	u8RetryCnt = 0;
	while(SPI_I2S_GetFlagStatus(FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET)  //检查指定的SPI标志位设置与否:接受缓存非空标志位
	{
		u8RetryCnt++;
		if(u8RetryCnt > 200)
			return 0;
	}
	return SPI_I2S_ReceiveData(FLASH_SPI); //返回通过SPIx最近接收的数据
}

void SPIFlashInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;


	GPIO_InitStructure.GPIO_Pin = SPI_CS_PIN;  			// PB12 推挽
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SPI_CS_PORT, &GPIO_InitStructure);

	SPI_FLASH_CS_DISABLE();


	SPIInit();		   					//初始化SPI
	g_u16FlashID = SPIFlashReadID();	//读取FLASH ID.
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



//读取SPI_FLASH的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
uint8_t SPIFlashReadSR(void)
{
	uint8_t byte = 0;
	SPI_FLASH_CS_ENABLE();                            
	SPIFlashReadWriteByte(W25X_ReadStatusReg);    //发送读取状态寄存器命令
	byte = SPIFlashReadWriteByte(0Xff);           //读取一个字节
	SPI_FLASH_CS_DISABLE();                            
	return byte;
}
//写SPI_FLASH状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void SPIFlashWriteSR(uint8_t u8SR)
{
	SPI_FLASH_CS_ENABLE();                            
	SPIFlashReadWriteByte(W25X_WriteStatusReg);   //发送写取状态寄存器命令
	SPIFlashReadWriteByte(u8SR);               //写入一个字节
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
	SPIFlashReadWriteByte(0x90);//发送读取ID命令
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
	SPIFlashReadWriteByte(W25X_ReadData);         //发送读取命令
	SPIFlashReadWriteByte((uint8_t)((u32ReadAddr) >> 16)); 
	SPIFlashReadWriteByte((uint8_t)((u32ReadAddr) >> 8));
	SPIFlashReadWriteByte((uint8_t)u32ReadAddr);
	for(i = 0; i < u32NumByteToRead; i++)
	{
		pBuffer[i] = SPIFlashReadWriteByte(0XFF); //循环读数
	}
	SPI_FLASH_CS_DISABLE(); 

	return 0;
}

//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!
int32_t SPIFlashWritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint16_t i;
	if (SPIFlashIsBusy())
	{
		return -1;
	}

	SPIFlashWriteEnable();                  			
	SPI_FLASH_CS_ENABLE();								
	SPIFlashReadWriteByte(W25X_PageProgram);      		//发送写页命令
	SPIFlashReadWriteByte((uint8_t)((WriteAddr) >> 16)); 
	SPIFlashReadWriteByte((uint8_t)((WriteAddr) >> 8));
	SPIFlashReadWriteByte((uint8_t)WriteAddr);
	for(i = 0; i < NumByteToWrite; i++)
		SPIFlashReadWriteByte(pBuffer[i]);				//循环写数
		
	SPI_FLASH_CS_DISABLE();								
	
	return 0;
}

//无检验写SPI FLASH
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能
int32_t SPIFlashWriteNoCheck(uint8_t* pBuffer, uint32_t u32WriteAddr, uint32_t u32NumByteToWrite)
{
	uint32_t u32PageRemain;
	
	if (SPIFlashIsBusy())
	{
		return -1;
	}
	
	u32PageRemain = 256 - u32WriteAddr % 256; //单页剩余的字节数
	if(u32NumByteToWrite <= u32PageRemain)
		u32PageRemain = u32NumByteToWrite; //不大于256个字节
		
	while(1)
	{
		int32_t ret = SPIFlashWritePage(pBuffer, u32WriteAddr, u32PageRemain);
		if (ret != 0)
		{
			continue;
		}
		if(u32NumByteToWrite == u32PageRemain)
			break; //写入结束了
		else //NumByteToWrite>pageremain
		{
			pBuffer += u32PageRemain;
			u32WriteAddr += u32PageRemain;

			u32NumByteToWrite -= u32PageRemain;			 //减去已经写入了的字节数
			if(u32NumByteToWrite > 256)
				u32PageRemain = 256; //一次可以写入256个字节
			else 
				u32PageRemain = u32NumByteToWrite; 	 //不够256个字节了
		}
	};
	
	return 0;
}
#if 0
//写SPI FLASH
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
uint8_t SPI_FLASH_BUF[4096];
void SPIFlashWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;
	uint16_t i;

	secpos = WriteAddr / 4096; //扇区地址 0~511 for w25x16
	secoff = WriteAddr % 4096; //在扇区内的偏移
	secremain = 4096 - secoff; //扇区剩余空间大小

	if(NumByteToWrite <= secremain)secremain = NumByteToWrite; //不大于4096个字节
	while(1)
	{
		SPIFlashRead(SPI_FLASH_BUF, secpos * 4096, 4096); //读出整个扇区的内容
		for(i = 0; i < secremain; i++) //校验数据
		{
			if(SPI_FLASH_BUF[secoff + i] != 0XFF)break; //需要擦除
		}
		if(i < secremain) //需要擦除
		{
			SPIFlashEraseSector(secpos);//擦除这个扇区
			for(i = 0; i < secremain; i++)	 //复制
			{
				SPI_FLASH_BUF[i + secoff] = pBuffer[i];
			}
			SPIFlashWriteNoCheck(SPI_FLASH_BUF, secpos * 4096, 4096); //写入整个扇区

		}
		else SPIFlashWrite_NoCheck(pBuffer, WriteAddr, secremain); //写已经擦除了的,直接写入扇区剩余区间.
		if(NumByteToWrite == secremain)break; //写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff = 0; //偏移位置为0

			pBuffer += secremain; //指针偏移
			WriteAddr += secremain; //写地址偏移
			NumByteToWrite -= secremain;				//字节数递减
			if(NumByteToWrite > 4096)secremain = 4096;	//下一个扇区还是写不完
			else secremain = NumByteToWrite;			//下一个扇区可以写完了
		}
	};
}
#endif

//擦除整个芯片
//整片擦除时间:
//W25X16:25s
//W25X32:40s
//W25X64:40s
//等待时间超长...
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
	while((SPIFlashReadSR() & 0x01) == 0x01); // 等待BUSY位清空
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

