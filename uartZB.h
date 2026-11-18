#ifndef __ZB_H_
#define __ZB_H_


	#include "r_cg_macrodriver.h"
	#include "r_cg_cgc.h"
	#include "r_cg_port.h"
	#include "global.h"
	
	#define ZBDE_PIN_STATUS	P6.5
	#define ZBRE_PIN_STATUS	P6.6
	

	#define ZBRE_ENABLE 		0
	#define ZBRE_DISABLE 		1
	#define ZBDE_ENABLE 		1
	#define ZBDE_DISABLE 		0
	
	//init rsRS485 comm
	UINT8 initZB();

	//Send char over RS485
	//ucData -> data to be send via RS485
	UINT8 ZBSendChar(unsigned char ucData);
	
	//send string over RS485
	//ucStr -> pointer to string to be trasmitted over RS485
	UINT8 ZBSendStr(unsigned char const *ucStr);
	
	UINT8 displayZBRecBufferTillCount(void);
	UINT8 clearZBRecBuffer(void);
	
#endif	