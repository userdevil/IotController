#include "rtc.h"
#include "global.h"
#include "r_cg_rtc.h"
#include "uartDebug.h"

#include "dio.h"
#include "controller.h"
#include "memory.h"
#include "gprs.h"
#include "ui.h"
#include "data.h"
#include "display.h"

struct RTC sRTC;

UINT8 ucRTCReadFlag;
UINT8 ucOneSecBlinkFlag=0;

//1015
void rtcInterrupt(void)
{
	ucRTCReadFlag=1;
	sRTC.ucSeconds++;						// increment seconds
	uiParaAcqSeconds++;

	if(ucBaordTestInProgress!=0)
		ucBaordTestInProgress++;
		
	//if(ucAcqDataDisable!=0)
	//	ucAcqDataDisable++;
	
	if(ucMenuTimerMin<200)
		ucMenuTimerMin++;
	if(ucDispBackFlag==1)
		LCD_BL=1;
	else
		LCD_BL=0;
	
  
	
	ulFreqPulsesPerSecond=ulIntCntr-ulPreviousIntCntr;
	ulPreviousIntCntr=ulIntCntr;
	
	ucOneSecBlinkFlag=!ucOneSecBlinkFlag;
	if( (ucSystemRegistrationStatus!=1) || (ucSystemDownloadUnitConfig!=0) || (ucSystemDownloadParaConfig!=0) )
	{
		DB_LED_PWR=ucOneSecBlinkFlag;
	}
	else
		DB_LED_PWR=DB_LED_ON;
	
	
	DB_LED_ACQ=ucPraAcqFlag;
	
	if(uiPendingFiles!=0)
		DB_LED_MSG=DB_LED_ON;
	else
		DB_LED_MSG=DB_LED_OFF;
		
				
	    
        if(sRTC.ucSeconds>59)					// if seconds>59 reset seconds to zero
        {
	
            sRTC.ucSeconds=0;
            sRTC.ucMinutes++;					// increment minutes
	    if(sRTC.ucMinutes>59)
	    {
		sRTC.ucMinutes=0;
		sRTC.ucHours++;
		if(sRTC.ucHours>23)
		{
			sRTC.ucHours=0;
			incrementRTCDate();
		}
	    }
	    
	    ucTimerNonComm++;
            
	    
	    	ucCheckRelayOperation=1;
		uiAcqIntMin++;
		if(uiAcqIntMin>=ulAcqInterwal)
		{
			uiAcqIntMin=0;
			ucPraAcqFlag=1;
		}
	    
	    
	    //check if data has to be acquired	    
	    uiTraIntMin++;
	    if( ((sRTC.ucMinutes%5)==0) && (ulTransmissionInterwal==5) )
	    {
		    uiTraIntMin=0;
		    ucPraTraFlag=1;
	    }
	    else if( ((sRTC.ucMinutes%5)==0) && (uiTraIntMin>=ulTransmissionInterwal) )
	    {
		    uiTraIntMin=0;
		    ucPraTraFlag=1;
	    }
	    
	    if ( (ulTransmissionInterwal<5) && ((uiTraIntMin>=ulTransmissionInterwal)) )
	    {
		    uiTraIntMin=0;
		    ucPraTraFlag=1;
	    }
	    
	    
	    
	    if(sRTC.ucMinutes>59)				// if minutes is > 59
            {
		  ucGPRSSMStateMax=0;
                  sRTC.ucMinutes=0;				// reset minutes to 0
                  sRTC.ucHours++;					// increment hours
                  if(sRTC.ucHours>23)				// if hours >23
                  {
                       sRTC.ucHours=0;				// reset hours
                  		ucCumilativeResetFlag=1;
                  }        
		  
		  
		  
      //            if(sRTC.ucHours==4)
     //             	ucGetSimBalanceSMFlag=1;
            }
	    
	    if(sRTC.ucHours==0)
	    	ucCumilativeResetFlag=1;
		
        }
}


void adjustOffset(void)
{
	unsigned int ucMinutesOffset = 	sRTC.ucOffset * 4;
	
}

void readFromRTC(void)
{
	sRTC.ucYear=BCDToInteger(YEAR);
	sRTC.ucMonth=BCDToInteger(MONTH);
	sRTC.ucDate=BCDToInteger(DAY);
	
	sRTC.ucHours=BCDToInteger(HOUR);
	sRTC.ucMinutes=BCDToInteger(MIN);
	sRTC.ucSeconds=BCDToInteger(SEC);
}

void writeToProcessorRTC(void)
{
	SEC=intergerToBCD(sRTC.ucSeconds);
	MIN=intergerToBCD(sRTC.ucMinutes);
	HOUR=intergerToBCD(sRTC.ucHours);
	DAY=intergerToBCD(sRTC.ucDate);
	MONTH=intergerToBCD(sRTC.ucMonth);
	YEAR=intergerToBCD(sRTC.ucYear);
}

void sendRtcViaDebug(void)
{
	debugSendStr((UINT8*)"\r     RTC -> ");
	debugSendLong(sRTC.ucDate,2);
	debugSendStr((UINT8*)"/");
	debugSendLong(sRTC.ucMonth,2);
	debugSendStr((UINT8*)"/");
	debugSendLong(sRTC.ucYear,2);
	debugSendStr((UINT8*)" ");
	debugSendLong(sRTC.ucHours,2);
	debugSendStr((UINT8*)":");
	debugSendLong(sRTC.ucMinutes,2);
	debugSendStr((UINT8*)":");
	debugSendLong(sRTC.ucSeconds,2);
}

UINT8 BCDToInteger(UINT8 ucNumber)
{
	UINT8 ucUpperNibble,ucLowerNibble;
	ucLowerNibble=ucNumber&0x0F;
	ucUpperNibble=ucNumber&0xF0;
	ucUpperNibble=ucUpperNibble>>4;
	return((ucUpperNibble*10)+ucLowerNibble);
}

UINT8 intergerToBCD(UINT8 ucNumber)
{
	UINT8 ucUpperNibble,ucLowerNibble;
	ucLowerNibble=ucNumber%10;
	ucUpperNibble=ucNumber/10;
	ucUpperNibble=ucUpperNibble<<4;
	return(ucUpperNibble|ucLowerNibble);
}


void incrementRTCDate()
{
	UINT8 ucDaysInMonth[13]={31,28,31,30,31,30,31,31,30,31,30,31};
	
	if(sRTC.ucYear%4==0)  
       	{
        	ucDaysInMonth[1]=29;     //February = 29 days for leap years
        }
	
	if(sRTC.ucDate == ucDaysInMonth[sRTC.ucMonth-1] )
	{
		sRTC.ucDate=1;
		sRTC.ucMonth++;
		if(sRTC.ucMonth>12)
		{
			sRTC.ucMonth=1;
			sRTC.ucYear++;
		}
		
	}
	else
		sRTC.ucDate++;
	
	
	
	
}
