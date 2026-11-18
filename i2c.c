#include "i2c.h"
#include "delay.h"
#include "r_cg_serial.h"

unsigned char ucI2CFlag;

unsigned char i2cSendChar(unsigned char ucDevAddress, unsigned char ucDataAddress, unsigned char ucData)
{
	unsigned char ucDataAdd=ucDataAddress;
	unsigned char ucDataValue=ucData;
	unsigned char ucTraBuf[3];
	
	ucTraBuf[0]=ucDataAddress;
	ucTraBuf[1]=ucData;
	
	ucI2CFlag=0;	
	R_IICA0_Master_Send(ucDevAddress,ucTraBuf,2,0);		
	while(ucI2CFlag==0);
	delayMs(10);
	
	
	return 0;
}
unsigned char i2cRecChar(unsigned char ucDevAddress, unsigned char ucDataAddress, unsigned char* pucData)
{
	unsigned char ucDataAdd=ucDataAddress;
	ucI2CFlag=0;	
	R_IICA0_Master_Send(ucDevAddress,&ucDataAddress,1,0);		
	while(ucI2CFlag==0);
	delayMs(10);

	ucI2CFlag=0;
	R_IICA0_Master_Receive(ucDevAddress,pucData,1,0);
	while(ucI2CFlag==0);
	delayMs(10);
	
	return 0;	
}

