#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f10x_conf.h"
#include "user_conf.h"
#include "user_api.h"
#include "I2C.h"
#include "lcd.h"
#include "gt811.h"

#if 0
#define GT811_I2C			I2C2
#define GT811_I2C_CLOCK		RCC_APB1Periph_I2C2			
#define GT811_I2C_SCL_PIN	GPIO_Pin_10
#define GT811_I2C_SCL_PORT	GPIOB

#define GT811_I2C_SDA_PIN	GPIO_Pin_11
#define GT811_I2C_SDA_PORT	GPIOB

#define GT811_DMA_TX		DMA1_Channel4
#define GT811_DMA_RX		DMA1_Channel5


#define GT811_DMA_TX_IRQHandler		DMA1_Channel4_IRQHandler
#define GT811_DMA_RX_IRQHandler		DMA1_Channel5_IRQHandler

#define GT811_DMA_TX_IRQ	DMA1_Channel4_IRQn
#define GT811_DMA_RX_IRQ	DMA1_Channel5_IRQn

#define GT811_DMA_TX_TC		DMA1_IT_TC4
#define GT811_DMA_TX_GL		DMA1_FLAG_GL4

#define GT811_DMA_RX_TC		DMA1_IT_TC5
#define GT811_DMA_RX_GL		DMA1_FLAG_GL5

#define I2C_TIMEOUT				(0x100000)


#endif

#define GT811_CTRL_PIN				GPIO_Pin_2
#define GT811_CTRL_PIN_PORT			GPIOB

#define GT811_INT_PIN				GPIO_Pin_10
#define GT811_INT_PIN_PORT			GPIOF

#define GT811_EXIT_LINE_CONFIG()	GPIO_EXTILineConfig(GPIO_PortSourceGPIOF, GPIO_PinSource10)
#define GT811_EXIT_LINE				EXTI_Line10
#define GT811_EXIT_IRQ				EXTI15_10_IRQn
#define GT811_EXIT_IRQHandler		EXTI15_10_IRQHandler

#define GT811_POINT_BUF_SIZE	(34)

#define GT811_CMD_WR		0XBA
#define GT811_CMD_RD		0XBB
#define GT811_MAX_TOUCH    	5		//电容触摸屏最大支持的点数
#define GT811_READ_XY_REG	0x721  	//读取坐标寄存器 
#define GT811_CONFIG_REG	0x6A2	//配置参数起始寄存器

const u8 c_GT811ConfigData[]=
{
    0x12,0x10,0x0E,0x0C,0x0A,0x08,0x06,0x04,0x02,0x00,0x05,0x55,0x15,0x55,0x25,0x55,
    0x35,0x55,0x45,0x55,0x55,0x55,0x65,0x55,0x75,0x55,0x85,0x55,0x95,0x55,0xA5,0x55,
    0xB5,0x55,0xC5,0x55,0xD5,0x55,0xE5,0x55,0xF5,0x55,0x1B,0x03,0x00,0x00,0x00,0x13,
    0x13,0x13,0x0F,0x0F,0x0A,0x50,0x30,0x05,0x03,0x64,0x05,0xe0,0x01,0x20,0x03,0x00,
    0x00,0x32,0x2C,0x34,0x2E,0x00,0x00,0x04,0x14,0x22,0x04,0x00,0x00,0x00,0x00,0x00,
    0x20,0x14,0xEC,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x30,
    0x25,0x28,0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x01, 
};	 



u8 s_u8Buf[GT811_POINT_BUF_SIZE];

#if 0
#define I2C_SLAVE_ADDRESS7	0xA0 
static DMA_InitTypeDef s_DMA_InitStructure;

#if 0
static bool s_boHasNewData = false;
static bool s_boDMACanUse = true;
static bool s_boHasInt = false;
#endif


void I2CDMAConfig(uint32_t pBuffer, uint32_t BufferSize, bool boIsSend)
{ 
	/* Initialize the DMA with the new parameters */
	if (boIsSend)
	{
		/* Configure the DMA Tx Channel with the buffer address and the buffer size */
		s_DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pBuffer;
		s_DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;    
		s_DMA_InitStructure.DMA_BufferSize = (uint32_t)BufferSize;  
		DMA_Init(GT811_DMA_TX, &s_DMA_InitStructure);  
	}
	else
	{ 
		/* Configure the DMA Rx Channel with the buffer address and the buffer size */
		s_DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)pBuffer;
		s_DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
		s_DMA_InitStructure.DMA_BufferSize = (uint32_t)BufferSize;      
		DMA_Init(GT811_DMA_RX, &s_DMA_InitStructure);    
	}
}


void GT811I2CInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;  
	I2C_InitTypeDef  I2C_InitStructure;

	/*!< sEE_I2C Periph clock enable */
	RCC_APB1PeriphClockCmd(GT811_I2C_CLOCK, ENABLE);
	  
	/*!< GPIO configuration */  
	/*!< Configure GT811_I2C pins: SCL */
	GPIO_InitStructure.GPIO_Pin = GT811_I2C_SCL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(GT811_I2C_SCL_PORT, &GPIO_InitStructure);

	/*!< Configure GT811_I2C pins: SDA */
	GPIO_InitStructure.GPIO_Pin = GT811_I2C_SDA_PIN;
	GPIO_Init(GT811_I2C_SDA_PORT, &GPIO_InitStructure); 

	/* Configure and enable GT811_I2C DMA TX Channel interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = GT811_DMA_TX_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* Configure and enable GT811_I2C DMA RX Channel interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = GT811_DMA_RX_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);  

	/*!< I2C DMA TX and RX channels configuration */
	/* Enable the DMA clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	s_DMA_InitStructure.DMA_PeripheralBaseAddr = ((uint32_t)&(GT811_I2C->DR));
	s_DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)0;   /* This parameter will be configured durig communication */
	s_DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;    /* This parameter will be configured durig communication */
	s_DMA_InitStructure.DMA_BufferSize = 0xFFFF;            /* This parameter will be configured durig communication */
	s_DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	s_DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	s_DMA_InitStructure.DMA_PeripheralDataSize = DMA_MemoryDataSize_Byte;
	s_DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	s_DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	s_DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
	s_DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	/* I2C TX DMA Channel configuration */
	DMA_DeInit(GT811_DMA_TX);
	DMA_Init(GT811_DMA_TX, &s_DMA_InitStructure);  

	/* I2C RX DMA Channel configuration */
	DMA_DeInit(GT811_DMA_RX);
	DMA_Init(GT811_DMA_RX, &s_DMA_InitStructure);  

	/* Enable the DMA Channels Interrupts
	DMA_ITConfig(GT811_DMA_TX, DMA_IT_TC, ENABLE); */
	DMA_ITConfig(GT811_DMA_RX, DMA_IT_TC, ENABLE); 


	/*!< I2C configuration */
	/* sEE_I2C configuration */
	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = I2C_SLAVE_ADDRESS7;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 40000;

	/* sEE_I2C Peripheral Enable */
	I2C_Cmd(GT811_I2C, ENABLE);
	/* Apply sEE_I2C configuration after enabling it */
	I2C_Init(GT811_I2C, &I2C_InitStructure);

	/* Enable the sEE_I2C peripheral DMA requests */
	I2C_DMACmd(GT811_I2C, ENABLE);
}


bool GT811WriteReg(u16 u16Reg, u8 *pBuf, u8 u8Len)
{
	u32 u32TimeOut = I2C_TIMEOUT;
	u8 i;
	bool boIsGood = false;
	while(I2C_GetFlagStatus(GT811_I2C, I2C_FLAG_BUSY))
	{
		if((u32TimeOut--) == 0)
		{
			return false;
		}			
	}

	/*!< Send START condition */
	I2C_GenerateSTART(GT811_I2C, ENABLE);

	/*!< Test on EV5 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}

	/*!< Send address for write */
	u32TimeOut = I2C_TIMEOUT;
	I2C_Send7bitAddress(GT811_I2C, GT811_CMD_WR, I2C_Direction_Transmitter);

	/*!< Test on EV6 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}

	/*!< Send internal address to write to : MSB of the address first */
	I2C_SendData(GT811_I2C, (uint8_t)((u16Reg & 0xFF00) >> 8));

	/*!< Test on EV8 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}  
	/*!< Send internal address to write to : MSB of the address first */
	I2C_SendData(GT811_I2C, (uint8_t)(u16Reg & 0xFF));

	/*!< Test on EV8 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}  
	
	for (i = 0; i < u8Len; i++)
	{
		/*!< Send internal address to write to : MSB of the address first */
		I2C_SendData(GT811_I2C, pBuf[i]);

		/*!< Test on EV8 and clear it */
		u32TimeOut = I2C_TIMEOUT;
		while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
		{
			if((u32TimeOut--) == 0)
			{
				goto end;
			}			
		}  
	}
	boIsGood = true;
end:
	/*!< Send STOP condition */
	I2C_GenerateSTOP(GT811_I2C, ENABLE);

	/* Perform a read on SR1 and SR2 register to clear eventualaly pending flags */
	//(void)GT811_I2C->SR1;
	//(void)GT811_I2C->SR2;
	return boIsGood;
}


void GT811ReadRegOneByte(u16 u16Reg, u8 *pBuf)
{
	u32 u32TimeOut = I2C_TIMEOUT;

	while(I2C_GetFlagStatus(GT811_I2C, I2C_FLAG_BUSY))
	{
		if((u32TimeOut--) == 0)
		{
			return;
		}			
	}

	/*!< Send START condition */
	I2C_GenerateSTART(GT811_I2C, ENABLE);

	/*!< Test on EV5 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}

	/*!< Send address for write */
	u32TimeOut = I2C_TIMEOUT;
	I2C_Send7bitAddress(GT811_I2C, GT811_CMD_WR, I2C_Direction_Transmitter);

	/*!< Test on EV6 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}

	/*!< Send internal address to write to : MSB of the address first */
	I2C_SendData(GT811_I2C, (uint8_t)((u16Reg & 0xFF00) >> 8));

	/*!< Test on EV8 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}  
	/*!< Send internal address to write to : MSB of the address first */
	I2C_SendData(GT811_I2C, (uint8_t)(u16Reg & 0xFF));

	/*!< Test on EV8 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}  

	
	/*!< Send START condition */
	I2C_GenerateSTART(GT811_I2C, ENABLE);

	/*!< Test on EV5 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}

	
	/*!< Send address for read */
	u32TimeOut = I2C_TIMEOUT;
	I2C_Send7bitAddress(GT811_I2C, GT811_CMD_RD, I2C_Direction_Receiver);

	/* Wait on ADDR flag to be set (ADDR is still not cleared at this level */
	u32TimeOut = I2C_TIMEOUT;
	while(I2C_GetFlagStatus(GT811_I2C, I2C_FLAG_ADDR) == RESET)
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}     

	/*!< Disable Acknowledgement */
	I2C_AcknowledgeConfig(GT811_I2C, DISABLE);   

	/* Clear ADDR register by reading SR1 then SR2 register (SR1 has already been read) */
	(void)GT811_I2C->SR2;

	/*!< Send STOP Condition */
	I2C_GenerateSTOP(GT811_I2C, ENABLE);

	/* Wait for the byte to be received */
	u32TimeOut = I2C_TIMEOUT;
	while(I2C_GetFlagStatus(GT811_I2C, I2C_FLAG_RXNE) == RESET)
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}

	/*!< Read the byte received from the device */
	*pBuf = I2C_ReceiveData(GT811_I2C);

	/* Wait to make sure that STOP control bit has been cleared */
	u32TimeOut = I2C_TIMEOUT;
	while(GT811_I2C->CR1 & I2C_CR1_STOP)
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}  

	/*!< Re-Enable Acknowledgement to be ready for another reception */
	I2C_AcknowledgeConfig(GT811_I2C, ENABLE);    
	return;
end:
	/*!< Send STOP condition */
	I2C_GenerateSTOP(GT811_I2C, ENABLE);
}

void GT811ReadReg(u16 u16Reg, u8 *pBuf, u8 u8Len)
{
	u32 u32TimeOut = I2C_TIMEOUT;
	
	if (u8Len < 2)
	{
		GT811ReadRegOneByte(u16Reg, pBuf);
		return;
	}
	
	while(I2C_GetFlagStatus(GT811_I2C, I2C_FLAG_BUSY))
	{
		if((u32TimeOut--) == 0)
		{
			return;
		}			
	}

	/*!< Send START condition */
	I2C_GenerateSTART(GT811_I2C, ENABLE);

	/*!< Test on EV5 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}

	/*!< Send address for write */
	u32TimeOut = I2C_TIMEOUT;
	I2C_Send7bitAddress(GT811_I2C, GT811_CMD_WR, I2C_Direction_Transmitter);

	/*!< Test on EV6 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}

	/*!< Send internal address to write to : MSB of the address first */
	I2C_SendData(GT811_I2C, (uint8_t)((u16Reg & 0xFF00) >> 8));

	/*!< Test on EV8 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}  
	/*!< Send internal address to write to : MSB of the address first */
	I2C_SendData(GT811_I2C, (uint8_t)(u16Reg & 0xFF));

	/*!< Test on EV8 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}  

	
	/*!< Send START condition */
	I2C_GenerateSTART(GT811_I2C, ENABLE);

	/*!< Test on EV5 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}

	
	/*!< Send address for read */
	I2C_Send7bitAddress(GT811_I2C, GT811_CMD_RD, I2C_Direction_Receiver);

	/*!< Test on EV6 and clear it */
	u32TimeOut = I2C_TIMEOUT;
	while(!I2C_CheckEvent(GT811_I2C, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
	{
		if((u32TimeOut--) == 0)
		{
			goto end;
		}			
	}  

	/* Configure the DMA Rx Channel with the buffer address and the buffer size */
	I2CDMAConfig((uint32_t)pBuf, u8Len, false);

	/* Inform the DMA that the next End Of Transfer Signal will be the last one */
	I2C_DMALastTransferCmd(GT811_I2C, ENABLE); 

	/* Enable the DMA Rx Channel */
	DMA_Cmd(GT811_DMA_RX, ENABLE);  
	return;
end:
	/*!< Send STOP condition */
	I2C_GenerateSTOP(GT811_I2C, ENABLE);
}



/**
  * @brief  This function handles the DMA Tx Channel interrupt Handler.
  * @param  None
  * @retval None
  */
void GT811_DMA_TX_IRQHandler(void)
{
	u32 u32TimeOut;
	/* Check if the DMA transfer is complete */ 
	if(DMA_GetFlagStatus(GT811_DMA_TX_TC) != RESET)
	{  
		/* Disable the DMA Tx Channel and Clear all its Flags */  
		DMA_Cmd(GT811_DMA_TX, DISABLE);
		DMA_ClearFlag(GT811_DMA_TX_GL);

		/*!< Wait till all data have been physically transferred on the bus */
		u32TimeOut = I2C_TIMEOUT;
		while(!I2C_GetFlagStatus(GT811_I2C, I2C_FLAG_BTF))
		{
			if((u32TimeOut--) == 0)
			{
				break;
			}			
		}

		/*!< Send STOP condition */
		I2C_GenerateSTOP(GT811_I2C, ENABLE);

		/* Perform a read on SR1 and SR2 register to clear eventualaly pending flags */
		(void)GT811_I2C->SR1;
		(void)GT811_I2C->SR2;

	}
}

/**
  * @brief  This function handles the DMA Rx Channel interrupt Handler.
  * @param  None
  * @retval None
  */
void GT811_DMA_RX_IRQHandler(void)
{
	/* Check if the DMA transfer is complete */
	if(DMA_GetFlagStatus(GT811_DMA_RX_TC) != RESET)
	{      
		/*!< Send STOP Condition */
		I2C_GenerateSTOP(GT811_I2C, ENABLE);    

		/* Disable the DMA Rx Channel and Clear all its Flags */  
		DMA_Cmd(GT811_DMA_RX, DISABLE);
		DMA_ClearFlag(GT811_DMA_RX_GL);
	}
}

#endif

#define GT811ReadReg(reg, buf, len)	I2CRead(0x5D, reg, 2, len, buf)
#define GT811WriteReg(reg, buf, len) I2CWriteBuffer(0x5D, reg, 2, len, buf)


//GT811唤醒
void GT811Wakeup(void)
{

	GPIO_ResetBits(GT811_CTRL_PIN_PORT, GT811_CTRL_PIN);
	Delay(10);
	GPIO_SetBits(GT811_CTRL_PIN_PORT, GT811_CTRL_PIN);
	Delay(100);
}	    

bool GT811SendConfig(u8 *pBuf, u16 u16Len)
{
	u8 u8Cnt;
	for(u8Cnt = 0; u8Cnt < 5; u8Cnt++)
	{
		if (GT811WriteReg(GT811_CONFIG_REG, pBuf, u16Len))
		{
			return true;
		}
		Delay(10);	 
	}
	return false;
}



bool GT811Init(void)
{
 	u16 u16Version = 0;
	u8 u8Tmp = 0;
	bool boIsGood = false;
	EXTI_InitTypeDef   EXTI_InitStructure;
	GPIO_InitTypeDef   GPIO_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GT811_CTRL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GT811_CTRL_PIN_PORT, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	
	/* Connect EXTI Line to the pin */
	GT811_EXIT_LINE_CONFIG();

	/* Configure EXTI0 line */
	EXTI_InitStructure.EXTI_Line = GT811_EXIT_LINE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;  
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	//GT811I2CInit();
	GT811Wakeup(); 						//唤醒GT811 
  	GT811ReadReg(0x717, &u8Tmp, 1);				//读取版本高八位
	u16Version = ((u16)u8Tmp << 8); 
	Delay(1);
	GT811ReadReg(0x718, &u8Tmp, 1);				//读取版本低八位  
	u16Version |= u8Tmp;
	if (u16Version == 0x2010)							//版本正确,发送配置参数
	{ 
		boIsGood = GT811SendConfig((u8*)c_GT811ConfigData,sizeof(c_GT811ConfigData));//发送配置参数
	} 
	else
	{
		boIsGood = false;	//版本错误
	}		
	return boIsGood;   
}

bool GT811Scan(StPoint stPoint[POINT_CNT], u8 *pCnt)
{
	u8 u8Cnt = 0, i, u8Point;
	if ((s_u8Buf[0] & 0x1F) != 0)
	{
		__disable_irq();
		u8Point = s_u8Buf[0];
		stPoint[0].u16Y = LCD_HEIGHT - (((u16)s_u8Buf[2] << 8) + s_u8Buf[3]);	//触摸点0坐标
		stPoint[0].u16X = ((u16)s_u8Buf[4] << 8) + s_u8Buf[5];	  
		stPoint[1].u16Y = LCD_HEIGHT - (((u16)s_u8Buf[7] << 8) + s_u8Buf[8]);	//触摸点1坐标
		stPoint[1].u16X = ((u16)s_u8Buf[9] << 8) + s_u8Buf[10];	  
		stPoint[2].u16Y = LCD_HEIGHT - (((u16)s_u8Buf[12] << 8) + s_u8Buf[13]);//触摸点2坐标
		stPoint[2].u16X = ((u16)s_u8Buf[14] << 8) + s_u8Buf[15];	  
		stPoint[3].u16Y = LCD_HEIGHT - (((u16)s_u8Buf[17] << 8) + s_u8Buf[24]);//触摸点3坐标
		stPoint[3].u16X = ((u16)s_u8Buf[25] << 8) + s_u8Buf[26];	  
		stPoint[4].u16Y = LCD_HEIGHT - (((u16)s_u8Buf[28] << 8) + s_u8Buf[29]);//触摸点4坐标
		stPoint[4].u16X = ((u16)s_u8Buf[30] << 8) + s_u8Buf[31];	  
		__enable_irq();
		
		for (i = 0; i < POINT_CNT; i++)
		{
			if (((u8Point >> i) & 0x01) != 0)
			{
				stPoint[u8Cnt++] = stPoint[i];
			}
		}
		*pCnt = u8Cnt;
		return true;
	}
	return false;
}

void GT811_EXIT_IRQHandler(void)
{
	if(EXTI_GetITStatus(GT811_EXIT_LINE) != RESET)
	{
		GT811ReadReg(GT811_READ_XY_REG, s_u8Buf, GT811_POINT_BUF_SIZE);	
		/* Clear the  EXTI line 0 pending bit */
		EXTI_ClearITPendingBit(GT811_EXIT_LINE);
	}
} 



