#include "ui.h"
#include "display.h"
#include "memory.h"
#include "rtc.h"
#include "generalfunctions.h"
#include "controller.h"
#include "gprs.h"
#include "dio.h"
#include "delay.h"
#include "data.h"

unsigned char ucMenuTimerMin=0;
unsigned char ucDispBackFlag=0;
unsigned char ucKeyPressed=0;


const char *months[12]={"JANUARY","FEBRUARY","MARCH","APRIL","MAY","JUNE","JULY","AUGUST","SEPTEMBER","OCTOBER","NOVEMBER","DECEMBER"};
const char *days[12]={"MONDAY","TUESDAY","WEDNESDAY","THURSDAY","FRIDAY","SATURDAY","SUNDAY"};

void displayScreen(void)
{
	UINT8 uisTime[10]="00:00:00";
	UINT8 ucBTNStat;
	static UINT8 ucPreDispPara=SCREEN_IDLE,ucDispPara= SCREEN_IDLE,ucRowNumber=0;
	
	ucKeyPressed=ucBTNStat=dio_checkKeyPressed();
	if((ucDispPara==SCREEN_IDLE) && (ucKeyPressed==KP_KEY_RIGHT) )
	{
		ucDispPara=SCREEN_PARA;
		delayMs(100);
	}
	else if((ucDispPara==SCREEN_PARA) && (ucKeyPressed==KP_KEY_RIGHT) )
	{
		ucDispPara=SCREEN_DEBUG;
		delayMs(100);
	}
	
	
	
	if((ucDispPara==SCREEN_DEBUG) && (ucKeyPressed==KP_KEY_LEFT) )
	{
		ucDispPara=SCREEN_PARA;
		delayMs(100);
	}
	else if((ucDispPara==SCREEN_PARA) && (ucKeyPressed==KP_KEY_LEFT) )
	{
		ucDispPara=SCREEN_IDLE;
		delayMs(100);
	}
	
	
	
	if(ucPreDispPara!=ucDispPara)
	{
		ucPreDispPara=ucDispPara;
		lcdInit();
		lcdClear();
		glcdFillScreen(OFF);
	}
	
	if( (ucMenuTimerMin>180) )
	{
		ucDispPara=SCREEN_IDLE;
		ucDispBackFlag=0;
	}	
	else
		ucDispBackFlag=1;
	
	switch(ucDispPara)
	{
		case SCREEN_NONE:
		
		
		break;
		
		case SCREEN_IDLE:
		
		
		
			
			drawdSenZlogo();
			glcdWriteLongCallibri(2,35,ulSystemID,5,11,1);
			
			intToArray(sRTC.ucHours,2,5,&uisTime[0]);
			intToArray(sRTC.ucMinutes,2,5,&uisTime[3]);
			intToArray(sRTC.ucSeconds,2,5,&uisTime[6]);
			glcdWriteStrCallibri(2,45,uisTime,11,1);
		
			
			glcdDrawRectangle(0,55,90,63,YES,0);
			glcdWriteLongCallibri(2,55,sRTC.ucDate,2,11,1);
			glcdWriteStrCallibri(25,55,(UINT8*)months[sRTC.ucMonth-1],11,1);
			
			
			{
				UINT32 ulFileOnSD = uiPendingFiles;
				if(ulFileOnSD>99)
					ulFileOnSD=99;
				glcdDrawRectangle(112,52,127,63,1,1);
				glcdWriteLongCallibri(113,53,ulFileOnSD,2,11,0);
				glcdPixel(112,52,0);
				glcdPixel(112,63,0);
				glcdPixel(127,63,0);
				
				glcdPixel(127,52,1);
				glcdPixel(127,51,1);
				glcdPixel(127,50,1);
				glcdPixel(127,49,1);
				glcdPixel(126,52,1);
				glcdPixel(126,51,1);
				glcdPixel(126,50,1);
				glcdPixel(125,52,1);
				glcdPixel(125,51,1);
				glcdPixel(124,52,1);
			}
   		
	   		displayNetworkStatus(0,0,1);	
			
			
			
			
			glcdWriteLongCallibri(107,0,ucParaBeingAcquired,3,11,1);
			
//			if(ucPraAcqFlag==1)
//			{
//				glcdDrawRectangle(119,0,124,4,0,1);
//				glcdDrawRectangle(117,6,120,9,0,1);
//				glcdDrawRectangle(123,6,126,9,0,1);
//				
//			}
//			else
//			{
//				glcdDrawRectangle(117,0,126,9,1,0);
//			
//			}
			
		break;
		
		
		case SCREEN_PARA :
	
			glcdFillScreen(OFF);
			glcdWriteStrCallibri(30,0,(unsigned char*)"e - SenZ",12,ON);
			glcdWriteStrCallibri(80,0,(unsigned char*)"#",12,ON);
			glcdWriteLongCallibri(86,0,ulSystemID,6,11,1);
        		glcdDrawLine(0,11,127,11,1);
        		displayNetworkStatus(0,0,1);
     

	        	if(ucBTNStat==KP_KEY_DOWN )
	        	{
	        		ucRowNumber++;
	        		if(ucRowNumber>ulSystemNoOfParameters)
	        			ucRowNumber=0;
	        		
	        		delayMs(150);
	        		ucMenuTimerMin=0;
	        	}
	        	if(ucBTNStat==KP_KEY_UP)
	        	{
		        	
				if(ucRowNumber==0)
					ucRowNumber=(UINT8)ulSystemNoOfParameters-1;
				else
        			ucRowNumber--;
        		
        		    	delayMs(150);
	        		ucMenuTimerMin=0;
			}
        		
	         	glcdWriteStr(0,2, (UINT8*)sParaConfig[ucRowNumber+0].para.ucParaStr,1,1);
	         	ucDisplayParaValue(ucRowNumber+0,2);
	         	
	        	glcdWriteStr(0,3, (UINT8*)sParaConfig[ucRowNumber+1].para.ucParaStr,1,1);
	        	ucDisplayParaValue(ucRowNumber+1,3);
	        	
	        	glcdWriteStr(0,4, (UINT8*)sParaConfig[ucRowNumber+2].para.ucParaStr,1,1);
	        	ucDisplayParaValue(ucRowNumber+2,4);
	        	
	        	glcdWriteStr(0,5, (UINT8*)sParaConfig[ucRowNumber+3].para.ucParaStr,1,1);
	        	ucDisplayParaValue(ucRowNumber+3,5);
	        	
	        	glcdWriteStr(0,6, (UINT8*)sParaConfig[ucRowNumber+4].para.ucParaStr,1,1);
				ucDisplayParaValue(ucRowNumber+4,6);
				
		break;
		
		case SCREEN_DEBUG:
		
			glcdFillScreen(OFF);
			glcdWriteStrCallibri(30,0,(unsigned char*)"e - SenZ",12,ON);
			glcdWriteStrCallibri(80,0,(unsigned char*)"#",12,ON);
			glcdWriteLongCallibri(86,0,ulSystemID,6,11,1);
        		glcdDrawLine(0,11,127,11,1);
        		displayNetworkStatus(0,0,1);
    

        		
	         	glcdWriteStr(0,2,(UINT8*)"Pend File",1,1);
	         	glcdWriteLong(11,2, uiPendingFiles,5,1,1);
			
			
		glcdWriteStr(0,3,(UINT8*)"Internet",1,1);
			glcdWriteStr(11,3,(UINT8*)"GPRS",1,1);
			
				
			glcdWriteStr(0,4,(UINT8*)"State",1,1);
			glcdWriteLong(11,4, ucGPRSSMState,2,1,1);
				
			
			glcdWriteStr(0,5,(UINT8*)"M State",1,1);
			glcdWriteLong(11,5, ucGPRSSMStateMax,2,1,1);
	         	
	         	glcdWriteStr(0,6,(UINT8*)"Tra int",1,1);
			glcdWriteLong(11,6, ulTransmissionInterwal,3,1,1);
			
		
		break;
		
		
		default:
		
		break;
	}
	glcdUpdate();
}
		
		
// network status
UINT8 displayNetworkStatus(UINT8 ucX, UINT8 ucY, UINT8 ucPixel)
{
    if(ucSignalStrength>50)
            return FAILURE;
    if(ucNetworkStatus>1)
            return FAILURE;
	    
    glcdDrawRectangle(ucX,ucY+1,ucX+11,ucY+9,YES,!ucPixel);	// CLEAR THIS SECION OF SCREEN
    
	{
	    

	    if(ucSIMStatus!=1)								/// DRAW SIM signal
	    {
	            glcdDrawRectangle(ucX+4,ucY+2,ucX+8,ucY+8,YES,ucPixel);
	            glcdPixel(ucX+5,ucY+2,!ucPixel);
	            glcdPixel(ucX+4,ucY+2,!ucPixel);
	            glcdPixel(ucX+4,ucY+3,!ucPixel);

	            glcdPixel(ucX+6,ucY+5,!ucPixel);
	            glcdPixel(ucX+6,ucY+6,!ucPixel);
	            glcdPixel(ucX+7,ucY+5,!ucPixel);
	            glcdPixel(ucX+7,ucY+6,!ucPixel);

	            return SUCCESS;
	    }

	    if(ucSignalStrength>24)
	    {
	            glcdPixel(ucX+8,ucY+2,ucPixel);
	            glcdPixel(ucX+8,ucY+3,ucPixel);
	            glcdPixel(ucX+8,ucY+4,ucPixel);
	            glcdPixel(ucX+8,ucY+5,ucPixel);
	            glcdPixel(ucX+8,ucY+6,ucPixel);
	            glcdPixel(ucX+8,ucY+7,ucPixel);
	            glcdPixel(ucX+8,ucY+8,ucPixel);
	    }
	    if(ucSignalStrength>15)
	    {
	            glcdPixel(ucX+6,ucY+4,ucPixel);
	            glcdPixel(ucX+6,ucY+5,ucPixel);
	            glcdPixel(ucX+6,ucY+6,ucPixel);
	            glcdPixel(ucX+6,ucY+7,ucPixel);
	            glcdPixel(ucX+6,ucY+8,ucPixel);
	    }
	    if(ucSignalStrength>7)
	    {
	            glcdPixel(ucX+4,ucY+6,ucPixel);
	            glcdPixel(ucX+4,ucY+7,ucPixel);
	            glcdPixel(ucX+4,ucY+8,ucPixel);
	    }
	    if(ucSignalStrength>0)
	            glcdPixel(ucX+2,ucY+8,ucPixel);

	    if(ucGPRSConStatus==1)
	    {
	            glcdPixel(ucX+3,ucY+2,ucPixel);
	            glcdPixel(ucX+2,ucY+1,ucPixel);
	            glcdPixel(ucX+1,ucY+1,ucPixel);
	            glcdPixel(ucX+0,ucY+2,ucPixel);
	            glcdPixel(ucX+0,ucY+3,ucPixel);
	            glcdPixel(ucX+0,ucY+4,ucPixel);
	            glcdPixel(ucX+1,ucY+5,ucPixel);
	            glcdPixel(ucX+2,ucY+5,ucPixel);
	            glcdPixel(ucX+2,ucY+4,ucPixel);
	            glcdPixel(ucX+3,ucY+4,ucPixel);
	    }
	    
	    if(ucGPRSDataTranferFlag==1)
	    {
		glcdPixel(ucX+5,ucY+1,ucPixel);
	    	glcdPixel(ucX+5,ucY+2,ucPixel);
	    	glcdPixel(ucX+5,ucY+3,ucPixel);
	    	glcdPixel(ucX+6,ucY+2,ucPixel);
	    }
	    else
	    {
		    glcdPixel(ucX+5,ucY+1,!ucPixel);
	    	glcdPixel(ucX+5,ucY+2,!ucPixel);
	    	glcdPixel(ucX+5,ucY+3,!ucPixel);
	    	glcdPixel(ucX+6,ucY+2,!ucPixel);
	    }
    }
	if(ucModemType==MODEM_TYPE_4G)
	glcdWriteStrCallibri(10,0,(unsigned char*)"4G",12,ON);	
    else
    	glcdWriteStrCallibri(10,0,(unsigned char*)"2G",12,ON);	
    
    return SUCCESS;
}


UINT8 ucDisplayParaValue(UINT8 ucPara, UINT8 ucRow)
{
	//sParaConfig[ucRowNumber+0].ucParaStr
	//sParaConfig[ucPara].fValue
	
	float fTemp=0;
	fTemp=sParaConfig[ucPara].para.value.fValue;

	if(fTemp>1000000000)
	{
		fTemp=fTemp/1000000000;
		glcdWriteFloat(11,ucRow, fTemp,3,1,1,1);
		glcdWriteStr(17,ucRow, (UINT8*)"G",1,1);
	}
	else if(fTemp>1000000)
	{
		fTemp=fTemp/1000000;
		glcdWriteFloat(11,ucRow, fTemp,3,1,1,1);
		glcdWriteStr(17,ucRow, (UINT8*)"M",1,1);
	}
	else if(fTemp>1000)
	{
		fTemp=fTemp/1000;
		glcdWriteFloat(11,ucRow, fTemp,3,1,1,1);
		glcdWriteStr(17,ucRow, (UINT8*)"K",1,1);
	}
	else if(fTemp<1)
	{
		fTemp=fTemp*1000;
		glcdWriteFloat(11,ucRow, fTemp,3,1,1,1);
		glcdWriteStr(17,ucRow, (UINT8*)"m",1,1);
	}
	else 
	{
		glcdWriteFloat(11,ucRow, fTemp,3,1,1,1);
		glcdWriteStr(17,ucRow, (UINT8*)" ",1,1);
	}

	if(sParaConfig[ucPara].para.ucErrorFlag!=0)
		glcdWriteStr(18,ucRow, (UINT8*)"ERR",1,1);
	else
		glcdWriteStr(18,ucRow, (UINT8*)"   ",1,1);
	
	return SUCCESS;
}


