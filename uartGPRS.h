#ifndef __GPRS_H_
#define __GPRS_H_


	#include "r_cg_macrodriver.h"
	#include "r_cg_cgc.h"
	#include "r_cg_port.h"
	#include "global.h"
	
	#define GPRS_RESET P1.0
	#define GPRS_STATE P6.0
	#define GPRS 	1
	#define ACTIVE 	0
	
	#define CMD_SEND 0
	#define CMD_WAIT 1
	

	
	
	#define SIM800_RESPONSE_NULL		0
	#define SIM800_RESPONSE_TIMEOUT		1
	#define SIM800_RESPONSE_RESP1		2
	#define SIM800_RESPONSE_RESP2		3
	#define SIM800_RESPONSE_RESP3		4
	
	#define SIM800_RETRY_VALID_RESP_NONE	0
	#define SIM800_RETRY_VALID_RESP_1	1
	#define SIM800_RETRY_VALID_RESP_2	2
	#define SIM800_RETRY_VALID_RESP_3	3
	
	extern UINT16 uiGPRSTimer;	
	
	
	//init rsRS485 comm
	UINT8 initGPRS();

	//Send char over RS485
	//ucData -> data to be send via RS485
	UINT8 GPRSSendChar(unsigned char ucData);
	
	//send string over RS485
	//ucStr -> pointer to string to be trasmitted over RS485
	UINT8 GPRSSendStr(unsigned char const *ucStr);
	
	// cleare GPRS rec buffer, 0x00 will be written
	UINT8 clearGPRSRecBuffer(void);

	// will send the full GPRS receive buffer via debug
	UINT8 displayGPRSRecBuffer(void);

	// will send the GPRS receive buffer(data rec on debug port, not full buffer) via debug, 
	UINT8 displayGPRSRecBufferTillCount(void);
	
	
	//this function will reset the modem
	UINT8 resetModem(void);
	
	unsigned char processModemCmd(unsigned char* pucStatus, unsigned int uiTimeOut,unsigned int uiTimoutAfterResp, unsigned char* pucCommand, unsigned char* pucResp1,unsigned char* pucResp2,unsigned char* pucResp3,unsigned char ucNoOfRetry,unsigned int ucRetryOnValidResponse, unsigned int* uiRespLoc,unsigned char* pucCmdDesc,unsigned char* pucResp1Desc,unsigned char* pucResp2Desc,unsigned char* pucResp3Desc, unsigned char ucDontClearBuffer);
	
	UINT8 receiveSMS(void);
	
	
#endif	