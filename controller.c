#include "controller.h"
#include "gprs.h"
#include "global.h"
#include "uartGPRS.h"
#include "uart.h"
#include "delay.h"
#include "generalfunctions.h"
#include "memory.h"
#include "uartDebug.h"
#include "eeprom.h"
#include "memory.h"
#include "rtc.h"
#include "dio.h"
#include "flash.h"
#include "data.h"
#include "modbus.h"
#include "display.h"
#include "analog.h"
#include "uartRS485.h"
#include "controller.h"


UINT8 ucIMIENumber[16]="               ";
UINT8 ucGPRSSMStateMax;

UINT8 ucClockReadFromModem;
PARA_CONFIG sParaConfig[N0_OF_PARAMETERS];

UINT16 uiAcqIntMin;
UINT16 uiTraIntMin;
UINT8 ucPraAcqFlag;
UINT8 ucPraTraFlag;
UINT8 ucCheckRelayOperation;
UINT16 uiPendingFiles;

UINT8 ucBoardTestSMFlag;

UINT8 ucModemType=MODEM_TYPE_2G;
static UINT8 ucParaCntrParaConfig=0;

static UINT8 appendJsonConfigField(UINT8 *pucJson, UINT8 *pucKey,
					   UINT8 *pucCommand, UINT8 *pucOut,
					   UINT16 uiOutSize)
{
	UINT8 ucValue[64];
	if (getParameterFromJsonString(pucJson,pucKey,ucValue,sizeof(ucValue)) != SUCCESS)
		return SUCCESS;
	if (concatStirngsToStr1(pucOut,pucCommand,uiOutSize) == FAILURE ||
	    concatStirngsToStr1(pucOut,ucValue,uiOutSize) == FAILURE ||
	    concatStirngsToStr1(pucOut,(UINT8*)"#",uiOutSize) == FAILURE)
		return FAILURE;
	return SUCCESS;
}

static UINT8 buildSystemConfigCommandsFromJson(UINT8 *pucJson, UINT8 *pucOut,
						       UINT16 uiOutSize)
{
	UINT32 ulLength=0;
	pucOut[0]=0;
	if (appendJsonConfigField(pucJson,(UINT8*)"SERVER_ADD",(UINT8*)"$SER_ADD=",pucOut,uiOutSize)==FAILURE ||
	    appendJsonConfigField(pucJson,(UINT8*)"SERVER_PORT",(UINT8*)"$SER_PORT=",pucOut,uiOutSize)==FAILURE ||
	    appendJsonConfigField(pucJson,(UINT8*)"SERVER_PATH",(UINT8*)"$SER_PATH=",pucOut,uiOutSize)==FAILURE ||
	    appendJsonConfigField(pucJson,(UINT8*)"SER_APP_DATA",(UINT8*)"$SER_APP_DATA=",pucOut,uiOutSize)==FAILURE ||
	    appendJsonConfigField(pucJson,(UINT8*)"SERVER_APP_DATA",(UINT8*)"$SER_APP_DATA=",pucOut,uiOutSize)==FAILURE ||
	    appendJsonConfigField(pucJson,(UINT8*)"NO_OF_PARAMETERS",(UINT8*)"$NO_OF_PARAMETERS=",pucOut,uiOutSize)==FAILURE ||
	    appendJsonConfigField(pucJson,(UINT8*)"DATA_ACQ_MINUTES",(UINT8*)"ACQ_INT=",pucOut,uiOutSize)==FAILURE ||
	    appendJsonConfigField(pucJson,(UINT8*)"DATA_TRANS_MINUTES",(UINT8*)"TRANS_INT=",pucOut,uiOutSize)==FAILURE)
		return FAILURE;
	getStrLength(pucOut,&ulLength);
	if(ulLength==0)
		return FAILURE;
	return SUCCESS;
}

static UINT8 loadCompactParameter(UINT8 *pucRecord, UINT16 uiLength, UINT16 uiParaId)
{
	UINT16 uiIndex, uiPacketLength=0, uiNameLength=0;
	UINT16 uiSeparator=30;
	UINT8 ucTemp[9];
	PARA_CONFIG *pPara;
	if(uiParaId>=N0_OF_PARAMETERS || uiLength<33)
		return FAILURE;
	for(uiIndex=30;uiIndex<uiLength;uiIndex++)
	{
		if(pucRecord[uiIndex]=='#')
		{
			uiSeparator=uiIndex;
			break;
		}
	}
	if(uiSeparator==30 || uiSeparator>=uiLength-1)
		return FAILURE;
	pPara=&sParaConfig[uiParaId];
	for(uiIndex=0;uiIndex<sizeof(pPara->ucPraConfigArray);uiIndex++)
		pPara->ucPraConfigArray[uiIndex]=0;
	pPara->para.uiParaID=uiParaId;
	pPara->para.ucParaSource=(UINT8)convAsciiIntToInt(pucRecord,2);
	pPara->para.ucParaType=(UINT8)convAsciiIntToInt(&pucRecord[2],2);
	pPara->para.ucRS485MeterType=pPara->para.ucParaType;
	pPara->para.ucRS485Mode=(UINT8)(pucRecord[4]-'0');
	pPara->para.ucRS485DataType=(UINT8)(pucRecord[5]-'0');
	pPara->para.ucRS485Baud=(UINT8)(pucRecord[6]-'0');
	pPara->para.ucRS485Bits=(UINT8)(pucRecord[7]-'0');
	pPara->para.ucRS485Parity=(UINT8)(pucRecord[8]-'0');
	pPara->para.ucRS485Stopbits=(UINT8)(pucRecord[9]-'0');
	pPara->para.uiRS485Timout=convAsciiIntToInt(&pucRecord[10],4);
	for(uiIndex=0;uiIndex<4;uiIndex++)
	{
		ucTemp[uiIndex*2]=pucRecord[20-uiIndex*2];
		ucTemp[uiIndex*2+1]=pucRecord[21-uiIndex*2];
	}
	ucTemp[8]=0;
	ASCIIHexToNumber(ucTemp,2);
	pPara->para.fMultiplyFactor=uConvert.fFloatVar;
	for(uiIndex=0;uiIndex<4;uiIndex++)
	{
		ucTemp[uiIndex*2]=pucRecord[28-uiIndex*2];
		ucTemp[uiIndex*2+1]=pucRecord[29-uiIndex*2];
	}
	ucTemp[8]=0;
	ASCIIHexToNumber(ucTemp,2);
	pPara->para.fAdditionFactor=uConvert.fFloatVar;
	for(uiIndex=0;uiIndex<30;uiIndex++) pPara->para.ucRS485Packet[uiIndex]=0;
	uiPacketLength=(UINT16)(uiSeparator-30);
	if(uiPacketLength>sizeof(pPara->para.ucRS485Packet)-1)
		uiPacketLength=sizeof(pPara->para.ucRS485Packet)-1;
	for(uiIndex=0;uiIndex<uiPacketLength;uiIndex++) pPara->para.ucRS485Packet[uiIndex]=pucRecord[30+uiIndex];
	uiNameLength=(UINT16)(uiLength-uiSeparator-1);
	if(uiNameLength>sizeof(pPara->para.ucParaStr)-1)
		uiNameLength=sizeof(pPara->para.ucParaStr)-1;
	for(uiIndex=0;uiIndex<sizeof(pPara->para.ucParaStr);uiIndex++) pPara->para.ucParaStr[uiIndex]=0;
	for(uiIndex=0;uiIndex<uiNameLength;uiIndex++) pPara->para.ucParaStr[uiIndex]=pucRecord[uiSeparator+1+uiIndex];
	storeParaConfig(uiParaId);
	return SUCCESS;
}

static UINT8 loadParameterBatchFromJson(UINT8 *pucJson, UINT16 uiJsonLength)
{
	UINT8 ucRange[8], ucRecord[256];
	UINT16 uiFrom,uiTo,uiLoaded=0,uiRecordLength;
	UINT32 ulPosition=0,ulParaKey=0,ulLength=0;
	if(getParameterFromJsonString(pucJson,(UINT8*)"FROM",ucRange,sizeof(ucRange))!=SUCCESS)
		return FAILURE;
	uiFrom=convAsciiIntToInt(ucRange,3);
	if(getParameterFromJsonString(pucJson,(UINT8*)"TO",ucRange,sizeof(ucRange))!=SUCCESS)
		return FAILURE;
	uiTo=convAsciiIntToInt(ucRange,3);
	if(uiTo<uiFrom || uiTo>=N0_OF_PARAMETERS ||
	   searchStr(pucJson,(UINT8*)"\"PARA\"",uiJsonLength,&ulParaKey)!=SUCCESS)
		return FAILURE;
	getStrLength((UINT8*)"\"PARA\"",&ulLength);
	ulPosition=ulParaKey+ulLength;
	while(ulPosition<uiJsonLength && pucJson[ulPosition]!='[') ulPosition++;
	if(ulPosition>=uiJsonLength) return FAILURE;
	while(ulPosition<uiJsonLength && uiFrom<=uiTo)
	{
		while(ulPosition<uiJsonLength && pucJson[ulPosition]!='"') ulPosition++;
		if(ulPosition>=uiJsonLength) break;
		ulPosition++;
		uiRecordLength=0;
		while(ulPosition<uiJsonLength && pucJson[ulPosition]!='"' && pucJson[ulPosition]!='\\' && uiRecordLength<sizeof(ucRecord)-1)
			ucRecord[uiRecordLength++]=pucJson[ulPosition++];
		if(ulPosition>=uiJsonLength || pucJson[ulPosition]!='"') return FAILURE;
		ulPosition++;
		ucRecord[uiRecordLength]=0;
		if(loadCompactParameter(ucRecord,uiRecordLength,uiFrom)!=SUCCESS) return FAILURE;
		uiFrom++;
		uiLoaded++;
	}
	if(uiLoaded==0) return FAILURE;
	/* The parameter table is capped at 200 entries, which fits the counter. */
	ucParaCntrParaConfig=(UINT8)uiFrom;
	return SUCCESS;
}

UINT8 ucInitComplete = 0;

UINT8 value[32];

void downloadParaConfig(void)
{
	UINT8 ucFlagRegistration=1,ucStatusF;
	UINT32 ulTempF;
	UINT8 ucParaRemaining;	
	
	
	
	ucParaRemaining=(UINT8)ulSystemNoOfParameters-ucParaCntrParaConfig;
	if(ucModemType==MODEM_TYPE_4G)
	{
		if(ucParaRemaining>10)
			ucParaRemaining=10;
	}
	else
	{
		if(ucParaRemaining>10)
			ucParaRemaining=10;
	}
	glcdWriteStrCallibri(2,25,(UINT8*)"Para Con. ",11,1);
	glcdWriteLongCallibri(60,25,(UINT32)(((F32)ucParaCntrParaConfig*100)/(F32)ulSystemNoOfParameters) ,3,11,1);
	glcdWriteStrCallibri(84,25,(UINT8*)"%",11,1);
	
	{
		UINT8 ucFirstField=1;
		UINT8 ucRange[8];
		jsonBegin(ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE,&ucFirstField);
		jsonAddStrField(ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE,(UINT8*)"TYPE",(UINT8*)"PARA_CONFIG",&ucFirstField);
		jsonAddHexField(ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE,(UINT8*)"SYS_ID",ulSystemID,8,&ucFirstField);
		intToASCIIStr(ucParaCntrParaConfig,3,sizeof(ucRange),ucRange);
		jsonAddStrField(ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE,(UINT8*)"FROM",ucRange,&ucFirstField);
		intToASCIIStr((unsigned long)(ucParaCntrParaConfig+ucParaRemaining-1),3,sizeof(ucRange),ucRange);
		jsonAddStrField(ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE,(UINT8*)"TO",ucRange,&ucFirstField);
		jsonEnd(ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE);
	}
	prepareGPRSTransmitPacket(ucGeneralPurposeBuffer,ucGPRSTransmitBuffer,ucSerAdd,ucSerPort,ucSerPath,ucSerAppParaConfig,GPRS_TRANS_MODE_POST);
	tranferDataToServer(&ucFlagRegistration,ucGPRSTransmitBuffer,ucSerAppParaConfig,GPRS_TRANS_MODE_POST,GPRS_CONNECTION_CLOSE);
	if(ucFlagRegistration==0)
	{
		if(loadParameterBatchFromJson(ucGPRSRecvBuffer,uiGPRSRecvDataCount)==SUCCESS)
		{
			ucStatusF=SUCCESS;
		}
		else
		{
		//search for 1st para -> PARA_NO=0
		ucStatusF = searchStr(ucGPRSRecvBuffer,(UINT8*)"PARA_NO=0",GPRS_REC_BUFFER_SIZE,&ulTempF);
		if (ucStatusF == SUCCESS) 
		{
			getIndivitualParaConfig(&ucGPRSRecvBuffer[ulTempF]);	
		}
		}
		
		ucStatusF = searchStr(ucGPRSRecvBuffer,(UINT8*)"PARA_NO=1",GPRS_REC_BUFFER_SIZE,&ulTempF);
		if (ucStatusF == SUCCESS) 
		{
			getIndivitualParaConfig(&ucGPRSRecvBuffer[ulTempF]);	
		}
		
		ucStatusF = searchStr(ucGPRSRecvBuffer,(UINT8*)"PARA_NO=2",GPRS_REC_BUFFER_SIZE,&ulTempF);
		if (ucStatusF == SUCCESS) 
		{
			getIndivitualParaConfig(&ucGPRSRecvBuffer[ulTempF]);	
		}
		
		
		//1013
		//if(ucParaCntr==ulSystemNoOfParameters)
		if( (ucParaCntrParaConfig>=ulSystemNoOfParameters) )
		{
			ucParaCntrParaConfig=0;
			ucSystemDownloadParaConfig=0;
			writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DL_PARA_CONIG,ucSystemDownloadParaConfig);
			
			glcdWriteStrCallibri(2,25,(UINT8*)"                              ",11,1);
			
			
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r Flash erase");
			chipErase();
			
			ulFlashFileSaveLocation=0;
			writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SAVE_CNTR,ulFlashFileSaveLocation);
			ulFlashFileSentLocation=0;
			writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SENT_CNTR,ulFlashFileSentLocation);
			if(ulSystemNoOfParameters<25)
			{
				uiFlashStorageSize=325;
				uiFlashMaxReadCntr=3000;
				uiFlashMaxWriteCntr=6000;	
			}
			else if(ulSystemNoOfParameters<50)
			{
				uiFlashStorageSize=575;
				uiFlashMaxReadCntr=1700;
				uiFlashMaxWriteCntr=3400;
			}
			else if(ulSystemNoOfParameters<100)
			{
				uiFlashStorageSize=1075;
				uiFlashMaxReadCntr=900;
				uiFlashMaxWriteCntr=1800;
			}
			else
			{
				uiFlashStorageSize=2150;
				uiFlashMaxReadCntr=450;
				uiFlashMaxWriteCntr=900;
			}
			
			
		}
		
		
	}	
}

void getIndivitualParaConfig(UINT8* pucRecData)
{
	UINT8 ucStatusF;
	UINT32 ulTempF;
	ucStatusF = getValueFromBuff(pucRecData, (UINT8*) "$P_ID=", (UINT8*) "#", ucGeneralPurposeBuffer, GENERAL_PURPOSE_BUFF_SIZE);
	if (ucStatusF == SUCCESS) 
	{
	    UARTSendStr(UART_DEBUG, (UINT8*) "\rPara config, Para ID str =");
	    UARTSendStr(UART_DEBUG, ucGeneralPurposeBuffer);

	    ASCIIHexToNumber(ucGeneralPurposeBuffer, 2);
	    ulTempF = uConvert.ulLongVar;
	    
	    
	    if(ulTempF==ucParaCntrParaConfig)
	    {
		UARTSendStr(UART_DEBUG, (UINT8*) ", Valid para received");
		
		decodeParaConfigPacket(pucRecData);
		
		
		ucParaCntrParaConfig++;
			
	    }   
	}	
}


void downloadUnitConfig(void)
{
	UINT8 ucFlagRegistration=1,ucStatusF;
	UINT32 ulTempF;
	UINT32 ulJsonCommandsLength;
	UINT8 ucJsonCommands[512];
	
	glcdWriteStrCallibri(2,25,(UINT8*)"Unit Config",11,1);	
	{
		UINT8 ucFirstField=1;
		jsonBegin(ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE,&ucFirstField);
		jsonAddStrField(ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE,(UINT8*)"TYPE",(UINT8*)"SERVER_DETAILS",&ucFirstField);
		jsonAddHexField(ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE,(UINT8*)"SYSTEM_ID",ulSystemID,8,&ucFirstField);
		jsonEnd(ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE);
	}
	
	
	prepareGPRSTransmitPacket(ucGeneralPurposeBuffer,ucGPRSTransmitBuffer,ucSerAdd,ucSerPort,ucSerPath,ucSerAppSysConfig,GPRS_TRANS_MODE_POST);
	tranferDataToServer(&ucFlagRegistration,ucGPRSTransmitBuffer,ucSerAppSysConfig,GPRS_TRANS_MODE_POST,GPRS_CONNECTION_CLOSE);
	if(ucFlagRegistration==0)
	{
		ucStatusF = getParameterFromJsonString(ucGPRSRecvBuffer,(UINT8*)"SYSTEM_ID",ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE);
		if(ucStatusF==SUCCESS)
		{
			ASCIIHexToNumber(ucGeneralPurposeBuffer,2);
			ulTempF=uConvert.ulLongVar;
			if(ulTempF==ulSystemID && buildSystemConfigCommandsFromJson(ucGPRSRecvBuffer,ucJsonCommands,sizeof(ucJsonCommands))==SUCCESS)
			{
				ucSystemDownloadUnitConfig=0;
				writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DL_UNIT_CONIG,ucSystemDownloadUnitConfig);
				getStrLength(ucJsonCommands,&ulJsonCommandsLength);
				checkBufferForCommands(ucJsonCommands,(UINT16)ulJsonCommandsLength,BUFFER_DONT_CLEAR);
				glcdWriteStrCallibri(2,25,(UINT8*)"                              ",11,1);
				return;
			}
		}
		ucStatusF = getValueFromBuff(ucGPRSRecvBuffer, (UINT8*) "$SysID=", (UINT8*) "#", ucGeneralPurposeBuffer, GENERAL_PURPOSE_BUFF_SIZE);
                if (ucStatusF == SUCCESS) 
		{
                    UARTSendStr(UART_DEBUG, (UINT8*) "\rSystem config, System ID str =");
                    UARTSendStr(UART_DEBUG, ucGeneralPurposeBuffer);

                    ASCIIHexToNumber(ucGeneralPurposeBuffer, 2);
                    ulTempF = uConvert.ulLongVar;
		    
		    if( (ulTempF==ulSystemID))
		    {
                   	 UARTSendStr(UART_DEBUG, (UINT8*) ", Received");
			 ucSystemDownloadUnitConfig=0;
			 writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DL_UNIT_CONIG,ucSystemDownloadUnitConfig);
			 //check to see if all the data is received
			checkBufferForCommands(ucGPRSRecvBuffer,uiGPRSRecvDataCount,BUFFER_DONT_CLEAR);
			
			glcdWriteStrCallibri(2,25,(UINT8*)"                              ",11,1);
			
		    }
              } 		
	}
}


void imieRegistration(void)
{
	UINT8 ucFlagRegistration=1,ucStatusF;
	clearGeneralPurposeBuffer();
	ucGeneralPurposeBuffer[0]=0x00;
	concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"IMIENumber=",GENERAL_PURPOSE_BUFF_SIZE);
	concatStirngsToStr1(ucGeneralPurposeBuffer,ucIMIENumber,GENERAL_PURPOSE_BUFF_SIZE);
	prepareGPRSTransmitPacket(ucGeneralPurposeBuffer,ucGPRSTransmitBuffer,ucSerAdd,ucSerPort,ucSerPath,ucSerAppReg,GPRS_TRANS_MODE_POST);
	
	glcdWriteStrCallibri(2,25,(UINT8*)"Reg. System",11,1);	
	
	tranferDataToServer(&ucFlagRegistration,ucGPRSTransmitBuffer,ucSerAppReg,GPRS_TRANS_MODE_POST,GPRS_CONNECTION_CLOSE);
	if(ucFlagRegistration==0)
	{
		//check to see if all the data is received
		ucStatusF = getValueFromBuff(ucGPRSRecvBuffer, (UINT8*) "$SysID=", (UINT8*) "#", ucGeneralPurposeBuffer, GENERAL_PURPOSE_BUFF_SIZE);
                if (ucStatusF == SUCCESS) 
		{
			UARTSendStr(UART_DEBUG, (UINT8*) "\rRegisterUnit, System ID str =");
			UARTSendStr(UART_DEBUG, ucGeneralPurposeBuffer);

			ASCIIHexToNumber(ucGeneralPurposeBuffer, 2);
			ulSystemID = uConvert.ulLongVar;
			UARTSendStr(UART_DEBUG, (UINT8*) ", ID = ");
			UARTSendLong(UART_DEBUG, ulSystemID, 8);

			if(ulSystemID!=0)
			{
				ucSystemRegistrationStatus=1;
				writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_SYSTEM_REG_STATUS,ucSystemRegistrationStatus);
				writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_SYSTEM_ID,ulSystemID);
				checkBufferForCommands(ucGPRSRecvBuffer,uiGPRSRecvDataCount,BUFFER_DONT_CLEAR);
				
				glcdWriteStrCallibri(2,25,(UINT8*)"                              ",11,1);

			}
		    
                } 
		
		
	}

}

void uploadBoardTestReport(void)
{
	UINT8 ucFlagRegistration=1,ucStatusF;
	clearGeneralPurposeBuffer();
	ucGeneralPurposeBuffer[0]=0x00;
	concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"IMIENumber=",GENERAL_PURPOSE_BUFF_SIZE);
	concatStirngsToStr1(ucGeneralPurposeBuffer,ucIMIENumber,GENERAL_PURPOSE_BUFF_SIZE);
	prepareGPRSTransmitPacket(ucGeneralPurposeBuffer,ucGPRSTransmitBuffer,ucSerAdd,ucSerPort,ucSerPath,(UINT8*)"boardtestpost.aspx",GPRS_TRANS_MODE_POST);
	
	glcdWriteStrCallibri(2,25,(UINT8*)"Reg. System",11,1);	
	
	tranferDataToServer(&ucFlagRegistration,ucGPRSTransmitBuffer,(UINT8*)"boardtestpost.aspx",GPRS_TRANS_MODE_POST,GPRS_CONNECTION_CLOSE);
	if(ucFlagRegistration==0)
	{
		//check to see if all the data is received
		ucStatusF = getValueFromBuff(ucGPRSRecvBuffer, (UINT8*) "$BOARD_ID=", (UINT8*) "#", ucGeneralPurposeBuffer, GENERAL_PURPOSE_BUFF_SIZE);
                if (ucStatusF == SUCCESS) 
		{
			UARTSendStr(UART_DEBUG, (UINT8*) "\rRegisterUnit, board ID str =");
			UARTSendStr(UART_DEBUG, ucGeneralPurposeBuffer);

			ASCIIHexToNumber(ucGeneralPurposeBuffer, 2);
			ulBoardTestID = uConvert.ulLongVar;
			UARTSendStr(UART_DEBUG, (UINT8*) ", ID = ");
			UARTSendLong(UART_DEBUG, ulBoardTestID, 8);

			if(ulBoardTestID!=0)
			{
				ucBoardTestStatus=1;
				ucBoardTestSMFlag=0;
				writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_BOARD_TEST_STATUS,ucBoardTestStatus);
				writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_BOARD_TEST_ID,ulBoardTestID);
				checkBufferForCommands(ucGPRSRecvBuffer,uiGPRSRecvDataCount,BUFFER_DONT_CLEAR);
				writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_BOARD_TEST_DATE,sRTC.ucDate);
				writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_BOARD_TEST_MONTH,sRTC.ucMonth);
				writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_BOARD_TEST_YEAR,sRTC.ucYear);
				
				glcdWriteStrCallibri(2,25,(UINT8*)"                              ",11,1);

			}
		    
                } 
		
		
	}

}


void tranferDataToServer(UINT8* ucFlag,UINT8* ucTransmitBuffer,UINT8* ucApplication,UINT8 ucMode,UINT8 ucClosConState)
{
	gprsTranferDataToServer4G(ucFlag,ucTransmitBuffer,ucApplication,ucMode,ucClosConState);
	
}

UINT8 decodeParaConfigPacket(UINT8* ucTempStr)
{
	UINT8 ucStatusF;
	UINT8 ucParaValueTemp[40];
	UINT32 ulParaID;
	UINT32 ucparaStrLenth=0;
	getStrLength(ucTempStr,&ucparaStrLenth);
	UARTSendStr(UART_DEBUG,(UINT8*)"\r\r Para string length = ");
	UARTSendLong(UART_DEBUG,ucparaStrLenth,5);

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$P_ID=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
                ulParaID=(UINT16)uConvert.ulLongVar;
		sParaConfig[ulParaID].para.uiParaID=(UINT16)uConvert.ulLongVar;
                
	}
	else
		return FAILURE;

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$P_STR=",(UINT8*)"#",sParaConfig[ulParaID].para.ucParaStr,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{

	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$P_SRC=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ucParaSource=uConvert.ucCharVal;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$P_TYPE=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ucParaType=uConvert.ucCharVal;

	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$P_AS=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ucParaAcqTime=(UINT8)uConvert.ulLongVar;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$P_CUMULATE=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ucParaCumulate=uConvert.ucCharVal;

	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$P_RSTAT=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		sParaConfig[ulParaID].para.ucParaResetHrs = ( (ucParaValueTemp[0]-0x30) * 10 ) + ( (ucParaValueTemp[1]-0x30) * 1 );
		sParaConfig[ulParaID].para.ucParaResetMin = ( (ucParaValueTemp[2]-0x30) * 10 ) + ( (ucParaValueTemp[3]-0x30) * 1 );
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$R_MT=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ucRS485MeterType=uConvert.ucCharVal;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$R_MODE=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ucRS485Mode=uConvert.ucCharVal;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$R_DT=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ucRS485DataType=uConvert.ucCharVal;

	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$R_BAUD=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ucRS485Baud=uConvert.ucCharVal;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$R_BITS=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ucRS485Bits=uConvert.ucCharVal;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$R_PARITY=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ucRS485Parity=uConvert.ucCharVal;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$R_SB=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ucRS485Stopbits=uConvert.ucCharVal;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$R_PACKET=",(UINT8*)"#",sParaConfig[ulParaID].para.ucRS485Packet,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{

	}
	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$R_TIMEOUT=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.uiRS485Timout=uConvert.uiIntVar;
	}
	

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$A_ENABLE=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ucAlarmEnable=uConvert.ucCharVal;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$A_TYPE=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ucAlarmType=uConvert.ucCharVal;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$A_ALERT=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ucAlarmAlert=uConvert.ucCharVal;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$A_DELAY=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.ulAlarmDelay=uConvert.ulLongVar;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$A_FROM_TIME=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		for(ucStatusF=0;ucStatusF<4;ucStatusF++)
		{
			sParaConfig[ulParaID].para.ucAlermOnTime[ucStatusF]=ucParaValueTemp[ucStatusF];
		}
	}
	
	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$A_TO_TIME=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		for(ucStatusF=0;ucStatusF<4;ucStatusF++)
		{
			sParaConfig[ulParaID].para.ucAlermOffTime[ucStatusF]=ucParaValueTemp[ucStatusF];
		}
	}
	
	
	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$MIN_VAL=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.fMinValue=uConvert.fFloatVar;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$MAX_VAL=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.fMaxValue=uConvert.fFloatVar;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$MUL=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.fMultiplyFactor=uConvert.fFloatVar;
	}

	ucStatusF=getValueFromBuff(ucTempStr,(UINT8*)"$ADD=",(UINT8*)"#",ucParaValueTemp,ucparaStrLenth);
	if(ucStatusF==SUCCESS)
	{
		ASCIIHexToNumber(ucParaValueTemp,2);
		sParaConfig[ulParaID].para.fAdditionFactor=uConvert.fFloatVar;
	}

        
	displayPraConfig(ulParaID);
	storeParaConfig(ulParaID);

	return SUCCESS;
}


void checkBufferForCommands(UINT8* pucBuffer, UINT16 uiBufferSize, UINT8 ucClearBuffer)
{
	UINT8 ucStatusF = 0;
	UINT8 ucResponseStr[150] = {0};
	UINT8 ucRespStrIndex = 0;
	UINT16 uiTempF=0;
	
	
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$IST=", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		sRTC.ucYear=(UINT8)convAsciiIntToInt(&ucGeneralPurposeBuffer[2],2);
		sRTC.ucMonth=(UINT8)convAsciiIntToInt(&ucGeneralPurposeBuffer[4],2);
		sRTC.ucDate=(UINT8)convAsciiIntToInt(&ucGeneralPurposeBuffer[6],2);
		sRTC.ucHours=(UINT8)convAsciiIntToInt(&ucGeneralPurposeBuffer[8],2);
		sRTC.ucMinutes=(UINT8)convAsciiIntToInt(&ucGeneralPurposeBuffer[10],2);
		sRTC.ucSeconds=(UINT8)convAsciiIntToInt(&ucGeneralPurposeBuffer[12],2);
		
		writeToProcessorRTC();
		//if(ucDebugEnableFlag==DEBUG_ENABLE_ALL)
		
		setRTCModem(sRTC.ucDate,sRTC.ucMonth,sRTC.ucYear,sRTC.ucHours,sRTC.ucMinutes,sRTC.ucSeconds,0);
		storeRTCValue();
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$DEBUG_ALL", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ucDebugEnableFlag=DEBUG_ENABLE_ALL;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> debug all ");
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$DEBUG_NW", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ucDebugNetwork=1;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> debug NEWTORK ");
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$DEBUG_RS485", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ucDebugRS485=1;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> debug rs485 ");
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$DEBUG_NIL", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ucDebugEnableFlag=DEBUG_NIL;
		ucDebugRS485=0;
		ucDebugNetwork=0;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> debug nil ");
	}
	
	
	
	
	
	//1025
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$SET_IMIE=", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		uiTempF=0;
		for(uiTempF=0;uiTempF<15;uiTempF++)
		{
			ucIMIENumber[uiTempF]=ucGeneralPurposeBuffer[uiTempF];
			writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_STORED_IMIE_NUMBER+uiTempF,ucIMIENumber[uiTempF]);
		}
		ucIMIENumber[15]=0x00;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> IMIE number set ");
	}
	
	//1016
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$ENABLE_CSV", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ucEnableCSV=1;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_ENABLE_CSV,ucEnableCSV);
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> ENABLE_CSV ");
	}
	
	//1016
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$DISABLE_CSV", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ucEnableCSV=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_ENABLE_CSV,ucEnableCSV);
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> DISABLE_CSV ");
	}
	
	
	
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$DEBUG_PARA", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		sendJsonParaInfoViaDebug();
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$ACQ_NOW", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ucPraAcqFlag=1;
		ucPraTraFlag=1;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> acq now ");
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$DEBUG_BOARD", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		sendJsonBoardInfoViaDebug();
	}
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$DEBUG_SYSTEM", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		sendJsonSystemInfoViaDebug();
	}

	
	
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$BOARD_RETEST", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 	{
		
		UARTSendStr(UART_DEBUG, (UINT8*) "\r Retesting board");
		ucBoardTestStatus=0;
		ulBoardTestID=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_BOARD_TEST_STATUS,ucBoardTestStatus);
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_BOARD_TEST_ID,ulBoardTestID);
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$TEST_RELAY", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		RELAY_1=REALY_OFF;
		RELAY_2=REALY_OFF;
		RELAY_3 =REALY_OFF;
		delayMs(250);
		RELAY_1=REALY_ON;
		delayMs(250);
		RELAY_1=REALY_OFF;
		delayMs(250);
		RELAY_2=REALY_ON;
		delayMs(250);
		RELAY_2=REALY_OFF;
		delayMs(250);
		RELAY_3=REALY_ON;
		delayMs(250);
		RELAY_3=REALY_OFF;
	}
	
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$BOARD_SAVE_REPORT", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ucBoardTestSMFlag=1;
		UARTSendStr(UART_DEBUG, (UINT8*) "\r Saving board report");
	}
	
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$ACQ_DISABLE", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ucAcqDataDisable=1;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> acq disable");
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$ACQ_ENABLE", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ucAcqDataDisable=0;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> acq enable ");
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$UNIT_CONFIG", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> UNIT_CONFIG");
		ucSystemDownloadUnitConfig=1;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DL_UNIT_CONIG,ucSystemDownloadUnitConfig);
		concatStirngsToStr1(ucResponseStr,(UINT8*)",UNIT_CONFIG",145);
	}

	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$SEND_STATUS", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UINT32 ulTempAf,ulTempBf;
		
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> Send status");
		
		//sms 1 send server details
		ucResponseStr[0]=0x00;
		concatStirngsToStr1(ucResponseStr,(UINT8*)"ID = ",145);
		getStrLength(ucResponseStr,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
		intToASCIIStr(ulSystemID,6,11,&ucResponseStr[ulTempBf]);
		
		concatStirngsToStr1(ucResponseStr,(UINT8*)", server : ",145);
		concatStirngsToStr1(ucResponseStr,ucSerAdd,145);
		concatStirngsToStr1(ucResponseStr,(UINT8*)", ",145);
		concatStirngsToStr1(ucResponseStr,ucSerPath,145);
		concatStirngsToStr1(ucResponseStr,(UINT8*)", ",145);
		concatStirngsToStr1(ucResponseStr,ucSerPort,145);
		concatStirngsToStr1(ucResponseStr,(UINT8*)", ",145);
		concatStirngsToStr1(ucResponseStr,ucSerAppRecData,145);	
		sendInstantSms(ucSenderMobNumberStr,ucResponseStr);
		delayMs(2500);
		
		
		for(ucStatusF=0;ucStatusF<150;ucStatusF++)
			ucResponseStr[ucStatusF]=0x00;
		concatStirngsToStr1(ucResponseStr,(UINT8*)"FS = ",145);
		getStrLength(ucResponseStr,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
		intToASCIIStr(ulFlashFileSaveLocation,4,11,&ucResponseStr[ulTempBf]);
		
		concatStirngsToStr1(ucResponseStr,(UINT8*)" ,FR = ",145);
		getStrLength(ucResponseStr,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
		intToASCIIStr(ulFlashFileSentLocation,4,11,&ucResponseStr[ulTempBf]);
		
		
		concatStirngsToStr1(ucResponseStr,(UINT8*)" ,Max = ",145);
		getStrLength(ucResponseStr,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
		intToASCIIStr(ucGPRSSMStateMax,2,11,&ucResponseStr[ulTempBf]);
		delayMs(2500);
		sendInstantSms(ucSenderMobNumberStr,ucResponseStr);
		delayMs(2500);
		
	}
	

	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$PARA_CONFIG", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> PARA_CONFIG");
		ucSystemDownloadParaConfig=1;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DL_PARA_CONIG,ucSystemDownloadParaConfig);
		concatStirngsToStr1(ucResponseStr,(UINT8*)",PARA_CONFIG",145);
	}	
	
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$RESET_EEPROM", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> resetting eeprom ");
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_EEPROM_MEM_STATUS,'x');
		concatStirngsToStr1(ucResponseStr,(UINT8*)",RESET_EEPROM",145);
		
		sendInstantSms(ucSenderMobNumberStr,(UINT8*) "Resetting eeprom");
		delayMs(1000);
		GPRSSendStr((UINT8*)"AT+CMGF=1\r\n");
		delayMs(1000);
		GPRSSendStr((UINT8*)"AT+CMGDA=\"DEL ALL\"\r\n");
		delayMs(1000);
		
		while(1);
	}

	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$RESET_FLASH", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> resetting flash and cntr ");
		chipErase();
		ulFlashFileSaveLocation=0;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SAVE_CNTR,ulFlashFileSaveLocation);
		ulFlashFileSentLocation=0;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SENT_CNTR,ulFlashFileSentLocation);
		concatStirngsToStr1(ucResponseStr,(UINT8*)",Flash reset",145);
	}
	

	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$RESET_UNIT", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{

		sendInstantSms(ucSenderMobNumberStr,(UINT8*) "Resetting unit");
		delayMs(1000);
		GPRSSendStr((UINT8*)"AT+CMGF=1\r\n");
		delayMs(1000);
		GPRSSendStr((UINT8*)"AT+CMGDA=\"DEL ALL\"\r\n");
		delayMs(1000);

		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> Re-setting unit");
		while(1);
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$NO_OF_PARAMETERS=", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ASCIIHexToNumber(ucGeneralPurposeBuffer, 2);
		ulSystemNoOfParameters = uConvert.ulLongVar;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec-> Parameers = ");
		UARTSendLong(UART_DEBUG, ulSystemNoOfParameters, 8);
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_NO_OF_PARA,ulSystemNoOfParameters);
		//concatStirngsToStr1(ucResponseStr,(UINT8*)",Parameers",145);
	}
	
	//1043
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$MM_ID=", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ASCIIHexToNumber(ucGeneralPurposeBuffer, 2);
		ucMasterModbusID = uConvert.ucCharVal;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec-> MMID = ");
		UARTSendLong(UART_DEBUG, ucMasterModbusID, 4);
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_MODBUS_ID,ucMasterModbusID);
		//concatStirngsToStr1(ucResponseStr,(UINT8*)",Parameers",145);
	}
	
	//1044
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$DEBUG_BR=", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ASCIIHexToNumber(ucGeneralPurposeBuffer, 2);
		ucDebugBaudRate = uConvert.ucCharVal;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec-> DEBUG_BR = ");
		UARTSendLong(UART_DEBUG, ucDebugBaudRate, 4);
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DEBUG_BAUD_RATE,ucDebugBaudRate);
		//concatStirngsToStr1(ucResponseStr,(UINT8*)",Parameers",145);
	}
	
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "TRANS_INT=", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ASCIIHexToNumber(ucGeneralPurposeBuffer, 2);
		ulTransmissionInterwal = uConvert.ulLongVar;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec-> Data tranfer interwal = ");
		UARTSendLong(UART_DEBUG, ulTransmissionInterwal, 8);
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_TRANS_INTERVAL,ulTransmissionInterwal);
		concatStirngsToStr1(ucResponseStr,(UINT8*)",Tra int",145);
	}
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$APP_TYPE=", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ucAppType=(UINT8)convAsciiIntToInt(&ucGeneralPurposeBuffer[0],1);
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_APP_TYPE,ucAppType);
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec-> App type = ");
		UARTSendLong(UART_DEBUG, ucAppType, 2);
	//	concatStirngsToStr1(ucResponseStr,(UINT8*)",App type",145);
	}
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "ACQ_INT=", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		ASCIIHexToNumber(ucGeneralPurposeBuffer, 2);
		ulAcqInterwal = uConvert.ulLongVar;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec-> Data acq interwal = ");
		UARTSendLong(UART_DEBUG, ulAcqInterwal, 8);
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_ACQ_INTERVAL,ulAcqInterwal);
	//	concatStirngsToStr1(ucResponseStr,(UINT8*)",Acq int",145);
	}
	
	
	/* Wi-Fi commands are unsupported in the GPRS-only firmware. */
	
	
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$RESET_INT_CNTR", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> Pulse cntr reset ");
		ulIntCntr=0;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_INT_CNTR,ulIntCntr);
	//	concatStirngsToStr1(ucResponseStr,(UINT8*)",PulseRst",145);
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$MNW_APN=", (UINT8*) "#", ucGPRSAPNStr, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> APN =");
		UARTSendStr(UART_DEBUG, ucGPRSAPNStr);
		storeStringToEeprom(ucGPRSAPNStr,EEPROM_LOC_GPRS_APN,23);
		concatStirngsToStr1(ucResponseStr,(UINT8*)",APN",145);
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$MNW_USN=", (UINT8*) "#", ucGPRSUserNameStr, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> USN =");
		UARTSendStr(UART_DEBUG, ucGPRSUserNameStr);
		storeStringToEeprom(ucGPRSUserNameStr,EEPROM_LOC_GPRS_USERNAME,23);
		concatStirngsToStr1(ucResponseStr,(UINT8*)",USN",145);
	}
	
	//1045
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$M_SPL_COMMAND=", (UINT8*) "#", ucModemSpecialCmd, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UINT32 ulTempAf;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> M_SPL_CMD =");
		UARTSendStr(UART_DEBUG, ucModemSpecialCmd);
					
                getStrLength(ucModemSpecialCmd,&ulTempAf);
		ucModemSpecialCmd[ulTempAf++]=0x0D;
		ucModemSpecialCmd[ulTempAf++]=0x0A;
		ucModemSpecialCmd[ulTempAf++]=0x00;
		storeStringToEeprom(ucModemSpecialCmd,EEPROM_LOC_MODEM_SPL_COMMAND,45);
		concatStirngsToStr1(ucResponseStr,(UINT8*)"SPL CMD",145);
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$MNW_PSW=", (UINT8*) "#", ucGPRSPasswordStr, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> PSW =");
		UARTSendStr(UART_DEBUG, ucGPRSPasswordStr);
		storeStringToEeprom(ucGPRSPasswordStr,EEPROM_LOC_GPRS_PASSWORD,23);
		concatStirngsToStr1(ucResponseStr,(UINT8*)",PSW",145);
	}

	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$SER_ADD=", (UINT8*) "#", ucSerAdd, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> SERVER_ADD =");
		UARTSendStr(UART_DEBUG, ucSerAdd);
		storeStringToEeprom(ucSerAdd,EEPROM_LOC_DATA_SER_ADD,48);
		concatStirngsToStr1(ucResponseStr,(UINT8*)",SER_ADD",145);
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$SER_PORT=", (UINT8*) "#", ucSerPort, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> SERVER_PORT =");
		UARTSendStr(UART_DEBUG, ucSerPort);
		storeStringToEeprom(ucSerPort,EEPROM_LOC_DATA_SER_PORT,8);
		concatStirngsToStr1(ucResponseStr,(UINT8*)",SER_PRT",145);
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$SER_PATH=", (UINT8*) "#", ucSerPath, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> SERVER_PATH =");
		UARTSendStr(UART_DEBUG, ucSerPath);
		storeStringToEeprom(ucSerPath,EEPROM_LOC_DATA_SER_PATH,38);
		concatStirngsToStr1(ucResponseStr,(UINT8*)",SER_PTH",145);
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$SER_APP_REG=", (UINT8*) "#", ucSerAppReg, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> SERVER_APP_REG =");
		UARTSendStr(UART_DEBUG, ucSerAppReg);
		storeStringToEeprom(ucSerAppReg,EEPROM_LOC_SER_APP_REG,38);
		concatStirngsToStr1(ucResponseStr,(UINT8*)",SER_APP_REG",145);
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$SER_APP_SYS=", (UINT8*) "#", ucSerAppSysConfig, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> SERVER_APP_SYS =");
		UARTSendStr(UART_DEBUG, ucSerAppSysConfig);
		storeStringToEeprom(ucSerAppSysConfig,EEPROM_LOC_SER_APP_SYS_CONFIG,38);
	//	concatStirngsToStr1(ucResponseStr,(UINT8*)",SER_APP_SYS",145);
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$SER_APP_PARA=", (UINT8*) "#", ucSerAppParaConfig, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> SERVER_APP_PARA =");
		UARTSendStr(UART_DEBUG, ucSerAppParaConfig);
		storeStringToEeprom(ucSerAppParaConfig,EEPROM_LOC_SER_APP_PARA_CONFIG,38);
	//	concatStirngsToStr1(ucResponseStr,(UINT8*)",SERVER_APP_PARA",145);
	}
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$SER_APP_DATA=", (UINT8*) "#", ucSerAppRecData, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> SERVER_APP_DATA =");
		UARTSendStr(UART_DEBUG, ucSerAppRecData);
		storeStringToEeprom(ucSerAppRecData,EEPROM_LOC_SER_APP_REC_DATA,38);
	//	concatStirngsToStr1(ucResponseStr,(UINT8*)",SERVER_APP_DATA",145);
	}
	
	/* Wi-Fi credentials are no longer accepted or stored. */
	
	
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "RELAY1ON", (UINT8*) "Z", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> relay 1 ON");
		ucRelay1Status=1;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RELAY1_STATUS,ucRelay1Status);
		concatStirngsToStr1(ucResponseStr,(UINT8*)",RLY1 ON",145);
		RELAY_1=1;
	}
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "RELAY1OFF", (UINT8*) "Z", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec -> relay 1 OFF");
		ucRelay1Status=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RELAY1_STATUS,ucRelay1Status);
		concatStirngsToStr1(ucResponseStr,(UINT8*)",RLY1 OFF",145);
		RELAY_1=0;
	}
	
	
	
	
	
	//modbus packet
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$MODBUS_REQ=", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		
		UINT8 ucTempZ=0;
		UINT8 ucMeterType=ucGeneralPurposeBuffer[0]-0x30;
		UINT8 ucMode=ucGeneralPurposeBuffer[1]-0x30;
		UINT8 ucDataType=ucGeneralPurposeBuffer[2]-0x30;
		UINT8 ucBaud=ucGeneralPurposeBuffer[3]-0x30+1;
		UINT8 ucDataBits=ucGeneralPurposeBuffer[4]-0x30;
		UINT8 ucParity=ucGeneralPurposeBuffer[5]-0x30;
		UINT8 ucStopBits=ucGeneralPurposeBuffer[6]-0x30;
		UINT16 uiTimeout=convAsciiIntToInt(&ucGeneralPurposeBuffer[7],4);
		UINT8 ucPkt[50];
		UINT32 ulTempF;
		F32 fValue;
		UINT8 ucResponseStatus;
		
		
		for(ucTempZ=0;ucTempZ<16;ucTempZ++)
		{
			ucPkt[ucTempZ]=ucGeneralPurposeBuffer[11+ucTempZ];
		}
		ucPkt[16]=0x00;
		uiGPMsCntr=0;
		ucTempZ=1;
		
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r processing res485 pkt");
		while( (uiGPMsCntr<7000) && (ucTempZ==1))
		{
			R_WDT_Restart();
			processRS485Packet(&ucTempZ,ucMeterType,ucMode,ucDataType,ucBaud,ucDataBits,ucParity,ucStopBits,uiTimeout,ucPkt,8, &ucResponseStatus, &fValue,1);	
			
		}
		
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r modbus resp, value = ");
		UARTSendFloat(UART_DEBUG,fValue,8,2);
		UARTSendStr(UART_DEBUG,(UINT8 *)", status = ");
		UARTSendLong(UART_DEBUG,ucResponseStatus,2);
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r resp = ");
		displayModusRecBuff();
		
		clearGeneralPurposeBuffer();
		concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"modbus resp, Value =  ",GENERAL_PURPOSE_BUFF_SIZE);
		getStrLength(ucGeneralPurposeBuffer,&ulTempF);//LENGTH OF DATA
		floatToASCIIStr(fValue,5,1,100,&ucGeneralPurposeBuffer[ulTempF]);
		concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)", Status = ",GENERAL_PURPOSE_BUFF_SIZE);
		getStrLength(ucGeneralPurposeBuffer,&ulTempF);//LENGTH OF DATA
		floatToASCIIStr(ucResponseStatus,1,0,100,&ucGeneralPurposeBuffer[ulTempF]);
		
		concatStirngsToStr1(ucResponseStr,ucGeneralPurposeBuffer,145);
	}
	
	
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$DATA_STORE_TIME=", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\rCmd Rec-> Trans Per = ");
		for(ucStatusF=0;ucStatusF<8;ucStatusF++)
		{
			ucTransPeriod[ucStatusF]=ucGeneralPurposeBuffer[ucStatusF];
			writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_TRANS_PERIOD+ucStatusF,ucTransPeriod[ucStatusF]);
			UARTSendChar(UART_DEBUG,ucTransPeriod[ucStatusF]);
		}
		concatStirngsToStr1(ucResponseStr,(UINT8*)",Trans period",145);
	}
	
	
	//1004
	//send command directly to modem
	
	ucStatusF = getValueFromBuff(pucBuffer, (UINT8*) "$4G_CMD=", (UINT8*) "#", ucGeneralPurposeBuffer, uiBufferSize);
	if (ucStatusF == SUCCESS) 
	{
		UARTSendStr(UART_DEBUG, (UINT8*) "\r 4G_CMD rec = ");
		UARTSendStr(UART_DEBUG, ucGeneralPurposeBuffer);
		
		clearGPRSRecBuffer();
		GPRSSendStr(ucGeneralPurposeBuffer);
		GPRSSendStr((UINT8*)"\r\n");
		delayMs(4000);
		displayGPRSRecBufferTillCount();
		
		
	}

	//JSON Request
        if(getParameterFromJsonString(ucDebugRecvBuffer,(UINT8*)"REQ",value,DEBUG_REC_BUFFER_SIZE) == SUCCESS)
	{

			UINT32 lenCmd = 0;
			UINT32 lenValue = 0;
			UINT32 ulSysId;
			UINT8 hexStr[8];
			UINT8 jsonOut[160];
			getStrLength(value, &lenValue);
			
			// ---- REQ: "GET_SYS_INFO" ----
			getStrLength((UINT8*)"GET_SYS_INFO", &lenCmd);
			if(nStrnCmp(value,0,lenValue,(UINT8*)"GET_SYS_INFO",lenCmd) == SUCCESS)
			{
				if (buildGetSysInfoJSONHex(jsonOut, DEBUG_REC_BUFFER_SIZE) == SUCCESS)
				{
					UARTSendStr(UART_DEBUG, (UINT8*)"\r\n");
				        UARTSendStr(UART_DEBUG, (UINT8*)jsonOut);
				        UARTSendStr(UART_DEBUG, (UINT8*)"\r\n");
				}
	 		}
		        // ---- REQ: "PARA_VALUE" ----
			else if (getStrLength((UINT8*)"PARA_VALUE", &lenCmd),nStrnCmp(value, 0, lenValue, (UINT8*)"PARA_VALUE", lenCmd) == SUCCESS)
		        {

					 handleParaValueRequest(pucBuffer);
		        }
	
        }
	
	
	if(ucClearBuffer==BUFFER_CLEAR)
	{
		for(uiTempF=0;uiTempF<uiBufferSize;uiTempF++)
		{
			pucBuffer[uiTempF]=0x00;
		}
	}
	
	concatStirngsToStr1(ucResponseStr,(UINT8*)",SET",145);
	//send sms
	if(ucSMSReceivedFlag==1)
	{
		//storeSMSToEEPROM(ucResponseStr);
		ucSMSReceivedFlag=0;
		sendInstantSms(ucSenderMobNumberStr,ucResponseStr);
	}
	
}
UINT8 getRelayStatusBasedOnValue(F32 fParaValue, F32 fONValue, F32 fOFFValue)
{
	if (fONValue < fRelay1ParaOFF)
	{
	  //  	UARTSendStr(UART_DEBUG, (UINT8*) ", ON < OFF =");
		if (fParaValue <= fONValue) 
		{
		    	return 1;
		}

		if (fParaValue > fOFFValue) 
		{
		    	return  0;
		}
	}
	else 
	{
		if (fParaValue >= fONValue) 
		{
	  	  	return  1;
		}

		if (fParaValue < fOFFValue) 
		{
		    	return  0;
		}
	}
	return 0;
}

/*
#define RELAY_TIME_INVALID 	0
#define RELAY_TIME_TURNON 	1	
#define RELAY_TIME_TURNOFF	2
*/
UINT8 getRelayStatusBasedOnTime(UINT8 ucOneHrs,UINT8 ucOnMin,UINT8 ucOffHrs,UINT8 ucOffMin)
{
	UINT32 ulOnTime,ulOnTime2;
	UINT32 ulOffTime,ulOffTime2;
	UINT32 ulCurrentTime;
	
	if((ucOneHrs==0) && (ucOnMin==0) && (ucOffHrs==0) && (ucOffMin==0) )
	{
		return RELAY_TIME_INVALID;
	}
	
	
	ulOnTime=((UINT16)ucOneHrs*60)+ucOnMin;
	ulOffTime=((UINT16)ucOffHrs*60)+ucOffMin;
	ulCurrentTime = ((UINT16)sRTC.ucHours*60)+sRTC.ucMinutes;
	
	if(ulOnTime<ulOffTime)
	{
		if((ulCurrentTime>=ulOnTime) && (ulCurrentTime<ulOffTime))
		{
		//	UARTSendStr(UART_DEBUG,(UINT8*)"ON");
			return RELAY_TIME_TURNON;
		}	
		else
		{
			
		//	UARTSendStr(UART_DEBUG,(UINT8*)"OFF");
			return RELAY_TIME_TURNOFF;
		}
	}	
	else
	{
		if((ulCurrentTime>=ulOnTime) )
		{
		//	UARTSendStr(UART_DEBUG,(UINT8*)"_ON");
			return RELAY_TIME_TURNON;
		}	
		else if((ulCurrentTime<ulOffTime))
		{
			
		//	UARTSendStr(UART_DEBUG,(UINT8*)"_ON");
			return RELAY_TIME_TURNON;
		}
		else
		{
			
		//	UARTSendStr(UART_DEBUG,(UINT8*)"_OFF");
			return RELAY_TIME_TURNOFF;
		}
	}  
	return 0;
}

void checkRelay1Operation()
{
//	UINT8 ucRelay1Status=5;
//	UINT8 ucRlyTmr1Status=getRelayStatusBasedOnTime(ucRelay1OnHrs1,ucRelay1OnMin1,ucRelay1OffHrs1,ucRelay1OffMin1);
//	UINT8 ucRlyTmr2Status=getRelayStatusBasedOnTime(ucRelay1OnHrs2,ucRelay1OnMin2,ucRelay1OffHrs2,ucRelay1OffMin2);
//	
//	//check if any para is set for relay1 operation
//	if(ucDebugEnableFlag==DEBUG_ENABLE_ALL)
//	UARTSendStr(UART_DEBUG, (UINT8*) "\r Checking relay 1 para : ");
//	if(ucRelay1Parameter!=0)
//	{
//		if(ucDebugEnableFlag==DEBUG_ENABLE_ALL)
//		UARTSendStr(UART_DEBUG, (UINT8*) ", para : ");
//		if(sParaConfig[ucRelay1Parameter].para.ucValueStatusBit==1)
//		{
//			ucRelay1Status=getRelayStatusBasedOnValue(sParaConfig[ucRelay1Parameter].para.value.fValue,fRelay1ParaON,fRelay1ParaOFF);
//		}
//		else
//		{
//			ucRelay1Status = 0;
//		}
//	}
//	//check if timer is enabled
//	else 
//	{
//		
//		if(ucRlyTmr1Status)
//		{
//			if(ucRlyTmr1Status==RELAY_TIME_TURNON)
//			{
//				ucRelay1Status = 1;	
//			}
//			else
//				ucRelay1Status = 0;
//		}
//		
//		if((ucRlyTmr2Status) && (ucRelay3Status==0))
//		{
//			if(ucRlyTmr2Status==RELAY_TIME_TURNON)
//			{
//				ucRelay1Status = 1;	
//			}
//			else
//				ucRelay1Status = 0;
//		
//		}
//	}
//	
//	if(ucRelay1Status!=5)
//	{
//		if(ucDebugEnableFlag==DEBUG_ENABLE_ALL)
//		{
//		UARTSendStr(UART_DEBUG, (UINT8*) " status = ");
//		UARTSendLong(UART_DEBUG,ucRelay1Status,1);
//		}
//		
//		if(ucRelay1Status==0)
//			RELAY_3=0;
//		else
//			RELAY_3=1;
//	}
	
}

void checkRelay2Operation()
{
//	UINT8 ucRelay2Status=5;
//	UINT8 ucRlyTmr1Status=getRelayStatusBasedOnTime(ucRelay2OnHrs1,ucRelay2OnMin1,ucRelay2OffHrs1,ucRelay2OffMin1);
//	UINT8 ucRlyTmr2Status=getRelayStatusBasedOnTime(ucRelay2OnHrs2,ucRelay2OnMin2,ucRelay2OffHrs2,ucRelay2OffMin2);
//	//check if any para is set for Relay2 operation
//	if(ucDebugEnableFlag==DEBUG_ENABLE_ALL)
//	UARTSendStr(UART_DEBUG, (UINT8*) "\r Checking relay 2 para : ");
//	if(ucRelay2Parameter!=0)
//	{
//		UARTSendStr(UART_DEBUG, (UINT8*) ", para : ");
//		if(sParaConfig[ucRelay2Parameter].para.ucValueStatusBit==1)
//		{
//			ucRelay2Status=getRelayStatusBasedOnValue(sParaConfig[ucRelay2Parameter].para.value.fValue,fRelay2ParaON,fRelay2ParaOFF);
//		}
//		else
//		{
//			ucRelay2Status = 0;
//		}
//	}
//	//check if timer is enabled
//	else 
//	{
//		
//		if(ucRlyTmr1Status)
//		{
//			if(ucRlyTmr1Status==RELAY_TIME_TURNON)
//			{
//				ucRelay2Status = 1;	
//			}
//			else
//				ucRelay2Status = 0;
//		}
//		
//		if((ucRlyTmr2Status) && (ucRelay3Status==0))
//		{
//			if(ucRlyTmr2Status==RELAY_TIME_TURNON)
//			{
//				ucRelay2Status = 1;	
//			}
//			else
//				ucRelay2Status = 0;
//		
//		}
//	}
//	
//	if(ucRelay2Status!=5)
//	{
//		if(ucDebugEnableFlag==DEBUG_ENABLE_ALL)
//		{
//			UARTSendStr(UART_DEBUG, (UINT8*) " status = ");
//			UARTSendLong(UART_DEBUG,ucRelay2Status,1);
//		}
//		if(ucRelay2Status==0)
//			RELAY_2=0;
//		else
//			RELAY_2=1;
//	}
}

void checkRelay3Operation()
{
//	UINT8 ucRelay3Status=5;
//	UINT8 ucRlyTmr1Status=getRelayStatusBasedOnTime(ucRelay3OnHrs1,ucRelay3OnMin1,ucRelay3OffHrs1,ucRelay3OffMin1);
//	UINT8 ucRlyTmr2Status=getRelayStatusBasedOnTime(ucRelay3OnHrs2,ucRelay3OnMin2,ucRelay3OffHrs2,ucRelay3OffMin2);
//	//check if any para is set for Relay3 operation
//	if(ucDebugEnableFlag==DEBUG_ENABLE_ALL)
//	UARTSendStr(UART_DEBUG, (UINT8*) "\r Checking relay 3 para : ");
//	if(ucRelay3Parameter!=0)
//	{
//		if(ucDebugEnableFlag==DEBUG_ENABLE_ALL)
//		UARTSendStr(UART_DEBUG, (UINT8*) ", para : ");
//		if(sParaConfig[ucRelay3Parameter].para.ucValueStatusBit==1)
//		{
//			ucRelay3Status=getRelayStatusBasedOnValue(sParaConfig[ucRelay3Parameter].para.value.fValue,fRelay3ParaON,fRelay3ParaOFF);
//		}
//		else
//		{
//			ucRelay3Status = 0;
//		}
//	}
//	//check if timer is enabled
//	else 
//	{
//		
//		if(ucRlyTmr1Status)
//		{
//			if(ucRlyTmr1Status==RELAY_TIME_TURNON)
//			{
//				ucRelay3Status = 1;	
//			}
//			else
//				ucRelay3Status = 0;
//		}
//		
//		if((ucRlyTmr2Status) && (ucRelay3Status==0))
//		{
//			if(ucRlyTmr2Status==RELAY_TIME_TURNON)
//			{
//				ucRelay3Status = 1;	
//			}
//			else
//				ucRelay3Status = 0;
//		
//		}
//	}
//	
//	if(ucRelay3Status!=5)
//	{
//		if(ucDebugEnableFlag==DEBUG_ENABLE_ALL)
//		{
//			UARTSendStr(UART_DEBUG, (UINT8*) " status = ");
//			UARTSendLong(UART_DEBUG,ucRelay3Status,1);
//		}
//		if(ucRelay3Status==0)
//			RELAY_3=0;
//		else
//			RELAY_3=1;
//	}
}


void sendJsonParaInfoViaDebug(void)
{
	UINT8 ucParaCntrF=0;
	UINT8 ucHEXRep[10]="12345678";
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"DATA_TYPE\": \"PARA\",");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"NO_OF_PARA\": \"");
	UARTSendLong(UART_DEBUG,ulSystemNoOfParameters,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"DATA\":[");
	for(ucParaCntrF=0;ucParaCntrF<ulSystemNoOfParameters;ucParaCntrF++)
	{
		UARTSendStr(UART_DEBUG,(UINT8 *)"{");
		
		
		UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
		UARTSendLong(UART_DEBUG,ucParaCntrF,3);
		UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
		
		UARTSendStr(UART_DEBUG,(UINT8 *)"[");
		
		UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
		UARTSendStr(UART_DEBUG,sParaConfig[ucParaCntrF].para.ucParaStr);
		UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
		
		UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
		
		//		UARTSendFloat(UART_DEBUG,sParaConfig[ucParaCntrF].para.value.fValue,8,2);
		floatHexToHexBytes(sParaConfig[ucParaCntrF].para.value.fValue,ucHEXRep);
		UARTSendStr(UART_DEBUG,ucHEXRep);
		UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
		

		UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
		UARTSendLong(UART_DEBUG,sParaConfig[ucParaCntrF].para.ucValueStatusBit,2);
		UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
		
		UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
		UARTSendLong(UART_DEBUG,sParaConfig[ucParaCntrF].para.ucErrorFlag,2);
		UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
		
		
		UARTSendStr(UART_DEBUG,(UINT8 *)"]");
		
		if(ucParaCntrF==(ulSystemNoOfParameters-1))
			UARTSendStr(UART_DEBUG,(UINT8 *)"}");
		else
			UARTSendStr(UART_DEBUG,(UINT8 *)"},");

		
	}
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"}\r\n");
}



void sendJsonPaketString(UINT8* pucParameter, UINT8* pucValue,UINT8 ucSendComma)
{
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,pucParameter);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":\"");
	UARTSendStr(UART_DEBUG,pucValue);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	if(ucSendComma==1)
	UARTSendStr(UART_DEBUG,(UINT8 *)",");
}

void sendJsonPaketFloatHex(UINT8* pucParameter, float fValue,UINT8 ucSendComma)
{
	UINT8 ucHEXRep[10]="12345678";
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,pucParameter);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":\"");
	floatHexToHexBytes(fValue,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	if(ucSendComma==1)
	UARTSendStr(UART_DEBUG,(UINT8 *)",");
}

void sendJsonSystemInfoViaDebug(void)
{
	UINT8 ucParaCntrF=0;
	UINT8 ucHEXRep[10]="12345678";
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"DATA_TYPE\": \"SYSTEM\",");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"NO_OF_PARA\": \"");
	UARTSendLong(UART_DEBUG,13,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"DATA\":[");

	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,0,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"SYS_ID");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(ulSystemID,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,1,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"TRA_INT");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(ulTransmissionInterwal,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,2,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"PENDING_FILE");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(uiPendingFiles,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,3,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"INT_SOURCE");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"GPRS");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,4,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"MN_APN");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,ucGPRSAPNStr);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,5,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"GPRS_STATE");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(ucGPRSSMState,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,6,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"GPRS_STATE_MAX");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(ucGPRSSMStateMax,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,7,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"GPRS_NETWORK");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(ucNetworkStatus,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,8,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"GSM_SIGNAL");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(ucSignalStrength,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,9,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"ACQ_CNTR");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(ulDataAcqCntr,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,10,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"ACQ_INT");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(ulAcqInterwal,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,11,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"FLASH_SAVE_LOC");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(ulFlashFileSaveLocation,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,12,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"FLASH_SENT_LOCATION");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(ulFlashFileSentLocation,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"}");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"}\r\n");
	
}

void sendJsonBoardInfoViaDebug(void)
{
	UINT32 ulTempA,ulTempB;
	UINT8 ucParaCntrF=0;
	UINT8 ucStatusF=0;
	UINT8 ucHEXRep[10]="12345678";
	ucBaordTestInProgress=1;
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"DATA_TYPE\": \"BOARD\",");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"NO_OF_PARA\": \"");
	UARTSendLong(UART_DEBUG,24,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"DATA\":[");

	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,0,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"SUPPLY");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(fSupplyVolts,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,1,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"TEMPERATURE");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(fLM35,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,2,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"V1");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(fV1,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,3,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"V2");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(fV2,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,4,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"V3");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(fV3,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,5,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"V4");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(fV4,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,6,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"I1");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(fI1,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,7,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"I2");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(fI2,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,8,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"INT_PIN_STAT");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes((float)P14.1,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,9,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"INT_FREQ");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(ucSignalStrength,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,10,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"INT_CNTR");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes(ulIntCntr,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,11,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"RELAY_1");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes((F32)RELAY_1,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,12,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"RELAY_2");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes((F32)ucRelay2Status,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,13,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"RELAY_3");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes((F32)ucModemType,ucHEXRep);	//1024
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	//check eeprom
	ucStatusF=readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_EEPROM_MEM_STATUS,&ucBoardMemeoryStatus);
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,14,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"EEPROM");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes((F32)ucBoardMemeoryStatus,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	//check flash
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,15,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"FLASH");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes((F32)ucFlashID,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	//RS485 1
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,16,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"RS485_1");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes((F32)ucUartRS485RecData,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	//RS485
	RS485SendChar(0x01);
	delayMs(1);
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,17,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"RS485_2");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes((F32)ucUartZBRecData,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	//GPRS
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,18,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"GPRS");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes((F32)ucGPRSSMState,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	 
	//keypad
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,19,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"KEYPAD");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendLong(UART_DEBUG,uckeyPreTestUP,1);
	UARTSendLong(UART_DEBUG,uckeyPreTestDOWN,1);
	UARTSendLong(UART_DEBUG,uckeyPreTestRIGHT,1);
	UARTSendLong(UART_DEBUG,uckeyPreTestLEFT,1);
	UARTSendLong(UART_DEBUG,uckeyPreTestENTER,1);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	//imie
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,20,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"IMEI");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	for(ucStatusF=0;ucStatusF<15;ucStatusF++)
		UARTSendChar(UART_DEBUG,ucIMIENumber[ucStatusF]);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	
	//board id
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,21,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"BOARD_ID");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes((F32)ulBoardTestID,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	//board id
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,22,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"BOARD_TEST");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes((F32)ucBoardTestStatus,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"},");
	
	//boardtest storeSM
	UARTSendStr(UART_DEBUG,(UINT8 *)"{");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"PARA");
	UARTSendLong(UART_DEBUG,23,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\":");
	UARTSendStr(UART_DEBUG,(UINT8 *)"[");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"BOARD_TEST_SM");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\",");
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	floatHexToHexBytes((F32)ucBoardTestSMFlag,ucHEXRep);
	UARTSendStr(UART_DEBUG,ucHEXRep);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\"");
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"}");
	
	
	
	UARTSendStr(UART_DEBUG,(UINT8 *)"]");
	UARTSendStr(UART_DEBUG,(UINT8 *)"}\r\n");
	
}

UINT8 buildGetSysInfoJSONHex(UINT8 *pucJsonBuf, UINT16 uiMaxLen)
{
    unsigned long ulSysId      = 0;
    unsigned long ulNoOfPara   = 0;
    unsigned long ulSave       = 0;
    unsigned long ulSent       = 0;
    unsigned long ulPending    = 0;
    unsigned char ucMode       = 0;

    UINT8 ucFirstField = 0;

    if (uiMaxLen == 0)
        return FAILURE;

    // -------- Read values directly from EEPROM --------
    readExtEEPROMLong(EEPROM_ADD, EEPROM_LOC_SYSTEM_ID,        &ulSysId);
    readExtEEPROMLong(EEPROM_ADD, EEPROM_LOC_NO_OF_PARA,       &ulNoOfPara);
    readExtEEPROMLong(EEPROM_ADD, EEPROM_LOC_FLASH_SAVE_CNTR,  &ulSave);
    readExtEEPROMLong(EEPROM_ADD, EEPROM_LOC_FLASH_SENT_CNTR,  &ulSent);
    readExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_INTERNET_SOURCE,  &ucMode);

    if (ulNoOfPara > 200) ulNoOfPara = 0;
    if ((ulSave > 5000) || (ulSent > 5000))
    {
        ulSave = 0;
        ulSent = 0;
    }
    ulPending = (ulSave >= ulSent) ? (ulSave - ulSent) : 0;

    // -------- Generic JSON build --------
    if (jsonBegin(pucJsonBuf, uiMaxLen, &ucFirstField) == FAILURE)
        return FAILURE;

    if (jsonAddHexField(pucJsonBuf, uiMaxLen, (UINT8*)"SYS_ID",        ulSysId,    6, &ucFirstField) == FAILURE)
        return FAILURE;
    if (jsonAddHexField(pucJsonBuf, uiMaxLen, (UINT8*)"NO_OF_PARA",    ulNoOfPara, 4, &ucFirstField) == FAILURE)
        return FAILURE;
    if (jsonAddHexField(pucJsonBuf, uiMaxLen, (UINT8*)"PENDING_FILES", ulPending,  4, &ucFirstField) == FAILURE)
        return FAILURE;
    if (jsonAddHexField(pucJsonBuf, uiMaxLen, (UINT8*)"MODE",          (unsigned long)ucMode, 1, &ucFirstField) == FAILURE)
        return FAILURE;

    if (jsonEnd(pucJsonBuf, uiMaxLen) == FAILURE)
        return FAILURE;

    return SUCCESS;
}


void handleParaValueRequest(UINT8 *pucBuffer)
{
    UINT8  startStr[16];
    UINT8  endStr[16];

    UINT16 startParaId;
    UINT16 endParaId;
    UINT16 reqParaId;

    UINT16 i;
    UINT32 ulValue;
    UINT8  found;

    UINT8  jsonOut[2048];
    UINT8  tempStr[16];     // enough for "NOT_FOUND" or 8 HEX
    UINT8  firstVal = 1;

    jsonOut[0] = 0x00;

    /* -------- Get START (Para ID) -------- */
    if (getParameterFromJsonString(pucBuffer,
                                   (UINT8*)"START",
                                   startStr,
                                   sizeof(startStr)) == FAILURE)
        return;

    /* -------- Get SIZE (END Para ID) -------- */
    if (getParameterFromJsonString(pucBuffer,
                                   (UINT8*)"SIZE",
                                   endStr,
                                   sizeof(endStr)) == FAILURE)
        return;

    /* -------- Convert START -------- */
    if (ASCIIHexToNumber(startStr, 2) != SUCCESS)
        return;
    startParaId = (UINT16)uConvert.ulLongVar;

    /* -------- Convert END -------- */
    if (ASCIIHexToNumber(endStr, 2) != SUCCESS)
        return;
    endParaId = (UINT16)uConvert.ulLongVar;

    if (endParaId < startParaId)
        return;

    /* -------- JSON Header -------- */
    if (concatStirngsToStr1(jsonOut, (UINT8*)"{\"PARA_NO\":\"",
                            sizeof(jsonOut)) == FAILURE)
        return;

    if (concatStirngsToStr1(jsonOut, startStr,
                            sizeof(jsonOut)) == FAILURE)
        return;

    if (concatStirngsToStr1(jsonOut, (UINT8*)"\",\"SIZE\":\"",
                            sizeof(jsonOut)) == FAILURE)
        return;

    if (concatStirngsToStr1(jsonOut, endStr,
                            sizeof(jsonOut)) == FAILURE)
        return;

    if (concatStirngsToStr1(jsonOut, (UINT8*)"\",\"VALUES\":[",
                            sizeof(jsonOut)) == FAILURE)
        return;

    /* -------- PARA ID RANGE LOOP -------- */
    for (reqParaId = startParaId; reqParaId <= endParaId; reqParaId++)
    {
        found = 0;
        ulValue = 0;

        /* Search parameter table */
        for (i = 0; i < N0_OF_PARAMETERS; i++)
        {
            if (sParaConfig[i].para.uiParaID == reqParaId)
            {
                ulValue = sParaConfig[i].para.value.ulValue;
                found = 1;
                break;
            }
        }

        if (!firstVal)
        {
            if (concatStirngsToStr1(jsonOut, (UINT8*)",",
                                    sizeof(jsonOut)) == FAILURE)
                return;
        }
        firstVal = 0;

        if (concatStirngsToStr1(jsonOut, (UINT8*)"\"",
                                sizeof(jsonOut)) == FAILURE)
            return;

        if (found)
        {
            /* Convert value to HEX */
            if (uLongToHexAsciiStr(ulValue, 8, tempStr) != SUCCESS)
            {
                tempStr[0] = '0';
                tempStr[1] = '0';
                tempStr[2] = '0';
                tempStr[3] = '0';
                tempStr[4] = '0';
                tempStr[5] = '0';
                tempStr[6] = '0';
                tempStr[7] = '0';
                tempStr[8] = '\0';
            }

            if (concatStirngsToStr1(jsonOut, tempStr,
                                    sizeof(jsonOut)) == FAILURE)
                return;
        }
        else
        {
            /* NOT FOUND case */
            if (concatStirngsToStr1(jsonOut, (UINT8*)"NOT_FOUND",
                                    sizeof(jsonOut)) == FAILURE)
                return;
        }

        if (concatStirngsToStr1(jsonOut, (UINT8*)"\"",
                                sizeof(jsonOut)) == FAILURE)
            return;
    }

    /* -------- Close JSON -------- */
    if (concatStirngsToStr1(jsonOut, (UINT8*)"]}",
                            sizeof(jsonOut)) == FAILURE)
        return;

    UARTSendStr(UART_DEBUG, jsonOut);
    UARTSendStr(UART_DEBUG, (UINT8*)"\r\n");
}
