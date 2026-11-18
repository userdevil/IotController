#include "spi.h"
#include "global.h"
#include "r_cg_serial.h"
#include "delay.h"

UINT8 ucSPISendFlag;
UINT8 ucSPIRecFlag;

#define SPI_CLK 	P7.0
#define SPI_SI 		P7.1
#define SPI_SO 		P7.2


unsigned char SPISendRecChar(unsigned char ucCharSend, unsigned char* ucCharRec)
{
	unsigned char iCntr=0,ucMaskData=0x80,ucRet;
	
	SPI_CLK=0;
	delay10Us(1);
	ucRet=0;
	do  
	{
	        SPI_SO = 0;                //Clock out current bit onto SPI Out line
	        if (ucCharSend & ucMaskData) 
			SPI_SO = 1;
		
	        SPI_CLK = 1;                //Set SPI Clock line
		
	        if (SPI_SI) 
			ucRet |= ucMaskData;    //Read current bit fromSPI In line
	        delay10Us(1);             //Ensure minimum delay of 500nS between SPI Clock high and SPI Clock Low
	        SPI_CLK = 0;                //Set SPI Clock line
	        ucMaskData = ucMaskData >> 1;           //Shift mask so that next bit is written and read from SPI lines
	        delay10Us(1);                    //Ensure minimum delay of 1000ns between bits
    	} while (ucMaskData != 0);
	delay10Us(1);
	*ucCharRec=ucRet;

	return SUCCESS;
}

void flashInit(void)
{
	//R_CSI21_Start();
	FLASH_CE_PIN_STATUS=HIGH;
	FLASH_WP_PIN_STATUS=HIGH;
	FLASH_HOLD_PIN_STATUS=HIGH;
	SPI_CLK=0;
	SPI_SO = 0;
}

unsigned char SPITransmit(unsigned char ucData)
{
	unsigned char ucTemp;
	ucSPISendFlag=0;

	SPISendRecChar(ucData,&ucTemp);

	return SUCCESS;
}

unsigned char SPIReceive(unsigned char* pucData)
{
	unsigned char ucTemp=0xff,ucRecData;
	ucSPISendFlag=0;

	SPISendRecChar(ucTemp,&ucRecData);
	
	*pucData=ucRecData;
	return SUCCESS;
}