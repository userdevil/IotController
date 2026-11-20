#ifndef _DEVICE_MEMORY_H
#define _DEVICE_MEMORY_H

#include "global.h"

//starts from address 100
//EEPROM RELATED
#define EEPROM_LOC_STORED_IMIE_NUMBER	80	//16 byte modem IMEI storage

#define EEPROM_LOC_BOARD_TEST_STATUS	100	//1 byte, 	1   -> board has been tested
#define EEPROM_LOC_BOARD_TEST_ID	101	//4 byte,	
#define EEPROM_LOC_BOARD_TEST_DATE	105	//1 byte,
#define EEPROM_LOC_BOARD_TEST_MONTH	106	//1 byte,
#define EEPROM_LOC_BOARD_TEST_YEAR	107	//1 byte,
#define EEPROM_LOC_EEPROM_MEM_STATUS	108	//1 byte,	'e' -> default values had been loaded into memeory


//SYSTEM RELATED
#define EEPROM_LOC_SYSTEM_REG_STATUS	109	//1 byte,	1	->board has received valid system id and 
#define EEPROM_LOC_SYSTEM_ID		110	//4 bytes	
#define EEPROM_LOC_NO_OF_PARA		114	//4 BYTES
#define EEPROM_LOC_DL_UNIT_CONIG	118	//1 BYTE	1	-> DOWNLOAD UNIT CONFIG
#define EEPROM_LOC_DL_PARA_CONIG	119	//1 BYTE	1	-> DOWNLOAD UNIT CONFIG
#define EEPROM_LOC_DATA_CNTR		120	//4 bytes
#define EEPROM_LOC_TRANS_INTERVAL	124	//4 bytes
#define EEPROM_LOC_INTERNET_SOURCE	128	//1 bytes
#define EEPROM_LOC_DL_RELAY_MODULE_CONFIG 129 //1 byte

#define EEPROM_LOC_STORE_PARA_1_LOC     130   	//4 bytes    
#define EEPROM_LOC_STORE_PARA_2_LOC     134	//4 bytes     
#define EEPROM_LOC_STORE_PARA_3_LOC     138     //4 bytes
#define EEPROM_LOC_STORE_PARA_4_LOC     142     //4 bytes
#define EEPROM_LOC_STORE_PARA_5_LOC     146	//4 bytes
#define EEPROM_LOC_STORE_PARA_6_LOC     150     //4 bytes  
#define EEPROM_LOC_STORE_PARA_7_LOC     154     //4 bytes
#define EEPROM_LOC_STORE_PARA_8_LOC     158     //4 bytes
#define EEPROM_LOC_STORE_PARA_9_LOC     162     //4 bytes
#define EEPROM_LOC_STORE_PARA_10_LOC    166	//4 bytes
#define EEPROM_LOC_ACQ_INTERVAL		170	//4 bytes
#define EEPROM_LOC_APP_TYPE		174	//1 bytes
#define EEPROM_LOC_INT_CNTR		176	//4 bytes


#define EEPROM_LOC_STORE_PARA_11_LOC    550	//4 bytes
#define EEPROM_LOC_STORE_PARA_12_LOC    554	//4 bytes
#define EEPROM_LOC_STORE_PARA_13_LOC    558	//4 bytes
#define EEPROM_LOC_STORE_PARA_14_LOC    562	//4 bytes
#define EEPROM_LOC_STORE_PARA_15_LOC    566	//4 bytes
#define EEPROM_LOC_STORE_PARA_16_LOC    570	//4 bytes
#define EEPROM_LOC_STORE_PARA_17_LOC    574	//4 bytes
#define EEPROM_LOC_STORE_PARA_18_LOC    578	//4 bytes
#define EEPROM_LOC_STORE_PARA_19_LOC    582	//4 bytes
#define EEPROM_LOC_STORE_PARA_20_LOC    586	//4 bytes


//FLASH RELATED
#define EEPROM_LOC_FLASH_SAVE_CNTR	180	//4 bytes	
#define EEPROM_LOC_FLASH_SENT_CNTR	184	//4 bytes

//SMS LOCATION
#define EEPROM_LOC_SMS_SAVE_CNTR	190	//1 bytes	
#define EEPROM_LOC_SMS_SENT_CNTR	191	//1 bytes


//SERVER
#define EEPROM_LOC_DATA_SER_ADD		200	// 50 BYTES
#define EEPROM_LOC_DATA_SER_PATH	250	// 100 BYTES
#define EEPROM_LOC_DATA_SER_PORT	350	// 10 BYTES
#define EEPROM_LOC_SER_APP_REG		360
#define EEPROM_LOC_SER_APP_SYS_CONFIG	400
#define EEPROM_LOC_SER_APP_PARA_CONFIG	450
#define EEPROM_LOC_SER_APP_REC_DATA	500

//NETWORK SETTINGS
#define EEPROM_LOC_GPRS_APN 		600	// 25 BYTES
#define EEPROM_LOC_GPRS_USERNAME 	625	// 25 BYTES
#define EEPROM_LOC_GPRS_PASSWORD 	650	// 25 BYTES
#define EEPROM_LOC_SIM_BAL_N0		675	// 10 BYTES EX: *123#		number to get sim balance
#define EEPROM_LOC_SIM_BAL_KW		685	// 10 BYTES Balance			key work to be searched finding sim balance
#define EEPROM_LOC_SIM_BAL_CNTR		695	// 4 BYTES no of char to read after bal_kw

//USER SETTINGS
#define EEPROM_LOC_ALERT_MOB1 		700	// 15 BYTES	ALERT MOBILE NUMBER 1
#define EEPROM_LOC_ALERT_MOB2 		715	// 15 BYTES  ALERT MOBILE NUMBER 2


//RELAY SETTINGS
//PARA CONTROL
#define EEPROM_LOC_RELAY1_PARA		750		// parameter id for realy control
#define EEPROM_LOC_RELAY1_ON	    	751		// realy will turn on below this value
#define EEPROM_LOC_RELAY1_OFF		755		// realy will turn off above this value
#define EEPROM_LOC_RELAY1_STATUS	759		//STATUS OF RELAY ONE WILL TO STORED HERE,	

#define EEPROM_LOC_RELAY2_PARA      	760		// parameter id for realy control
#define EEPROM_LOC_RELAY2_ON        	761		// realy will turn on below this value
#define EEPROM_LOC_RELAY2_OFF       	765		// realy will turn off above this value
#define EEPROM_LOC_RELAY2_STATUS	769		//STATUS OF RELAY ONE WILL TO STORED HERE,

#define EEPROM_LOC_RELAY3_PARA      	770		// parameter id for realy control
#define EEPROM_LOC_RELAY3_ON        	771		// realy will turn on below this value
#define EEPROM_LOC_RELAY3_OFF       	775		// realy will turn off above this value
#define EEPROM_LOC_RELAY3_STATUS	779		//STATUS OF RELAY ONE WILL TO STORED HERE,

//TIME CONTROL  ALL 1 BYTE
#define EEPROM_LOC_RLY1__ON_HRS_1	780
#define EEPROM_LOC_RLY1__ON_MIN_1	781
#define EEPROM_LOC_RLY1_OFF_HRS_1	782
#define EEPROM_LOC_RLY1_OFF_MIN_1	783
#define EEPROM_LOC_RLY1__ON_HRS_2	784
#define EEPROM_LOC_RLY1__ON_MIN_2	785
#define EEPROM_LOC_RLY1_OFF_HRS_2	786
#define EEPROM_LOC_RLY1_OFF_MIN_2	787

#define EEPROM_LOC_RLY2__ON_HRS_1	790
#define EEPROM_LOC_RLY2__ON_MIN_1	791
#define EEPROM_LOC_RLY2_OFF_HRS_1	792
#define EEPROM_LOC_RLY2_OFF_MIN_1	793
#define EEPROM_LOC_RLY2__ON_HRS_2	794
#define EEPROM_LOC_RLY2__ON_MIN_2	795
#define EEPROM_LOC_RLY2_OFF_HRS_2	796
#define EEPROM_LOC_RLY2_OFF_MIN_2	797

#define EEPROM_LOC_RLY3__ON_HRS_1	800
#define EEPROM_LOC_RLY3__ON_MIN_1	801
#define EEPROM_LOC_RLY3_OFF_HRS_1	802
#define EEPROM_LOC_RLY3_OFF_MIN_1	803
#define EEPROM_LOC_RLY3__ON_HRS_2	804
#define EEPROM_LOC_RLY3__ON_MIN_2	805
#define EEPROM_LOC_RLY3_OFF_HRS_2	806
#define EEPROM_LOC_RLY3_OFF_MIN_2	807

#define EEPROM_LOC_TRANS_PERIOD		950	//10 LOCATIONS

#define EEPROM_LOC_ENABLE_CSV		962	//float 4 location required //1016

#define EEPROM_LOC_RTC_YEAR		975
#define EEPROM_LOC_RTC_MONTH		976
#define EEPROM_LOC_RTC_DAY		977
#define EEPROM_LOC_RTC_HOUR		978
#define EEPROM_LOC_RTC_MIN		979

#define EEPROM_LOC_MODBUS_ID		1040	//UPDATE 1043 
#define EEPROM_LOC_DEBUG_BAUD_RATE	1044	//UPDATE 1044

#define EEPROM_LOC_MODEM_SPL_COMMAND	1050    //1045


#define EEPROM_LOC_SMS_START            1500    // SMS will be stored in this location
#define EEPROM_SMS_OFFSET               100     // max size of the sms will be this, location 0 will hold the status 0-> sent 1-> yet to be sent
    
#define EEPROM_LOC_CONFIG_PARA_START 	3000
#define EEPROM_LOC_CONFIG_PARA_OFFSET 	100


	
extern UINT8 	ucFlashID;
extern UINT8 	ucBoardTestStatus;
extern UINT32 	ulBoardTestID;
extern UINT8 	ucBoardTestDate;
extern UINT8 	ucBoardTestMonth;
extern UINT8 	ucBoardTestYear;
extern UINT8 	ucBoardMemeoryStatus;

extern UINT8 	ucBaordTestInProgress;

extern UINT8 	ucEnableCSV;//1016

extern UINT8 	ucSystemRegistrationStatus;
extern UINT8 	ucInternetSource;
extern UINT32 	ulSystemID;
extern UINT8 	ucAppType;
extern UINT32 	ulSystemNoOfParameters;
extern UINT8 	ucSystemDownloadUnitConfig;
extern UINT8 	ucSystemDownloadParaConfig;
extern UINT8 	ucSystemDownloadRelayModuleConfig;
extern UINT32 	ulDataAcqCntr;
extern UINT32 	ulTransmissionInterwal;
extern UINT32 	ulAcqInterwal;
extern F32 	fParaStoreLocation1;
extern F32 	fParaStoreLocation2;
extern F32 	fParaStoreLocation3;
extern F32 	fParaStoreLocation4;
extern F32 	fParaStoreLocation5;
extern F32 	fParaStoreLocation6;
extern F32 	fParaStoreLocation7;
extern F32 	fParaStoreLocation8;
extern F32 	fParaStoreLocation9;
//1017
extern F32 	fParaStoreLocation10;
extern F32 	fParaStoreLocation11;
extern F32 	fParaStoreLocation12;
extern F32 	fParaStoreLocation13;
extern F32 	fParaStoreLocation14;
extern F32 	fParaStoreLocation15;
extern F32 	fParaStoreLocation16;
extern F32 	fParaStoreLocation17;
extern F32 	fParaStoreLocation18;
extern F32 	fParaStoreLocation19;
extern F32 	fParaStoreLocation20;

extern UINT32 ulFlashFileSaveLocation;
extern UINT32 ulFlashFileSentLocation;

extern UINT8 ucSMSStoredNo;
extern UINT8 ucSMSSentNo;

extern UINT8 ucSerAdd[50];
extern UINT8 ucSerPath[100];
extern UINT8 ucSerPort[10];
extern UINT8 ucSerAppReg[40];
extern UINT8 ucSerAppSysConfig[40];
extern UINT8 ucSerAppParaConfig[40];
extern UINT8 ucSerAppRecData[40];

extern UINT8 ucGPRSAPNStr[25];											
extern UINT8 ucGPRSUserNameStr[25];
extern UINT8 ucGPRSPasswordStr[25];
extern UINT8 ucSIMBalNo[10];
extern UINT8 ucSIMBalKeyWord[10];
extern UINT32 ulSIMBalCntr;


extern UINT8 ucAlertMobileNo1[15];
extern UINT8 ucAlertMobileNo2[15];

extern UINT8 ucModemSpecialCmd[49];


extern UINT8 ucRelay1Status;
extern UINT8 ucRelay1Parameter;
extern F32 fRelay1ParaON;
extern F32 fRelay1ParaOFF;

extern UINT8 ucRelay2Status;
extern UINT8 ucRelay2Parameter;
extern F32 fRelay2ParaON;
extern F32 fRelay2ParaOFF;

extern UINT8 ucRelay3Status;
extern UINT8 ucRelay3Parameter;
extern F32 fRelay3ParaON;
extern F32 fRelay3ParaOFF;

extern UINT8 ucRelay1OnHrs1;
extern UINT8 ucRelay1OnMin1;
extern UINT8 ucRelay1OffHrs1;
extern UINT8 ucRelay1OffMin1;
extern UINT8 ucRelay1OnHrs2;
extern UINT8 ucRelay1OnMin2;
extern UINT8 ucRelay1OffHrs2;
extern UINT8 ucRelay1OffMin2;

extern UINT8 ucRelay2OnHrs1;
extern UINT8 ucRelay2OnMin1;
extern UINT8 ucRelay2OffHrs1;
extern UINT8 ucRelay2OffMin1;
extern UINT8 ucRelay2OnHrs2;
extern UINT8 ucRelay2OnMin2;
extern UINT8 ucRelay2OffHrs2;
extern UINT8 ucRelay2OffMin2;

extern UINT8 ucRelay3OnHrs1;
extern UINT8 ucRelay3OnMin1;
extern UINT8 ucRelay3OffHrs1;
extern UINT8 ucRelay3OffMin1;
extern UINT8 ucRelay3OnHrs2;
extern UINT8 ucRelay3OnMin2;
extern UINT8 ucRelay3OffHrs2;
extern UINT8 ucRelay3OffMin2;

extern UINT8 ucMasterModbusID;//1043
extern UINT8 ucDebugBaudRate;//1043

extern UINT8 ucTransPeriod[8];

void readDataFromMemory(void);
void storeStringToEeprom(UINT8* pucString,UINT16 ucLocation, UINT8 maxLength);
UINT8 storeParaConfig(UINT32 ulParaCntF);
UINT8 readParaConfig(UINT32 ulParaCntF);
UINT8 displayPraConfig(UINT32 ulParaIDTemp);
UINT8 storeSMSToEEPROM(UINT8* ucStoreSMSMessage);
UINT8 storeSMSToEEPROM(UINT8* ucStoreSMSMessage);
UINT8 readStoredSMSFromEEPROM(UINT8* ucStoreSMSMessage);

void storeRTCValue();
void readRTCValue();

void readStringFromEeprom(UINT8* pucString,UINT16 ucLocation, UINT8 maxLength);

#endif
