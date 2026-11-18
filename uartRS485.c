#include "uartRS485.h"
#include "global.h"
#include "uart.h"
#include "delay.h"

UINT8 initRS485()
{
	RS485RE_PIN_STATUS = RS485RE_ENABLE;
    	RS485DE_PIN_STATUS = RS485DE_DISABLE;
	return SUCCESS;
}

UINT8 RS485SendChar(unsigned char ucData)
{
	RS485RE_PIN_STATUS = RS485RE_DISABLE;
    	RS485DE_PIN_STATUS = RS485DE_ENABLE;
	UARTSendChar(UART_RS485,ucData);
	RS485RE_PIN_STATUS = RS485RE_ENABLE;
    	RS485DE_PIN_STATUS = RS485DE_DISABLE;
	
	return SUCCESS;
}

UINT8 RS485SendStr(unsigned char const *ucStr)
{
	unsigned char ucTraData=0;
	RS485RE_PIN_STATUS = RS485RE_DISABLE;
    	RS485DE_PIN_STATUS = RS485DE_ENABLE;
	ucTraData=*ucStr;
	while(*ucStr)
	{
		UARTSendChar(UART_RS485,*ucStr++);
	}
	RS485RE_PIN_STATUS = RS485RE_ENABLE;
    	RS485DE_PIN_STATUS = RS485DE_DISABLE;
	return SUCCESS;
}