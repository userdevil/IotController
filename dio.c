#include "dio.h"
#include "delay.h"
#include "uartDebug.h"
#include "rtc.h"
#include "eeprom.h"
#include "memory.h"
#include "ui.h"

UINT8 ucFlagRstButtonPressed;
UINT8 ucKeyTimer;

UINT32 ulIntCntr;
UINT32 ulIntTimeMs;
UINT32 ulIntTimeMsPre;

UINT32 ulFreqPulsesPerSecond;
UINT32 ulPreviousIntCntr;

F32 fFreq;

UINT8 uckeyPreTestUP;
UINT8 uckeyPreTestDOWN;
UINT8 uckeyPreTestRIGHT;
UINT8 uckeyPreTestLEFT;
UINT8 uckeyPreTestENTER;

//1003
UINT16 uiResetKeyTimer;
UINT8 ucResetKeyCounter;


UINT8 dio_initRelay()
{
	RELAY_1=REALY_OFF;
	RELAY_2=REALY_OFF;
	RELAY_3 =REALY_OFF;
	return SUCCESS;
}

UINT8 dio_checkKeyPressed()
{
	UINT8 ucKey=0;

	KP_SW_1=0;
	KP_SW_2=1;
	delayMs(1);
	if(KP_SW_3==0)
	{
		ucKey=1;
		uckeyPreTestLEFT=1;
	}
	if(KP_SW_4==0)
	{
		ucKey=2;
		uckeyPreTestENTER=1;
	}
	if(KP_SW_5==0)
	{
		ucKey=3;
		uckeyPreTestUP=1;
	}
		
	KP_SW_1=1;
	KP_SW_2=0;
	delayMs(1);
	if(KP_SW_3==0)
	{
		ucKey=4;
		uckeyPreTestDOWN=1;
	}
	if(KP_SW_4==0)
	{
		ucKey=5;
		uckeyPreTestRIGHT=1;
	}
	if(KP_SW_5==0)
	{
		ucKey=6;
			
	}

	KP_SW_1=1;
	KP_SW_2=1;
	if(ucKey==2)
	{
		if(ucKeyTimer>=4)
			ucKey=9;
	}
	else
		ucKeyTimer=0;
		
	if(ucKey!=0)
	{	
		debugSendStr((UINT8*)"\rKey pressed: ");
		debugSendLong(ucKey,1);
		ucMenuTimerMin=0;
	}
	return ucKey;	
}


UINT8 dio_checkResetBtnPressed(void)
{
	if(RESET_BTN==RESET_BTN_PRESSED)
		return TRUE;
	else
		return FALSE;
}


UINT8 dio_initLED(void)
{
	DB_LED_PWR	=DB_LED_OFF;
	DB_LED_GPRS	=DB_LED_OFF;	
	DB_LED_MSG	=DB_LED_OFF;
	DB_LED_ACQ	=DB_LED_OFF;	
	return SUCCESS;
}

UINT8 dio_blink(UINT8 ucLEDNo,UINT16 ucDelay)
{
	switch(ucLEDNo)
	{
		case 1:
			DB_LED_PWR	=DB_LED_ON;
			delayMs(ucDelay);
			DB_LED_PWR	=DB_LED_OFF;
			delayMs(ucDelay);
		break;
		
		case 2:
			DB_LED_GPRS	=DB_LED_ON;
			delayMs(ucDelay);
			DB_LED_GPRS	=DB_LED_OFF;	
			delayMs(ucDelay);
		break;
		
		case 3:
			DB_LED_MSG	=DB_LED_ON;
			delayMs(ucDelay);
			DB_LED_MSG	=DB_LED_OFF;
			delayMs(ucDelay);
		break;
		
		case 4:
			DB_LED_ACQ	=DB_LED_ON;
			delayMs(ucDelay);
			DB_LED_ACQ	=DB_LED_OFF;	
			delayMs(ucDelay);
		break;
		
		default:
		
		break;
	}
	return SUCCESS;
}

void processInterruptData(void)
{
	static 
	UINT32 ulTempF;
	
	static UINT8 ucPreSec=0;
	
	
	if(ulFreqPulsesPerSecond<10)
	{
		if(ulIntTimeMs>10000)
		{
			fFreq=0;
		}
		else
		{
			if(ulIntTimeMsPre==0)
				fFreq=0;
			else
				fFreq=1000/ulIntTimeMsPre;
		}
	}
	else
	{
		fFreq=(F32)ulFreqPulsesPerSecond;
	}
	if(ulIntTimeMs>10000)
		fFreq=0;
	if(sRTC.ucSeconds==0)
	{
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_INT_CNTR,ulIntCntr);	
	}
	
	
}
