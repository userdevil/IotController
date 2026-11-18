#ifndef __DEVICE_H_
#define __DEVICE_H_

#include "global.h"

#define N0_OF_PARAMETERS 200

#define RELAY_TIME_INVALID 	0
#define RELAY_TIME_TURNON 	1	
#define RELAY_TIME_TURNOFF	2

typedef union {
        UINT8 ucPraConfigArray[100];
        struct {
		UINT16 uiParaID;  //0

		UINT8 ucParaSource;  //2
		UINT8 ucParaType;    //3
		UINT8 ucParaAcqTime; //4
		UINT8 ucParaStr[10]; //5

		UINT8 ucParaCumulate;//15 
		UINT8 ucParaResetHrs;//16
		UINT8 ucParaResetMin;//17

		UINT8 ucRS485MeterType; //18
		UINT8 ucRS485Mode;      //19
		UINT8 ucRS485DataType;  //20
		UINT8 ucRS485Baud;      //21
		UINT8 ucRS485Bits;      //22
		UINT8 ucRS485Parity;	//23
		UINT8 ucRS485Stopbits;	//24
		UINT8 ucRS485Packet[30];//25 
		UINT16 uiRS485Timout;   //55 

		UINT8 ucAlarmAlert; 	//57
		UINT8 ucAlarmEnable;	//58
		UINT8 ucAlarmType;	//59
		UINT32 ulAlarmDelay; 	//60
		UINT8 ucAlermOnTime[4];	//64
		UINT8 ucAlermOffTime[4];//68	


		F32 fMinValue;		//72
		F32 fMaxValue;		//76
		F32 fMultiplyFactor;	//80
		F32 fAdditionFactor;	//84

		UINT32 ulErrorTimer;	//88
		UINT8 ucErrorFlag;	//90	
		UINT8 ucdummy;		//91

		UINT8 ucValueProcessBit;//92	
		UINT8 ucValueStatusBit;	//93
		
		union {
	                UINT32 ulValue;	//84
	                F32 fValue;
	                UINT8 ucValue[4];
           	 }value;
		
	}para;
}PARA_CONFIG;

extern UINT8 ucIMIENumber[16];
extern UINT8 ucGPRSSMStateMax;

extern PARA_CONFIG sParaConfig[N0_OF_PARAMETERS];
		

extern UINT8 ucInitComplete;
		
		
extern UINT8 ucClockReadFromModem;

extern UINT16 uiAcqIntMin;
extern UINT16 uiTraIntMin;
extern UINT8 ucPraAcqFlag;
extern UINT8 ucPraTraFlag;
extern UINT8 ucCheckRelayOperation;
extern UINT16 uiPendingFiles;

extern UINT8 ucBoardTestSMFlag;

#define MODEM_TYPE_2G 0
#define MODEM_TYPE_4G 1
extern UINT8 ucModemType;

void uploadBoardTestReport(void);
void imieRegistration(void);
void downloadUnitConfig(void);
void downloadParaConfig(void);
void checkBufferForCommands(UINT8* pucBuffer, UINT16 uiBufferSize, UINT8 ucClearBuffer);
//void tranferDataToServer(UINT8* ucFlag,UINT8* ucTransmitBuffer,UINT8 ucMode,UINT8 ucClosConState);
void tranferDataToServer(UINT8* ucFlag,UINT8* ucTransmitBuffer,UINT8* ucApplication,UINT8 ucMode,UINT8 ucClosConState);
UINT8 decodeParaConfigPacket(UINT8* ucTempStr);
void checkRelay1Operation();
void checkRelay2Operation();
void checkRelay3Operation();

void sendJsonParaInfoViaDebug(void);
void sendJsonSystemInfoViaDebug(void);
void sendJsonBoardInfoViaDebug(void);

UINT8 getRelayStatusBasedOnTime(UINT8 ucOneHrs,UINT8 ucOnMin,UINT8 ucOffHrs,UINT8 ucOffMin);

void getIndivitualParaConfig(UINT8* pucPara);
void sendJsonPaketString(UINT8* pucParameter, UINT8* pucValue,UINT8 ucSendComma);

UINT8 buildGetSysInfoJSONHex(UINT8 *pucJsonBuf, UINT16 uiMaxLen);
void handleParaValueRequest(UINT8 *pucBuffer);
PARA_CONFIG* getParaById(UINT16 paraId);

#endif
