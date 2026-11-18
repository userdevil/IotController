#ifndef __DEVICE_GPRS_
#define __DEVICE_GPRS_

#include "global.h"

#define STATE_RESET 				0
#define GPRS_STATE_WAIT_BEFORE_NEXT_RESET	1
#define GPRS_STATE_CHECK_MODEM 			2
#define GPRS_STATE_GET_IMIE 			3
#define GPRS_STATE_SETUP_TIME_SYNC		4
#define GPRS_STATE_ENABLE_ERROR_MODE   		5
#define GPRS_STATE_CHECK_SIM 			6
#define GPRS_STATE_SPL_CMD	 		7
#define GPRS_STATE_CHECK_CONNECTION 		8

#define GPRS_STATE_SIGNAL_STRENGTH		9
#define GPRS_STATE_GET_TIME			10

#define GPRS_STATE_START_IP_STACK		22
#define GPRS_STATE_STOP_IP_STACK		23
#define GPRS_STATE_SET_GPRS_APN_USN_PWD		24
#define GPRS_STATE_GET_LOCAL_IP_ADD		25
#define GPRS_STATE_START_GPRS_BEARER		26
#define GPRS_STATE_STOP_GPRS_BEARER		27
#define GPRS_STATE_CHECK_CONN_STATUS		28
#define GPRS_STATE_SET_IP_MODE			29
#define GPRS_STATE_EXTABLISH_CONNECTION_CLIENT	30
#define GPRS_STATE_ENTER_DATA_MODE_CLIENT	31
#define GPRS_STATE_TRANSFER_DATA_CLIENT		32
#define GPRS_STATE_EXIT_DATA_MODE_CLIENT	33
#define GPRS_STATE_CLOSE_CONNECTION_CLIENT	34
#define GPRS_STATE_IDLE_CON_OPEN		35
#define GPRS_STATE_IDLE_CON_CLOSE		36

#define STATE_IDLE 				37


#define GPRS_STATE_ENABLE_SMS_RECP 				11
#define GPRS_STATE_ENABLE_SMS_RECP_EXT 				12
#define GPRS_STATE_CON_PARA_HTTP_SERVER_CONTEXT_ID		13
#define GPRS_STATE_CON_PARA_HTTP_SERVER_RESPONSE_HEADER		14

#define GPRS_STATE_ACTIVATE_CONTEXT		15

#define GPRS_STATE_SIGNAL_STRENGTH_POST		16
#define GPRS_STATE_SETUP_URL			17
#define GPRS_STATE_CONNECT_TO_URL		18
#define GPRS_STATE_SETUP_POST			19
#define GPRS_STATE_POST_DATA			20
#define GPRS_STATE_READ_RESPONSE		21




#define NO_OF_RETRY_1		1
#define NO_OF_RETRY_2		2
#define NO_OF_RETRY_3		3
#define NO_OF_RETRY_4		4
#define NO_OF_RETRY_5		5
#define NO_OF_RETRY_6		6
#define NO_OF_RETRY_7		7
#define NO_OF_RETRY_8		8
#define NO_OF_RETRY_9		9
#define NO_OF_RETRY_10		10
#define NO_OF_RETRY_30		30

	
#define GPRS_TRANS_MODE_GET	0
#define GPRS_TRANS_MODE_POST	1

#define GPRS_CONNECTION_CLOSE		0
#define GPRS_CONNECTION_KEEP_OPEN	1


extern UINT8 ucLocalIP[20];
extern UINT8 ucNetworkStatus;
extern UINT8 ucSIMStatus;
extern UINT8 ucGPRSConStatus;
extern UINT8 ucGPRSDataTranferFlag;
extern UINT8 ucSignalStrength;
extern UINT8 ucDataTransferSuccess;
extern UINT8 ucGPRSSMState;

extern UINT8 ucSendSMSFlag;
extern UINT8 ucSMSReceivedFlag;
extern UINT8 ucSenderMobNumberStr[15];

extern UINT16 ucMS_WAIT_BEFORE_RESET;

extern UINT8 	ucInComingCallFlag;
extern UINT16 	uiInComingCallTimer;



void gprsTranferDataToServer4G(unsigned char* pucSMSStart, UINT8* pucTransDataBuffer, UINT8* pucApplicationName, UINT8 ucTrasferMode, UINT8 ucKeepConnectionOpen);

void prepareGPRSTransmitPacket(UINT8* pucData, UINT8* pucTransmitBuffer, UINT8* pucServerAdd, UINT8* pucServerPort, UINT8* pucServerPath, UINT8* pucApplicationName, UINT8 ucTransferMode);

UINT8 setRTCModem(UINT8 ucDate,UINT8 ucMonth,UINT8 ucYear,UINT8 ucHours,UINT8 ucMinutes,UINT8 ucSeconds,UINT8 ucGTM);
UINT8 sendSMSStateMachine(UINT8* pucSMFlagF);
UINT8 sendInstantSms(UINT8* pucNumber, UINT8* pucMessage);

void checkForIncomingCall();

#endif