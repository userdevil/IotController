#include "data.h"
#include "global.h"
#include "math.h"

#include "uart.h"
#include "uartDebug.h"
#include "memory.h"
#include "controller.h"
#include "analog.h"
#include "dio.h"
#include "modbus.h"
#include "flash.h"
#include "generalfunctions.h"
#include "rtc.h"
#include "memory.h"
#include "eeprom.h"
#include "uartGprs.h"
#include "gprs.h"
#include "controller.h"


UINT8 ucCSVWriteHeader=1;
UINT8 ucCSVSendData=1;

UINT8 ucAcqDataDisable;


UINT16 uiFlashStorageSize=0;
UINT16 uiFlashMaxReadCntr=0;
UINT16 uiFlashMaxWriteCntr=0;

UINT16 ucTimerNonComm;

UINT8 ucRS485BusFlag;

UINT8 ucCumilativeResetFlag;

F32 fInverterPower;

UINT8 ucParaBeingAcquired =0;

UINT16 uiParaAcqSeconds = 0;


void acquireData(UINT8* pucPCF)
{
	static UINT8 ucFirstTimeFlag=1;
	static UINT32 ulParaCntrF = 0;
	static F32 fInvPowerTemp=0;
	static UINT8 ucNoOfInvertersCommunicating=0;//1042
	UINT8 ucIndPraAcqCmpFlag=0;
	UINT8 ucProcessMODBUSCmd=1;
	F32 fPreValueOfParameter=sParaConfig[ulParaCntrF].para.value.fValue;
	UINT8 ucResponseStatus;
	F32 fValue;
	F32 FParaValue=0;
	UINT8 ucParaStatus=0;
	UINT32 ulPktLength;
	ucParaBeingAcquired=(UINT8)ulParaCntrF;
	if (*pucPCF == 0) {
		ucFirstTimeFlag=1;
		ulParaCntrF=0;
		uiParaAcqSeconds=0;
		return;
	}
	if(ucFirstTimeFlag==1)
	{
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
		{
			UARTSendStr(UART_DEBUG, (UINT8*) "\r\r\r Acquiring board data");
		}
		ucFirstTimeFlag=0;
		fInvPowerTemp=0;
		//1042
		ucNoOfInvertersCommunicating=0;
	}
	// no para
	if (sParaConfig[ulParaCntrF].para.ucParaSource == 0)
	{
		FParaValue=0;
        	ucParaStatus = 1;
		ucIndPraAcqCmpFlag=1;
		
	}
	//power supply
	else if(sParaConfig[ulParaCntrF].para.ucParaSource == 1)
	{
		FParaValue=fSupplyVolts;
		ucParaStatus = 1;
		ucIndPraAcqCmpFlag=1;
		
		
	}
	//Amb Temp
	else if(sParaConfig[ulParaCntrF].para.ucParaSource == 2)
	{
		FParaValue=fLM35;
		ucParaStatus = 1;
		ucIndPraAcqCmpFlag=1;
	}
	//V1
	else if(sParaConfig[ulParaCntrF].para.ucParaSource == 3)
	{
		FParaValue=fV1;
		ucParaStatus = 1;
		ucIndPraAcqCmpFlag=1;
	}
	//V2
	else if(sParaConfig[ulParaCntrF].para.ucParaSource == 4)
	{
		FParaValue=fV2;
		ucParaStatus = 1;
		ucIndPraAcqCmpFlag=1;
	}
	//V3
	else if(sParaConfig[ulParaCntrF].para.ucParaSource == 5)
	{
		FParaValue=fV3;
		ucParaStatus = 1;
		ucIndPraAcqCmpFlag=1;
	}
	//V4
	else if(sParaConfig[ulParaCntrF].para.ucParaSource == 6)
	{
		FParaValue=fV4;
		ucParaStatus = 1;
		ucIndPraAcqCmpFlag=1;
	}
	//I1
	else if(sParaConfig[ulParaCntrF].para.ucParaSource == 7)
	{
		FParaValue=fI1;
		ucParaStatus = 1;
		ucIndPraAcqCmpFlag=1;
	}
	//I2
	else if(sParaConfig[ulParaCntrF].para.ucParaSource == 8)
	{
		FParaValue=fI2;
		ucParaStatus = 1;
		ucIndPraAcqCmpFlag=1;
	}
	//Digital input pin status
	else if(sParaConfig[ulParaCntrF].para.ucParaSource == 9)
	{
		FParaValue=(float)P14.1;
		ucParaStatus = 1;
		ucIndPraAcqCmpFlag=1;
	}
	//Digital input Freq
	else if(sParaConfig[ulParaCntrF].para.ucParaSource == 10)
	{
		FParaValue=fFreq;
		ucParaStatus = 1;
		ucIndPraAcqCmpFlag=1;
	}
	//Digital input Cntr
	else if(sParaConfig[ulParaCntrF].para.ucParaSource == 11)
	{
		FParaValue=ulIntCntr;
		ucParaStatus = 1;
		ucIndPraAcqCmpFlag=1;
	}
	//RS4851 //1007 else addes. //1007 source RS485-2 will also be used for rs485, ALL DG power will configured  as RS485 2 
        else if ( (sParaConfig[ulParaCntrF].para.ucParaSource == 12) ) 
	{
		ucProcessMODBUSCmd=1;
		
		getStrLength(sParaConfig[ulParaCntrF].para.ucRS485Packet,&ulPktLength);
		ulPktLength=ulPktLength/2;
		if(ulPktLength<2 || ulPktLength >15)
			ulPktLength=8;
		//ocessRS485Packet(UINT8* pucPCF      , UINT8 ucMeterType,                             UINT8 ucMode,                            UINT8 ucDataType,                               UINT8 ucBaudF,                              UINT8 ucDataBitsF,                      UINT8 ucParity,                              UINT8 ucStopBits,                          UINT16 uiTimeout, UINT8* pucPacket, UINT8* pucRespStatus, F32* fVal) 
		processRS485Packet(&ucProcessMODBUSCmd, sParaConfig[ulParaCntrF].para.ucRS485MeterType, sParaConfig[ulParaCntrF].para.ucRS485Mode, sParaConfig[ulParaCntrF].para.ucRS485DataType, sParaConfig[ulParaCntrF].para.ucRS485Baud, sParaConfig[ulParaCntrF].para.ucRS485Bits, sParaConfig[ulParaCntrF].para.ucRS485Parity, sParaConfig[ulParaCntrF].para.ucRS485Stopbits,sParaConfig[ulParaCntrF].para.uiRS485Timout, sParaConfig[ulParaCntrF].para.ucRS485Packet,ulPktLength,&ucResponseStatus, &fValue,1);
		if(ucProcessMODBUSCmd==0)
		{
			FParaValue=fValue;
			ucParaStatus = ucResponseStatus;
			ucIndPraAcqCmpFlag=1;
			
			if(sParaConfig[ulParaCntrF].para.ucParaType==25 && ucResponseStatus ==0x01 && ucPraTraFlag==1 )
			{
				//save to file	
				storeModbusDataToFlash(ulParaCntrF);
			}
			
			
		}
	}
	else
	{
		//1007 
		//FParaValue=999.99;
		ucIndPraAcqCmpFlag=1;
	}
	//1006
	/*
	if (ucIndPraAcqCmpFlag==1)
	{
		if(sParaConfig[ulParaCntrF].para.ucParaCumulate!=1)
		{
			sParaConfig[ulParaCntrF].para.value.fValue=FParaValue;
       		 		
		}
		sParaConfig[ulParaCntrF].para.ucValueStatusBit = ucParaStatus;
	}*/
	///1006
	
	
	if(ucIndPraAcqCmpFlag==1)
	{
		UINT16 uiAddressF;
		float fParaValue =sParaConfig[ulParaCntrF].para.value.fValue;
		ucIndPraAcqCmpFlag=0;
		
		//1006
		sParaConfig[ulParaCntrF].para.value.fValue=FParaValue;
		sParaConfig[ulParaCntrF].para.ucValueStatusBit = ucParaStatus;
		///1006
		
		//1030
//		//multiplication and addition
//		if (sParaConfig[ulParaCntrF].para.ucParaType != 1)//for thermistor
//        	{
//			 sParaConfig[ulParaCntrF].para.value.fValue *= sParaConfig[ulParaCntrF].para.fMultiplyFactor;
//            		 sParaConfig[ulParaCntrF].para.value.fValue += sParaConfig[ulParaCntrF].para.fAdditionFactor;	
//		}
		sParaConfig[ulParaCntrF].para.value.fValue *= sParaConfig[ulParaCntrF].para.fMultiplyFactor;
		sParaConfig[ulParaCntrF].para.value.fValue += sParaConfig[ulParaCntrF].para.fAdditionFactor;


		
		
		if(ulParaCntrF!=0)
		{
			//check para type
			if (sParaConfig[ulParaCntrF].para.ucParaType == 1)//for thermistor
	        	{
				 convertVoltageToResistance(fSupplyVolts, sParaConfig[ulParaCntrF].para.value.fValue, sParaConfig[ulParaCntrF].para.fMultiplyFactor, &sParaConfig[ulParaCntrF].para.value.fValue);
	            		 convert10KTheristerResitanceToTemp(sParaConfig[ulParaCntrF].para.value.fValue, &sParaConfig[ulParaCntrF].para.value.fValue);
			}
			else 
			if(sParaConfig[ulParaCntrF].para.ucParaType == 2)// 1 or zero parameter
			{
			    	if (sParaConfig[ulParaCntrF].para.fMinValue < sParaConfig[ulParaCntrF].para.fMaxValue) 
				{
			               if ((sParaConfig[ulParaCntrF].para.value.fValue >= sParaConfig[ulParaCntrF].para.fMinValue) && (sParaConfig[ulParaCntrF].para.value.fValue <= sParaConfig[ulParaCntrF].para.fMaxValue))
			                {
			                    sParaConfig[ulParaCntrF].para.value.fValue = 1;
			               
			                }
			                else
			                {
			                    sParaConfig[ulParaCntrF].para.value.fValue = 0;
			                }
		            	}	 
				else 
				{
		               	 	if ((sParaConfig[ulParaCntrF].para.value.fValue >= sParaConfig[ulParaCntrF].para.fMinValue))
		                    		sParaConfig[ulParaCntrF].para.value.fValue = 0;
		                	else
		                    		sParaConfig[ulParaCntrF].para.value.fValue = 1;
		            	}		
			}
			
			else if(sParaConfig[ulParaCntrF].para.ucParaCumulate == 1)// cumulative type of data
			{
				//do cumulate only if transmit packet is set
				if(ucPraTraFlag==1)
				{
					if(ucCumilativeResetFlag==1)
					{
						FParaValue=0;	
						fPreValueOfParameter=0;
						sParaConfig[ulParaCntrF].para.ucValueStatusBit=1;
					}
					sParaConfig[ulParaCntrF].para.value.fValue+=fPreValueOfParameter;
				}
				else
				{
					//1006
					sParaConfig[ulParaCntrF].para.value.fValue=fPreValueOfParameter;
				}
			}
			
			
			//save location
			//if( (sParaConfig[ulParaCntrF].para.ucParaType>=4) && (sParaConfig[ulParaCntrF].para.ucParaType<=13) ) 
			//1017
			if( (sParaConfig[ulParaCntrF].para.ucParaType>=4) && (sParaConfig[ulParaCntrF].para.ucParaType<=23) ) 
			{
				uiAddressF=sParaConfig[ulParaCntrF].para.ucParaType-4;
				uiAddressF=EEPROM_LOC_STORE_PARA_1_LOC+(4*uiAddressF);
				
				//1017
				if(sParaConfig[ulParaCntrF].para.ucParaType>13)
				{
					uiAddressF=uiAddressF-10;
					uiAddressF=EEPROM_LOC_STORE_PARA_11_LOC+(4*uiAddressF);
				}
				//									1013
				if( (sParaConfig[ulParaCntrF].para.ucValueStatusBit != 1 ) || (sParaConfig[ulParaCntrF].para.value.fValue==0)  )
				{
					readExtEEPROMFloat(EEPROM_ADD,uiAddressF,&sParaConfig[ulParaCntrF].para.value.fValue);
				}
				else
				{
				       	writeExtEEPROMFloat(EEPROM_ADD,uiAddressF, sParaConfig[ulParaCntrF].para.value.fValue);
				}
			}
		}
		//1018
		if( (sParaConfig[ulParaCntrF].para.ucParaType==24) && (sParaConfig[ulParaCntrF].para.ucValueStatusBit == 1 ) )
		{
			fInvPowerTemp += sParaConfig[ulParaCntrF].para.value.fValue;
		
			ucNoOfInvertersCommunicating++; //	1042
		}
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
		{
			
			UARTSendStr(UART_DEBUG, (UINT8*) "\r Acq Data, PARA ");
			UARTSendLong(UART_DEBUG,ulParaCntrF,3);
			
			UARTSendStr(UART_DEBUG, (UINT8*) " ->  ");
			UARTSendStr(UART_DEBUG,sParaConfig[ulParaCntrF].para.ucParaStr);
		
			UARTSendStr(UART_DEBUG, (UINT8*) " = ");
			UARTSendFloat(UART_DEBUG,sParaConfig[ulParaCntrF].para.value.fValue,6,3);
		
		}
		//check error in para only if parameter is valid
		if(sParaConfig[ulParaCntrF].para.ucValueStatusBit==1)
			checkParaForErrorAndSendAlert((UINT8)ulParaCntrF);
		
		
		ulParaCntrF++;
		if(ulParaCntrF>=ulSystemNoOfParameters)
		{
			ulParaCntrF=0;
			*pucPCF=0;
			ucCheckRelayOperation=1;
			
			ucCumilativeResetFlag=0;
			
			if(ucPraTraFlag==1)
			{
				ucPraTraFlag=0;
				//check for error
				
				storeParameterToFlash();
			}
			

			fInverterPower=fInvPowerTemp;
			fInvPowerTemp=0;
			if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			{
				UARTSendStr(UART_DEBUG, (UINT8*) "\rData Acq Complete   ");
				UARTSendLong(UART_DEBUG,uiParaAcqSeconds,3);
			}
			
			
			//1042
			if(ucNoOfInvertersCommunicating==0)
				RELAY_3=REALY_ON;
			else
				RELAY_3=REALY_OFF;
			
			
		}
		
		
		
	}
}



UINT8 checkParaForErrorAndSendAlert(UINT8 ulParaCntrF)
{
    UINT32 ulTempF=0;
    UINT8 ucStartHrs;
    UINT8 ucStartMin;
    UINT8 ucStopHrs;
    UINT8 ucStopMin;
    UINT8 ucTimeStatus;
    
    
    if (sParaConfig[ulParaCntrF].para.ucAlarmEnable == 1) 
    {
	    ucStartHrs=(UINT8)convAsciiIntToInt(&sParaConfig[ulParaCntrF].para.ucAlermOnTime[0],2);
	    ucStartMin=(UINT8)convAsciiIntToInt(&sParaConfig[ulParaCntrF].para.ucAlermOnTime[2],2);
	    
	    ucStopHrs=(UINT8)convAsciiIntToInt(&sParaConfig[ulParaCntrF].para.ucAlermOffTime[0],2);
	    ucStopMin=(UINT8)convAsciiIntToInt(&sParaConfig[ulParaCntrF].para.ucAlermOffTime[2],2);
	    
	    ucTimeStatus=getRelayStatusBasedOnTime(ucStartHrs,ucStartMin,ucStopHrs,ucStopMin);
	    
	    if ( (ucTimeStatus!=RELAY_TIME_TURNON) )
	    	return 0;
	    
	    
	    if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
	    {
	    
	    
		    UARTSendStr(UART_DEBUG, (UINT8*) "\r                        checking for error and alarms of para = ");
	    	    UARTSendLong(UART_DEBUG, ulParaCntrF, 4);

		
		    UARTSendStr(UART_DEBUG, (UINT8*) "\r alarm parameters : ");
	    	    UARTSendStr(UART_DEBUG, sParaConfig[ulParaCntrF].para.ucParaStr);

		    
	            UARTSendStr(UART_DEBUG, (UINT8*) " ,value = ");
	            UARTSendFloat(UART_DEBUG, sParaConfig[ulParaCntrF].para.value.fValue, 4, 1);


	            UARTSendStr(UART_DEBUG, (UINT8*) " ,Max = ");
	            UARTSendFloat(UART_DEBUG, sParaConfig[ulParaCntrF].para.fMaxValue, 4, 1);


	            UARTSendStr(UART_DEBUG, (UINT8*) " ,Min = ");
	            UARTSendFloat(UART_DEBUG, sParaConfig[ulParaCntrF].para.fMinValue, 4, 1);

	            UARTSendStr(UART_DEBUG, (UINT8*) " ,delay = ");
	            UARTSendLong(UART_DEBUG, sParaConfig[ulParaCntrF].para.ulAlarmDelay, 4);
	    }


            if ((sParaConfig[ulParaCntrF].para.value.fValue >= sParaConfig[ulParaCntrF].para.fMaxValue) || (sParaConfig[ulParaCntrF].para.value.fValue <= sParaConfig[ulParaCntrF].para.fMinValue))
            {
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
		{
               	 	UARTSendStr(UART_DEBUG, (UINT8*) ", Error Timer = ");
		}
                sParaConfig[ulParaCntrF].para.ulErrorTimer += (ulAcqInterwal * 60);
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
	                UARTSendLong(UART_DEBUG, sParaConfig[ulParaCntrF].para.ulErrorTimer, 4);
                if (sParaConfig[ulParaCntrF].para.ulErrorTimer >= sParaConfig[ulParaCntrF].para.ulAlarmDelay) {
                    if (sParaConfig[ulParaCntrF].para.ucErrorFlag == 0)
                    {
			    
			clearGeneralPurposeBuffer();
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"e-SenZ Alert, Para : ",GENERAL_PURPOSE_BUFF_SIZE);
			concatStirngsToStr1(ucGeneralPurposeBuffer,sParaConfig[ulParaCntrF].para.ucParaStr,GENERAL_PURPOSE_BUFF_SIZE);
			getStrLength(ucGeneralPurposeBuffer,&ulTempF);//LENGTH OF DATA
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)", value = ",GENERAL_PURPOSE_BUFF_SIZE);
			getStrLength(ucGeneralPurposeBuffer,&ulTempF);//LENGTH OF DATA
			floatToASCIIStr(sParaConfig[ulParaCntrF].para.value.fValue,5,1,200,&ucGeneralPurposeBuffer[ulTempF]);
			concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)", Time = ",GENERAL_PURPOSE_BUFF_SIZE);
			getStrLength(ucGeneralPurposeBuffer,&ulTempF);//LENGTH OF DATA
			intToArray(sRTC.ucDate,2,25,&ucGeneralPurposeBuffer[ulTempF+0]);
			ucGeneralPurposeBuffer[ulTempF+2]='/'; 
			intToArray(sRTC.ucMonth,2,25,&ucGeneralPurposeBuffer[ulTempF+3]);
			ucGeneralPurposeBuffer[ulTempF+5]='/'; 
			intToArray(sRTC.ucYear,2,25,&ucGeneralPurposeBuffer[ulTempF+6]);
			ucGeneralPurposeBuffer[ulTempF+8]=' '; 
			intToArray(sRTC.ucHours,2,25,&ucGeneralPurposeBuffer[ulTempF+9]);
			ucGeneralPurposeBuffer[ulTempF+11]=':'; 
			intToArray(sRTC.ucMinutes,2,25,&ucGeneralPurposeBuffer[ulTempF+12]);
			ucGeneralPurposeBuffer[ulTempF+14]=':'; 
			intToArray(sRTC.ucSeconds,2,25,&ucGeneralPurposeBuffer[ulTempF+15]);
			ucGeneralPurposeBuffer[ulTempF+17]=0x00; 
			
                        sParaConfig[ulParaCntrF].para.ucErrorFlag = 1;

			if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			{
	                        UARTSendStr(UART_DEBUG, (UINT8*) ", Send alarm ");
       		                UARTSendStr(UART_DEBUG, ucGeneralPurposeBuffer);
			}
			ucSendSMSFlag=1;
			storeSMSToEEPROM(ucGeneralPurposeBuffer);
			
                    }
		    
                    sParaConfig[ulParaCntrF].para.ucErrorFlag = 1;
		    if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG, (UINT8*) ", ALARM");
                    sParaConfig[ulParaCntrF].para.ulErrorTimer = sParaConfig[ulParaCntrF].para.ulAlarmDelay;

                }

            } else if ((sParaConfig[ulParaCntrF].para.value.fValue <= ((sParaConfig[ulParaCntrF].para.fMaxValue) - (.05 * (sParaConfig[ulParaCntrF].para.fMaxValue)))) || (sParaConfig[ulParaCntrF].para.value.fValue >= ((sParaConfig[ulParaCntrF].para.fMinValue)+(.05 * (sParaConfig[ulParaCntrF].para.fMaxValue))))) {
                sParaConfig[ulParaCntrF].para.ucErrorFlag = 0;
                sParaConfig[ulParaCntrF].para.ulErrorTimer = 0;
            }
        } else {
            sParaConfig[ulParaCntrF].para.ucErrorFlag = 0;
            sParaConfig[ulParaCntrF].para.ulErrorTimer = 0;
        }

        if (sParaConfig[ulParaCntrF].para.ucErrorFlag == 1) {

        }

    return sParaConfig[ulParaCntrF].para.ucErrorFlag;
}


void storeParameterToFlash(void)
{
	UINT8 ucTempAf=0;
	UINT32 ulTempBf=0;
	UINT32 ulParaCntrF=0;
	UINT8 ucParaAlarmStatus=0;
	UINT8 ucParaStatus=0;
	F32 fParaValue=0;
	static UINT8 ucSendPktCnr=0;
	
	//check to see if current time is between so
	UINT16 ucFromHrs=0;
	UINT16 ucFromMin=0;
	UINT16 ucToHrs=0;
	UINT16 ucTomin=0;
	UINT16 uiFromTime=0;
	UINT16 uiToTime=0;
	UINT16 uiCurrentTime=0;
	
	
	uiCurrentTime = ((UINT16)sRTC.ucHours*(UINT16)60)+(UINT16)sRTC.ucMinutes;
	uiFromTime = (convAsciiIntToInt(&ucTransPeriod[0],2)*60)+convAsciiIntToInt(&ucTransPeriod[2],2);
	uiToTime   = (convAsciiIntToInt(&ucTransPeriod[4],2)*60)+convAsciiIntToInt(&ucTransPeriod[6],2);
	if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
	{
		
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r Trans period = ");	
		for(ucTempAf=0;ucTempAf<8;ucTempAf++)
		{
			UARTSendChar(UART_DEBUG,ucTransPeriod[ucTempAf]);
		}
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r Current Time =");
		UARTSendLong(UART_DEBUG,uiCurrentTime,5);
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r From Time =");
		UARTSendLong(UART_DEBUG,uiFromTime,5);
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r To Time =");
		UARTSendLong(UART_DEBUG,uiToTime,5);
	}
	
	if(ucSendPktCnr<3)
	{
		ucSendPktCnr++;
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r Initial packs");
	}
	else if((uiFromTime=0) && (uiToTime=0) )
	{
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r default time");
	}
	else if((uiCurrentTime>=uiFromTime) && (uiCurrentTime<=uiToTime) )
	{
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r Between TimeInterwal");
	}
	else if(uiCurrentTime<3)
	{
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r Board started");
	}
	else if(sRTC.ucYear<17)
	{
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r error in current time");
	}
	else
	{
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r not saving data");	
		return;
	}
	
	FillBuffer(0x00,ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE);
	//writeFileTypeToMemory
	//concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"10000000",GENERAL_PURPOSE_BUFF_SIZE);
	
	concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"DATA000=",100);
	
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);
	longHexToHexBytes(ulFlashFileSaveLocation,&ucGeneralPurposeBuffer[ulTempBf]);
		
	//write system id
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);
	longHexToHexBytes(ulSystemID,&ucGeneralPurposeBuffer[ulTempBf]);
	//writeDataAndTime
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
	intToASCIIStr(sRTC.ucDate,2,11,&ucGeneralPurposeBuffer[ulTempBf]);
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
	intToASCIIStr(sRTC.ucMonth,2,11,&ucGeneralPurposeBuffer[ulTempBf]);
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
	intToASCIIStr(sRTC.ucYear,2,11,&ucGeneralPurposeBuffer[ulTempBf]);
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
	intToASCIIStr(sRTC.ucHours,2,11,&ucGeneralPurposeBuffer[ulTempBf]);
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
	intToASCIIStr(sRTC.ucMinutes,2,11,&ucGeneralPurposeBuffer[ulTempBf]);
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
	intToASCIIStr(sRTC.ucSeconds,2,11,&ucGeneralPurposeBuffer[ulTempBf]);
	
	//write ss,r1,r2,r3 values
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
	intToASCIIStr(ucSignalStrength,2,11,&ucGeneralPurposeBuffer[ulTempBf]);
	ucGeneralPurposeBuffer[ulTempBf+2]=P10.2+0x30;
	ucGeneralPurposeBuffer[ulTempBf+3]=P0.4+0x30;
	ucGeneralPurposeBuffer[ulTempBf+4]=P13.0+0x30;
	ucGeneralPurposeBuffer[ulTempBf+5]='0';
	ucGeneralPurposeBuffer[ulTempBf+6]='0';
	ucGeneralPurposeBuffer[ulTempBf+7]='0';
	
	
	//write acq cntr
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);
	longHexToHexBytes(ulDataAcqCntr,&ucGeneralPurposeBuffer[ulTempBf]);
	//write number of parameters
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);
	longHexToHexBytes(ulSystemNoOfParameters,&ucGeneralPurposeBuffer[ulTempBf]);
	
	
	
	/*sParaConfig[ulParaCntrF].para.value.fValue=fV1;
		sParaConfig[ulParaCntrF].para.ucValueStatusBit = 1;*/
	//writeall the parameters
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);
	for(ulParaCntrF=0;ulParaCntrF<ulSystemNoOfParameters;ulParaCntrF++)
	{
		ucParaStatus=sParaConfig[ulParaCntrF].para.ucValueStatusBit;
		ucParaAlarmStatus=sParaConfig[ulParaCntrF].para.ucErrorFlag;
		fParaValue=sParaConfig[ulParaCntrF].para.value.fValue;
		if(ucParaStatus==2)//->RS485_RESP_STAT_TIMEOUT
			fParaValue=-2222;
		if(ucParaStatus>2)
			fParaValue=-3333;
		
		//intToASCIIStr(ucParaAlarmStatus,1,11,&ucGeneralPurposeBuffer[ulTempBf+(ulParaCntrF*10)]);
		//intToASCIIStr(ucParaStatus,1,11,&ucGeneralPurposeBuffer[ulTempBf+(ulParaCntrF*10)+1]);
		floatHexToHexBytes(fParaValue,&ucGeneralPurposeBuffer[ulTempBf+(ulParaCntrF*8)+0]);
	}
	
	if ((ulFlashFileSaveLocation>uiFlashMaxWriteCntr) )
	{
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r Flash chip erase, write");
		chipErase();
		
		ulFlashFileSaveLocation=0;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SAVE_CNTR,ulFlashFileSaveLocation);
		ulFlashFileSentLocation=0;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SENT_CNTR,ulFlashFileSentLocation);
		
	}
	
	if(ulFlashFileSentLocation>ulFlashFileSaveLocation)
	{
		//1008
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r Flash chip erase, write, send counter grater than save cntr");
		chipErase();
		
		ulFlashFileSaveLocation=0;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SAVE_CNTR,ulFlashFileSaveLocation);
		ulFlashFileSentLocation=0;
		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SENT_CNTR,ulFlashFileSentLocation);
	}	
	
	
	if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
	{
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r writing flash file no =");
		UARTSendLong(UART_DEBUG,ulFlashFileSaveLocation,5);
		displayFlashBuffer(ucGeneralPurposeBuffer);
		
	}
	
	
	writeDataToFlash(ulFlashFileSaveLocation,ucGeneralPurposeBuffer);
	
	ulFlashFileSaveLocation++;
	writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SAVE_CNTR,ulFlashFileSaveLocation);
	
	ulDataAcqCntr++;
	writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_DATA_CNTR,ulDataAcqCntr);
	
}


void storeModbusDataToFlash(UINT32 ulParamNumber)
{
	UINT8 ucTempAf=0;
	UINT32 ulTempBf=0;
	UINT32 ulParaCntrF=0;
	UINT8 ucParaAlarmStatus=0;
	UINT8 ucParaStatus=0;
	F32 fParaValue=0;
	static UINT8 ucSendPktCnr=0;
	
	//check to see if current time is between so
	UINT16 ucFromHrs=0;
	UINT16 ucFromMin=0;
	UINT16 ucToHrs=0;
	UINT16 ucTomin=0;
	UINT16 uiFromTime=0;
	UINT16 uiToTime=0;
	UINT16 uiCurrentTime=0;
	
	UINT8 ucPktDataSize=1;
	UINT8 ucPktCurentPosition=1;
	UINT16 ucNoOFDataPoints=1;
	
	
	uiCurrentTime = ((UINT16)sRTC.ucHours*(UINT16)60)+(UINT16)sRTC.ucMinutes;
	uiFromTime = (convAsciiIntToInt(&ucTransPeriod[0],2)*60)+convAsciiIntToInt(&ucTransPeriod[2],2);
	uiToTime   = (convAsciiIntToInt(&ucTransPeriod[4],2)*60)+convAsciiIntToInt(&ucTransPeriod[6],2);
	if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
	{
			
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r Trans period = ");	
		for(ucTempAf=0;ucTempAf<8;ucTempAf++)
		{
			UARTSendChar(UART_DEBUG,ucTransPeriod[ucTempAf]);
		}
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r Current Time =");
		UARTSendLong(UART_DEBUG,uiCurrentTime,5);
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r From Time =");
		UARTSendLong(UART_DEBUG,uiFromTime,5);
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r To Time =");
		UARTSendLong(UART_DEBUG,uiToTime,5);
	}
	
	if(ucSendPktCnr<3)
	{
		ucSendPktCnr++;
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r Initial packs");
	}
	else if((uiFromTime=0) && (uiToTime=0) )
	{
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r default time");
	}
	else if((uiCurrentTime>=uiFromTime) && (uiCurrentTime<=uiToTime) )
	{
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r Between TimeInterwal");
	}
	else if(uiCurrentTime<3)
	{
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r Board started");
	}
	else if(sRTC.ucYear<17)
	{
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r error in current time");
	}
	else
	{
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r not saving data");	
		return;
	}
	
	
	
	
	FillBuffer(0x00,ucGeneralPurposeBuffer,GENERAL_PURPOSE_BUFF_SIZE);
	//writeFileTypeToMemory
	//concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"10000000",GENERAL_PURPOSE_BUFF_SIZE);
	
	concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"STR000=",100);
	
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);
	longHexToHexBytes(ulParamNumber,&ucGeneralPurposeBuffer[ulTempBf]);
		
	//write system id
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);
	longHexToHexBytes(ulSystemID,&ucGeneralPurposeBuffer[ulTempBf]);
	//writeDataAndTime
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
	intToASCIIStr(sRTC.ucDate,2,11,&ucGeneralPurposeBuffer[ulTempBf]);
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
	intToASCIIStr(sRTC.ucMonth,2,11,&ucGeneralPurposeBuffer[ulTempBf]);
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
	intToASCIIStr(sRTC.ucYear,2,11,&ucGeneralPurposeBuffer[ulTempBf]);
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
	intToASCIIStr(sRTC.ucHours,2,11,&ucGeneralPurposeBuffer[ulTempBf]);
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
	intToASCIIStr(sRTC.ucMinutes,2,11,&ucGeneralPurposeBuffer[ulTempBf]);
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
	intToASCIIStr(sRTC.ucSeconds,2,11,&ucGeneralPurposeBuffer[ulTempBf]);
	
	//write ss,r1,r2,r3 values
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);//POSITION OF LAST CHAR IN TRANSMIT BUFFER
	intToASCIIStr(ucSignalStrength,2,11,&ucGeneralPurposeBuffer[ulTempBf]);
	ucGeneralPurposeBuffer[ulTempBf+2]=P10.2+0x30;
	ucGeneralPurposeBuffer[ulTempBf+3]=P0.4+0x30;
	ucGeneralPurposeBuffer[ulTempBf+4]=P13.0+0x30;
	ucGeneralPurposeBuffer[ulTempBf+5]='0';
	ucGeneralPurposeBuffer[ulTempBf+6]='0';
	ucGeneralPurposeBuffer[ulTempBf+7]='0';
	
	
	if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
	{
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r Decoding bulk packet, recPkt size = ");
		UARTSendLong(UART_DEBUG,uiRS485RecvDataCount,5);
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r dataType = ");
		UARTSendLong(UART_DEBUG,sParaConfig[ulParamNumber].para.ucRS485DataType,3);
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r meteType = ");
		UARTSendLong(UART_DEBUG,sParaConfig[ulParamNumber].para.ucRS485MeterType,3);
		
	}
	if (sParaConfig[ulParamNumber].para.ucRS485DataType == RS485_DATATYPE_UINT8)
		ucPktDataSize=1;
	else if ( (sParaConfig[ulParamNumber].para.ucRS485DataType == RS485_DATATYPE_UINT16) || (sParaConfig[ulParamNumber].para.ucRS485DataType == RS485_DATATYPE_INT16) )
		ucPktDataSize=2;
	else 
		ucPktDataSize=4;
		
	if(uiRS485RecvDataCount<=5)
		ucNoOFDataPoints=0;
	else
		ucNoOFDataPoints=uiRS485RecvDataCount-5; // remove id, fc, length and CRC
	ucNoOFDataPoints=ucNoOFDataPoints/ucPktDataSize;
	ucPktCurentPosition=3;
	
	
	//write acq cntr
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);
	longHexToHexBytes(ulDataAcqCntr,&ucGeneralPurposeBuffer[ulTempBf]);
	//write number of parameters
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);
	longHexToHexBytes(ucNoOFDataPoints,&ucGeneralPurposeBuffer[ulTempBf]);
	
	
	
	/*sParaConfig[ulParaCntrF].para.value.fValue=fV1;
		sParaConfig[ulParaCntrF].para.ucValueStatusBit = 1;*/
	//writeall the parameters
	getStrLength(ucGeneralPurposeBuffer,&ulTempBf);
	for(ulParaCntrF=0;ulParaCntrF<ucNoOFDataPoints;ulParaCntrF++)
	{
		
		fParaValue=getDataFromModbusPAcket(sParaConfig[ulParamNumber].para.ucRS485DataType,sParaConfig[ulParamNumber].para.ucRS485MeterType, &ucRS485RecvBuffer[ucPktCurentPosition]);
		
		fParaValue *= sParaConfig[ulParamNumber].para.fMultiplyFactor;
		fParaValue += sParaConfig[ulParamNumber].para.fAdditionFactor;
		
		ucPktCurentPosition=ucPktCurentPosition+ucPktDataSize;
		floatHexToHexBytes(fParaValue,&ucGeneralPurposeBuffer[ulTempBf+(ulParaCntrF*8)+0]);
		
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
		{
			
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r RS485 Dec Parameter = ");
			UARTSendLong(UART_DEBUG,ulParaCntrF,5);
			UARTSendStr(UART_DEBUG,(UINT8 *)", fParaValue = ");
			UARTSendFloat(UART_DEBUG,fParaValue,6,5);
			
			
		}
		
	}
	
	
	
	if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
	{
		UARTSendStr(UART_DEBUG,(UINT8 *)"\r writing flash file no =");
		UARTSendLong(UART_DEBUG,ulFlashFileSaveLocation,5);
		displayFlashBuffer(ucGeneralPurposeBuffer);
		
	}
	
	
	writeDataToFlash(ulFlashFileSaveLocation,ucGeneralPurposeBuffer);
	
	ulFlashFileSaveLocation++;
	writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SAVE_CNTR,ulFlashFileSaveLocation);
	
//	ulDataAcqCntr++;
//	writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_DATA_CNTR,ulDataAcqCntr);
	
}



void sendStoredDataToserver(void)
{
	#define SM_READ_DATA 0
	#define SM_TRA_DATA 1

	static UINT8 ucStateF=0;
	UINT32 ulNoOfParaF=0;
	UINT32 ulTempAf,ulTempBf;
	UINT16 ucTempF,ucTempAf;
	UINT8 ucSMFlagF=0;
	
	uiPendingFiles=(UINT16)(ulFlashFileSaveLocation-ulFlashFileSentLocation);
	
	if(ulFlashFileSentLocation>=ulFlashFileSaveLocation)
	{
		ucStateF=SM_READ_DATA;
		return;
	}	
	if(ucStateF==SM_READ_DATA)
	{
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
		{
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r reading flash file no =");
			UARTSendLong(UART_DEBUG,ulFlashFileSentLocation,5);
		}
		
		clearGeneralPurposeBuffer();
		//concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"ENCR_DATA=",GENERAL_PURPOSE_BUFF_SIZE);
		readFlashData(&ucGeneralPurposeBuffer[0],ulFlashFileSentLocation);
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
			displayFlashBuffer(ucGeneralPurposeBuffer);
		
		
		
		prepareGPRSTransmitPacket(ucGeneralPurposeBuffer,ucGPRSTransmitBuffer,ucSerAdd,ucSerPort,ucSerPath,ucSerAppRecData,GPRS_TRANS_MODE_POST);
		ucDataTransferSuccess=0;
		ucStateF=SM_TRA_DATA;
	}
	else
	{
		ucSMFlagF=1;
		
		getStrLength(ucGPRSTransmitBuffer,&ulTempBf );
		if(ulTempBf < 15)
		{
			ucStateF=SM_READ_DATA;
			if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
			{
			UARTSendStr(UART_DEBUG,(UINT8 *)"\r Flash chip erase, becasue of invalid data read");
			}
			chipErase();
			
			ulFlashFileSaveLocation=0;
			writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SAVE_CNTR,ulFlashFileSaveLocation);
			ulFlashFileSentLocation=0;
			writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SENT_CNTR,ulFlashFileSentLocation);
			
			
		}
		
		if(uiPendingFiles>2)
			tranferDataToServer(&ucSMFlagF,ucGPRSTransmitBuffer,ucSerAppRecData,GPRS_TRANS_MODE_POST,GPRS_CONNECTION_CLOSE);
		else
			tranferDataToServer(&ucSMFlagF,ucGPRSTransmitBuffer,ucSerAppRecData,GPRS_TRANS_MODE_POST,GPRS_CONNECTION_CLOSE);
		if(ucSMFlagF==0)
		{
			if(ucDataTransferSuccess==1)
			{
				ucStateF=SM_READ_DATA;
				
				ulFlashFileSentLocation++;
				writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SENT_CNTR,ulFlashFileSentLocation);
				checkBufferForCommands(ucGPRSRecvBuffer,uiGPRSRecvDataCount,BUFFER_DONT_CLEAR);
				
				//if max read has been crosssed and its the last packet then deleted data
				if ( (ulFlashFileSaveLocation>uiFlashMaxReadCntr) && (ulFlashFileSentLocation>=ulFlashFileSaveLocation) )
				{
					if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugNetwork==1) ) 
					{
					UARTSendStr(UART_DEBUG,(UINT8 *)"\r Flash chip erase, read");
					}
					chipErase();
					
					ulFlashFileSaveLocation=0;
					writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SAVE_CNTR,ulFlashFileSaveLocation);
					ulFlashFileSentLocation=0;
					writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_FLASH_SENT_CNTR,ulFlashFileSentLocation);
					
				}
				
			}
			else
			{
				ucStateF=SM_READ_DATA;
			}
		}		
		
	}
	
}


UINT8 convertVoltageToResistance(F32 fSupplyVolts, F32 fSensorVolts, F32 fpullUpresistor, F32* pfResistance) {
    F32 fResistance;
    fResistance = (fSensorVolts * fpullUpresistor) / (fSupplyVolts - fSensorVolts);
    *pfResistance = fResistance;
    return SUCCESS;
}

UINT8 convert10KTheristerResitanceToTemp(F32 fResistance, F32* fTemperaure) {
    F32 fTemperature;
    fTemperature = log(fResistance);
    fTemperature = 1 / (0.001129148 + (0.000234125 * fTemperature) + (0.0000000876741 * fTemperature * fTemperature * fTemperature));
    fTemperature = fTemperature - 273.15;
    *fTemperaure = fTemperature;
    return SUCCESS;
}


void sendCSVData(void)
{
	UINT8 ucParaCntr=0;
	if(ucEnableCSV==0)
		return;
	if(ucCSVWriteHeader==1)
	{
		ucCSVWriteHeader=0;
		
		//
		UARTSendStr(UART_DEBUG,(UINT8C*)"\r\n DATE,");
		for(ucParaCntr=0;ucParaCntr<ulSystemNoOfParameters;ucParaCntr++)
		{
			
			UARTSendStr(UART_DEBUG,sParaConfig[ucParaCntr].para.ucParaStr);
			UARTSendStr(UART_DEBUG,(UINT8C*)",");
		}	
		UARTSendStr(UART_DEBUG,(UINT8C*)"\r\n");
	}
	
	UARTSendLong(UART_DEBUG,sRTC.ucDate,2);
	UARTSendStr(UART_DEBUG,(UINT8*)"-");
	UARTSendLong(UART_DEBUG,sRTC.ucMonth,2);
	UARTSendStr(UART_DEBUG,(UINT8*)"-");
	UARTSendLong(UART_DEBUG,sRTC.ucYear,2);
	UARTSendStr(UART_DEBUG,(UINT8*)" ");
	UARTSendLong(UART_DEBUG,sRTC.ucHours,2);
	UARTSendStr(UART_DEBUG,(UINT8*)":");
	UARTSendLong(UART_DEBUG,sRTC.ucMinutes,2);
	UARTSendStr(UART_DEBUG,(UINT8*)":");
	UARTSendLong(UART_DEBUG,sRTC.ucSeconds,2);
	UARTSendStr(UART_DEBUG,(UINT8*)",");
	for(ucParaCntr=0;ucParaCntr<ulSystemNoOfParameters;ucParaCntr++)
	{
		
		UARTSendFloat(UART_DEBUG,sParaConfig[ucParaCntr].para.value.fValue,10,2);
		UARTSendStr(UART_DEBUG,(UINT8C*)",");
	}	
	UARTSendStr(UART_DEBUG,(UINT8C*)"\r\n");
	
}
