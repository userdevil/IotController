#include "uartGPRS.h"
#include "uart.h"
#include "global.h"
#include "uartDebug.h"
#include "generalfunctions.h"
#include "delay.h"

UINT16 uiGPRSTimer;

	
UINT8 initGPRS()
{
	GPRS_RESET=HIGH;
	return SUCCESS;
}

UINT8 GPRSSendChar(unsigned char ucData)
{
	
	UARTSendChar(UART_GPRS,ucData);
	return SUCCESS;
}

UINT8 GPRSSendStr(unsigned char const *ucStr)
{
	unsigned char ucTraData=0;
	ucTraData=*ucStr;
	while(*ucStr)
	{
		UARTSendChar(UART_GPRS,*ucStr++);
	}
	return SUCCESS;
}

UINT8 clearGPRSRecBuffer(void)
{
	UINT16 ucCntr=0;
	for(ucCntr=0;ucCntr<GPRS_REC_BUFFER_SIZE;ucCntr++)
		ucGPRSRecvBuffer[ucCntr]=0x00;
		
	uiGPRSRecvDataCount=0;	
	return SUCCESS;
}

UINT8 displayGPRSRecBuffer(void)
{
	UINT16 ucCntr=0;
	
	debugSendStr((UINT8*)"\rGPRS recBuff :");	
	for(ucCntr=0;ucCntr<GPRS_REC_BUFFER_SIZE;ucCntr++)
		debugSendChar(ucGPRSRecvBuffer[ucCntr]);
		
	return SUCCESS;
}

UINT8 displayGPRSRecBufferTillCount(void)
{
	UINT16 ucCntr=0;
	
	if(uiGPRSRecvDataCount>GPRS_REC_BUFFER_SIZE)
		uiGPRSRecvDataCount=0;
	
	debugSendStr((UINT8*)"\rGPRS recBuffCount :");	
	debugSendLong(uiGPRSRecvDataCount,4);
	debugSendStr((UINT8*)", message :");	
	for(ucCntr=0;ucCntr<uiGPRSRecvDataCount;ucCntr++)
		debugSendChar(ucGPRSRecvBuffer[ucCntr]);
		
	return SUCCESS;
}






unsigned char processModemCmd(unsigned char* pucStatus, unsigned int uiTimeOut,unsigned int uiTimoutAfterResp, unsigned char* pucCommand, unsigned char* pucResp1,unsigned char* pucResp2,unsigned char* pucResp3,unsigned char ucNoOfRetry,unsigned int ucRetryOnValidResponse, unsigned int* uiRespLoc,unsigned char* pucCmdDesc,unsigned char* pucResp1Desc,unsigned char* pucResp2Desc,unsigned char* pucResp3Desc, unsigned char ucDontClearBuffer)
{
	#define STATE_SEND_CMD			0
	#define STATE_WAIT_FOR_RESP		1
	#define STATE_WAIT_AFTER_RESP		2
	#define STATE_EXIT			3	
	

	static unsigned char ucState=STATE_SEND_CMD;
	unsigned char ucRespStatus;
	static unsigned char ucResp1Length=0;
	static unsigned char ucResp2Length=0;
	static unsigned char ucResp3Length=0;
	static unsigned char ucRetryAttempt=0;
	unsigned long ulTemp;
	static unsigned char ucReturnValue=0;
	static unsigned int uiRepLocF;
	
	if(*pucStatus==0)
	{
		ucRetryAttempt=0;
		return SUCCESS;
		
	}
	switch (ucState)
	{
		case STATE_SEND_CMD:
		
			getStrLength(pucResp1,&ulTemp);
			ucResp1Length=(unsigned char)ulTemp;
			
			getStrLength(pucResp2,&ulTemp);
			ucResp2Length=(unsigned char)ulTemp;
			
			getStrLength(pucResp3,&ulTemp);
			ucResp3Length=(unsigned char)ulTemp;
			if(ucDontClearBuffer==BUFFER_CLEAR)
				clearGPRSRecBuffer();
			
			GPRSSendStr(pucCommand);
			uiGPRSTimer=0;
			ucState=STATE_WAIT_FOR_RESP;
			

			//debugSendStr(pucCmdDesc);
			if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
			{
				debugSendStr((UINT8C*)"\r\rGPRS-> cmd : ");
				debugSendATCommand(pucCommand);
			}
			
			
		break;
		
		
		case STATE_WAIT_FOR_RESP:
			
			if(uiGPRSTimer>uiTimeOut)
			{
				ucState=STATE_SEND_CMD;
				
				if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
				{
					debugSendStr(pucCmdDesc);
					debugSendStr((UINT8C*)" -> Timeout");
				}	
				if(ucRetryAttempt<ucNoOfRetry)
				{
					ucRetryAttempt++;
					
				}
				else
				{
					//*pucStatus=0;
					//ucRetryAttempt=0;
					//ucState=STATE_SEND_CMD;
					//return SIM800_RESPONSE_TIMEOUT;
					
					//
					ucState=STATE_WAIT_AFTER_RESP;
					ucReturnValue=SIM800_RESPONSE_TIMEOUT;//return SIM800_RESPONSE_RESP1;
					uiGPRSTimer=0;
					
				}
				//displayGPRSRecBufferTillCount();
			}
		
			if (ucResp1Length>0)
			{
				if(searchString(ucGPRSRecvBuffer, GPRS_REC_BUFFER_SIZE,pucResp1,ucResp1Length, &uiRepLocF) == SUCCESS) 
				{

					ucState=STATE_WAIT_AFTER_RESP;
					ucReturnValue=SIM800_RESPONSE_RESP1;//return SIM800_RESPONSE_RESP1;
					uiGPRSTimer=0;
				//	displayGPRSRecBufferTillCount();
				}
				
			}
			
			if (ucResp2Length>0)
			{
				if(searchString(ucGPRSRecvBuffer, GPRS_REC_BUFFER_SIZE,pucResp2,ucResp2Length, &uiRepLocF) == SUCCESS) 
				{

					ucState=STATE_WAIT_AFTER_RESP;
					ucReturnValue=SIM800_RESPONSE_RESP2;//return SIM800_RESPONSE_RESP2;
					uiGPRSTimer=0;
				//	displayGPRSRecBufferTillCount();
				}
				
			}
			
			if (ucResp3Length>0)
			{
				if(searchString(ucGPRSRecvBuffer, GPRS_REC_BUFFER_SIZE,pucResp3,ucResp3Length, &uiRepLocF) == SUCCESS) 
				{
					ucState=STATE_WAIT_AFTER_RESP;
					ucReturnValue=SIM800_RESPONSE_RESP3;//return SIM800_RESPONSE_RESP3;
					uiGPRSTimer=0;
				//	displayGPRSRecBufferTillCount();
				}
				
			}
			
			
			
		break;
		
		case STATE_WAIT_AFTER_RESP:
		
			if(uiGPRSTimer>uiTimoutAfterResp)
			{
				*pucStatus=0;
				*uiRespLoc=uiRepLocF;
				ucState=STATE_SEND_CMD;
			//	displayGPRSRecBufferTillCount();
			
				if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
				{
					debugSendStr(pucCmdDesc);
					debugSendStr((UINT8C*)" -> ");
					
					if(ucReturnValue==SIM800_RESPONSE_RESP1)
					{
						debugSendStr(pucResp1);
					}
					if(ucReturnValue==SIM800_RESPONSE_RESP2)
					{

						debugSendStr(pucResp2);
					}
					if(ucReturnValue==SIM800_RESPONSE_RESP3)
					{

						debugSendStr(pucResp3);
					}
				}
			
				//check to see if retry has to happen in case of valid reponse
				if( (ucRetryOnValidResponse==SIM800_RETRY_VALID_RESP_2) && (ucReturnValue==SIM800_RESPONSE_RESP2) )
				{
					if(ucRetryAttempt<ucNoOfRetry)
					{
						ucRetryAttempt++;
						return SIM800_RESPONSE_NULL;
					}
					else
					{
						*pucStatus=0;
						ucRetryAttempt=0;
						ucState=STATE_SEND_CMD;
						//ucReturnValue=STATE_WAIT_AFTER_RESP;//
						return SIM800_RESPONSE_TIMEOUT;
					}
				}
				else
				{
					ucRetryAttempt=0;
					return ucReturnValue;
				}
			}
		
		break;
		
		default:
			ucState=STATE_SEND_CMD;
		break;
		
		
		
	}
	
	return SIM800_RESPONSE_NULL;
}

UINT8 resetModem(void)
{
	static UINT8 ucSIM900ResetState=0;
	UINT16 uiLoc;
	switch(ucSIM900ResetState)
	{

		case 0:
			if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
				debugSendStr((UINT8*)"\rGRPS-> Rst mdm 0");
   			GPRS_RESET=0;
    			uiGPRSTimer=0;
			ucSIM900ResetState=1;
		break;

		case 1:
    			if(uiGPRSTimer<3000)
    				break;
			if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
				debugSendStr((UINT8*)"\rGRPS-> Rst mdm 1");
    			uiGPRSTimer=0;
  			GPRS_RESET=1;
  			ucSIM900ResetState=2;
    		break;

    		case 2:
	    		if(uiGPRSTimer<11000)
	    			break;
	    		uiGPRSTimer=0;
	  		ucSIM900ResetState=0;
			return SUCCESS;
	  	break;

    	
		default:
			ucSIM900ResetState=0;
		break;

	}
	return FAILURE;
}	