#include "flash.h"
#include "global.h"
#include "spi.h"
#include "delay.h"
#include "uart.h"
#include "uartDebug.h"
#include "data.h"


//  UINT8 ucFlashBuffer[FLASH_BUFFER_SIZE];

UINT8 flashReadID(UINT8 ucIDAdd,UINT8* ucID)
{
	UINT8 ucReturnValue;
	CE_LOW;
	SPITransmit(0x90);
	SPITransmit(0x00);
	SPITransmit(0x00);
	SPITransmit(ucIDAdd);
	SPIReceive(&ucReturnValue);
	*ucID=ucReturnValue;	
	CE_HIGH;
	return SUCCESS;
}

UINT8 readStatusRegister(UINT8* ucData)
{
	UINT8 ucReturnValue;
	CE_LOW;
	SPITransmit(0x05);
	SPIReceive(&ucReturnValue);
	*ucData =ucReturnValue;
	CE_HIGH;
	
//	UARTSendStr(UART_DEBUG,(UINT8*)"\r Status reg : ");
//	UARTSendLong(UART_DEBUG,ucReturnValue,3);
	
	return SUCCESS;
}

UINT8 enableWriteSatusRegister(void)
{
	UINT8 ucReturnValue;
	CE_LOW;
	SPITransmit(0x50);
	CE_HIGH;
	return SUCCESS;
}

UINT8 writeStatusRegister(UINT8 ucData)
{
	
	CE_LOW;
	SPITransmit(0x01);
	SPITransmit(ucData);
	CE_HIGH;
	return SUCCESS;
}


UINT8 enableWriteEnableLatch(void)
{
	CE_LOW;
	SPITransmit (0x06);
	CE_HIGH;
	return SUCCESS;
}

UINT8 disableWriteEnableLatch(void)
{
	CE_LOW;
	SPITransmit(0x04);
	CE_HIGH;
	return SUCCESS;
}

UINT8 enableSOforAII(void)
{
	CE_LOW;
	SPITransmit(0x70);
	CE_HIGH;
	return SUCCESS;
}

UINT8 disableSOforAII(void)
{
	CE_LOW;
	SPITransmit(0x80);
	CE_HIGH;
	return SUCCESS;
}

UINT8 chipErase(void)
{
	enableWriteSatusRegister();
	delay10Us(5);
	writeStatusRegister(0x00);
	delay10Us(5);
	enableWriteSatusRegister();
	delay10Us(5);
	enableWriteEnableLatch();
	delay10Us(5);
	
	CE_LOW;
	SPITransmit(0x60);
	CE_HIGH;
	delayMs(500);
	
	enableWriteSatusRegister();
	delay10Us(5);
	writeStatusRegister(0x00);
	delay10Us(5);
	enableWriteSatusRegister();
	delay10Us(5);
	enableWriteEnableLatch();
	delay10Us(5);
	
	CE_LOW;
	SPITransmit(0x60);
	CE_HIGH;
	delayMs(2000);
	
	
	return SUCCESS;
}

UINT8 readID(UINT8 ucIDAdd,UINT8* ucID)
{
	UINT8 ucReturnValue;
	CE_LOW;
	SPITransmit(0x90);
	SPITransmit(0x00);
	SPITransmit(0x00);
	SPITransmit(ucIDAdd);
	SPIReceive(&ucReturnValue);
	*ucID=ucReturnValue;	
	CE_HIGH;
	return SUCCESS;
}

UINT8 ucReadData(UINT32 uLAdd,  UINT8* pucData)
{
	UINT8 ucReturnValue;
	CE_LOW;
	SPITransmit(0x03);
	SPITransmit((UINT8)((uLAdd & 0xFFFFFF) >> 16));
	SPITransmit((UINT8)((uLAdd & 0xFFFF) >> 8));
	SPITransmit((UINT8)(uLAdd & 0xFF));
	SPIReceive(&ucReturnValue);
	*pucData=ucReturnValue;	
	CE_HIGH;
	return SUCCESS;
}

UINT8 ucWriteData(UINT32 uLAdd, UINT8 ucData)
{
	enableWriteEnableLatch();
	CE_LOW;
	SPITransmit(0x02);
	SPITransmit((UINT8)((uLAdd & 0xFFFFFF) >> 16));
	SPITransmit((UINT8)((uLAdd & 0xFFFF) >> 8));
	SPITransmit((UINT8)(uLAdd & 0xFF));
	SPITransmit(ucData);	
	CE_HIGH;
	ucWaitFillWriteBusy();
	return SUCCESS;
}

UINT8 ucWaitFillWriteBusy(void)
{
	UINT8 ucStatusR;
	readStatusRegister(&ucStatusR);
	while (ucStatusR & BUSY)
	{
		readStatusRegister(&ucStatusR);
	}
	return SUCCESS;
}

UINT8 writeDataToFlash(UINT32 ulFlashLoc,UINT8* pucBuffer)
{
	UINT32 ulCntr=0;
	UINT32 ulLocation;
	UINT8 ucData;
	if(ucDebugEnableFlag==DEBUG_ENABLE_ALL)
	UARTSendStr(UART_DEBUG,(UINT8*)"\r write to flash internal : ");
	
	enableWriteSatusRegister();
	writeStatusRegister(0x00);
	
	for(ulCntr=0;ulCntr<uiFlashStorageSize;ulCntr++)
	{
		ulLocation=(ulFlashLoc*uiFlashStorageSize)+ulCntr;
		
		/*UARTSendStr(UART_DEBUG,(UINT8*)"\r loc");
		UARTSendLong(UART_DEBUG,ulLocation,4);
		UARTSendStr(UART_DEBUG,(UINT8*)", data = ");
		UARTSendChar(UART_DEBUG,ucFlashBuffer[ulCntr]);*/
		
		ucWriteData(ulLocation,pucBuffer[ulCntr]);
		delay10Us(2);
		//delayMs(10);
	}
	
	return SUCCESS;
}

UINT8 readFlashData(UINT8* ucBuffer,UINT32 ulFlashLoc)
{
	UINT32 ulCntr=0;
	UINT8 ucTempV;
	UINT32 ulLocation;
	for(ulCntr=0;ulCntr<uiFlashStorageSize;ulCntr++)
	{
		ulLocation=(ulFlashLoc*uiFlashStorageSize)+ulCntr;
		
		
		ucReadData(ulLocation, &ucTempV);
		
		
		/*UARTSendStr(UART_DEBUG,(UINT8*)"\r loc");
		UARTSendLong(UART_DEBUG,ulLocation,4);
		UARTSendStr(UART_DEBUG,(UINT8*)", data = ");
		UARTSendChar(UART_DEBUG,ucTempV);*/
		
		*ucBuffer++=ucTempV;
		//delayMs(1);
	}
	return SUCCESS;
}

UINT8 displayFlashBuffer(UINT8* ucBuffer)
{
	UINT32 ulCntr=0;
	if(ucDebugEnableFlag==DEBUG_ENABLE_ALL)
	{
		UARTSendStr(UART_DEBUG,(UINT8*)"\r Flash buffer : ");
		UARTSendStr(UART_DEBUG,ucBuffer);
	}

	return SUCCESS;
}

UINT8 FillBuffer(UINT8 ucData,UINT8* ucBuffer,UINT16 uiSizeOfBuffer)
{
	UINT32 ulCntr=0;
	for(ulCntr=0;ulCntr<uiSizeOfBuffer;ulCntr++)
	{
		ucBuffer[ulCntr]=ucData;	
	}	
	return SUCCESS;
}



