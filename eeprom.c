#include "eeprom.h"
#include "i2c.h"
#include "global.h"
#include "generalfunctions.h"
#include "uart.h"
#include "r_cg_wdt.h"


void readFromEEPROM()
{
	//First check if the board has been configured. If not, load default values
	unsigned char ucChar, i;
	

}

UINT8 writeExtEEPROMFloat(UINT8 ucDeviceAddress,UINT16 uiLocAddress,float fData)
{
	UINT8 pucData[4],pucAddress[2],ucStatus;
	UINT8 ucCntr = 0;
	UINT16 uiAddress = 0;

	floatToArrayInHex(fData,4, pucData);					//Convert Float to Hex
	uiAddress = uiLocAddress;
	for(ucCntr = 0; ucCntr < 4 ; ucCntr++ )
	{
		intToArrayInHex(uiAddress,2,2,pucAddress);			//Split the 16bit address to two 8bit chunks
		ucStatus=writeExtEEPROM(ucDeviceAddress,pucAddress[1],pucAddress[0],pucData[ucCntr]);
		if(ucStatus == 1)
			return FAILURE;
		uiAddress++;
	}
	return SUCCESS;
}

UINT8 readExtEEPROMFloat(UINT8 ucDeviceAddress, UINT16 uiLocAddress, float *fData)
{
	UINT8 pucData[4],pucAddress[2],ucStatus;
	UINT8 ucCntr = 0;
	UINT16 uiAddress = 0;

	uiAddress = uiLocAddress;

	for(ucCntr = 0;ucCntr < 4;ucCntr++)
	{
		intToArrayInHex(uiAddress,2,2,pucAddress);			//Split the 16bit address to two 8bit chunks
		ucStatus=readExtEEPROM(ucDeviceAddress,pucAddress[1],pucAddress[0],&pucData[ucCntr]);
		if(ucStatus == 1)
			return FAILURE;
		uiAddress++;
	}
	hexToFloat(pucData,fData);
	return SUCCESS;
}

UINT8 writeExtEEPROMLong(UINT8 ucDeviceAddress,UINT16 uiLocAddress,UINT32 ulData)
{
	UINT8 ucStat, ucData;
	UINT32 ulTemp;
	
	ulTemp = ulData;
	ucData = (UINT8)(ulTemp % 0x100);
	ucStat=writeExtEEPROMChar((UINT8) ucDeviceAddress,(UINT16) uiLocAddress, (UINT8) ucData);
	if (ucStat==FAILURE)
		return FAILURE;
	
	ulTemp = (ulData / 0x100);
	ucData = (UINT8)(ulTemp % 0x100);
	ucStat=writeExtEEPROMChar((UINT8) ucDeviceAddress, (UINT16) (uiLocAddress+1), (UINT8) ucData);
	if (ucStat==FAILURE)
		return FAILURE;
	
	ulTemp = (ulTemp / 0x100);
	ucData = (UINT8)(ulTemp % 0x100);
	ucStat=writeExtEEPROMChar((UINT8) ucDeviceAddress, (UINT16) (uiLocAddress+2), (UINT8) ucData);
	if (ucStat==FAILURE)
		return FAILURE;

	ulTemp = (ulTemp / 0x100);
	ucData = (UINT8)(ulTemp % 0x100);
	ucStat=writeExtEEPROMChar((UINT8) ucDeviceAddress, (UINT16) (uiLocAddress+3), (UINT8) ucData);
	if (ucStat==FAILURE)
		return FAILURE;

	return SUCCESS;
}


UINT8 readExtEEPROMLong(UINT8 ucDeviceAddress,UINT16 uiLocAddress,UINT32* pulData)
{
	UINT8 ucStat, ucData;
	UINT32 ulTemp;
	
	ucData=0;
	ucStat = readExtEEPROMChar( (UINT8) ucDeviceAddress, (UINT16) (uiLocAddress+3), &ucData);
	if(ucStat==FAILURE)
		return;
	ulTemp = ucData;
	
	ucStat = readExtEEPROMChar( (UINT8) ucDeviceAddress, (UINT16) (uiLocAddress+2), &ucData);
	if(ucStat==FAILURE)
		return;
	ulTemp = ulTemp * 0x100 + ucData;
	
	ucStat = readExtEEPROMChar( (UINT8) ucDeviceAddress, (UINT16) (uiLocAddress+1), &ucData);
	if(ucStat==FAILURE)
		return;
	ulTemp = ulTemp * 0x100 + ucData;
	
	ucStat = readExtEEPROMChar( (UINT8) ucDeviceAddress, (UINT16) (uiLocAddress), &ucData);
	if(ucStat==FAILURE)
		return;
	ulTemp = ulTemp * 0x100 + ucData;
	*pulData = ulTemp;
	
	return SUCCESS;
}

UINT8 writeExtEEPROMInt(UINT8 ucDeviceAddress,UINT16 uiLocAddress,UINT16 uiData)
{
	UINT8 ucStat;
	
	ucStat=writeExtEEPROMChar( (UINT8) ucDeviceAddress, (UINT16) uiLocAddress, (UINT8) (uiData % 0x100));
	if (ucStat==FAILURE)
		return FAILURE;
	writeExtEEPROMChar( (UINT8) ucDeviceAddress, (UINT16) (uiLocAddress+1), (UINT8) (uiData / 0x100));
	if (ucStat==FAILURE)
		return FAILURE;
	
	return SUCCESS;
}


UINT8 readExtEEPROMInt(UINT8 ucDeviceAddress,UINT16 uiLocAddress,UINT16* puiData)
{
	UINT8 ucStat, ucData;
	
	*puiData=0;
	ucStat = readExtEEPROMChar( (UINT8) ucDeviceAddress, (UINT16) (uiLocAddress+1), &ucData);
	if(ucStat==FAILURE)
		return;
	*puiData = ucData;
	ucStat = readExtEEPROMChar( (UINT8) ucDeviceAddress, (UINT16) (uiLocAddress), &ucData);
	if(ucStat==FAILURE)
	{
		*puiData=0;
		return;
	}
	*puiData = (*puiData) * 0x100 + ucData;
	return SUCCESS;
}


UINT8 writeExtEEPROMChar(UINT8 ucDeviceAddress,UINT16 uiLocAddress,UINT8 ucData)
{
	UINT8 pucAddress[2],ucStat;
	intToArrayInHex(uiLocAddress,2,2,pucAddress);						// Split 16-bit address into 8-bit chucks
	ucStat = writeExtEEPROM(ucDeviceAddress,pucAddress[1],pucAddress[0],ucData);
        
	if(ucStat == 1)
		return FAILURE;
	return SUCCESS;
}


UINT8 readExtEEPROMChar(UINT8 ucDeviceAddress,UINT16 uiLocAddress,UINT8* ucData)
{
	UINT8 pucAddress[2],ucStat;
	 R_WDT_Restart();
	intToArrayInHex(uiLocAddress,2,2,pucAddress);						// Split 16-bit address into 8-bit chucks
	ucStat = readExtEEPROM(ucDeviceAddress,pucAddress[1],pucAddress[0],ucData);
	if(ucStat == 1)
		return FAILURE;
	return SUCCESS;
}


UINT8 writeExtEEPROM( UINT8 ucDeviceAddress,UINT8 ucLocAddressH,UINT8 ucLocAddressL,UINT8 ucData)
{

	unsigned char ucTraBuf[3];
	 R_WDT_Restart();
	
	ucTraBuf[0]=ucLocAddressH;
	ucTraBuf[1]=ucLocAddressL;
	ucTraBuf[2]=ucData;
	
	ucI2CFlag=0;	
	R_IICA0_Master_Send(ucDeviceAddress,&ucTraBuf[0],3,0);		
	while(ucI2CFlag==0);
	
	R_IICA0_Stop_condition();
	
	delayMs(10);
	//delay10Us(50);
	
	return SUCCESS;														// Write was sucessful and device ready for another write
}



UINT8 readExtEEPROM(UINT8 ucDeviceAddress,UINT8 ucLocAddressH,UINT8 ucLocAddressL,UINT8* ucReadData)
{

	unsigned char ucStatus=FAILURE,ucTemp;
	unsigned char ucLowNibble,ucHighNibble;
	unsigned char ucTempData;


	unsigned char ucDataAdd[2];//=ucRTCREGAddress;
	ucDataAdd[0]=ucLocAddressH;
	ucDataAdd[1]=ucLocAddressL;
	
	ucI2CFlag=0;	
	R_IICA0_Master_Send(ucDeviceAddress,&ucDataAdd[0],2,0);		
	while(ucI2CFlag==0);

	ucI2CFlag=0;
	R_IICA0_Master_Receive(ucDeviceAddress,&ucTempData,1,0);
	while(ucI2CFlag==0);
	R_IICA0_Stop_condition();
	
	//delayMs(1);
	delay10Us(10);
	
	*ucReadData = ucTempData;

	return 0;

}

