#ifndef __SPI_H_
#define __SPI_H_

	#include "r_cg_macrodriver.h"
	#include "r_cg_cgc.h"
	#include "r_cg_port.h"

	#include "global.h"
	
	#define FLASH_CE_PIN_STATUS	P7.4
	#define FLASH_WP_PIN_STATUS	P7.3
	#define FLASH_HOLD_PIN_STATUS	P0.6
	
	#define CE_HIGH FLASH_CE_PIN_STATUS = 1
	#define CE_LOW FLASH_CE_PIN_STATUS = 0
	
	extern UINT8 ucSPISendFlag;
	extern UINT8 ucSPIRecFlag;
	
	void flashInit(void);
	unsigned char SPITransmit(unsigned char ucData);
	unsigned char SPIReceive(unsigned char* pucData);

#endif