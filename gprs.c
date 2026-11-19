#include "gprs.h"
#include "global.h"
#include "uartGPRS.h"
#include "uart.h"
#include "delay.h"
#include "generalfunctions.h"
#include "memory.h"
#include "uartDebug.h"
#include "eeprom.h"
#include "rtc.h"
#include "controller.h"
#include "data.h"
#include "dio.h"



UINT8 ucSenderMobNumberStr[15];
UINT8 ucLocalIP[20];
UINT8 ucSignalStrength;
UINT8 ucNetworkStatus;
UINT8 ucSIMStatus;
UINT8 ucGPRSConStatus;
UINT8 ucGPRSDataTranferFlag;
UINT8 ucDataTransferSuccess;
UINT8 ucGPRSSMState;

UINT8 ucSendSMSFlag;
UINT8 ucSMSReceivedFlag;

UINT8 	ucInComingCallFlag=0;
UINT16 	uiInComingCallTimer=0;

UINT16 ucMS_WAIT_BEFORE_RESET=60000;


void checkForIncomingCall()
{
	UINT16 uiRepLocF;
	switch(ucInComingCallFlag)
	{
		case 0: //waiting for incoming call
			if(searchString(ucGPRSRecvBuffer, GPRS_REC_BUFFER_SIZE,(UINT8*)"RING",4, &uiRepLocF) == SUCCESS) 
			{
				ucInComingCallFlag=1;
				uiInComingCallTimer=5000;
			}
		break;
		
		case 1:// phone rining wait some time and disconnect the call
		{
			if(uiInComingCallTimer!=0)
				return;
			
			ucInComingCallFlag=0;
			GPRSSendStr((UINT8*)"ATH\r\n");	
			delayMs(1000);
			clearGPRSRecBuffer();
			
		}
		break;
			
		default:
			ucInComingCallFlag=0;
		break;
	}
	
	
}




void prepareGPRSTransmitPacket(UINT8* pucData, UINT8* pucTransmitBuffer, UINT8* pucServerAdd, UINT8* pucServerPort, UINT8* pucServerPath, UINT8* pucApplicationName, UINT8 ucTransferMode)
{
	UINT32 ulTempAf,ulTempBf;
	if(ucTransferMode==GPRS_TRANS_MODE_GET)
	{
		pucTransmitBuffer[0]=0x00;
		concatStirngsToStr1(pucTransmitBuffer,(UINT8*)"GET ",GPRS_TRA_BUFFER_SIZE);
		concatStirngsToStr1(pucTransmitBuffer,pucServerPath,GPRS_TRA_BUFFER_SIZE);
	//	concatStirngsToStr1(pucTransmitBuffer,(UINT8*)"/",GPRS_TRA_BUFFER_SIZE);
		concatStirngsToStr1(pucTransmitBuffer,pucApplicationName,GPRS_TRA_BUFFER_SIZE);
		concatStirngsToStr1(pucTransmitBuffer,(UINT8*)"?",GPRS_TRA_BUFFER_SIZE);
		concatStirngsToStr1(pucTransmitBuffer,pucData,GPRS_TRA_BUFFER_SIZE);
		concatStirngsToStr1(pucTransmitBuffer,(UINT8*)" HTTP/1.1\r\nHost: ",GPRS_TRA_BUFFER_SIZE);
		concatStirngsToStr1(pucTransmitBuffer,pucServerAdd,GPRS_TRA_BUFFER_SIZE);
		concatStirngsToStr1(pucTransmitBuffer,(UINT8*)":",GPRS_TRA_BUFFER_SIZE);
		concatStirngsToStr1(pucTransmitBuffer,pucServerPort,GPRS_TRA_BUFFER_SIZE);
		concatStirngsToStr1(pucTransmitBuffer,(UINT8*)"\r\n\r\n",GPRS_TRA_BUFFER_SIZE);
		getStrLength(pucTransmitBuffer,&ulTempAf);
		pucTransmitBuffer[ulTempAf]=0X1A;
		pucTransmitBuffer[ulTempAf+1]=0X00;
	}
	else
	{
		if (ucModemType==MODEM_TYPE_4G)
		{
			pucTransmitBuffer[0]=0x00;
			concatStirngsToStr1(pucTransmitBuffer,pucData,GPRS_TRA_BUFFER_SIZE);	
		}
		else
		{
			pucTransmitBuffer[0]=0x00;
			concatStirngsToStr1(pucTransmitBuffer,(UINT8*)"POST ",GPRS_TRA_BUFFER_SIZE);
			concatStirngsToStr1(pucTransmitBuffer,pucServerPath,GPRS_TRA_BUFFER_SIZE);
		//	concatStirngsToStr1(pucTransmitBuffer,(UINT8*)"/",GPRS_TRA_BUFFER_SIZE);
			concatStirngsToStr1(pucTransmitBuffer,pucApplicationName,GPRS_TRA_BUFFER_SIZE);
			concatStirngsToStr1(pucTransmitBuffer,(UINT8*)" HTTP/1.1\r\nHost: ",GPRS_TRA_BUFFER_SIZE);
			concatStirngsToStr1(pucTransmitBuffer,pucServerAdd,GPRS_TRA_BUFFER_SIZE);
			concatStirngsToStr1(pucTransmitBuffer,(UINT8*)":",GPRS_TRA_BUFFER_SIZE);
			concatStirngsToStr1(pucTransmitBuffer,pucServerPort,GPRS_TRA_BUFFER_SIZE);
			concatStirngsToStr1(pucTransmitBuffer,(UINT8*)"\r\n",GPRS_TRA_BUFFER_SIZE);
			getStrLength(pucData,&ulTempAf);//LENGTH OF DATA
			concatStirngsToStr1(pucTransmitBuffer,(UINT8*)"Content-Length :",GPRS_TRA_BUFFER_SIZE);
			getStrLength(pucTransmitBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
			intToASCIIStr(ulTempAf,4,11,&pucTransmitBuffer[ulTempBf]);
			concatStirngsToStr1(pucTransmitBuffer,(UINT8*)"\r\n",GPRS_TRA_BUFFER_SIZE);
			concatStirngsToStr1(pucTransmitBuffer,(UINT8*)"Connection: keepalive\r\nUser-Agent: Mozilla/5.0\r\n",GPRS_TRA_BUFFER_SIZE);
			if(pucData[0]=='{')
				concatStirngsToStr1(pucTransmitBuffer,(UINT8*)"Content-Type: application/json\r\n",GPRS_TRA_BUFFER_SIZE);
			else
				concatStirngsToStr1(pucTransmitBuffer,(UINT8*)"Content-Type: application/x-www-form-urlencoded\r\n",GPRS_TRA_BUFFER_SIZE);
			concatStirngsToStr1(pucTransmitBuffer,(UINT8*)"Accept: */*\r\nAccept-Encoding: gzip,deflate,sdch\r\nAccept-Language: en-US,en;q=0.8\r\n\r\n",GPRS_TRA_BUFFER_SIZE);
			concatStirngsToStr1(pucTransmitBuffer,pucData,GPRS_TRA_BUFFER_SIZE);
			getStrLength(pucTransmitBuffer,&ulTempAf);
			pucTransmitBuffer[ulTempAf]=0X1A;
			pucTransmitBuffer[ulTempAf+1]=0X00;
		}
		
		
	}
}

void gprsTranferDataToServer4G(unsigned char* pucSMSStart, UINT8* pucTransDataBuffer, UINT8* pucApplicationName, UINT8 ucTrasferMode, UINT8 ucKeepConnectionOpen)
{
	static UINT8 ucState=GPRS_STATE_CHECK_MODEM;
	static UINT8 ucRetryCntr=0;
	static UINT8 ucErrorRespCntr=0;
	static UINT8 ucResetCounterF=0;
	unsigned int uiLoc;
	unsigned char ucSMCmd=0;
	unsigned char ucStatus=0;
	unsigned int uiCntr=1;
	
	if(*pucSMSStart==0)
	{
		
		ucRetryCntr=0;
		ucDataTransferSuccess=0;
		
		return;	
	}
	ucGPRSSMState=ucState;
	if(ucGPRSSMStateMax<ucGPRSSMState)
		ucGPRSSMStateMax=ucGPRSSMState;
	
	if(ucErrorRespCntr>100)
		ucState=STATE_RESET;
		
	
	
	switch (ucState)
	{
		case GPRS_STATE_CHECK_MODEM:
		{
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,1000,0,
			(UINT8*)"AT\r\n",
			(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_NONE,
			&uiLoc,
			(UINT8*)"\rGPRS4G : AT",
			(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_ENABLE_ERROR_MODE;	
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=STATE_RESET;
					uiGPRSTimer=0;
				}
			}
						
		}
		break;
		
		case GPRS_STATE_ENABLE_ERROR_MODE:
		
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,1000,0,
			(UINT8*)"AT+CMEE=1\r\n",
			(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_NONE,
			&uiLoc,
			(UINT8*)"\rGPRS4G : ENA ERR MODE",(UINT8*)"OK",(UINT8*)"",
			(UINT8*)"",BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_GET_IMIE;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=STATE_RESET;
					uiGPRSTimer=0;
				}
			}
		break;
		
		
		case GPRS_STATE_GET_IMIE:
		
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,1000,100,
			(UINT8*)"AT+GSN\r\n",
			(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_NONE,
			&uiLoc,
			(UINT8*)"\rGPRS4G : IMIE",(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
					{
						UARTSendStr(UART_DEBUG, (UINT8*) "\r GPRS4G driver -> IMIE");
					}
					
					for(ucStatus=0;ucStatus< 15;ucStatus++)
					{
						ucIMIENumber[ucStatus] = ucGPRSRecvBuffer[uiLoc-19+ucStatus];
						if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
						{
							UARTSendChar(UART_DEBUG, ucIMIENumber[ucStatus]);
						}
	
					}
					ucIMIENumber[15]=0x00;
					ucState=GPRS_STATE_SETUP_TIME_SYNC;
					
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=STATE_RESET;
					uiGPRSTimer=0;
				}
			}
		break;
		
		case GPRS_STATE_SETUP_TIME_SYNC:
		
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,1000,0,
			(UINT8*)"AT+CTZU=3\r\n",
			(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_NONE,
			&uiLoc,
			(UINT8*)"\rGPRS4G : ENA ERR MODE",(UINT8*)"OK",(UINT8*)"",
			(UINT8*)"",BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_CHECK_SIM;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=STATE_RESET;
					uiGPRSTimer=0;
				}
			}
		break;
		
		
		
		case GPRS_STATE_CHECK_SIM:
		{
			ucClockReadFromModem=1;
			ucNetworkStatus=0;
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,5000,100,
			(UINT8*)"AT+CPIN?\r\n",
			(UINT8*)"READY",(UINT8*)"",(UINT8*)"",
			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_NONE,
			&uiLoc,
			(UINT8*)"\rGPRS4G : CHECK SIM",(UINT8*)"DETECTED",(UINT8*)"NOT FOUND",(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_SPL_CMD;				
				}
				if(ucStatus==SIM800_RESPONSE_RESP2)
				{
					ucState=STATE_RESET;
					uiGPRSTimer=0;
					
					//blink power led to indicate sim not detected
					
					dio_blink(LED_PWR,100);
					dio_blink(LED_PWR,100);
					dio_blink(LED_PWR,100);
					dio_blink(LED_PWR,100);
					
				//	ucInternetSource=2;
					
					
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					GPRSSendStr((UINT8C*)"AT+CSMINS=1\r\n");
					
					//blink power led to indicate sim not detected
					delayMs(100);
					ucState=STATE_RESET;
					uiGPRSTimer=0;
					
					dio_blink(LED_PWR,100);
					dio_blink(LED_PWR,100);
					dio_blink(LED_PWR,100);
					dio_blink(LED_PWR,100);
					
				//	ucInternetSource=2;

				}
			}
	
		}
		break;
		
		
		case GPRS_STATE_SPL_CMD:
		{
			UINT32 ulTempAf;			
			getStrLength(ucModemSpecialCmd,&ulTempAf);
			if( (ulTempAf<3) || ((ulTempAf<3)>46) ) 
			{
				ucState=GPRS_STATE_CHECK_CONNECTION;
				break;
			}
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,1000,0,
			ucModemSpecialCmd,
			(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_NONE,
			&uiLoc,
			(UINT8*)"\rGPRS4G : SPL COMMAND",(UINT8*)"OK",(UINT8*)"",
			(UINT8*)"",BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_CHECK_CONNECTION;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=GPRS_STATE_CHECK_CONNECTION;
					uiGPRSTimer=0;
				}
			}
		}
		break;
		
		case GPRS_STATE_CHECK_CONNECTION:
		{
			
			ucSIMStatus=1;
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,1000,0,
			(UINT8*)"AT+CREG?\r\n",
			(UINT8*)"0,1",(UINT8*)"0,5",(UINT8*)"",
			NO_OF_RETRY_30,SIM800_RETRY_VALID_RESP_NONE,
			&uiLoc,
			(UINT8*)"\rGPRS4G : NW CONN",(UINT8*)"CONN",(UINT8*)"ROAMING",(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_ENABLE_SMS_RECP;				
				}
				if(ucStatus==SIM800_RESPONSE_RESP2)
				{
					ucState=GPRS_STATE_ENABLE_SMS_RECP;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					uiGPRSTimer=0;
				}
			}

		}
		break;
		
		
		case GPRS_STATE_ENABLE_SMS_RECP:
		
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,1000,0,
			(UINT8*)"AT+QURCCFG=\"urcport\",\"uart1\"\r\n",
			(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_NONE,
			&uiLoc,
			(UINT8*)"\rGPRS4G : ENA SMS_RECP",(UINT8*)"OK",(UINT8*)"",
			(UINT8*)"",BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_ENABLE_SMS_RECP_EXT;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=STATE_RESET;
					uiGPRSTimer=0;
				}
			}
		break;
		
		
		case GPRS_STATE_ENABLE_SMS_RECP_EXT:
		
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,1000,0,
			(UINT8*)"AT+CSCS=\"IRA\"\r\n",
			(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_NONE,
			&uiLoc,
			(UINT8*)"\rGPRS4G : ENA SMS_RECP",(UINT8*)"OK",(UINT8*)"",
			(UINT8*)"",BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_SIGNAL_STRENGTH;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=STATE_RESET;
					uiGPRSTimer=0;
				}
			}
		break;
		
		
		
		
		
		case GPRS_STATE_SIGNAL_STRENGTH:
			ucNetworkStatus=1;
			ucSignalStrength=0;
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,3000,100,
			(UINT8*)"AT+CSQ\r\n",
			(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_NONE,
			&uiLoc,
			(UINT8*)"\rGPRS4G : SIGNAL STR",(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					if (searchString(ucGPRSRecvBuffer, GPRS_REC_BUFFER_SIZE, (UINT8*) "+CSQ: ", 6, &uiLoc) == SUCCESS) {
			                        ucSignalStrength = 0;
			                        for (uiCntr = 0; uiCntr < 2; uiCntr++) {
			            
			                            ucSignalStrength *= 10;
			                            ucSignalStrength += (ucGPRSRecvBuffer[uiLoc + 6 + uiCntr] - 0x30);
			                        }
						if(ucSignalStrength>40)
							ucSignalStrength=2;
						if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
						{
						UARTSendStr(UART_DEBUG,(UINT8C*)"\r GPRS, Signal Strength : ");
						UARTSendLong(UART_DEBUG,ucSignalStrength,2);
						}
			                    }
					
					ucState=GPRS_STATE_GET_TIME;
					uiGPRSTimer=0;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					uiGPRSTimer=0;
				}
			}
		break;
		
		
		case GPRS_STATE_GET_TIME:
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,1000,100,
			(UINT8*)"AT+CCLK?\r\n",
			(UINT8*)"+CCLK:",(UINT8*)"",(UINT8*)"",
			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_NONE,
			&uiLoc,
			(UINT8*)"\rGPRS4G : IMIE",(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucStatus = getValueFromBuff(ucGPRSRecvBuffer, (UINT8*) "+CCLK: \"", (UINT8*) "\"", ucGeneralPurposeBuffer, GENERAL_PURPOSE_BUFF_SIZE);
					if (ucStatus == SUCCESS) 
					{
						if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
						{
						UARTSendStr(UART_DEBUG, (UINT8*) "\r GPRS driver -> RTC Buf :");
						UARTSendStr(UART_DEBUG, ucGeneralPurposeBuffer);
						}
						uiLoc =    ((ucGeneralPurposeBuffer[0] - '0')*10) + (ucGeneralPurposeBuffer[1] - '0');
						if(uiLoc>20 && uiLoc <50)
						{
							sRTC.ucYear  = 	((ucGeneralPurposeBuffer[0] - '0')*10) + (ucGeneralPurposeBuffer[1] - '0');
							sRTC.ucMonth =   ((ucGeneralPurposeBuffer[3] - '0')*10) + (ucGeneralPurposeBuffer[4] - '0');
							sRTC.ucDate =    ((ucGeneralPurposeBuffer[6] - '0')*10) + (ucGeneralPurposeBuffer[7] - '0');
							sRTC.ucHours =   ((ucGeneralPurposeBuffer[9] - '0')*10) + (ucGeneralPurposeBuffer[10] - '0');
							sRTC.ucMinutes = ((ucGeneralPurposeBuffer[12] - '0')*10) + (ucGeneralPurposeBuffer[13] - '0');
							sRTC.ucSeconds = ((ucGeneralPurposeBuffer[15] - '0')*10) + (ucGeneralPurposeBuffer[16] - '0');
							sRTC.ucOffset=   ((ucGeneralPurposeBuffer[18] - '0')*10) + (ucGeneralPurposeBuffer[19] - '0');
							sRTC.ucDay = 0;
						}
						
						

					}
					ucState=GPRS_STATE_CON_PARA_HTTP_SERVER_CONTEXT_ID;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=STATE_RESET;
					uiGPRSTimer=0;
				}
			}
		
		break;
		
		
		case GPRS_STATE_WAIT_BEFORE_NEXT_RESET:
		{
			if(uiGPRSTimer<ucMS_WAIT_BEFORE_RESET)
				return;
				
			uiGPRSTimer=0;
			ucState=STATE_RESET;
			
		}
		break;
		
		
		case GPRS_STATE_CON_PARA_HTTP_SERVER_CONTEXT_ID:
		{
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,5000,5000,
			(UINT8*)"AT+QHTTPCFG=\"contextid\",1\r\n",
			(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",
			NO_OF_RETRY_6,SIM800_RETRY_VALID_RESP_2,
			&uiLoc,(UINT8*)"\rGPRS4G : HTTPCFG contextid ",
			(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_CON_PARA_HTTP_SERVER_RESPONSE_HEADER;				
				}
				if(ucStatus==SIM800_RESPONSE_RESP2)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					uiGPRSTimer=0;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					uiGPRSTimer=0;
				}
			}

		}
		break;
		
		
		case GPRS_STATE_CON_PARA_HTTP_SERVER_RESPONSE_HEADER:
		{
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,5000,0,
			(UINT8*)"AT+QHTTPCFG=\"responseheader\",1\r\n",
			(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",
			NO_OF_RETRY_6,SIM800_RETRY_VALID_RESP_2,
			&uiLoc,(UINT8*)"\rGPRS4G : HTTPCFG responseheader ",
			(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_SET_GPRS_APN_USN_PWD;
					ucErrorRespCntr++;	// to avoid looping between start ip stack and stiop ip stack
					
				}
				if(ucStatus==SIM800_RESPONSE_RESP2)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					uiGPRSTimer=0;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					uiGPRSTimer=0;
				}
			}

		}
		break;
		
		
		case GPRS_STATE_SET_GPRS_APN_USN_PWD:
		{
			UINT32 ulTempAf;			
			getStrLength(ucModemSpecialCmd,&ulTempAf);
			if( (ulTempAf>3)) 
			{
				ucState=GPRS_STATE_ACTIVATE_CONTEXT;
				break;
			}
			ucGeneralPurposeBuffer[0]=0x00;
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"AT+QICSGP=1,1,\"",GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,ucGPRSAPNStr,GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"\",\"",GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,ucGPRSUserNameStr,GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"\",\"",GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,ucGPRSPasswordStr,GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"\",1\r\n",GENERAL_PURPOSE_BUFF_SIZE);
			
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,1000,0,
			ucGeneralPurposeBuffer,
			(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",
			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_2,
			&uiLoc,
			(UINT8*)"\rGPRS : SET APN_USN_PWD",
			(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_ACTIVATE_CONTEXT;				
				}
				if(ucStatus==SIM800_RESPONSE_RESP2)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					uiGPRSTimer=0;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					uiGPRSTimer=0;
				}
			}

		}
		break;
		
		
		
		
	
		case GPRS_STATE_ACTIVATE_CONTEXT:
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,15000,0,
			(UINT8*)"AT+QIACT=1\r\n",
			(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",
			3,SIM800_RETRY_VALID_RESP_2,
			&uiLoc,
			(UINT8*)"\rGPRS4G : START BEARER",
			(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_GET_LOCAL_IP_ADD;
				}
				if(ucStatus==SIM800_RESPONSE_RESP2)
				{
//					ucState=GPRS_STATE_STOP_GPRS_BEARER;
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;	
					ucErrorRespCntr++;
				}
				
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					uiGPRSTimer=0;
				}
			}
		break;
		
		
//		case GPRS_STATE_STOP_GPRS_BEARER:
//			ucSMCmd=1;
//			ucStatus=processModemCmd(&ucSMCmd,2000,3000,
//			(UINT8*)"AT+CIPSHUT\r\n",
//			(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",
//			3,SIM800_RETRY_VALID_RESP_2,
//			&uiLoc,(UINT8*)"\rGPRS : STOP BEARER",
//			(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",
//			BUFFER_CLEAR);
//			if(ucSMCmd==0)
//			{
//				if(ucStatus==SIM800_RESPONSE_RESP1)
//				{
//					ucState=GPRS_STATE_START_GPRS_BEARER;
//					ucErrorRespCntr++;
//				}
//				if(ucStatus==SIM800_RESPONSE_RESP2)
//				{
//					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
//					uiGPRSTimer=0;
//				}
//				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
//				{
//					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
//					uiGPRSTimer=0;
//				}
//			}
//		break;
		
		
		case GPRS_STATE_GET_LOCAL_IP_ADD:
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,1000,5000,
			(UINT8*)"AT+QIACT?\r\n",
			(UINT8*)"QIACT",(UINT8*)"ERROR",(UINT8*)"",
			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_2,
			&uiLoc,(UINT8*)"\rGPRS4G : GET IP",
			(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					if (searchString(ucGPRSRecvBuffer, GPRS_REC_BUFFER_SIZE, (UINT8*) "QIACT:", 6, &uiLoc) == SUCCESS) 
					{
						
			                       for (uiCntr = 0; uiCntr < 20; uiCntr++)
                        				ucLocalIP[uiCntr] = 0;

				                    for (uiCntr = 0; uiCntr < 20; uiCntr++) {
				                        ucLocalIP[uiCntr] = ucGPRSRecvBuffer[uiCntr + uiLoc + 14];
				                    }
					       ucState=GPRS_STATE_SIGNAL_STRENGTH_POST;
			                }
				}
				if(ucStatus==SIM800_RESPONSE_RESP2)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					ucErrorRespCntr++;
				}
				
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					uiGPRSTimer=0;
				}
			}
		break;
		
		
//		case GPRS_STATE_CHECK_CONN_STATUS:
//			ucSMCmd=1;
//			ucStatus=processModemCmd(&ucSMCmd,2000,3000,
//			(UINT8*)"AT+CIPSTATUS\r\n",
//			(UINT8*)"IP STATUS",(UINT8*)"",(UINT8*)"",
//			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_2,
//			&uiLoc,(UINT8*)"\rGPRS : IP Status",
//			(UINT8*)"Connected",(UINT8*)"",(UINT8*)"",
//			BUFFER_CLEAR);
//			if(ucSMCmd==0)
//			{
//				if(ucStatus==SIM800_RESPONSE_RESP1)
//				{
//					ucState=GPRS_STATE_SET_IP_MODE;
//				}
//				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
//				{
////					ucState=GPRS_STATE_STOP_GPRS_BEARER;
//					ucErrorRespCntr++;
//				}
//			}
//		break;
//		
//		case GPRS_STATE_SET_IP_MODE:
//			ucSMCmd=1;
//			ucStatus=processModemCmd(&ucSMCmd,2000,3000,
//			(UINT8*)"AT+CIPHEAD=1\r\n",
//			(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
//			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_2,
//			&uiLoc,(UINT8*)"\rGPRS : settings IP mode",
//			(UINT8*)"Set",(UINT8*)"",(UINT8*)"",
//			BUFFER_CLEAR);
//			if(ucSMCmd==0)
//			{
//				if(ucStatus==SIM800_RESPONSE_RESP1)
//				{
//					ucState=GPRS_STATE_SETUP_URL;


//				}
//				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
//				{
//					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
//					uiGPRSTimer=0;
//				}
//			}
//		break;
		

//0035
		case GPRS_STATE_SIGNAL_STRENGTH_POST:
			ucNetworkStatus=1;
			ucSignalStrength=0;
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,3000,100,
			(UINT8*)"AT+CSQ\r\n",
			(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
			NO_OF_RETRY_3,SIM800_RETRY_VALID_RESP_NONE,
			&uiLoc,
			(UINT8*)"\rGPRS4G : SIGNAL STR",(UINT8*)"OK",(UINT8*)"",(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					if (searchString(ucGPRSRecvBuffer, GPRS_REC_BUFFER_SIZE, (UINT8*) "+CSQ: ", 6, &uiLoc) == SUCCESS) {
			                        ucSignalStrength = 0;
			                        for (uiCntr = 0; uiCntr < 2; uiCntr++) {
			            
			                            ucSignalStrength *= 10;
			                            ucSignalStrength += (ucGPRSRecvBuffer[uiLoc + 6 + uiCntr] - 0x30);
			                        }
						if(ucSignalStrength>40)
							ucSignalStrength=2;
						if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
						{
						UARTSendStr(UART_DEBUG,(UINT8C*)"\r GPRS, Signal Strength : ");
						UARTSendLong(UART_DEBUG,ucSignalStrength,2);
						}
			                    }
					
					ucState=GPRS_STATE_SETUP_URL;
					uiGPRSTimer=0;
				}
				
				ucState=GPRS_STATE_SETUP_URL;
				
				
			}
		break;

		
		case GPRS_STATE_SETUP_URL:
		{
			unsigned long ulTempAf=0,ulTempBf=0;;
			ucGPRSConStatus=1;
			ucGeneralPurposeBuffer[0]=0x00;
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"http://",GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,ucSerAdd,GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)":",GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,ucSerPort,GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,ucSerPath,GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,pucApplicationName,GENERAL_PURPOSE_BUFF_SIZE);
			
			//here
			getStrLength(ucGeneralPurposeBuffer,&ulTempAf);
			getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
			intToASCIIStr(ulTempAf,4,11,&ucGeneralPurposeBuffer[ulTempBf]);
		
			ucGeneralPurposeBuffer[0]=0x00;
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"AT+QHTTPURL=",GENERAL_PURPOSE_BUFF_SIZE);
			getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
			intToASCIIStr(ulTempAf,4,11,&ucGeneralPurposeBuffer[ulTempBf]);
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)",60\r\n",GENERAL_PURPOSE_BUFF_SIZE);
			
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,10000,0,
			ucGeneralPurposeBuffer,
			(UINT8*)"CONNECT",(UINT8*)"ERROR",(UINT8*)"",
			3,SIM800_RETRY_VALID_RESP_2,
			&uiLoc,
			(UINT8*)"\rGPRS4G : Conn to server",(UINT8*)"Connected",(UINT8*)"ERROR",
			(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_CONNECT_TO_URL;				
				}
				if(ucStatus==SIM800_RESPONSE_RESP2)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					ucErrorRespCntr++;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					uiGPRSTimer=0;
				}
			}

		}
		break;
		
		
		case GPRS_STATE_CONNECT_TO_URL:
		{
			unsigned long ulTempAf=0,ulTempBf=0;;
			ucGPRSConStatus=1;
			ucGeneralPurposeBuffer[0]=0x00;
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"http://",GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,ucSerAdd,GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)":",GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,ucSerPort,GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,ucSerPath,GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,pucApplicationName,GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"\r\n",GENERAL_PURPOSE_BUFF_SIZE);
			
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,10000,0,
			ucGeneralPurposeBuffer,
			(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",
			3,SIM800_RETRY_VALID_RESP_2,
			&uiLoc,
			(UINT8*)"\rGPRS4G : Conn to server",(UINT8*)"Connected",(UINT8*)"ERROR",
			(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_SETUP_POST;				
				}
				if(ucStatus==SIM800_RESPONSE_RESP2)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					ucErrorRespCntr++;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					uiGPRSTimer=0;
				}
			}

		}
		break;
		
		
		case GPRS_STATE_SETUP_POST:
		{
			unsigned long ulTempAf=0,ulTempBf=0;;
			ucGPRSConStatus=1;
			ucGeneralPurposeBuffer[0]=0x00;
			
			getStrLength(pucTransDataBuffer,&ulTempAf);
					
			ucGeneralPurposeBuffer[0]=0x00;
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"AT+QHTTPPOST=",GENERAL_PURPOSE_BUFF_SIZE);
			getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
			intToASCIIStr(ulTempAf,4,11,&ucGeneralPurposeBuffer[ulTempBf]);
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)",80,80\r\n",GENERAL_PURPOSE_BUFF_SIZE);
			
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,10000,0,
			ucGeneralPurposeBuffer,
			(UINT8*)"CONNECT",(UINT8*)"ERROR",(UINT8*)"",
			3,SIM800_RETRY_VALID_RESP_2,
			&uiLoc,
			(UINT8*)"\rGPRS4G : Conn to server",(UINT8*)"Connected",(UINT8*)"ERROR",
			(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_POST_DATA;				
				}
				if(ucStatus==SIM800_RESPONSE_RESP2)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					ucErrorRespCntr++;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					uiGPRSTimer=0;
				}
			}

		}
		break;
		
		
		case GPRS_STATE_POST_DATA:
		{
			unsigned long ulTempAf=0,ulTempBf=0;;
			ucGPRSConStatus=1;
			ucGeneralPurposeBuffer[0]=0x00;
						
			ucSMCmd=1;
			ucStatus=processModemCmd(&ucSMCmd,10000,2000,
			pucTransDataBuffer,
			(UINT8*)"QHTTPPOST:",(UINT8*)"ERROR",(UINT8*)"",
			3,SIM800_RETRY_VALID_RESP_2,
			&uiLoc,
			(UINT8*)"\rGPRS4G : Conn to server",(UINT8*)"Connected",(UINT8*)"ERROR",
			(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucState=GPRS_STATE_READ_RESPONSE;				
				}
				if(ucStatus==SIM800_RESPONSE_RESP2)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					ucErrorRespCntr++;
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=GPRS_STATE_WAIT_BEFORE_NEXT_RESET;
					uiGPRSTimer=0;
				}
			}

		}
		break;
		
		
		
		
		
		
		case GPRS_STATE_READ_RESPONSE:
			ucGPRSDataTranferFlag=1;
			ucSMCmd=1;
			
			ucGeneralPurposeBuffer[0]=0x00;
			ucStatus=processModemCmd(&ucSMCmd,15000,2000,
			(UINT8*)"AT+QHTTPREAD=80\r\n",
			(UINT8*)"#####",(UINT8*)"",(UINT8*)"",
			1,SIM800_RETRY_VALID_RESP_2,
			&uiLoc,(UINT8*)"\rGPRS4G : Transmitting data",
			(UINT8*)"Tansfer complete",(UINT8*)"",(UINT8*)"",
			BUFFER_CLEAR);
			if(ucSMCmd==0)
			{
				if(ucStatus==SIM800_RESPONSE_RESP1)
				{
					ucDataTransferSuccess=1;
					ucErrorRespCntr=0;
					ucResetCounterF=0;
					ucTimerNonComm=0;
					ucState=GPRS_STATE_IDLE_CON_CLOSE;//0035
					if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
						displayGPRSRecBufferTillCount();
				}
				if(ucStatus==SIM800_RESPONSE_TIMEOUT)
				{
					ucState=GPRS_STATE_SETUP_URL;
					if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
						displayGPRSRecBufferTillCount();
				}
			}
		break;
		
		
		
		
		
		
		case GPRS_STATE_IDLE_CON_CLOSE:
		
			*pucSMSStart=0;
			ucState=GPRS_STATE_SIGNAL_STRENGTH_POST;
		
		break;
		
		
		case STATE_RESET:
			ucNetworkStatus=0;
			ucSignalStrength=0;
			ucErrorRespCntr=0;
			ucSIMStatus=0;
			ucGPRSConStatus=0;
			
			if (resetModem())
		           	ucNetworkStatus=0;
		        else
			{
				//check
				ucResetCounterF++;
		        	ucState = GPRS_STATE_CHECK_MODEM;
			}
		
		break;
		
		
		default:
			ucState = GPRS_STATE_CHECK_MODEM;
		break;
		
	}
}	


UINT8 setRTCModem(UINT8 ucDate,UINT8 ucMonth,UINT8 ucYear,UINT8 ucHours,UINT8 ucMinutes,UINT8 ucSeconds,UINT8 ucGTM)
{
	UINT8 ucDateTime[25]="00/01/01,00:06:33+00";
     //   UARTSendStr(UART_DEBUG,(UINT8*)"\rGPRS driver -> writing DateTime");
	
	intToArray(ucYear,2,25,&ucDateTime[0]);
	intToArray(ucMonth,2,25,&ucDateTime[3]);
	intToArray(ucDate,2,25,&ucDateTime[6]);
	intToArray(ucHours,2,25,&ucDateTime[9]);
	intToArray(ucMinutes,2,25,&ucDateTime[12]);
	intToArray(ucSeconds,2,25,&ucDateTime[15]);
	intToArray(ucGTM,2,25,&ucDateTime[18]);
	GPRSSendStr((UINT8*)"AT+CCLK=");
	GPRSSendStr((UINT8*)"\"");
	GPRSSendStr(ucDateTime);
	GPRSSendStr((UINT8*)"\"");
	GPRSSendStr((UINT8*)"\r\n");
	
//	SIM7600SendStr((UINT8*)"AT+CCLK=");
//	SIM7600SendStr((UINT8*)"\"");
//	SIM7600SendStr(ucDateTime);
//	SIM7600SendStr((UINT8*)"\"");
//	SIM7600SendStr((UINT8*)"\r\n");
//	

    //    UARTSendStr(UART_DEBUG,ucDateTime);
	delayMs(500);
	//displayGPRSBufferTillCount();
	return SUCCESS;
}






UINT8 sendSMSStateMachine(UINT8* pucSMFlagF)
{
    	#define SMS_STATE_READ_SMS 0
    	#define SMS_STATE_CHECK_NW_CONN 1
	#define SMS_STATE_SETUP 2
	#define SMS_STATE_SEND_REQ 3
	#define SMS_STATE_SEND_SMS 4
	#define SMS_STATE_EXIT 5
	UINT8 ucStatusF;
	UINT32 ulTempF;
	static UINT8 ucTempVal[120];
	static UINT8 ucStateF=SMS_STATE_READ_SMS;
	static UINT8 ucSMSCount=0;
	
	if(*pucSMFlagF==0)
	{
		ucStateF=SMS_STATE_READ_SMS;
		ucSMSCount=0;
		return SUCCESS;
	}
	
	switch (ucStateF)
	{
		case SMS_STATE_READ_SMS:
		        {
		           if(readStoredSMSFromEEPROM(ucTempVal)==SUCCESS)
		            {
				if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) )
				{
		                UARTSendStr(UART_DEBUG, (UINT8*) "\r\r reading sms from eeprom : ");
		                UARTSendStr(UART_DEBUG,ucTempVal);
				}
				ucStateF=SMS_STATE_CHECK_NW_CONN;
		            }
		           else
		           {
		               ucStateF=SMS_STATE_READ_SMS;
		               *pucSMFlagF=0;
		               return SUCCESS;
		           }
		        }
           	 break;
                
                
                
	        case SMS_STATE_CHECK_NW_CONN:
		        {
		            UINT8 ucTempPCF=1,ucTempState=0;
		            
			   
			    gprsTranferDataToServer4G(&ucTempPCF,ucTempVal,(UINT8*)"boardtestpost.aspx",GPRS_TRANS_MODE_POST,GPRS_CONNECTION_CLOSE);
			   
			    
		            if(ucNetworkStatus==1)
		            {
		                ucStateF=SMS_STATE_SETUP;
		            }
		        }
	        break;
		
		case SMS_STATE_SETUP:
			GPRSSendStr((UINT8*)" AT+CMGF=1\r\n");
			clearGPRSRecBuffer();
			if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
					UARTSendStr(UART_DEBUG,(UINT8*)"\r sending sms");
			
			
			uiGPRSTimer=0;
			ucStateF=SMS_STATE_SEND_REQ;
		break;
	
		case SMS_STATE_SEND_REQ:
		
			if(uiGPRSTimer<2000)
				break;
				
			GPRSSendStr((UINT8*)"AT+CMGS=\"");
			if(ucSMSCount==0)
				GPRSSendStr(ucAlertMobileNo1);
			else
				GPRSSendStr(ucAlertMobileNo2);
			GPRSSendStr((UINT8*)"\"\r\n");
			
			uiGPRSTimer=0;
			ucStateF=SMS_STATE_SEND_SMS;	
				
		break;
		
		case SMS_STATE_SEND_SMS:
			
		    if(uiGPRSTimer<3000)
			break;
				
		    ucStatusF=searchStr(ucGPRSRecvBuffer,(UINT8*)"ERROR",GPRS_REC_BUFFER_SIZE,&ulTempF); // if ERROR response is received then          
		    if(ucStatusF==FAILURE)
		    {         
	            	GPRSSendStr(ucTempVal);		// if ERROR was not received as response then then send the message text
	            	GPRSSendChar(0x1A);
			
			ucStateF=SMS_STATE_EXIT;
			if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
			{
			 UARTSendStr(UART_DEBUG, (UINT8*) "\r\r Sending SMS");
			 UARTSendStr(UART_DEBUG, ucTempVal);
			}
		    }
		    else
		    {
			if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
		    		UARTSendStr(UART_DEBUG,(UINT8*)"\r GPRS driver -> SMS send fail");
		    	ucStateF=SMS_STATE_EXIT;
		    }	
		    uiGPRSTimer=0;
		    
		    
		break;
	
		case SMS_STATE_EXIT:
			
			
			if(uiGPRSTimer<2000)
				break;
			//check to see it alertmobilenumber 2 exsists, if it exsists then send sms to second number
			if((ucSMSCount==0) && (ucAlertMobileNo2[0]!=0x00) )
			{
				   ucSMSCount++;
				   ucStateF=SMS_STATE_SEND_REQ;
				   uiGPRSTimer=0;
			}
			else	
			{
				ucSMSCount=0;
				ucStateF=SMS_STATE_READ_SMS;
			//	*pucSMFlagF=0;
			}
			
		break;
	
		default:
			ucStateF=SMS_STATE_READ_SMS;
		break;
	}
	
	return SUCCESS;
}



UINT8 receiveSMS(void)
{
    unsigned char ucStatusF = 0, ucTempF, ucTempAF = 0;
    unsigned int uiStartAdd = 0;
    unsigned long ulTempF = 0;

    
  //  if(ucInternetSource==0)
    {
	    ucStatusF = searchStr(ucGPRSRecvBuffer, (UINT8*) "+CMTI:", GPRS_REC_BUFFER_SIZE, &ulTempF); // check if +CMTI: is received from // modem indicating that an SMS has been revieved
	    if (ucStatusF == SUCCESS) {
		    ucSMSReceivedFlag=0;
	       
		delayMs(250);
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
		{
			displayGPRSRecBufferTillCount();
	     		UARTSendStr(UART_DEBUG, (UINT8*) "\r GPRS driver -> SMS received");
	     		//UARTSendStr(UART_DEBUG, (UINT8*) "\r\r\r SMS received");
		}
	//	delayMs(2000);

		if(ucModemType==MODEM_TYPE_4G)
			ucStatusF = searchStr(ucGPRSRecvBuffer, (UINT8*) "\"ME\",", GPRS_REC_BUFFER_SIZE, &ulTempF);
		else
		        ucStatusF = searchStr(ucGPRSRecvBuffer, (UINT8*) "\"SM\",", GPRS_REC_BUFFER_SIZE, &ulTempF);

		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
		{
			UARTSendStr(UART_DEBUG, (UINT8*) ", SMS number : ");
			
			UARTSendChar(UART_DEBUG,ucGPRSRecvBuffer[(ulTempF+5)]);
			UARTSendChar(UART_DEBUG,ucGPRSRecvBuffer[(ulTempF+6)]);
			UARTSendChar(UART_DEBUG,ucGPRSRecvBuffer[(ulTempF+7)]);
		}
		ucTempAF=0x00;
	        ucTempF  = ucGPRSRecvBuffer[ulTempF + 5]; // GET SMS number for the buffer
		ucTempAF = ucGPRSRecvBuffer[ulTempF + 6]; // GET SMS number for the buffer	

		clearGPRSRecBuffer();
		
		delayMs(250);
		GPRSSendStr((unsigned char *) "AT+CMGF=1\r\n"); // issue AT command to open the reveived message
		delayMs(500);
		
	        
	        GPRSSendStr((unsigned char *) "AT+CMGR="); // issue AT command to open the reveived message
	        GPRSSendChar(ucTempF);
		if( (ucTempAF>='0') && ((ucTempAF<='9')) )
			GPRSSendChar(ucTempAF);
	        GPRSSendStr((unsigned char *) "\r");
	        GPRSSendStr((unsigned char *) "\n");

	        delayMs(1000);
	        displayGPRSRecBufferTillCount();
	        ucStatusF = searchStr(ucGPRSRecvBuffer, (UINT8*) "D\",\"", GPRS_REC_BUFFER_SIZE, &ulTempF); // search the receiver buffer to
	        if (ucStatusF == 0) {
	            ucTempAF = 0;
	            for (uiStartAdd = (UINT16)ulTempF + 4; uiStartAdd < ((UINT16)ulTempF + 18); uiStartAdd++) {
	                ucSenderMobNumberStr[ucTempAF++] = ucGPRSRecvBuffer[uiStartAdd]; // get sender mobile number from buffer

	            }
	            ucSenderMobNumberStr[13] = 0x00;
		    if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
			{
		            UARTSendStr(UART_DEBUG, (UINT8*) " No: ");
		            UARTSendStr(UART_DEBUG, (UINT8*) ucSenderMobNumberStr);
			}
	        }

		ucSMSReceivedFlag=0;
	        ucStatusF = searchStr(ucGPRSRecvBuffer, (UINT8*) "YuS", GPRS_REC_BUFFER_SIZE, &ulTempF); // search the receiver buffer to
	        if (ucStatusF == FAILURE) 
		{
			if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
		            UARTSendStr(UART_DEBUG, (UINT8*) "\r GPRS driver -> Invalid message");
			    
			clearGPRSRecBuffer();
		       	delayMs(1000);
			GPRSSendStr((UINT8*)"AT+CMGF=1\r\n");
			delayMs(1000);
			//0026
			if(ucModemType==MODEM_TYPE_4G)
				GPRSSendStr((UINT8*)"AT+CMGD=1,4\r\n");
			else
				GPRSSendStr((UINT8*)"AT+CMGDA=\"DEL ALL\"\r\n");
			delayMs(1500);
			if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
		        	displayGPRSRecBufferTillCount();
		        clearGPRSRecBuffer();   
			    
			    
	        } 
		else 
		{
	            ucSMSReceivedFlag = 1;
	            delayMs(1000);
		    GPRSSendStr((UINT8*)"AT+CMGF=1\r\n");
		    delayMs(1000);
		    //0026
		    if(ucModemType==MODEM_TYPE_4G)
				GPRSSendStr((UINT8*)"AT+CMGD=1,4\r\n");
		    else
				GPRSSendStr((UINT8*)"AT+CMGDA=\"DEL ALL\"\r\n");
		    delayMs(1500);  		
		    checkBufferForCommands(ucGPRSRecvBuffer,DEBUG_REC_BUFFER_SIZE,BUFFER_CLEAR);
		    if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
		    {
			UARTSendStr(UART_DEBUG, (UINT8*) "\r GPRS driver -> deleting store messages");
			displayGPRSRecBufferTillCount();
		    }
		    clearGPRSRecBuffer();   
	        }
	    }
    }

    return SUCCESS;
}


UINT8 sendInstantSms(UINT8* pucNumber, UINT8* pucMessage) {
    unsigned char ucStatusF = 0;
    unsigned long ulTempF = 0;

    if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
	{
	    UARTSendStr(UART_DEBUG, (UINT8*) "\r Sending SMS No :");
    		UARTSendStr(UART_DEBUG, pucNumber);
	}
    if(pucNumber[0]!='+')
    {
	    if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
		{
        		UARTSendStr(UART_DEBUG, (UINT8*) " invalid number :");
		}
        return FAILURE;
    }
    if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
	{
	    UARTSendStr(UART_DEBUG, (UINT8*) "\r Message : ");
	    UARTSendStr(UART_DEBUG, pucMessage);
	}
		
	{
	    GPRSSendStr((UINT8*) " AT+CMGF=1\r\n");
	    delayMs(500);
	    GPRSSendStr((UINT8*) "AT+CMGS=\"");
	    GPRSSendStr(pucNumber);
	    GPRSSendStr((UINT8*) "\"\r\n");
	    delayMs(2000);
	    ucStatusF = searchStr(ucGPRSRecvBuffer, (unsigned char*) "ERROR", GPRS_REC_BUFFER_SIZE, &ulTempF); // if ERROR response is received then
	    if (ucStatusF == 1) {

	        GPRSSendStr(pucMessage); // if ERROR was not received as response then then send the message text
	        GPRSSendChar(0x1A);
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
			
	        UARTSendStr(UART_DEBUG, (UINT8*) ": success");
	    } else {
		    if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
			UARTSendStr(UART_DEBUG, (UINT8*) ": fail");
	    }
	}
	
    clearGPRSRecBuffer();
//    delayMs(2000);
//	GPRSSendStr((UINT8*) "AT+CMGDA=\"DEL ALL\"\r\n");
//	delayMs(800);
   
    return SUCCESS;
}

