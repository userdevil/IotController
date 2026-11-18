#include "uartDebug.h"
#include "global.h"
#include "uart.h"


UINT8 ucDebugEnableFlag;
UINT8 ucDebugNetwork;
UINT8 ucDebugRS485;

UINT8 debugSendChar(unsigned char ucData)
{
	UARTSendChar(UART_DEBUG,ucData);
	return SUCCESS;
}

UINT8 debugSendStr(unsigned char const *ucStr)
{
	unsigned char ucTraData=0;
	ucTraData=*ucStr;
	while(*ucStr)
	{
		UARTSendChar(UART_DEBUG,*ucStr++);
	}
    	return SUCCESS;
}

UINT8 debugSendATCommand(unsigned char const *ucStr)
{
	unsigned char ucTraData=0;
	ucTraData=*ucStr;
	while(*ucStr)
	{
		ucTraData=*ucStr++;
		if((ucTraData==0x0d)|| (ucTraData==0x0a))
			ucTraData='_';
		UARTSendChar(UART_DEBUG,ucTraData);
	}
    	return SUCCESS;
}

UINT8 debugSendLong(unsigned long ulNumber,unsigned char ucNoOfDigits)
{
	UARTSendLong(UART_DEBUG,ulNumber,ucNoOfDigits);
	return SUCCESS;
}

unsigned char debugSendFloat(float fData,unsigned char ucNoOfDigitsReal,unsigned char ucNoOfDigitFraction)
{
	UARTSendFloat(UART_DEBUG,fData,ucNoOfDigitsReal,ucNoOfDigitFraction);
	return SUCCESS;
}

UINT8 clearDebugRecBuffer(void)
{
	UINT16 ucCntr=0;
	for(ucCntr=0;ucCntr<DEBUG_REC_BUFFER_SIZE;ucCntr++)
		ucDebugRecvBuffer[ucCntr]=0x00;
		
	uiDebugRecvDataCount=0;	
	return SUCCESS;
}

UINT8 displayDebugRecBuffer(void)
{
	UINT16 ucCntr=0;
	UARTSendStr(UART_DEBUG,(UINT8*)"\rDebug recBuff :");	
	for(ucCntr=0;ucCntr<DEBUG_REC_BUFFER_SIZE;ucCntr++)
		UARTSendChar(UART_DEBUG,ucDebugRecvBuffer[ucCntr]);
		
	return SUCCESS;
}

UINT8 displayDebugRecBufferTillCount(void)
{
	UINT16 ucCntr=0;
	
	if(uiDebugRecvDataCount>DEBUG_REC_BUFFER_SIZE)
		uiDebugRecvDataCount=0;
	
	UARTSendStr(UART_DEBUG,(UINT8*)"\rDebug recBuffCount :");	
	for(ucCntr=0;ucCntr<uiDebugRecvDataCount;ucCntr++)
		UARTSendChar(UART_DEBUG,ucDebugRecvBuffer[ucCntr]);
		
	return SUCCESS;
}