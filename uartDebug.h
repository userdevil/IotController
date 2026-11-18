#ifndef __UART_DEBUG_H
#define __UART_DEBUG_H

#include "global.h"

#define DEBUG_NIL 		0
#define DEBUG_ALL 		1
#define DEBUG_ENABLE_ALL	1
#define DEBUG_JSON 		3


extern UINT8 ucDebugEnableFlag;
extern UINT8 ucDebugNetwork;
extern UINT8 ucDebugRS485;


//send character via debug port
UINT8 debugSendChar(unsigned char ucData);

// send string via debug port
UINT8 debugSendStr(unsigned char const *ucStr);

// send number via debug port
// ulNumber -> number to be transmitted
// ucNoOfDigits -> number of digits
UINT8 debugSendLong(unsigned long ulNumber,unsigned char ucNoOfDigits);

// send floating point number via debug port, 
//fData - > floating point number to be transmitted
//ucNoOfDigitsReal -> number of digits before decimal point
//ucNoOfDigitFraction -> number of digits after decimal point
unsigned char debugSendFloat(float fData,unsigned char ucNoOfDigitsReal,unsigned char ucNoOfDigitFraction);

// cleare dubug rec buffer, 0x00 will be written
UINT8 clearDebugRecBuffer(void);

// will send the full debug receive buffer via debug
UINT8 displayDebugRecBuffer(void);

// will send the debug receive buffer(data rec on debug port, not full buffer) via debug, 
UINT8 displayDebugRecBufferTillCount(void);


UINT8 debugSendATCommand(unsigned char const *ucStr);




#endif