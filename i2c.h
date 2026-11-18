#ifndef __I2C__H_
#define __I2C__H_

#include "global.h"


extern unsigned char ucI2CFlag;
unsigned char i2cSendChar(unsigned char ucDevAddress, unsigned char ucDataAddress, unsigned char ucData);
unsigned char i2cRecChar(unsigned char ucDevAddress, unsigned char ucDataAddress, unsigned char* pucData);




#endif