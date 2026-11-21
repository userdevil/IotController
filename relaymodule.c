#include "relaymodule.h"
#include "controller.h"
#include "data.h"
#include "memory.h"
#include "gprs.h"
#include "uartGPRS.h"
#include "uartRS485.h"
#include "modbus.h"
#include "eeprom.h"
#include "uart.h"
#include "generalfunctions.h"

#define RELAY_EEPROM_BASE 23500
#define RELAY_EEPROM_VALID 0xA9
#define RELAY_TOPIC_SIZE 96
#define RELAY_CREDENTIAL_SIZE 64
#define RELAY_CLIENT_SIZE 48

extern UINT8 ucGPRSRecvBuffer[];
extern UINT16 uiGPRSRecvDataCount;
extern UINT8 ucAcqDataDisable;
extern UINT8 ucPraAcqFlag;

static UINT8 ucRelayModuleCount;
static UINT8 ucRelaySlaveIds[RELAY_MODULE_MAX_COUNT];
static UINT16 uiRelayRegisters[RELAY_MODULE_MAX_COUNT][RELAY_MODULE_CHANNEL_COUNT];
static UINT8 ucRelayWriteFunctions[RELAY_MODULE_MAX_COUNT];
static UINT16 uiRelayOnValues[RELAY_MODULE_MAX_COUNT];
static UINT16 uiRelayOffValues[RELAY_MODULE_MAX_COUNT];
static UINT8 ucMqttHost[RELAY_CREDENTIAL_SIZE];
static UINT8 ucMqttPort[8];
static UINT8 ucMqttClientId[RELAY_CLIENT_SIZE];
static UINT8 ucMqttUsername[RELAY_CREDENTIAL_SIZE];
static UINT8 ucMqttPassword[RELAY_CREDENTIAL_SIZE];
static UINT8 ucMqttCommandTopic[RELAY_TOPIC_SIZE];
static UINT8 ucMqttStatusTopic[RELAY_TOPIC_SIZE];
static UINT8 ucRelayModuleReady;
static UINT8 ucMqttState;
static UINT8 ucMqttCmdFlag;
static UINT8 ucModbusFlag;
static UINT8 ucPendingRelayWrite;
static UINT8 ucPendingPublish;
static UINT8 ucPublishState;
static UINT8 ucPublishFlag;
static UINT16 uiPublishLength;
static UINT8 ucRelayWriteResult;
static UINT8 ucPendingSlave;
static UINT8 ucPendingChannel;
static UINT8 ucPendingState;
static UINT8 ucSavedAcqDisable;
static UINT8 ucPendingCommandId[32];
static UINT8 ucPendingTopic[RELAY_TOPIC_SIZE];
static UINT8 ucMqttCommand[256];
static UINT8 ucAtCommand[256];
static UINT8 ucRelayModbusFrame[8];
static UINT8 ucRelayResponseStatus;
static F32 fRelayResponseValue;

static UINT8 relayCopyJsonString(UINT8 *pucJson, UINT8 *pucKey, UINT8 *pucDest, UINT8 ucMax)
{
	UINT16 uiAt=0;
	UINT16 uiKey=0;
	UINT16 uiJson=0;
	UINT16 uiLen=0;
	while(pucKey[uiKey] != 0) uiKey++;
	while(pucJson[uiJson] != 0) uiJson++;
	for(uiAt=0; uiAt+uiKey+2<uiJson; uiAt++)
	{
		UINT16 uiK;
		if(pucJson[uiAt] != '"') continue;
		for(uiK=0; uiK<uiKey && pucJson[uiAt+1+uiK]==pucKey[uiK]; uiK++);
		if(uiK==uiKey && pucJson[uiAt+uiKey+1]=='"')
		{
			uiAt=(UINT16)(uiAt+uiKey+2);
			while(uiAt<uiJson && pucJson[uiAt]!=':') uiAt++;
			if(uiAt>=uiJson) return FAILURE;
			uiAt++;
			while(uiAt<uiJson && (pucJson[uiAt]==' ' || pucJson[uiAt]=='\t')) uiAt++;
			if(pucJson[uiAt++]!='"') return FAILURE;
			while(uiAt<uiJson && pucJson[uiAt]!='"' && uiLen+1<ucMax)
				pucDest[uiLen++]=pucJson[uiAt++];
			if(pucJson[uiAt]!='"') return FAILURE;
			pucDest[uiLen]=0;
			return SUCCESS;
		}
	}
	return FAILURE;
}

static UINT8 relayCopyJsonNumber(UINT8 *pucJson, UINT8 *pucKey, UINT16 *puiValue)
{
	UINT16 uiAt=0,uiKey=0,uiJson=0,uiValue=0;
	UINT8 ucFound=0;
	while(pucKey[uiKey]!=0) uiKey++;
	while(pucJson[uiJson]!=0) uiJson++;
	for(uiAt=0;uiAt+uiKey+2<uiJson;uiAt++)
	{
		UINT16 uiK;
		if(pucJson[uiAt]!='"') continue;
		for(uiK=0;uiK<uiKey && pucJson[uiAt+1+uiK]==pucKey[uiK];uiK++);
		if(uiK==uiKey && pucJson[uiAt+uiKey+1]=='"')
		{
			uiAt=(UINT16)(uiAt+uiKey+2);
			while(uiAt<uiJson && pucJson[uiAt]!=':')uiAt++;
			if(uiAt>=uiJson)return FAILURE;
			uiAt++;
			while(uiAt<uiJson && (pucJson[uiAt]==' ' || pucJson[uiAt]=='\t' || pucJson[uiAt]=='\r' || pucJson[uiAt]=='\n'))uiAt++;
			if(pucJson[uiAt]=='"')uiAt++;
			while(uiAt<uiJson && pucJson[uiAt]>='0' && pucJson[uiAt]<='9')
			{
				UINT8 ucDigit=(UINT8)(pucJson[uiAt]-'0');
				if(uiValue>6553 || (uiValue==6553 && ucDigit>5))return FAILURE;
				ucFound=1;
				uiValue=(UINT16)(uiValue*10+ucDigit);
				uiAt++;
			}
			if(ucFound){*puiValue=uiValue;return SUCCESS;}
			return FAILURE;
		}
	}
	return FAILURE;
}

static UINT8 relayIsSafeATValue(UINT8 *pucValue)
{
	UINT16 uiI=0;
	while(pucValue[uiI]!=0)
	{
		if(pucValue[uiI]=='"' || pucValue[uiI]=='\\' || pucValue[uiI]=='\r' || pucValue[uiI]=='\n')return FAILURE;
		uiI++;
	}
	return SUCCESS;
}

static UINT8 relayStoreConfig(void)
{
	UINT8 ucI,ucValue;
	UINT16 uiAddress=RELAY_EEPROM_BASE;
	writeExtEEPROMChar(EEPROM_ADD,uiAddress++,RELAY_EEPROM_VALID);
	writeExtEEPROMChar(EEPROM_ADD,uiAddress++,ucRelayModuleCount);
	for(ucI=0;ucI<RELAY_MODULE_MAX_COUNT;ucI++)
	{
		UINT8 ucChannel;
		writeExtEEPROMChar(EEPROM_ADD,uiAddress++,ucRelaySlaveIds[ucI]);
		writeExtEEPROMChar(EEPROM_ADD,uiAddress++,ucRelayWriteFunctions[ucI]);
		writeExtEEPROMChar(EEPROM_ADD,uiAddress++,(UINT8)(uiRelayOnValues[ucI]>>8));
		writeExtEEPROMChar(EEPROM_ADD,uiAddress++,(UINT8)uiRelayOnValues[ucI]);
		writeExtEEPROMChar(EEPROM_ADD,uiAddress++,(UINT8)(uiRelayOffValues[ucI]>>8));
		writeExtEEPROMChar(EEPROM_ADD,uiAddress++,(UINT8)uiRelayOffValues[ucI]);
		for(ucChannel=0;ucChannel<RELAY_MODULE_CHANNEL_COUNT;ucChannel++)
		{
			writeExtEEPROMChar(EEPROM_ADD,uiAddress++,(UINT8)(uiRelayRegisters[ucI][ucChannel]>>8));
			writeExtEEPROMChar(EEPROM_ADD,uiAddress++,(UINT8)uiRelayRegisters[ucI][ucChannel]);
		}
	}
	{
		UINT8 *pucArrays[7];
		UINT8 ucLimits[7];
		pucArrays[0]=ucMqttHost; ucLimits[0]=RELAY_CREDENTIAL_SIZE;
		pucArrays[1]=ucMqttPort; ucLimits[1]=8;
		pucArrays[2]=ucMqttClientId; ucLimits[2]=RELAY_CLIENT_SIZE;
		pucArrays[3]=ucMqttUsername; ucLimits[3]=RELAY_CREDENTIAL_SIZE;
		pucArrays[4]=ucMqttPassword; ucLimits[4]=RELAY_CREDENTIAL_SIZE;
		pucArrays[5]=ucMqttCommandTopic; ucLimits[5]=RELAY_TOPIC_SIZE;
		pucArrays[6]=ucMqttStatusTopic; ucLimits[6]=RELAY_TOPIC_SIZE;
		for(ucI=0;ucI<7;ucI++)
		{
			for(ucValue=0;ucValue<ucLimits[ucI];ucValue++)
				writeExtEEPROMChar(EEPROM_ADD,uiAddress++,pucArrays[ucI][ucValue]);
		}
	}
	return SUCCESS;
}

static UINT8 relayLoadConfig(void)
{
	UINT8 ucI,ucValue,ucValid;
	UINT16 uiAddress=RELAY_EEPROM_BASE;
	readExtEEPROMChar(EEPROM_ADD,uiAddress++,&ucValid);
	if(ucValid!=RELAY_EEPROM_VALID)return FAILURE;
	readExtEEPROMChar(EEPROM_ADD,uiAddress++,&ucRelayModuleCount);
	if(ucRelayModuleCount>RELAY_MODULE_MAX_COUNT)return FAILURE;
	for(ucI=0;ucI<RELAY_MODULE_MAX_COUNT;ucI++)
	{
		UINT8 ucChannel;
		readExtEEPROMChar(EEPROM_ADD,uiAddress++,&ucRelaySlaveIds[ucI]);
		readExtEEPROMChar(EEPROM_ADD,uiAddress++,&ucRelayWriteFunctions[ucI]);
		readExtEEPROMChar(EEPROM_ADD,uiAddress++,&ucValue);uiRelayOnValues[ucI]=(UINT16)ucValue<<8;
		readExtEEPROMChar(EEPROM_ADD,uiAddress++,&ucValue);uiRelayOnValues[ucI]|=ucValue;
		readExtEEPROMChar(EEPROM_ADD,uiAddress++,&ucValue);uiRelayOffValues[ucI]=(UINT16)ucValue<<8;
		readExtEEPROMChar(EEPROM_ADD,uiAddress++,&ucValue);uiRelayOffValues[ucI]|=ucValue;
		for(ucChannel=0;ucChannel<RELAY_MODULE_CHANNEL_COUNT;ucChannel++)
		{
			readExtEEPROMChar(EEPROM_ADD,uiAddress++,&ucValue);uiRelayRegisters[ucI][ucChannel]=(UINT16)ucValue<<8;
			readExtEEPROMChar(EEPROM_ADD,uiAddress++,&ucValue);uiRelayRegisters[ucI][ucChannel]|=ucValue;
		}
	}
	{
		UINT8 *pucArrays[7];
		UINT8 ucLimits[7];
		pucArrays[0]=ucMqttHost; ucLimits[0]=RELAY_CREDENTIAL_SIZE;
		pucArrays[1]=ucMqttPort; ucLimits[1]=8;
		pucArrays[2]=ucMqttClientId; ucLimits[2]=RELAY_CLIENT_SIZE;
		pucArrays[3]=ucMqttUsername; ucLimits[3]=RELAY_CREDENTIAL_SIZE;
		pucArrays[4]=ucMqttPassword; ucLimits[4]=RELAY_CREDENTIAL_SIZE;
		pucArrays[5]=ucMqttCommandTopic; ucLimits[5]=RELAY_TOPIC_SIZE;
		pucArrays[6]=ucMqttStatusTopic; ucLimits[6]=RELAY_TOPIC_SIZE;
		for(ucI=0;ucI<7;ucI++)
			for(ucValue=0;ucValue<ucLimits[ucI];ucValue++)
				readExtEEPROMChar(EEPROM_ADD,uiAddress++,&pucArrays[ucI][ucValue]);
	}
	return SUCCESS;
}

static UINT8 relayParseChannelRegisters(UINT8 *pucObject, UINT16 *puiRegisters)
{
	UINT16 uiAt=0,uiObjectStart,uiObjectEnd,uiCount=0,uiChannel,uiRegister;
	UINT8 ucDepth,ucI,ucSeen[RELAY_MODULE_CHANNEL_COUNT];
	UINT8 ucRelayObject[96];
	for(ucI=0;ucI<RELAY_MODULE_CHANNEL_COUNT;ucI++)ucSeen[ucI]=0;
	while(pucObject[uiAt]!=0 && !(pucObject[uiAt]=='"' && pucObject[uiAt+1]=='r' && pucObject[uiAt+2]=='e' && pucObject[uiAt+3]=='l' && pucObject[uiAt+4]=='a' && pucObject[uiAt+5]=='y' && pucObject[uiAt+6]=='s' && pucObject[uiAt+7]=='"'))uiAt++;
	if(pucObject[uiAt]==0)return FAILURE;
	while(pucObject[uiAt]!=0 && pucObject[uiAt]!='[')uiAt++;
	if(pucObject[uiAt++]!='[')return FAILURE;
	while(pucObject[uiAt]!=0 && pucObject[uiAt]!=']')
	{
		if(pucObject[uiAt]=='{')
		{
			uiObjectStart=uiAt;ucDepth=1;uiAt++;
			while(pucObject[uiAt]!=0 && ucDepth)
			{
				if(pucObject[uiAt]=='{')ucDepth++;
				else if(pucObject[uiAt]=='}')ucDepth--;
				uiAt++;
			}
			if(ucDepth || uiAt-uiObjectStart>=sizeof(ucRelayObject))return FAILURE;
			uiObjectEnd=uiAt;
			for(ucI=0;ucI<sizeof(ucRelayObject);ucI++)ucRelayObject[ucI]=0;
			for(uiRegister=0;uiRegister<uiObjectEnd-uiObjectStart;uiRegister++)ucRelayObject[uiRegister]=pucObject[uiObjectStart+uiRegister];
			if(relayCopyJsonNumber(ucRelayObject,(UINT8*)"channel",&uiChannel)!=SUCCESS || uiChannel<1 || uiChannel>RELAY_MODULE_CHANNEL_COUNT)return FAILURE;
			if(relayCopyJsonNumber(ucRelayObject,(UINT8*)"register",&uiRegister)!=SUCCESS)return FAILURE;
			if(ucSeen[uiChannel-1])return FAILURE;
			ucSeen[uiChannel-1]=1;puiRegisters[uiChannel-1]=uiRegister;uiCount++;
		}
		else uiAt++;
	}
	if(pucObject[uiAt]!=']' || uiCount!=RELAY_MODULE_CHANNEL_COUNT)return FAILURE;
	return SUCCESS;
}

static UINT8 relayParseModules(UINT8 *pucJson)
{
	UINT16 uiJson=0,uiStart=0,uiEnd=0,uiCount=0,uiId,uiChannels,uiFunction,uiOnValue,uiOffValue;
	UINT8 ucDepth=0,ucI;
	UINT8 ucObject[500];
	while(pucJson[uiJson]!=0 && !(pucJson[uiJson]=='"' && pucJson[uiJson+1]=='M' && pucJson[uiJson+2]=='O' && pucJson[uiJson+3]=='D' && pucJson[uiJson+4]=='U' && pucJson[uiJson+5]=='L' && pucJson[uiJson+6]=='E' && pucJson[uiJson+7]=='S'))uiJson++;
	if(pucJson[uiJson]==0)return FAILURE;
	while(pucJson[uiJson]!=0 && pucJson[uiJson]!='[')uiJson++;
	if(pucJson[uiJson++]!='[')return FAILURE;
	while(pucJson[uiJson]!=0 && pucJson[uiJson]!=']')
	{
		if(pucJson[uiJson]=='{')
		{
			uiStart=uiJson;ucDepth=1;uiJson++;
			while(pucJson[uiJson]!=0 && ucDepth)
			{
				if(pucJson[uiJson]=='{')ucDepth++;
				else if(pucJson[uiJson]=='}')ucDepth--;
				uiJson++;
			}
			if(ucDepth || uiJson-uiStart>=sizeof(ucObject))return FAILURE;
			uiEnd=uiJson;
			for(ucI=0;ucI<sizeof(ucObject);ucI++)ucObject[ucI]=0;
			for(uiId=0;uiId<uiEnd-uiStart;uiId++)ucObject[uiId]=pucJson[uiStart+uiId];
			if(relayCopyJsonNumber(ucObject,(UINT8*)"slaveId",&uiId)!=SUCCESS || uiId<1 || uiId>247)return FAILURE;
			if(relayCopyJsonNumber(ucObject,(UINT8*)"channels",&uiChannels)!=SUCCESS || uiChannels!=RELAY_MODULE_CHANNEL_COUNT)return FAILURE;
			if(relayCopyJsonNumber(ucObject,(UINT8*)"writeFunction",&uiFunction)!=SUCCESS || uiFunction!=5)return FAILURE;
			if(relayCopyJsonNumber(ucObject,(UINT8*)"onValue",&uiOnValue)!=SUCCESS)return FAILURE;
			if(relayCopyJsonNumber(ucObject,(UINT8*)"offValue",&uiOffValue)!=SUCCESS)return FAILURE;
			if(uiOnValue!=65280 || uiOffValue!=0)return FAILURE;
			if(uiCount>=RELAY_MODULE_MAX_COUNT)return FAILURE;
			if(relayParseChannelRegisters(ucObject,uiRelayRegisters[uiCount])!=SUCCESS)return FAILURE;
			ucRelaySlaveIds[uiCount]=(UINT8)uiId;
			ucRelayWriteFunctions[uiCount]=(UINT8)uiFunction;
			uiRelayOnValues[uiCount]=uiOnValue;
			uiRelayOffValues[uiCount]=uiOffValue;
			uiCount++;
		}
		else uiJson++;
	}
	if(pucJson[uiJson]!=']')return FAILURE;
	ucRelayModuleCount=(UINT8)uiCount;
	return SUCCESS;
}

static UINT8 relayReadResponse(void)
{
	UINT8 ucValid[12];
	UINT16 uiPort;
	UINT8 ucI;
	if(relayCopyJsonString(ucGPRSRecvBuffer,(UINT8*)"SYSTEM_ID",ucValid,sizeof(ucValid))!=SUCCESS)return FAILURE;
	ASCIIHexToNumber(ucValid,2);
	if(uConvert.ulLongVar!=ulSystemID)return FAILURE;
	if(relayCopyJsonString(ucGPRSRecvBuffer,(UINT8*)"MQTT_BROKER",ucMqttHost,sizeof(ucMqttHost))!=SUCCESS)return FAILURE;
	if(relayCopyJsonNumber(ucGPRSRecvBuffer,(UINT8*)"MQTT_PORT",&uiPort)!=SUCCESS || uiPort==0 || uiPort>65535)return FAILURE;
	{
		UINT8 ucDigits=1;
		UINT16 uiTemp=uiPort;
		while(uiTemp>=10){uiTemp=(UINT16)(uiTemp/10);ucDigits++;}
		intToASCIIStr(uiPort,ucDigits,sizeof(ucMqttPort),ucMqttPort);
	}
	if(relayCopyJsonString(ucGPRSRecvBuffer,(UINT8*)"MQTT_CLIENT_ID",ucMqttClientId,sizeof(ucMqttClientId))!=SUCCESS)return FAILURE;
	if(relayCopyJsonString(ucGPRSRecvBuffer,(UINT8*)"MQTT_USERNAME",ucMqttUsername,sizeof(ucMqttUsername))!=SUCCESS)return FAILURE;
	if(relayCopyJsonString(ucGPRSRecvBuffer,(UINT8*)"MQTT_PASSWORD",ucMqttPassword,sizeof(ucMqttPassword))!=SUCCESS)return FAILURE;
	if(relayCopyJsonString(ucGPRSRecvBuffer,(UINT8*)"MQTT_COMMAND_TOPIC",ucMqttCommandTopic,sizeof(ucMqttCommandTopic))!=SUCCESS)return FAILURE;
	if(relayCopyJsonString(ucGPRSRecvBuffer,(UINT8*)"MQTT_STATUS_TOPIC",ucMqttStatusTopic,sizeof(ucMqttStatusTopic))!=SUCCESS)return FAILURE;
	if(relayIsSafeATValue(ucMqttHost)!=SUCCESS || relayIsSafeATValue(ucMqttClientId)!=SUCCESS ||
	   relayIsSafeATValue(ucMqttUsername)!=SUCCESS || relayIsSafeATValue(ucMqttPassword)!=SUCCESS ||
	   relayIsSafeATValue(ucMqttCommandTopic)!=SUCCESS || relayIsSafeATValue(ucMqttStatusTopic)!=SUCCESS)return FAILURE;
	if(relayParseModules(ucGPRSRecvBuffer)!=SUCCESS)return FAILURE;
	for(ucI=0;ucI<ucRelayModuleCount;ucI++)
	{
		UINT8 ucJ;
		for(ucJ=ucI+1;ucJ<ucRelayModuleCount;ucJ++)
			if(ucRelaySlaveIds[ucI]==ucRelaySlaveIds[ucJ])return FAILURE;
	}
	return relayStoreConfig();
}

void downloadRelayModuleConfig(void)
{
	static UINT8 ucTransferFlag=1;
	UINT8 ucFirst=1;
	if(ucTransferFlag==0)
	{
		if(ucDataTransferSuccess==1 && uiGPRSRecvDataCount<GPRS_REC_BUFFER_SIZE)
		{
			if(relayReadResponse()==SUCCESS)
			{
				ucRelayModuleReady=1;
				ucSystemDownloadRelayModuleConfig=0;
				writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DL_RELAY_MODULE_CONFIG,0);
				ucTransferFlag=1;
			}
			else
			{
				ucTransferFlag=1;
				ucSystemDownloadRelayModuleConfig=1;
				ucDataTransferSuccess=0;
			}
		}
		else if(ucDataTransferSuccess==1)
		{
			ucTransferFlag=1;
			ucSystemDownloadRelayModuleConfig=1;
			ucDataTransferSuccess=0;
		}
		else if(ucDataTransferSuccess==2)
		{
			ucTransferFlag=1;
			ucSystemDownloadRelayModuleConfig=1;
			ucDataTransferSuccess=0;
		}
		return;
	}
	jsonBegin(ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE,&ucFirst);
	jsonAddStrField(ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE,(UINT8*)"TYPE",(UINT8*)"RELAY_MODULE_CONFIG",&ucFirst);
	jsonAddHexField(ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE,(UINT8*)"SYSTEM_ID",ulSystemID,8,&ucFirst);
	jsonEnd(ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE);
	prepareGPRSTransmitPacket(ucGeneralPurposeBuffer,ucGPRSTransmitBuffer,ucSerAdd,ucSerPort,ucSerPath,(UINT8*)"api/relay-config",GPRS_TRANS_MODE_POST);
	ucTransferFlag=0;
	ucDataTransferSuccess=0;
	tranferDataToServer(&ucTransferFlag,ucGPRSTransmitBuffer,(UINT8*)"api/relay-config",GPRS_TRANS_MODE_POST,GPRS_CONNECTION_CLOSE);
}

static UINT8 relayCaptureMqttCommand(void)
{
	UINT16 uiAt=0,uiLen=0;
	UINT8 ucPayload=0,ucTopic=0;
	while(uiAt+10<uiGPRSRecvDataCount)
	{
		if(ucGPRSRecvBuffer[uiAt]=='+' && ucGPRSRecvBuffer[uiAt+1]=='Q' && ucGPRSRecvBuffer[uiAt+2]=='M' && ucGPRSRecvBuffer[uiAt+3]=='T' && ucGPRSRecvBuffer[uiAt+4]=='R' && ucGPRSRecvBuffer[uiAt+5]=='E' && ucGPRSRecvBuffer[uiAt+6]=='C' && ucGPRSRecvBuffer[uiAt+7]=='V')
		{
			while(uiAt<uiGPRSRecvDataCount && ucGPRSRecvBuffer[uiAt]!='"')uiAt++;
			if(uiAt>=uiGPRSRecvDataCount)return FAILURE;
			uiAt++;
			while(uiAt<uiGPRSRecvDataCount && ucGPRSRecvBuffer[uiAt]!='"' && uiLen+1<RELAY_TOPIC_SIZE)ucPendingTopic[uiLen++]=ucGPRSRecvBuffer[uiAt++];
			ucPendingTopic[uiLen]=0;ucTopic=1;
			while(uiAt<uiGPRSRecvDataCount && ucGPRSRecvBuffer[uiAt]!='{')uiAt++;
			if(uiAt>=uiGPRSRecvDataCount)return FAILURE;
			uiLen=0;
			while(uiAt<uiGPRSRecvDataCount && ucGPRSRecvBuffer[uiAt]!='}' && uiLen+1<sizeof(ucMqttCommand))ucMqttCommand[uiLen++]=ucGPRSRecvBuffer[uiAt++];
			if(uiAt>=uiGPRSRecvDataCount)return FAILURE;
			ucMqttCommand[uiLen++]='}';ucMqttCommand[uiLen]=0;ucPayload=1;
			break;
		}
		uiAt++;
	}
	if(!ucTopic || !ucPayload)return FAILURE;
	{
		UINT16 uiI;
		for(uiI=0;ucMqttCommandTopic[uiI] && ucPendingTopic[uiI] && ucMqttCommandTopic[uiI]==ucPendingTopic[uiI];uiI++);
		if(ucMqttCommandTopic[uiI] || ucPendingTopic[uiI])return FAILURE;
	}
	clearGPRSRecBuffer();
	return SUCCESS;
}

static UINT8 relayProcessCommand(void)
{
	UINT8 ucId[16],ucChannel[8],ucState[8];
	UINT16 uiI,uiChannel;
	if(relayCopyJsonString(ucMqttCommand,(UINT8*)"module",ucId,sizeof(ucId))!=SUCCESS)return FAILURE;
	if(relayCopyJsonString(ucMqttCommand,(UINT8*)"channel",ucChannel,sizeof(ucChannel))!=SUCCESS)return FAILURE;
	if(relayCopyJsonString(ucMqttCommand,(UINT8*)"state",ucState,sizeof(ucState))!=SUCCESS)return FAILURE;
	if(ucId[0]==0 || ucId[1]==0 || ucId[2]!=0)return FAILURE;
	if(!((ucId[0]>='0'&&ucId[0]<='9')||(ucId[0]>='A'&&ucId[0]<='F')||(ucId[0]>='a'&&ucId[0]<='f')) || !((ucId[1]>='0'&&ucId[1]<='9')||(ucId[1]>='A'&&ucId[1]<='F')||(ucId[1]>='a'&&ucId[1]<='f')))return FAILURE;
	uiChannel=0;
	for(uiI=0;ucChannel[uiI]>='0' && ucChannel[uiI]<='9';uiI++)uiChannel=(UINT16)(uiChannel*10+ucChannel[uiI]-'0');
	if(ucChannel[uiI]!=0 || uiChannel<1 || uiChannel>RELAY_MODULE_CHANNEL_COUNT)return FAILURE;
	for(uiI=0;uiI<ucRelayModuleCount;uiI++)
	{
		UINT8 ucHigh,ucLow,ucIdNumber;
		if(ucId[0]>='0'&&ucId[0]<='9')ucHigh=(UINT8)(ucId[0]-'0');
		else if(ucId[0]>='a'&&ucId[0]<='f')ucHigh=(UINT8)(ucId[0]-'a'+10);
		else ucHigh=(UINT8)(ucId[0]-'A'+10);
		if(ucId[1]>='0'&&ucId[1]<='9')ucLow=(UINT8)(ucId[1]-'0');
		else if(ucId[1]>='a'&&ucId[1]<='f')ucLow=(UINT8)(ucId[1]-'a'+10);
		else ucLow=(UINT8)(ucId[1]-'A'+10);
		ucIdNumber=(UINT8)(ucHigh*16+ucLow);
		if(ucRelaySlaveIds[uiI]==ucIdNumber)
		{
			ucPendingSlave=ucRelaySlaveIds[uiI];
			ucPendingChannel=(UINT8)uiChannel;
			if(ucState[0]=='O' && ucState[1]=='N' && ucState[2]==0)ucPendingState=1;
			else if(ucState[0]=='O' && ucState[1]=='F' && ucState[2]=='F' && ucState[3]==0)ucPendingState=0;
			else return FAILURE;
			ucPendingCommandId[0]=0;
			relayCopyJsonString(ucMqttCommand,(UINT8*)"commandId",ucPendingCommandId,sizeof(ucPendingCommandId));
			for(ucHigh=0;ucPendingCommandId[ucHigh]!=0;ucHigh++)
			{
				UINT8 ucChar=ucPendingCommandId[ucHigh];
				if(!((ucChar>='0'&&ucChar<='9')||(ucChar>='A'&&ucChar<='Z')||(ucChar>='a'&&ucChar<='z')||ucChar=='-'||ucChar=='_'||ucChar=='.'))
				{ucPendingCommandId[0]=0;break;}
			}
			return SUCCESS;
		}
	}
	return FAILURE;
}

static void relayPrepareStatus(void)
{
	UINT8 ucFirst=1;
	UINT8 ucChannel[4];
	ucMqttCommand[0]=0;
	intToASCIIStr(ucPendingChannel,1,sizeof(ucChannel),ucChannel);
	jsonBegin(ucMqttCommand,sizeof(ucMqttCommand),&ucFirst);
	jsonAddStrField(ucMqttCommand,sizeof(ucMqttCommand),(UINT8*)"commandId",ucPendingCommandId,&ucFirst);
	jsonAddHexField(ucMqttCommand,sizeof(ucMqttCommand),(UINT8*)"module",ucPendingSlave,2,&ucFirst);
	jsonAddStrField(ucMqttCommand,sizeof(ucMqttCommand),(UINT8*)"channel",ucChannel,&ucFirst);
	jsonAddStrField(ucMqttCommand,sizeof(ucMqttCommand),(UINT8*)"state",ucPendingState?(UINT8*)"ON":(UINT8*)"OFF",&ucFirst);
	jsonAddStrField(ucMqttCommand,sizeof(ucMqttCommand),(UINT8*)"result",ucRelayWriteResult?(UINT8*)"OK":(UINT8*)"ERROR",&ucFirst);
	jsonEnd(ucMqttCommand,sizeof(ucMqttCommand));
	{
		UINT32 ulLength=0;
		getStrLength(ucMqttCommand,&ulLength);
		uiPublishLength=(UINT16)ulLength;
	}
	ucAtCommand[0]=0;
	concatStirngsToStr1(ucAtCommand,(UINT8*)"AT+QMTPUBEX=0,1,1,0,\"",sizeof(ucAtCommand));
	concatStirngsToStr1(ucAtCommand,ucMqttStatusTopic,sizeof(ucAtCommand));
	concatStirngsToStr1(ucAtCommand,(UINT8*)"\",",sizeof(ucAtCommand));
	{
		UINT8 ucLength[5];
		UINT8 ucDigits=1;
		UINT16 uiTemp=uiPublishLength;
		while(uiTemp>=10){uiTemp=(UINT16)(uiTemp/10);ucDigits++;}
		intToASCIIStr(uiPublishLength,ucDigits,sizeof(ucLength),ucLength);
		concatStirngsToStr1(ucAtCommand,ucLength,sizeof(ucAtCommand));
	}
	concatStirngsToStr1(ucAtCommand,(UINT8*)"\r\n",sizeof(ucAtCommand));
}

static UINT8 relayPublishStatus(void)
{
	UINT8 ucStatus;
	UINT16 uiLoc;
	if(ucPublishState==0)
	{
		relayPrepareStatus();
		ucPublishFlag=1;
		ucPublishState=1;
	}
	if(ucPublishState==1)
	{
		ucStatus=processModemCmd(&ucPublishFlag,5000,0,ucAtCommand,(UINT8*)">",(UINT8*)"ERROR",(UINT8*)"",NO_OF_RETRY_1,SIM800_RETRY_VALID_RESP_NONE,&uiLoc,(UINT8*)"MQTT publish prompt",(UINT8*)"Prompt",(UINT8*)"ERROR",(UINT8*)"",BUFFER_CLEAR);
		if(ucPublishFlag==0)
		{
			if(ucStatus!=SIM800_RESPONSE_RESP1)
			{
				ucPublishState=0;ucPendingPublish=0;ucMqttState=6;
				return SUCCESS;
			}
			GPRSSendStr(ucMqttCommand);
			ucPublishFlag=1;
			ucPublishState=2;
		}
		return FAILURE;
	}
	if(ucPublishState==2)
	{
		ucStatus=processModemCmd(&ucPublishFlag,15000,0,(UINT8*)"",(UINT8*)"+QMTPUBEX: 0,1,0",(UINT8*)"ERROR",(UINT8*)"",NO_OF_RETRY_1,SIM800_RETRY_VALID_RESP_NONE,&uiLoc,(UINT8*)"MQTT publish result",(UINT8*)"Published",(UINT8*)"ERROR",(UINT8*)"",BUFFER_DONT_CLEAR);
		if(ucPublishFlag==0)
		{
			ucPublishState=0;ucPendingPublish=0;
			if(ucStatus!=SIM800_RESPONSE_RESP1)ucMqttState=6;
			return SUCCESS;
		}
	}
	return FAILURE;
}

void relayModuleService(void)
{
	UINT8 ucStatus;
	UINT16 uiLoc;
	UINT16 uiWriteValue;
	static UINT8 ucMqttStepStarted=0;
	static UINT8 ucRelayWriteStarted=0;
	if(!ucRelayModuleReady)
	{
		if(relayLoadConfig()==SUCCESS)ucRelayModuleReady=1;
		else
		{
			ucSystemDownloadRelayModuleConfig=1;
			writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DL_RELAY_MODULE_CONFIG,1);
			return;
		}
	}
	if(ucGPRSHttpTransferBusy || ucSendSMSFlag || ucBaordTestInProgress)return;
	if(ucPendingPublish)
	{
		relayPublishStatus();
		return;
	}
	if(ucPendingRelayWrite)
	{
		if(ucRS485BusFlag!=RS485_CON_DATA_ACQ || ucPraAcqFlag!=0)return;
		if(!ucRelayWriteStarted){ucSavedAcqDisable=ucAcqDataDisable;ucAcqDataDisable=1;ucRelayWriteStarted=1;}
		ucRelayModbusFrame[0]=ucPendingSlave;
		{
			UINT8 ucModule;
			for(ucModule=0;ucModule<ucRelayModuleCount && ucRelaySlaveIds[ucModule]!=ucPendingSlave;ucModule++);
			if(ucModule>=ucRelayModuleCount){ucPendingRelayWrite=0;ucAcqDataDisable=ucSavedAcqDisable;ucRelayWriteStarted=0;return;}
			uiLoc=uiRelayRegisters[ucModule][ucPendingChannel-1];
			ucRelayModbusFrame[1]=ucRelayWriteFunctions[ucModule];
			uiWriteValue=ucPendingState?uiRelayOnValues[ucModule]:uiRelayOffValues[ucModule];
		}
		ucRelayModbusFrame[2]=(UINT8)(uiLoc>>8);ucRelayModbusFrame[3]=(UINT8)uiLoc;
		ucRelayModbusFrame[4]=(UINT8)(uiWriteValue>>8);ucRelayModbusFrame[5]=(UINT8)uiWriteValue;
		insertCRC16(ucRelayModbusFrame,6);
		if(ucModbusFlag==0)ucModbusFlag=1;
		ucRS485BusFlag=RS485_CON_DATA_ACQ;
		ucStatus=processRS485Packet(&ucModbusFlag,RS485_MODBUS_RTU,RS485_MODBUS_RTU,RS485_DATATYPE_COIL_RESP,UART_BAUD_9600,UART_DATABIT_EIGHT,UART_PARITY_NONE,UART_STOPBIT_ONE,1500,ucRelayModbusFrame,8,&ucRelayResponseStatus,&fRelayResponseValue,1);
		if(ucStatus!=FAILURE && ucModbusFlag==0)
		{
			ucPendingRelayWrite=0;ucAcqDataDisable=ucSavedAcqDisable;ucRelayWriteStarted=0;
			ucRelayWriteResult=0;
			if(ucRelayResponseStatus==RS485_RESP_STAT_OK && uiRS485RecvDataCount>=8 &&
			   ucRS485RecvBuffer[0]==ucRelayModbusFrame[0] && ucRS485RecvBuffer[1]==ucRelayModbusFrame[1] &&
			   ucRS485RecvBuffer[2]==ucRelayModbusFrame[2] && ucRS485RecvBuffer[3]==ucRelayModbusFrame[3] &&
			   ucRS485RecvBuffer[4]==ucRelayModbusFrame[4] && ucRS485RecvBuffer[5]==ucRelayModbusFrame[5])
				ucRelayWriteResult=1;
			ucPendingPublish=1;
		}
		return;
	}
	if(ucNetworkStatus==0){ucMqttState=0;ucMqttStepStarted=0;return;}
	if(ucMqttState==0)
	{
		if(!ucMqttStepStarted){ucMqttCmdFlag=1;ucMqttStepStarted=1;}
		ucStatus=processModemCmd(&ucMqttCmdFlag,3000,0,(UINT8*)"AT+QMTCFG=\"recv/mode\",0,0,1\r\n",(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",NO_OF_RETRY_2,SIM800_RETRY_VALID_RESP_NONE,&uiLoc,(UINT8*)"MQTT receive mode",(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",BUFFER_CLEAR);
		if(ucMqttCmdFlag==0){ucMqttState=(ucStatus==SIM800_RESPONSE_RESP1)?1:0;ucMqttStepStarted=0;}
	}
	else if(ucMqttState==1)
	{
		if(!ucMqttStepStarted){ucMqttCmdFlag=1;ucMqttStepStarted=1;}
		ucStatus=processModemCmd(&ucMqttCmdFlag,3000,0,(UINT8*)"AT+QMTCFG=\"version\",0,4\r\n",(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",NO_OF_RETRY_2,SIM800_RETRY_VALID_RESP_NONE,&uiLoc,(UINT8*)"MQTT version",(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",BUFFER_CLEAR);
		if(ucMqttCmdFlag==0){ucMqttState=(ucStatus==SIM800_RESPONSE_RESP1)?2:6;ucMqttStepStarted=0;}
	}
	else if(ucMqttState==2)
	{
		if(!ucMqttStepStarted){ucMqttCmdFlag=1;ucMqttStepStarted=1;}
		ucAtCommand[0]=0;concatStirngsToStr1(ucAtCommand,(UINT8*)"AT+QMTOPEN=0,\"",sizeof(ucAtCommand));concatStirngsToStr1(ucAtCommand,ucMqttHost,sizeof(ucAtCommand));concatStirngsToStr1(ucAtCommand,(UINT8*)"\",",sizeof(ucAtCommand));concatStirngsToStr1(ucAtCommand,ucMqttPort,sizeof(ucAtCommand));concatStirngsToStr1(ucAtCommand,(UINT8*)"\r\n",sizeof(ucAtCommand));
		ucStatus=processModemCmd(&ucMqttCmdFlag,10000,10000,ucAtCommand,(UINT8*)"+QMTOPEN: 0,0",(UINT8*)"+QMTOPEN: 0,1",(UINT8*)"ERROR",NO_OF_RETRY_1,SIM800_RETRY_VALID_RESP_NONE,&uiLoc,(UINT8*)"MQTT open",(UINT8*)"OK",(UINT8*)"BUSY",(UINT8*)"ERROR",BUFFER_CLEAR);
		if(ucMqttCmdFlag==0){ucMqttState=(ucStatus==SIM800_RESPONSE_RESP1)?3:6;ucMqttStepStarted=0;}
	}
	else if(ucMqttState==3)
	{
		if(!ucMqttStepStarted){ucMqttCmdFlag=1;ucMqttStepStarted=1;}
		ucAtCommand[0]=0;concatStirngsToStr1(ucAtCommand,(UINT8*)"AT+QMTCONN=0,\"",sizeof(ucAtCommand));concatStirngsToStr1(ucAtCommand,ucMqttClientId,sizeof(ucAtCommand));concatStirngsToStr1(ucAtCommand,(UINT8*)"\",\"",sizeof(ucAtCommand));concatStirngsToStr1(ucAtCommand,ucMqttUsername,sizeof(ucAtCommand));concatStirngsToStr1(ucAtCommand,(UINT8*)"\",\"",sizeof(ucAtCommand));concatStirngsToStr1(ucAtCommand,ucMqttPassword,sizeof(ucAtCommand));concatStirngsToStr1(ucAtCommand,(UINT8*)"\"\r\n",sizeof(ucAtCommand));
		ucStatus=processModemCmd(&ucMqttCmdFlag,10000,10000,ucAtCommand,(UINT8*)"+QMTCONN: 0,0,0",(UINT8*)"ERROR",(UINT8*)"",NO_OF_RETRY_1,SIM800_RETRY_VALID_RESP_NONE,&uiLoc,(UINT8*)"MQTT connect",(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",BUFFER_CLEAR);
		if(ucMqttCmdFlag==0){ucMqttState=(ucStatus==SIM800_RESPONSE_RESP1)?4:6;ucMqttStepStarted=0;}
	}
	else if(ucMqttState==4)
	{
		ucAtCommand[0]=0;
		if(!ucMqttStepStarted){ucMqttCmdFlag=1;ucMqttStepStarted=1;}
		concatStirngsToStr1(ucAtCommand,(UINT8*)"AT+QMTSUB=0,1,\"",sizeof(ucAtCommand));concatStirngsToStr1(ucAtCommand,ucMqttCommandTopic,sizeof(ucAtCommand));concatStirngsToStr1(ucAtCommand,(UINT8*)"\",1\r\n",sizeof(ucAtCommand));
		ucStatus=processModemCmd(&ucMqttCmdFlag,10000,10000,ucAtCommand,(UINT8*)"+QMTSUB: 0,1,0,1",(UINT8*)"ERROR",(UINT8*)"",NO_OF_RETRY_1,SIM800_RETRY_VALID_RESP_NONE,&uiLoc,(UINT8*)"MQTT subscribe",(UINT8*)"OK",(UINT8*)"ERROR",(UINT8*)"",BUFFER_CLEAR);
		if(ucMqttCmdFlag==0){ucMqttState=(ucStatus==SIM800_RESPONSE_RESP1)?5:6;ucMqttStepStarted=0;}
	}
	else if(ucMqttState==5)
	{
		if(!ucMqttStepStarted){ucMqttCmdFlag=1;ucMqttStepStarted=1;}
		ucStatus=processModemCmd(&ucMqttCmdFlag,1000,0,(UINT8*)"",(UINT8*)"+QMTRECV:",(UINT8*)"+QMTSTAT:",(UINT8*)"",NO_OF_RETRY_1,SIM800_RETRY_VALID_RESP_NONE,&uiLoc,(UINT8*)"MQTT receive",(UINT8*)"message",(UINT8*)"disconnect",(UINT8*)"",BUFFER_DONT_CLEAR);
		if(ucMqttCmdFlag==0){ucMqttStepStarted=0;if(ucStatus==SIM800_RESPONSE_RESP1 && relayCaptureMqttCommand()==SUCCESS && relayProcessCommand()==SUCCESS)ucPendingRelayWrite=1;else if(ucStatus==SIM800_RESPONSE_RESP2)ucMqttState=6;clearGPRSRecBuffer();}
	}
	else if(ucMqttState==6)
	{
		if(!ucMqttStepStarted){ucMqttCmdFlag=1;ucMqttStepStarted=1;}
		ucStatus=processModemCmd(&ucMqttCmdFlag,3000,0,(UINT8*)"AT+QMTDISC=0\r\n",(UINT8*)"+QMTDISC: 0,0",(UINT8*)"ERROR",(UINT8*)"",NO_OF_RETRY_1,SIM800_RETRY_VALID_RESP_NONE,&uiLoc,(UINT8*)"MQTT disconnect",(UINT8*)"Disconnected",(UINT8*)"ERROR",(UINT8*)"",BUFFER_CLEAR);
		if(ucMqttCmdFlag==0){ucMqttState=7;ucMqttStepStarted=0;}
		(void)ucStatus;
	}
	else
	{
		if(!ucMqttStepStarted){ucMqttCmdFlag=1;ucMqttStepStarted=1;}
		ucStatus=processModemCmd(&ucMqttCmdFlag,3000,0,(UINT8*)"AT+QMTCLOSE=0\r\n",(UINT8*)"+QMTCLOSE: 0,0",(UINT8*)"ERROR",(UINT8*)"",NO_OF_RETRY_1,SIM800_RETRY_VALID_RESP_NONE,&uiLoc,(UINT8*)"MQTT close",(UINT8*)"Closed",(UINT8*)"ERROR",(UINT8*)"",BUFFER_CLEAR);
		if(ucMqttCmdFlag==0){ucMqttState=0;ucMqttStepStarted=0;}
		(void)ucStatus;
	}
}
