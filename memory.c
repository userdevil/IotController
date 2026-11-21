#include "memory.h"
#include "global.h"
#include "eeprom.h"
#include "uart.h"
#include "memory.h"
#include "controller.h"
#include "dio.h"
#include "flash.h"
#include "data.h"
#include "rtc.h"

#define SHOW_VERBOSE_STARTUP 0


UINT8 ucFlashID;
UINT8 	ucBoardTestStatus;
UINT32 	ulBoardTestID;
UINT8 	ucBoardTestDate;
UINT8 	ucBoardTestMonth;
UINT8 	ucBoardTestYear;
UINT8 	ucBoardMemeoryStatus;
UINT8 	ucBaordTestInProgress;

UINT8 	ucEnableCSV=0;//1016

UINT8 	ucSystemRegistrationStatus;
UINT8 	ucInternetSource;
UINT32 	ulSystemID;
UINT8 	ucAppType;
UINT32 	ulSystemNoOfParameters;
UINT8 	ucSystemDownloadUnitConfig;
UINT8 	ucSystemDownloadParaConfig;
UINT8 	ucSystemDownloadRelayModuleConfig;
UINT32 	ulDataAcqCntr;
UINT32 	ulTransmissionInterwal;
UINT32 	ulAcqInterwal;
F32 	fParaStoreLocation1;
F32 	fParaStoreLocation2;
F32 	fParaStoreLocation3;
F32 	fParaStoreLocation4;
F32 	fParaStoreLocation5;
F32 	fParaStoreLocation6;
F32 	fParaStoreLocation7;
F32 	fParaStoreLocation8;
F32 	fParaStoreLocation9;
F32 	fParaStoreLocation10;
//1017
F32 	fParaStoreLocation11;
F32 	fParaStoreLocation12;
F32 	fParaStoreLocation13;
F32 	fParaStoreLocation14;
F32 	fParaStoreLocation15;
F32 	fParaStoreLocation16;
F32 	fParaStoreLocation17;
F32 	fParaStoreLocation18;
F32 	fParaStoreLocation19;
F32 	fParaStoreLocation20;

UINT32 ulFlashFileSaveLocation;
UINT32 ulFlashFileSentLocation;

UINT8 ucSMSStoredNo;
UINT8 ucSMSSentNo;


UINT8 ucSerAdd[50];
UINT8 ucSerPath[100];
UINT8 ucSerPort[10];
UINT8 ucSerAppReg[40];
UINT8 ucSerAppSysConfig[40];
UINT8 ucSerAppParaConfig[40];
UINT8 ucSerAppRecData[40];

UINT8 ucGPRSAPNStr[25];											
UINT8 ucGPRSUserNameStr[25];
UINT8 ucGPRSPasswordStr[25];
UINT8 ucSIMBalNo[10];
UINT8 ucSIMBalKeyWord[10];
UINT32 ulSIMBalCntr;



UINT8 ucAlertMobileNo1[15];
UINT8 ucAlertMobileNo2[15];


UINT8 ucRelay1Status;
UINT8 ucRelay1Parameter;
F32 fRelay1ParaON;
F32 fRelay1ParaOFF;

UINT8 ucRelay2Status;
UINT8 ucRelay2Parameter;
F32 fRelay2ParaON;
F32 fRelay2ParaOFF;

UINT8 ucRelay3Status;
UINT8 ucRelay3Parameter;
F32 fRelay3ParaON;
F32 fRelay3ParaOFF;

UINT8 ucRelay1OnHrs1;
UINT8 ucRelay1OnMin1;
UINT8 ucRelay1OffHrs1;
UINT8 ucRelay1OffMin1;
UINT8 ucRelay1OnHrs2;
UINT8 ucRelay1OnMin2;
UINT8 ucRelay1OffHrs2;
UINT8 ucRelay1OffMin2;

UINT8 ucRelay2OnHrs1;
UINT8 ucRelay2OnMin1;
UINT8 ucRelay2OffHrs1;
UINT8 ucRelay2OffMin1;
UINT8 ucRelay2OnHrs2;
UINT8 ucRelay2OnMin2;
UINT8 ucRelay2OffHrs2;
UINT8 ucRelay2OffMin2;

UINT8 ucRelay3OnHrs1;
UINT8 ucRelay3OnMin1;
UINT8 ucRelay3OffHrs1;
UINT8 ucRelay3OffMin1;
UINT8 ucRelay3OnHrs2;
UINT8 ucRelay3OnMin2;
UINT8 ucRelay3OffHrs2;
UINT8 ucRelay3OffMin2;

UINT8 ucMasterModbusID;//1043
UINT8 ucDebugBaudRate; //1044

UINT8 ucTransPeriod[8];

UINT8 ucModemSpecialCmd[49];//1045


void storeStringToEeprom(UINT8* pucString,UINT16 ucLocation, UINT8 maxLength)
{
	//check if 0x00 is present in the string
	UINT8 ucCntrT=0;
	UINT8 ucData=0x00;

	for(ucCntrT=0;ucCntrT<maxLength;ucCntrT++)
	{
		ucData=*pucString++;
		writeExtEEPROMChar(EEPROM_ADD,ucLocation+ucCntrT,ucData);
		if(ucData=0x00)
			return;
	}
	return;
}

void readStringFromEeprom(UINT8* pucString,UINT16 ucLocation, UINT8 maxLength)
{
	//check if 0x00 is present in the string
	UINT8 ucCntrT=0;
	UINT8 ucData=0x00;
	for(ucCntrT=0;ucCntrT<maxLength;ucCntrT++)
	{
		readExtEEPROMChar(EEPROM_ADD,ucLocation+ucCntrT,&ucData);
		*pucString=ucData;
		pucString++;
		if(ucData=0x00)
			return;
	}
	*pucString=0x00;
	return;
}

UINT8 validateUint8Para(UINT8 ucValue,UINT8 ucMinValue,UINT8 ucMaxValue)
{
	UINT8 ucReturnValue = ucValue;
	if(ucValue>ucMaxValue)
		return 0;
	if(ucValue<ucMinValue)
		return 0;
	return ucReturnValue;
}

void readDataFromMemory(void)
{
	UINT8 ucStatusF,ucData;
	UINT32 ulCntrF;
	float fTemp=0;
	
	ucEnableCSV=1;
	
	
	
	ucStatusF=readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_EEPROM_MEM_STATUS,&ucBoardMemeoryStatus);
	if(ucBoardMemeoryStatus!='e')// if the stored char is not 'e' then load all default config into eeprom
	{
		
		ucSystemRegistrationStatus=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_SYSTEM_REG_STATUS,ucSystemRegistrationStatus);
		ulSystemID=0;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_SYSTEM_ID,ulSystemID);
		ulSystemNoOfParameters=0;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_NO_OF_PARA,ulSystemNoOfParameters);
		ucSystemDownloadUnitConfig=1;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DL_UNIT_CONIG,ucSystemDownloadUnitConfig);
		ucSystemDownloadParaConfig=1;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DL_PARA_CONIG,ucSystemDownloadParaConfig);
		ucSystemDownloadRelayModuleConfig=1;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DL_RELAY_MODULE_CONFIG,ucSystemDownloadRelayModuleConfig);
		ulDataAcqCntr=0;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_DATA_CNTR,ulDataAcqCntr);
		ulTransmissionInterwal=5;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_TRANS_INTERVAL,ulTransmissionInterwal);
		ulAcqInterwal=5;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_ACQ_INTERVAL,ulAcqInterwal);
		
		ucAppType=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_APP_TYPE,ucAppType);
		
		//1016
		ucEnableCSV=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_ENABLE_CSV,ucEnableCSV);
		
		
		fParaStoreLocation1=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_1_LOC,fParaStoreLocation1);
		fParaStoreLocation2=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_2_LOC,fParaStoreLocation2);
		fParaStoreLocation3=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_3_LOC,fParaStoreLocation3);
		fParaStoreLocation4=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_4_LOC,fParaStoreLocation4);
		fParaStoreLocation5=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_5_LOC,fParaStoreLocation5);
		fParaStoreLocation6=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_6_LOC,fParaStoreLocation6);
		fParaStoreLocation7=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_7_LOC,fParaStoreLocation7);
		fParaStoreLocation8=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_8_LOC,fParaStoreLocation8);
		fParaStoreLocation9=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_9_LOC,fParaStoreLocation9);
		fParaStoreLocation10=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_10_LOC,fParaStoreLocation10);
		//1017
		fParaStoreLocation11=0;writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_11_LOC,fParaStoreLocation11);
		fParaStoreLocation12=0;writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_12_LOC,fParaStoreLocation12);
		fParaStoreLocation13=0;writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_13_LOC,fParaStoreLocation13);
		fParaStoreLocation14=0;writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_14_LOC,fParaStoreLocation14);
		fParaStoreLocation15=0;writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_15_LOC,fParaStoreLocation15);
		fParaStoreLocation16=0;writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_16_LOC,fParaStoreLocation16);
		fParaStoreLocation17=0;writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_17_LOC,fParaStoreLocation17);
		fParaStoreLocation18=0;writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_18_LOC,fParaStoreLocation18);
		fParaStoreLocation19=0;writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_19_LOC,fParaStoreLocation19);
		fParaStoreLocation20=0;writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_20_LOC,fParaStoreLocation20);
		
		
		
		ulIntCntr=0;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_INT_CNTR,ulIntCntr);
		
		ulFlashFileSaveLocation=0;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SAVE_CNTR,ulFlashFileSaveLocation);
		ulFlashFileSentLocation=0;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SENT_CNTR,ulFlashFileSentLocation);
				
		ucSMSStoredNo=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_SMS_SAVE_CNTR,ucSMSStoredNo);
		ucSMSSentNo=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_SMS_SENT_CNTR,ucSMSSentNo);
		
				
		storeStringToEeprom((UINT8*)"iot.mavinsandeep.co.in",EEPROM_LOC_DATA_SER_ADD,48);
		storeStringToEeprom((UINT8*)"/",EEPROM_LOC_DATA_SER_PATH,98);
		storeStringToEeprom((UINT8*)"8085",EEPROM_LOC_DATA_SER_PORT,8);
		storeStringToEeprom((UINT8*)"api/register",EEPROM_LOC_SER_APP_REG,38);
		storeStringToEeprom((UINT8*)"api/system-config",EEPROM_LOC_SER_APP_SYS_CONFIG,38);
		storeStringToEeprom((UINT8*)"api/param-config",EEPROM_LOC_SER_APP_PARA_CONFIG,38);
		storeStringToEeprom((UINT8*)"api/data",EEPROM_LOC_SER_APP_REC_DATA,38);
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_SERVER_API_VERSION,SERVER_API_VERSION_CURRENT);
				
		//1003
		
		storeStringToEeprom((UINT8*)"*123",EEPROM_LOC_SIM_BAL_N0,8);
		storeStringToEeprom((UINT8*)"Balance",EEPROM_LOC_SIM_BAL_KW,8);
		ulSIMBalCntr=0;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_SIM_BAL_CNTR,ulSIMBalCntr);
		
		
		if(ucBoardMemeoryStatus!='x')
		{
			ucInternetSource=0;
			writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_INTERNET_SOURCE,ucInternetSource);
		
			storeStringToEeprom((UINT8*)"airtelgprs.com",EEPROM_LOC_GPRS_APN,23);
			storeStringToEeprom((UINT8*)"",EEPROM_LOC_GPRS_USERNAME,23);
			storeStringToEeprom((UINT8*)"",EEPROM_LOC_GPRS_PASSWORD,23);
						
			storeStringToEeprom((UINT8*)"",EEPROM_LOC_MODEM_SPL_COMMAND,45);
		
		}
		
		//1045
		//storeStringToEeprom((UINT8*)"",EEPROM_LOC_MODEM_SPL_COMMAND,45);
				
		storeStringToEeprom((UINT8*)"",EEPROM_LOC_ALERT_MOB1,13);
		storeStringToEeprom((UINT8*)"",EEPROM_LOC_ALERT_MOB2,13);
		
		
		ucRelay1Status=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RELAY1_STATUS,ucRelay1Status);
		ucRelay1Parameter=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RELAY1_PARA,ucRelay1Parameter);
		fRelay1ParaON=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_RELAY1_ON,fRelay1ParaON);
		fRelay1ParaOFF=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_RELAY1_OFF,fRelay1ParaOFF);
		
		ucRelay2Status=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RELAY2_STATUS,ucRelay2Status);
		ucRelay2Parameter=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RELAY2_PARA,ucRelay2Parameter);
		fRelay2ParaON=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_RELAY2_ON,fRelay2ParaON);
		fRelay2ParaOFF=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_RELAY2_OFF,fRelay2ParaOFF);
		
		
		ucRelay3Status=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RELAY3_STATUS,ucRelay3Status);
		ucRelay3Parameter=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RELAY3_PARA,ucRelay3Parameter);
		fRelay3ParaON=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_RELAY3_ON,fRelay3ParaON);
		fRelay3ParaOFF=0;
		writeExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_RELAY3_OFF,fRelay3ParaOFF);
		
		ucRelay1OnHrs1=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1__ON_HRS_1,ucRelay1OnHrs1);
		ucRelay1OnMin1=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1__ON_MIN_1,ucRelay1OnMin1);
		ucRelay1OffHrs1=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1_OFF_HRS_1,ucRelay1OffHrs1);
		ucRelay1OffMin1=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1_OFF_MIN_1,ucRelay1OffMin1);
		ucRelay1OnHrs2=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1__ON_HRS_2,ucRelay1OnHrs2);
		ucRelay1OnMin2=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1__ON_MIN_2,ucRelay1OnMin2);
		ucRelay1OffHrs2=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1_OFF_HRS_2,ucRelay1OffHrs2);
		ucRelay1OffMin2=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1_OFF_MIN_2,ucRelay1OffMin2);
		
		ucRelay2OnHrs1=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2__ON_HRS_1,ucRelay2OnHrs1);
		ucRelay2OnMin1=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2__ON_MIN_1,ucRelay2OnMin1);
		ucRelay2OffHrs1=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2_OFF_HRS_1,ucRelay2OffHrs1);
		ucRelay2OffMin1=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2_OFF_MIN_1,ucRelay2OffMin1);
		ucRelay2OnHrs2=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2__ON_HRS_2,ucRelay2OnHrs2);
		ucRelay2OnMin2=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2__ON_MIN_2,ucRelay2OnMin2);
		ucRelay2OffHrs2=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2_OFF_HRS_2,ucRelay2OffHrs2);
		ucRelay2OffMin2=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2_OFF_MIN_2,ucRelay2OffMin2);
		
		ucRelay3OnHrs1=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3__ON_HRS_1,ucRelay3OnHrs1);
		ucRelay3OnMin1=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3__ON_MIN_1,ucRelay3OnMin1);
		ucRelay3OffHrs1=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3_OFF_HRS_1,ucRelay3OffHrs1);
		ucRelay3OffMin1=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3_OFF_MIN_1,ucRelay3OffMin1);
		ucRelay3OnHrs2=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3__ON_HRS_2,ucRelay3OnHrs2);
		ucRelay3OnMin2=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3__ON_MIN_2,ucRelay3OnMin2);
		ucRelay3OffHrs2=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3_OFF_HRS_2,ucRelay3OffHrs2);
		ucRelay3OffMin2=0;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3_OFF_MIN_2,ucRelay3OffMin2);
		
		
		//1043
		ucMasterModbusID=231;
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_MODBUS_ID,ucMasterModbusID);
		
		for(ucStatusF=0;ucStatusF<8;ucStatusF++)
		{
			ucTransPeriod[ucStatusF]='0';
			writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_TRANS_PERIOD+ucStatusF,ucTransPeriod[ucStatusF]);
		}
		
		//format flash device
		chipErase();
		
		
		ucBoardMemeoryStatus='e';
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_EEPROM_MEM_STATUS,ucBoardMemeoryStatus);
	}
	
	
	//1016
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_ENABLE_CSV,&ucEnableCSV);
	
	
	
	if(SHOW_VERBOSE_STARTUP && ucEnableCSV==0)
	UARTSendStr(UART_DEBUG,(UINT8 *)" \r------------------------------- board data -------------------------------");	
		
	
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_BOARD_TEST_STATUS,&ucBoardTestStatus);
	readExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_BOARD_TEST_ID,&ulBoardTestID);

	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_BOARD_TEST_DATE,&ucBoardTestDate);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_BOARD_TEST_MONTH,&ucBoardTestMonth);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_BOARD_TEST_YEAR,&ucBoardTestYear);
	if(SHOW_VERBOSE_STARTUP && ucEnableCSV==0)
	{
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r Board Test Status : ");
		UARTSendLong(UART_DEBUG,ucBoardTestStatus,1);
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r Board ID :");
		UARTSendLong(UART_DEBUG,ulBoardTestID,6);
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r Tested on :");
		UARTSendLong(UART_DEBUG,ucBoardTestDate,2);
		UARTSendStr(UART_DEBUG,(UINT8 *)"/");
		UARTSendLong(UART_DEBUG,ucBoardTestMonth,2);
		UARTSendStr(UART_DEBUG,(UINT8 *)"/");
		UARTSendLong(UART_DEBUG,ucBoardTestYear,2);
		
		
	}
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_SYSTEM_REG_STATUS,&ucSystemRegistrationStatus);
	if(ucSystemRegistrationStatus>1)
		ucSystemRegistrationStatus=0;
	readExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_SYSTEM_ID,&ulSystemID);
	readExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_NO_OF_PARA,&ulSystemNoOfParameters);
	if(ulSystemNoOfParameters>200)
	{
		ulSystemNoOfParameters=0;
		ucSystemDownloadUnitConfig=1;
		ucSystemDownloadParaConfig=1;
		
	}
	else
	{
		readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DL_UNIT_CONIG,&ucSystemDownloadUnitConfig);
		readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DL_PARA_CONIG,&ucSystemDownloadParaConfig);
		if(ucSystemDownloadUnitConfig>1)
			ucSystemDownloadUnitConfig=1;
		if(ucSystemDownloadParaConfig>1)
			ucSystemDownloadParaConfig=1;
	}
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DL_RELAY_MODULE_CONFIG,&ucSystemDownloadRelayModuleConfig);
	if(ucSystemDownloadRelayModuleConfig>1)
		ucSystemDownloadRelayModuleConfig=1;
	
	if(ulSystemNoOfParameters<25)
	{
		uiFlashStorageSize=325;
		uiFlashMaxReadCntr=1500;
		uiFlashMaxWriteCntr=3000;	
	}
	else if(ulSystemNoOfParameters<50)
	{
		uiFlashStorageSize=575;
		uiFlashMaxReadCntr=850;
		uiFlashMaxWriteCntr=1700;
	}
	else if(ulSystemNoOfParameters<100)
	{
		uiFlashStorageSize=1075;
		uiFlashMaxReadCntr=450;
		uiFlashMaxWriteCntr=900;
	}
	else
	{
		uiFlashStorageSize=2150;
		uiFlashMaxReadCntr=225;
		uiFlashMaxWriteCntr=450;
	}
//	UARTSendStr(UART_DEBUG,(UINT8 *)"\r File size :");
//	UARTSendLong(UART_DEBUG,uiFlashStorageSize,5);
//	UARTSendStr(UART_DEBUG,(UINT8 *)"\r File max read :");
//	UARTSendLong(UART_DEBUG,uiFlashMaxReadCntr,5);
//	UARTSendStr(UART_DEBUG,(UINT8 *)"\r File max write :");
//	UARTSendLong(UART_DEBUG,uiFlashMaxWriteCntr,5);
	
	readExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_DATA_CNTR,&ulDataAcqCntr);
	readExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_TRANS_INTERVAL,&ulTransmissionInterwal);
	if(ulTransmissionInterwal>120)
		ulTransmissionInterwal=5;
	readExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_ACQ_INTERVAL,&ulAcqInterwal);
	if(ulAcqInterwal>30)
		ulAcqInterwal=5;
	
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_INTERNET_SOURCE,&ucInternetSource);
	ucInternetSource=0;
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_APP_TYPE,&ucAppType);
		
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_1_LOC,&fParaStoreLocation1);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_2_LOC,&fParaStoreLocation2);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_3_LOC,&fParaStoreLocation3);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_4_LOC,&fParaStoreLocation4);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_5_LOC,&fParaStoreLocation5);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_6_LOC,&fParaStoreLocation6);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_7_LOC,&fParaStoreLocation7);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_8_LOC,&fParaStoreLocation8);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_9_LOC,&fParaStoreLocation9);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_10_LOC,&fParaStoreLocation10);
	//1017
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_11_LOC,&fParaStoreLocation11);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_12_LOC,&fParaStoreLocation12);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_13_LOC,&fParaStoreLocation13);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_14_LOC,&fParaStoreLocation14);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_15_LOC,&fParaStoreLocation15);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_16_LOC,&fParaStoreLocation16);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_17_LOC,&fParaStoreLocation17);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_18_LOC,&fParaStoreLocation18);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_19_LOC,&fParaStoreLocation19);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_STORE_PARA_20_LOC,&fParaStoreLocation20);
	
	readExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_INT_CNTR,&ulIntCntr);
	for(ucData=0;ucData<15;ucData++)
		readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_STORED_IMIE_NUMBER+ucData,&ucIMIENumber[ucData]);
	ucIMIENumber[15]=0x00;
	if(ulSystemID==0)
	{
		ucSystemDownloadUnitConfig=1;
		ucSystemDownloadUnitConfig=1;
		ucSystemRegistrationStatus=0;
	}
	
	
	
	
	if(SHOW_VERBOSE_STARTUP && ucEnableCSV==0)
	{
	UARTSendStr(UART_DEBUG,(UINT8 *)"\r No Of parameters :");
	UARTSendLong(UART_DEBUG,ulSystemNoOfParameters,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\r DL Unit config :");
	UARTSendLong(UART_DEBUG,ucSystemDownloadUnitConfig,1);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\r DL Para config :");
	UARTSendLong(UART_DEBUG,ucSystemDownloadParaConfig,1);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\r Tran Int Min :");
	UARTSendLong(UART_DEBUG,ulTransmissionInterwal,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\r Acq Int Min :");
	UARTSendLong(UART_DEBUG,ulAcqInterwal,3);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\r App Type :");
	UARTSendLong(UART_DEBUG,ucAppType,2);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\r Int Cntr :");
	UARTSendLong(UART_DEBUG,ulIntCntr,8);
	}
	readExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SAVE_CNTR,&ulFlashFileSaveLocation);
	readExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SENT_CNTR,&ulFlashFileSentLocation);
	
	//1008
	if ( (ulFlashFileSaveLocation>5000) || (ulFlashFileSentLocation>5000) )
	{
		ulFlashFileSaveLocation=0;
		ulFlashFileSentLocation=0;
	}
	if(SHOW_VERBOSE_STARTUP && ucEnableCSV==0)
	{
	UARTSendStr(UART_DEBUG,(UINT8 *)"\r Flash Save Cntr :");
	UARTSendLong(UART_DEBUG,ulFlashFileSaveLocation,5);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\r Flash Sent Cntr :");
	UARTSendLong(UART_DEBUG,ulFlashFileSentLocation,5);
	}
			
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_SMS_SAVE_CNTR,&ucSMSStoredNo);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_SMS_SENT_CNTR,&ucSMSSentNo);
	if(ucSMSStoredNo>10)
		ucSMSStoredNo=0;
	if(ucSMSSentNo>10)
		ucSMSSentNo=0;
	if(SHOW_VERBOSE_STARTUP && ucEnableCSV==0)
	{
	UARTSendStr(UART_DEBUG,(UINT8 *)"\r SMS Save Cntr :");
	UARTSendLong(UART_DEBUG,ucSMSStoredNo,5);
	UARTSendStr(UART_DEBUG,(UINT8 *)"\r SMS Sent Cntr :");
	UARTSendLong(UART_DEBUG,ucSMSSentNo,5);
	}
	
	
	
	readStringFromEeprom(ucSerAdd,EEPROM_LOC_DATA_SER_ADD,48);
	readStringFromEeprom(ucSerPath,EEPROM_LOC_DATA_SER_PATH,98);
	readStringFromEeprom(ucSerPort,EEPROM_LOC_DATA_SER_PORT,8);
	readStringFromEeprom(ucSerAppReg,EEPROM_LOC_SER_APP_REG,38);
	readStringFromEeprom(ucSerAppSysConfig,EEPROM_LOC_SER_APP_SYS_CONFIG,38);
	readStringFromEeprom(ucSerAppParaConfig,EEPROM_LOC_SER_APP_PARA_CONFIG,38);
	readStringFromEeprom(ucSerAppRecData,EEPROM_LOC_SER_APP_REC_DATA,38);
	
	{
		UINT8 ucApiVersion=0;
		readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_SERVER_API_VERSION,&ucApiVersion);
		if(ucApiVersion!=SERVER_API_VERSION_CURRENT)
		{
			storeStringToEeprom((UINT8*)"iot.mavinsandeep.co.in",EEPROM_LOC_DATA_SER_ADD,48);
			storeStringToEeprom((UINT8*)"/",EEPROM_LOC_DATA_SER_PATH,98);
			storeStringToEeprom((UINT8*)"8085",EEPROM_LOC_DATA_SER_PORT,8);
			storeStringToEeprom((UINT8*)"api/register",EEPROM_LOC_SER_APP_REG,38);
			storeStringToEeprom((UINT8*)"api/system-config",EEPROM_LOC_SER_APP_SYS_CONFIG,38);
			storeStringToEeprom((UINT8*)"api/param-config",EEPROM_LOC_SER_APP_PARA_CONFIG,38);
			storeStringToEeprom((UINT8*)"api/data",EEPROM_LOC_SER_APP_REC_DATA,38);
			writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_SERVER_API_VERSION,SERVER_API_VERSION_CURRENT);
			readStringFromEeprom(ucSerAdd,EEPROM_LOC_DATA_SER_ADD,48);
			readStringFromEeprom(ucSerPath,EEPROM_LOC_DATA_SER_PATH,98);
			readStringFromEeprom(ucSerPort,EEPROM_LOC_DATA_SER_PORT,8);
			readStringFromEeprom(ucSerAppReg,EEPROM_LOC_SER_APP_REG,38);
			readStringFromEeprom(ucSerAppSysConfig,EEPROM_LOC_SER_APP_SYS_CONFIG,38);
			readStringFromEeprom(ucSerAppParaConfig,EEPROM_LOC_SER_APP_PARA_CONFIG,38);
			readStringFromEeprom(ucSerAppRecData,EEPROM_LOC_SER_APP_REC_DATA,38);
		}
	}
	UARTSendStr(UART_DEBUG,(UINT8*)"\r\n====================================================\r\nVANMEN IoT Controller\r\nServer: https://");
	UARTSendStr(UART_DEBUG,ucSerAdd);
	UARTSendStr(UART_DEBUG,(UINT8*)":");
	UARTSendStr(UART_DEBUG,ucSerPort);
	UARTSendStr(UART_DEBUG,(UINT8*)"\r\nEndpoints:\r\nPOST /");
	UARTSendStr(UART_DEBUG,ucSerAppReg);
	UARTSendStr(UART_DEBUG,(UINT8*)" - Device Registration\r\nPOST /");
	UARTSendStr(UART_DEBUG,ucSerAppSysConfig);
	UARTSendStr(UART_DEBUG,(UINT8*)" - System Configuration\r\nPOST /");
	UARTSendStr(UART_DEBUG,ucSerAppParaConfig);
	UARTSendStr(UART_DEBUG,(UINT8*)" - DAQ Parameter Config\r\nPOST /");
	UARTSendStr(UART_DEBUG,ucSerAppRecData);
	UARTSendStr(UART_DEBUG,(UINT8*)" - Telemetry Ingestion\r\nPOST /api/relay-config - Relay Module Config\r\nGET  /health - Health Check\r\n====================================================\r\n");
	
	
	readStringFromEeprom(ucGPRSAPNStr,EEPROM_LOC_GPRS_APN,23);
	readStringFromEeprom(ucGPRSUserNameStr,EEPROM_LOC_GPRS_USERNAME,23);
	readStringFromEeprom(ucGPRSPasswordStr,EEPROM_LOC_GPRS_PASSWORD,23);
	readStringFromEeprom(ucSIMBalNo,EEPROM_LOC_SIM_BAL_N0,8);
	readStringFromEeprom(ucSIMBalKeyWord,EEPROM_LOC_SIM_BAL_KW,8);
	readExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_SIM_BAL_CNTR,&ulSIMBalCntr);
//	UARTSendStr(UART_DEBUG,(UINT8 *)"\r NW APN : ");
//	UARTSendStr(UART_DEBUG,ucGPRSAPNStr);
//	UARTSendStr(UART_DEBUG,(UINT8 *)"\r NW USN : ");
//	UARTSendStr(UART_DEBUG,ucGPRSUserNameStr);
//	UARTSendStr(UART_DEBUG,(UINT8 *)"\r NW PWD : ");
//	UARTSendStr(UART_DEBUG,ucGPRSPasswordStr);


	readStringFromEeprom(ucModemSpecialCmd,EEPROM_LOC_MODEM_SPL_COMMAND,45);
	
	
	
	readStringFromEeprom(ucAlertMobileNo1,EEPROM_LOC_ALERT_MOB1,13);
	readStringFromEeprom(ucAlertMobileNo2,EEPROM_LOC_ALERT_MOB2,13);
//	UARTSendStr(UART_DEBUG,(UINT8 *)"\r Alert Mob 1 : ");
//	UARTSendStr(UART_DEBUG,ucAlertMobileNo1);
//	UARTSendStr(UART_DEBUG,(UINT8 *)"\r Alert Mob 2 : ");
//	UARTSendStr(UART_DEBUG,ucAlertMobileNo2);
	
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RELAY1_STATUS,&ucRelay1Status);
	if(ucRelay1Status>1)
		ucRelay1Status=0;
	if((ucRelay1OnHrs1==0) && (ucRelay1OnMin1==0) && (ucRelay1OffHrs1==0) && (ucRelay1OffMin1==0))
	{
		RELAY_1=ucRelay1Status;
	}
		
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RELAY1_PARA,&ucRelay1Parameter);
	if(ucRelay1Parameter>250)
		ucRelay1Parameter=0;
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_RELAY1_ON,&fRelay1ParaON);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_RELAY1_OFF,&fRelay1ParaOFF);
//	UARTSendStr(UART_DEBUG,(UINT8 *)"\r Rly 1 Status : ");
//	UARTSendLong(UART_DEBUG,ucRelay1Status,1);
//	UARTSendStr(UART_DEBUG,(UINT8 *)", Para : ");
//	UARTSendLong(UART_DEBUG,ucRelay1Parameter,3);
//	UARTSendStr(UART_DEBUG,(UINT8 *)", ON : ");
//	UARTSendFloat(UART_DEBUG,fRelay1ParaON,5,3);
//	UARTSendStr(UART_DEBUG,(UINT8 *)", OFF : ");
//	UARTSendFloat(UART_DEBUG,fRelay1ParaOFF,5,3);
	
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RELAY2_STATUS,&ucRelay2Status);
	if(ucRelay2Status>1)
		ucRelay2Status=0;
	if((ucRelay2OnHrs1==0) && (ucRelay2OnMin1==0) && (ucRelay2OffHrs1==0) && (ucRelay2OffMin1==0))
	{
		RELAY_2=ucRelay2Status;
	}
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RELAY2_PARA,&ucRelay2Parameter);
	if(ucRelay2Parameter>250)
		ucRelay2Parameter=0;
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_RELAY2_ON,&fRelay2ParaON);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_RELAY2_OFF,&fRelay2ParaOFF);
//	UARTSendStr(UART_DEBUG,(UINT8 *)"\r Rly 2 Status : ");
//	UARTSendLong(UART_DEBUG,ucRelay2Status,1);
//	UARTSendStr(UART_DEBUG,(UINT8 *)", Para : ");
//	UARTSendLong(UART_DEBUG,ucRelay2Parameter,3);
//	UARTSendStr(UART_DEBUG,(UINT8 *)", ON : ");
//	UARTSendFloat(UART_DEBUG,fRelay2ParaON,5,3);
//	UARTSendStr(UART_DEBUG,(UINT8 *)", OFF : ");
//	UARTSendFloat(UART_DEBUG,fRelay2ParaOFF,5,3);
	
	//1043
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_MODBUS_ID,&ucMasterModbusID);	
	if( (ucMasterModbusID == 0 ) || (ucMasterModbusID==0xff) )
		ucMasterModbusID=1;

	//1044	
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_DEBUG_BAUD_RATE,&ucDebugBaudRate);	
	if( (ucDebugBaudRate == 0 ) || (ucDebugBaudRate==0xff) )
		ucDebugBaudRate=7;
		
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RELAY3_STATUS,&ucRelay3Status);
	if(ucRelay3Status>1)
		ucRelay3Status=0;
	if((ucRelay3OnHrs1==0) && (ucRelay3OnMin1==0) && (ucRelay3OffHrs1==0) && (ucRelay3OffMin1==0))
	{
		RELAY_3=ucRelay3Status;
	}
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RELAY3_PARA,&ucRelay3Parameter);
	if(ucRelay3Parameter>250)
		ucRelay3Parameter=0;
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_RELAY3_ON,&fRelay3ParaON);
	readExtEEPROMFloat(EEPROM_ADD,EEPROM_LOC_RELAY3_OFF,&fRelay3ParaOFF);
//	UARTSendStr(UART_DEBUG,(UINT8 *)"\r Rly 3 Status : ");
//	UARTSendLong(UART_DEBUG,ucRelay3Status,1);
//	UARTSendStr(UART_DEBUG,(UINT8 *)", Para : ");
//	UARTSendLong(UART_DEBUG,ucRelay3Parameter,3);
//	UARTSendStr(UART_DEBUG,(UINT8 *)", ON : ");
//	UARTSendFloat(UART_DEBUG,fRelay3ParaON,5,3);
//	UARTSendStr(UART_DEBUG,(UINT8 *)", OFF : ");
//	UARTSendFloat(UART_DEBUG,fRelay3ParaOFF,5,3);
	
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1__ON_HRS_1,&ucRelay1OnHrs1);
	ucRelay1OnHrs1=validateUint8Para(ucRelay1OnHrs1,0,23);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1__ON_MIN_1,&ucRelay1OnMin1);
	ucRelay1OnMin1=validateUint8Para(ucRelay1OnMin1,0,59);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1_OFF_HRS_1,&ucRelay1OffHrs1);
	ucRelay1OffHrs1=validateUint8Para(ucRelay1OffHrs1,0,23);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1_OFF_MIN_1,&ucRelay1OffMin1);
	ucRelay1OffMin1=validateUint8Para(ucRelay1OffMin1,0,59);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1__ON_HRS_2,&ucRelay1OnHrs2);
	ucRelay1OnHrs2=validateUint8Para(ucRelay1OnHrs2,0,23);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1__ON_MIN_2,&ucRelay1OnMin2);
	ucRelay1OnMin2=validateUint8Para(ucRelay1OnMin2,0,59);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1_OFF_HRS_2,&ucRelay1OffHrs2);
	ucRelay1OffHrs2=validateUint8Para(ucRelay1OffHrs2,0,23);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY1_OFF_MIN_2,&ucRelay1OffMin2);
	ucRelay1OffMin2=validateUint8Para(ucRelay1OffMin2,0,59);
//	UARTSendStr(UART_DEBUG,(UINT8 *)"\r Rly 1 TMR1 : ");
//	UARTSendLong(UART_DEBUG,ucRelay1OnHrs1,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)":");
//	UARTSendLong(UART_DEBUG,ucRelay1OnMin1,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)" ");
//	UARTSendLong(UART_DEBUG,ucRelay1OffHrs1,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)":");
//	UARTSendLong(UART_DEBUG,ucRelay1OffMin1,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)", TMR2 : ");
//	UARTSendLong(UART_DEBUG,ucRelay1OnHrs2,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)":");
//	UARTSendLong(UART_DEBUG,ucRelay1OnMin2,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)" ");
//	UARTSendLong(UART_DEBUG,ucRelay1OffHrs2,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)":");
//	UARTSendLong(UART_DEBUG,ucRelay1OffMin2,2);
	
	
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2__ON_HRS_1,&ucRelay2OnHrs1);
	ucRelay1OnHrs1=validateUint8Para(ucRelay2OnHrs1,0,23);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2__ON_MIN_1,&ucRelay2OnMin1);
	ucRelay2OnMin1=validateUint8Para(ucRelay2OnMin1,0,59);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2_OFF_HRS_1,&ucRelay2OffHrs1);
	ucRelay1OnHrs1=validateUint8Para(ucRelay2OffHrs1,0,23);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2_OFF_MIN_1,&ucRelay2OffMin1);
	ucRelay2OffMin1=validateUint8Para(ucRelay2OffMin1,0,59);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2__ON_HRS_2,&ucRelay2OnHrs2);
	ucRelay1OnHrs1=validateUint8Para(ucRelay2OnHrs2,0,23);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2__ON_MIN_2,&ucRelay2OnMin2);
	ucRelay2OnMin2=validateUint8Para(ucRelay2OnMin2,0,59);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2_OFF_HRS_2,&ucRelay2OffHrs2);
	ucRelay1OnHrs1=validateUint8Para(ucRelay2OffHrs2,0,23);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY2_OFF_MIN_2,&ucRelay2OffMin2);
	ucRelay2OffMin2=validateUint8Para(ucRelay2OffMin2,0,59);
//	UARTSendStr(UART_DEBUG,(UINT8 *)"\r Rly 2 TMR1 : ");
//	UARTSendLong(UART_DEBUG,ucRelay2OnHrs1,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)":");
//	UARTSendLong(UART_DEBUG,ucRelay2OnMin1,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)" ");
//	UARTSendLong(UART_DEBUG,ucRelay2OffHrs1,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)":");
//	UARTSendLong(UART_DEBUG,ucRelay2OffMin1,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)", TMR2 : ");
//	UARTSendLong(UART_DEBUG,ucRelay2OnHrs2,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)":");
//	UARTSendLong(UART_DEBUG,ucRelay2OnMin2,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)" ");
//	UARTSendLong(UART_DEBUG,ucRelay2OffHrs2,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)":");
//	UARTSendLong(UART_DEBUG,ucRelay2OffMin2,2);
	
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3__ON_HRS_1,&ucRelay3OnHrs1);
	ucRelay1OnHrs1=validateUint8Para(ucRelay3OnHrs1,0,23);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3__ON_MIN_1,&ucRelay3OnMin1);
	ucRelay3OnMin1=validateUint8Para(ucRelay3OnMin1,0,59);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3_OFF_HRS_1,&ucRelay3OffHrs1);
	ucRelay1OnHrs1=validateUint8Para(ucRelay3OffHrs1,0,23);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3_OFF_MIN_1,&ucRelay3OffMin1);
	ucRelay3OffMin1=validateUint8Para(ucRelay3OffMin1,0,59);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3__ON_HRS_2,&ucRelay3OnHrs2);
	ucRelay1OnHrs1=validateUint8Para(ucRelay3OnHrs2,0,23);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3__ON_MIN_2,&ucRelay3OnMin2);
	ucRelay3OnMin2=validateUint8Para(ucRelay3OnMin2,0,59);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3_OFF_HRS_2,&ucRelay3OffHrs2);
	ucRelay1OnHrs1=validateUint8Para(ucRelay3OffHrs2,0,23);
	readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_RLY3_OFF_MIN_2,&ucRelay3OffMin2);
	ucRelay3OffMin2=validateUint8Para(ucRelay3OffMin2,0,59);
//	UARTSendStr(UART_DEBUG,(UINT8 *)"\r Rly 3 TMR1 : ");
//	UARTSendLong(UART_DEBUG,ucRelay3OnHrs1,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)":");
//	UARTSendLong(UART_DEBUG,ucRelay3OnMin1,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)" ");
//	UARTSendLong(UART_DEBUG,ucRelay3OffHrs1,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)":");
//	UARTSendLong(UART_DEBUG,ucRelay3OffMin1,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)", TMR2 : ");
//	UARTSendLong(UART_DEBUG,ucRelay3OnHrs2,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)":");
//	UARTSendLong(UART_DEBUG,ucRelay3OnMin2,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)" ");
//	UARTSendLong(UART_DEBUG,ucRelay3OffHrs2,2);
//	UARTSendStr(UART_DEBUG,(UINT8 *)":");
//	UARTSendLong(UART_DEBUG,ucRelay3OffMin2,2);
	
	
	if(SHOW_VERBOSE_STARTUP && ucEnableCSV==0)
	{
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r Trans period = ");
	}
	
	for(ucStatusF=0;ucStatusF<8;ucStatusF++)
	{
		readExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_TRANS_PERIOD+ucStatusF,&ucTransPeriod[ucStatusF]);
		if(SHOW_VERBOSE_STARTUP && ucEnableCSV==0)
		UARTSendChar(UART_DEBUG,ucTransPeriod[ucStatusF]);
	}
	
	
	if( (ucModemType==MODEM_TYPE_2G) && (ulSystemNoOfParameters>200)  )
		ulSystemNoOfParameters = 200;
	
	for(ulCntrF=0;ulCntrF<ulSystemNoOfParameters;ulCntrF++)
	{
		readParaConfig(ulCntrF);
		
		
		//save location
		if( (sParaConfig[ulCntrF].para.ucParaType>=4) && (sParaConfig[ulCntrF].para.ucParaType<=13) ) 
		{
			UINT16 uiAddressF;
			uiAddressF=sParaConfig[ulCntrF].para.ucParaType-4;
			uiAddressF=EEPROM_LOC_STORE_PARA_1_LOC+(4*uiAddressF);
		//	if(sParaConfig[ulParaCntrF].para.ucValueStatusBit != 1)
			{
				readExtEEPROMFloat(EEPROM_ADD,uiAddressF,&sParaConfig[ulCntrF].para.value.fValue);
			}
		//	else
		//	{
		//	       writeExtEEPROMFloat(EEPROM_ADD,uiAddressF, sParaConfig[ulParaCntrF].para.value.fValue);
		//	}
		}
		if(SHOW_VERBOSE_STARTUP && ucEnableCSV==0)
		displayPraConfig(ulCntrF);
	}
	
}


UINT8 storeParaConfig(UINT32 ulParaCntF)
{
	UINT32 ulCntrF=0;
        //UARTSendStr(UART_DEBUG,(UINT8*)"\rStoring para config =");
	//UARTSendLong(UART_DEBUG,ulParaCntF,4);
	for(ulCntrF=0;ulCntrF<EEPROM_LOC_CONFIG_PARA_OFFSET;ulCntrF++)
	{
		writeExtEEPROMChar(EEPROM_ADD,(UINT16)(EEPROM_LOC_CONFIG_PARA_START+(EEPROM_LOC_CONFIG_PARA_OFFSET*ulParaCntF)+ulCntrF),sParaConfig[ulParaCntF].ucPraConfigArray[ulCntrF]);
	}
	return SUCCESS;
}

UINT8 readParaConfig(UINT32 ulParaCntF)
{
	UINT32 ulCntrF=0;
	UINT8 ucTempV;
	//UARTSendStr(UART_DEBUG,(UINT8*)"\r Reading  para config =");
	//UARTSendLong(UART_DEBUG,ulParaCntF,4);
	for(ulCntrF=0;ulCntrF<EEPROM_LOC_CONFIG_PARA_OFFSET;ulCntrF++)
	{
		readExtEEPROMChar(EEPROM_ADD,(UINT16)(EEPROM_LOC_CONFIG_PARA_START+(EEPROM_LOC_CONFIG_PARA_OFFSET*ulParaCntF)+ulCntrF),&ucTempV);
		sParaConfig[ulParaCntF].ucPraConfigArray[ulCntrF]=ucTempV;
	}
	sParaConfig[ulParaCntF].para.ucErrorFlag=0;
	sParaConfig[ulParaCntF].para.ulErrorTimer=0;
	sParaConfig[ulParaCntF].para.value.ulValue=0;
	sParaConfig[ulParaCntF].para.ucValueProcessBit=0;
	sParaConfig[ulParaCntF].para.ucValueStatusBit=0;

	
	
	
	
	return SUCCESS;
}

UINT8 displayPraConfig(UINT32 ulParaIDTemp)
{
	UINT8 ucTempF;
	UARTSendStr(UART_DEBUG,(UINT8*)"\r Para id = ");
	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.uiParaID,3);

	UARTSendStr(UART_DEBUG,(UINT8*)" ,Para String = ");
	UARTSendStr(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucParaStr);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rPara Source= ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucParaSource,3);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rPara Type= ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucParaType,3);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rPara Acq Time = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucParaAcqTime,3);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rPara Cumulate = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucParaCumulate,3);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rPara Reset = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucParaResetHrs,2);
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucParaResetMin,2);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rMB Meter type = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucRS485MeterType,3);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rMB mode = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucRS485Mode,3);
//
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rMB mode = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucRS485Mode,3);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rMB datatype = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucRS485DataType,3);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rMB baud = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucRS485Baud,3);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rMB DataBits = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucRS485Bits,3);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rMB Parity = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucRS485Parity,3);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rMB StopBits = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucRS485Stopbits,3);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rMB Packet = ");
//	UARTSendStr(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucRS485Packet);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rMB Timeout = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.uiRS485Timout,5);
//	
//	
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rAlarm Enable = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucAlarmEnable,3);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rAlarm Type= ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucAlarmAlert,3);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rAlarm Alert = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ulAlarmDelay,3);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rAlarm delay = ");
//	UARTSendLong(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucAlarmType,5);
//	
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rAlarm ON TIME = ");
//	for(ucTempF=0;ucTempF<4;ucTempF++)
//	{
//		UARTSendChar(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucAlermOnTime[ucTempF]);
//	}
//	
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rAlarm OFF TIME = ");
//	for(ucTempF=0;ucTempF<4;ucTempF++)
//	{
//		UARTSendChar(UART_DEBUG,sParaConfig[ulParaIDTemp].para.ucAlermOffTime[ucTempF]);
//	}
//	
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rMin Val = ");
//	UARTSendFloat(UART_DEBUG,sParaConfig[ulParaIDTemp].para.fMinValue,5,5);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rMax Val = ");
//	UARTSendFloat(UART_DEBUG,sParaConfig[ulParaIDTemp].para.fMaxValue,5,5);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rMultiply fac = ");
//	UARTSendFloat(UART_DEBUG,sParaConfig[ulParaIDTemp].para.fMultiplyFactor,5,5);
//
//	UARTSendStr(UART_DEBUG,(UINT8*)"\rAdd Fac = ");
//	UARTSendFloat(UART_DEBUG,sParaConfig[ulParaIDTemp].para.fAdditionFactor,5,5);

	return SUCCESS;
}

UINT8 storeSMSToEEPROM(UINT8* ucStoreSMSMessage)
{
    UINT16 ucSmsCntr=0;
    UINT8 ucCharCntr=0;
    UINT8 i=0;
    for(ucSmsCntr=0;ucSmsCntr<7;ucSmsCntr++)
    {
        UINT8 ucSMSStatus=0;
        readExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_SMS_START + (ucSmsCntr*EEPROM_SMS_OFFSET), &ucSMSStatus);
        if(ucSMSStatus!=1)
        {
            writeExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_SMS_START + (ucSmsCntr*EEPROM_SMS_OFFSET), 1);
            ucCharCntr=0;
            while(*ucStoreSMSMessage)									//TRANSMIT EACH CHARACTER IN STRING
            {
                writeExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_SMS_START + (ucSmsCntr*EEPROM_SMS_OFFSET)+1+ucCharCntr, *ucStoreSMSMessage);
                ucStoreSMSMessage++;
                ucCharCntr++;
            }
            i=0;
            for(i=ucCharCntr;i<97;i++)
            {
              //  UINT8 ucStoredChar=0;
                writeExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_SMS_START + (ucSmsCntr*EEPROM_SMS_OFFSET)+1+i, 0x00);
               
            }
            return SUCCESS;
        }
    }
    return FAILURE;
}

UINT8 readStoredSMSFromEEPROM(UINT8* ucStoreSMSMessage)
{
    UINT16 ucSmsCntr=0;
    UINT8 i=0;
    UINT8 ucStoredChar=0;
    for(ucSmsCntr=0;ucSmsCntr<7;ucSmsCntr++)
    {
        UINT8 ucSMSStatus=0;
        readExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_SMS_START + (ucSmsCntr*EEPROM_SMS_OFFSET), &ucSMSStatus);
        if(ucSMSStatus==1)
        {
            writeExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_SMS_START + (ucSmsCntr*EEPROM_SMS_OFFSET), 0);
            i=0;
            for(i=0;i<96;i++)
            {
                ucStoredChar=0;
                readExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_SMS_START + (ucSmsCntr*EEPROM_SMS_OFFSET)+1+i, &ucStoredChar);
                if((ucStoredChar==0x00) || (i==95))
                {
                    *ucStoreSMSMessage=0x00;
                    return SUCCESS;
                }
                *ucStoreSMSMessage=ucStoredChar;
                ucStoreSMSMessage++;
                
            }
        }
    }
    return FAILURE;
}


void storeRTCValue()
{
	writeExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_RTC_YEAR, sRTC.ucYear);
	writeExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_RTC_MONTH, sRTC.ucMonth);
	writeExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_RTC_DAY, sRTC.ucDate);
	writeExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_RTC_HOUR, sRTC.ucHours);
	writeExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_RTC_MIN, sRTC.ucMinutes);
	
}
void readRTCValue()
{
	readExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_RTC_YEAR, &sRTC.ucYear);
	readExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_RTC_MONTH, &sRTC.ucMonth);
	readExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_RTC_DAY, &sRTC.ucDate);
	readExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_RTC_HOUR, &sRTC.ucHours);
	readExtEEPROMChar(EEPROM_ADD, EEPROM_LOC_RTC_MIN, &sRTC.ucMinutes);
	
	if(sRTC.ucYear<20 || sRTC.ucYear > 50)
	{
		sRTC.ucYear=20;
		sRTC.ucMonth=01;
		sRTC.ucDate=01;
		sRTC.ucHours=01;
		sRTC.ucMinutes=01;
	}
	
	
}
