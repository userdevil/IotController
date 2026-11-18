#include "uartZB.h"
#include "global.h"
#include "uart.h"
#include "delay.h"

UINT8 initZB()
{
	ZBRE_PIN_STATUS = ZBRE_ENABLE;
    	ZBDE_PIN_STATUS = ZBDE_DISABLE;
	return SUCCESS;
}

UINT8 ZBSendChar(unsigned char ucData)
{
//	ZBRE_PIN_STATUS = ZBRE_DISABLE;
 //   	ZBDE_PIN_STATUS = ZBDE_ENABLE;
	UARTSendChar(UART_ZIGBEE,ucData);
//	ZBRE_PIN_STATUS = ZBRE_ENABLE;
  //  	ZBDE_PIN_STATUS = ZBDE_DISABLE;
	
	return SUCCESS;
}

UINT8 ZBSendStr(unsigned char const *ucStr)
{
	unsigned char ucTraData=0;
//	ZBRE_PIN_STATUS = ZBRE_DISABLE;
  //  	ZBDE_PIN_STATUS = ZBDE_ENABLE;
	delayMs(1);
	ucTraData=*ucStr;
	while(*ucStr)
	{
		UARTSendChar(UART_ZIGBEE,*ucStr++);
	}
//	ZBRE_PIN_STATUS = ZBRE_ENABLE;
  //  	ZBDE_PIN_STATUS = ZBDE_DISABLE;
	delayMs(1);
    	return SUCCESS;
}
UINT8 displayZBRecBufferTillCount(void)
{
	UINT16 ucCntr=0;
	
	if(uiZBRecvDataCount>ZB_REC_BUFFER_SIZE)
		uiZBRecvDataCount=0;
	
	UARTSendStr(UART_DEBUG,(UINT8*)"\rZIGBEE recBuffCount :");	
	for(ucCntr=0;ucCntr<uiZBRecvDataCount;ucCntr++)
		UARTSendChar(UART_DEBUG,ucZBRecvBuffer[ucCntr]);
		
	return SUCCESS;
}
UINT8 clearZBRecBuffer(void)
{
	UINT16 ucCntr=0;
	for(ucCntr=0;ucCntr<ZB_REC_BUFFER_SIZE;ucCntr++)
		ucZBRecvBuffer[ucCntr]=0x00;
		
	uiZBRecvDataCount=0;	
	return SUCCESS;
}
