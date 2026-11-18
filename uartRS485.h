#ifndef __RS485_H_
#define __RS485_H_


	#include "r_cg_macrodriver.h"
	#include "r_cg_cgc.h"
	#include "r_cg_port.h"
	#include "global.h"
	
	#define RS485DE_PIN_STATUS	P14.5
	#define RS485RE_PIN_STATUS	P0.0
	

	#define RS485RE_ENABLE 		0
	#define RS485RE_DISABLE 	1
	#define RS485DE_ENABLE 		1
	#define RS485DE_DISABLE 	0
	
	//init rsRS485 comm
	UINT8 initRS485();

	//Send char over RS485
	//ucData -> data to be send via RS485
	UINT8 RS485SendChar(unsigned char ucData);
	
	//send string over RS485
	//ucStr -> pointer to string to be trasmitted over RS485
	UINT8 RS485SendStr(unsigned char const *ucStr);
	
	
	
#endif	