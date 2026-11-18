#ifndef __UART_H_
#define __UART_H_

#include "r_cg_macrodriver.h"
#include "r_cg_cgc.h"
#include "r_cg_serial.h"
#include "global.h"


#define UART_GPRS 	0
#define UART_DEBUG 	1
#define UART_ZIGBEE 	2
#define UART_RS485 	3


#define UART_BAUD_2400 		1
#define UART_BAUD_4800 		2
#define UART_BAUD_9600 		3
#define UART_BAUD_19200		4
#define UART_BAUD_38400		5
#define UART_BAUD_57600		6
#define UART_BAUD_115200	7

#define UART_PARITY_NONE	0
#define UART_PARITY_ODD		1
#define UART_PARITY_EVEN	2	

#define UART_STOPBIT_ONE	0
#define UART_STOPBIT_NO		1
#define UART_STOPBIT_TWO	2

#define UART_DATABIT_EIGHT	0
#define UART_DATABIT_SEVEN	1
#define UART_DATABIT_NINE	2


#define DEBUG_REC_BUFFER_SIZE	256
extern unsigned char ucUartDebugSendFlag;
extern unsigned char ucUartDebugRecData;
extern unsigned char ucUartDebugRecDataFlag;
extern unsigned char ucDebugRecvBuffer[DEBUG_REC_BUFFER_SIZE];
extern unsigned int uiDebugRecvDataCount;

//1043
extern unsigned char ucUartDebugRecDataFlagMODBUS;
extern unsigned int uiUartDebugRecDataTimerMs;


#define RS485_REC_BUFFER_SIZE	100
extern unsigned char ucUartRS485SendFlag;
extern unsigned char ucUartRS485RecData;
extern unsigned char ucUartRS485RecDataFlag;
extern unsigned char ucRS485RecvBuffer[RS485_REC_BUFFER_SIZE];
extern unsigned int uiRS485RecvDataCount;

#define ZB_REC_BUFFER_SIZE	25
extern unsigned char ucUartZBSendFlag;
extern unsigned char ucUartZBRecData;
extern unsigned char ucUartZBRecDataFlag;
extern unsigned char ucZBRecvBuffer[ZB_REC_BUFFER_SIZE];
extern unsigned int uiZBRecvDataCount;

#define GPRS_REC_BUFFER_SIZE	2200
#define GPRS_TRA_BUFFER_SIZE	2200
extern unsigned char ucUartGPRSSendFlag;
extern unsigned char ucUartGPRSRecData;
extern unsigned char ucUartGPRSRecDataFlag;
extern unsigned char ucGPRSRecvBuffer[GPRS_REC_BUFFER_SIZE];
extern unsigned char ucGPRSTransmitBuffer[GPRS_TRA_BUFFER_SIZE];
extern unsigned int uiGPRSRecvDataCount;


UINT8 ucUartInit(UINT8 ucUARTModuleNo, UINT8 ucBaud, UINT8 ucDataBits, UINT8 ucParity, UINT8 ucStopBits);

unsigned char UARTSendChar(unsigned char ucUARTModuleNo,unsigned char ucData);
unsigned char UARTSendStr(unsigned char ucUARTModuleNo,unsigned char const *ucStr);
unsigned char UARTSendLong(unsigned char ucUARTModuleNo,unsigned long ulNumber,unsigned char ucNoOfDigits);
unsigned char UARTSendFloat(unsigned char ucUARTModuleNo,float fData,unsigned char ucNoOfDigitsReal,unsigned char ucNoOfDigitFraction);


#endif
