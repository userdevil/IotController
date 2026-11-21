/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2011, 2018 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_main.c
* Version      : CodeGenerator for RL78/G13 V2.05.03.01 [12 Nov 2018]
* Device(s)    : R5F100PL
* Tool-Chain   : CA78K0R
* Description  : This file implements main function.
* Creation Date: 09-09-2020
***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_cgc.h"
#include "r_cg_port.h"
#include "r_cg_intc.h"
#include "r_cg_serial.h"
#include "r_cg_adc.h"
#include "r_cg_timer.h"
#include "r_cg_wdt.h"
#include "r_cg_rtc.h"
/* Start user code for include. Do not edit comment generated here */
#include "global.h"
#include "generalfunctions.h"
#include "dio.h"
#include "delay.h"
#include "uart.h"
#include "uartDebug.h"
#include "uartRS485.h"
#include "uartZB.h"
#include "uartGPRS.h"
#include "analog.h"
#include "rtc.h"
#include "display.h"
#include "eeprom.h"
#include "spi.h"
#include "flash.h"
#include "modbus.h"
#include "memory.h"
#include "gprs.h"
#include "controller.h"
#include "data.h"
#include "ui.h"
#include "spi.h"
#include "relaymodule.h"

/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
UINT8 ucModemFlag;
UINT8 ucModemFlagSIM7600;
UINT8 ucRS485Flag;
UINT8 ucTempA,ucTempB;
UINT16 uiTempA;
UINT32 ulTempA;
F32 fTemp;

void checkResetButtonPress(void);

/* End user code. Do not edit comment generated here */
void R_MAIN_UserInit(void);

/***********************************************************************************************************************
* Function Name: main
* Description  : This function implements main function.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void main(void)
    {
    R_MAIN_UserInit();
    /* Start user code. Do not edit comment generated here */
    ucInitComplete=0;
    
    //init default values
    ucFlagRstButtonPressed=0;
    ucKeyTimer=0;
    ucModemFlag=1;
    uiMODBUSTimerReg=0;
    ucRS485Flag=1;
    
    uiAcqIntMin=0;
    uiTraIntMin=0;
    ucPraAcqFlag=1;
    ucPraTraFlag=1;

    
    //DEBUG
    ucDebugEnableFlag=DEBUG_NIL;
    
    //gprs
    ucGPRSSMState=0;
    ucNetworkStatus=0;
    ucSignalStrength=0;
    ucDataTransferSuccess=0;
    ucGPRSSMStateMax=0;
    ucSIMStatus=0;
    ucGPRSConStatus=0;
    
    ucCumilativeResetFlag=0;
    
    ucTimerNonComm=0;
    
    uiPendingFiles=0;
    
    
    uiTempA=0;//1005
    while(RESET_BTN==RESET_BTN_PRESSED)
    {
	    uiTempA++;
	    delayMs(100);
	    DB_LED_PWR=!DB_LED_PWR;
	    
	    if(uiTempA>50)
	    {
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_EEPROM_MEM_STATUS,'x'); 
		while(1);
	    }
    }
    
    
    ucTempA=0;
    ucTempB=0;
        
	
   glcdWriteStrCallibri(1,55,(UINT8*)"Loading X50_2..",11,1);
   glcdUpdate();
   //read stored data from eeprom
   readDataFromMemory();
   
   glcdWriteStrCallibri(1,55,(UINT8*)"Read complete..",11,1);
   glcdUpdate();	
    
	ucModemFlag=1;

	
   ucInternetSource=0;
   writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_INTERNET_SOURCE,ucInternetSource);

   ucMS_WAIT_BEFORE_RESET=1000;	
   clearGeneralPurposeBuffer();
   concatStirngsToStr1(ucGeneralPurposeBuffer,(UINT8*)"IMIENumber=",GENERAL_PURPOSE_BUFF_SIZE);
   concatStirngsToStr1(ucGeneralPurposeBuffer,ucIMIENumber,GENERAL_PURPOSE_BUFF_SIZE);
   prepareGPRSTransmitPacket(ucGeneralPurposeBuffer,ucGPRSTransmitBuffer,ucSerAdd,ucSerPort,ucSerPath,ucSerAppReg,GPRS_TRANS_MODE_POST);
  
   glcdWriteStrCallibri(1,55,(UINT8*)"Init modem......",11,1);
   glcdUpdate();
   
	
   
   uiGPMsCntr=0;
   ucClockReadFromModem=0;
   //1043
   while( (ucClockReadFromModem==0) && (uiGPMsCntr<50000) && (ucIMIENumber[0]!= '1'))
   {
	   R_WDT_Restart();
	   
	   
	   gprsTranferDataToServer4G(&ucModemFlag,ucGPRSTransmitBuffer,ucSerAppReg,GPRS_TRANS_MODE_POST,GPRS_CONNECTION_CLOSE);
	   
		if(ucUartDebugRecDataFlag==1)
		{
			ucUartDebugRecDataFlag=0;
			checkBufferForCommands(ucDebugRecvBuffer,DEBUG_REC_BUFFER_SIZE,BUFFER_CLEAR);
		}
	   
   }
   
   readRTCValue();
   
   ucMS_WAIT_BEFORE_RESET=60000;
   
   getAllAnalogValues();
   
   ucSendSMSFlag=1;
   

   ucDebugEnableFlag=0;
   
   ucAcqDataDisable=0;
   
   ucBoardTestSMFlag=0;
   ucBaordTestInProgress=0;
   
   P1.3=1;
   
   ucUartInit(UART_ZIGBEE,UART_BAUD_9600,UART_DATABIT_EIGHT,UART_PARITY_NONE,UART_STOPBIT_ONE);
   R_UART2_Receive(&ucUartZBRecData,1);
   
   ucRS485BusFlag=RS485_CON_DATA_ACQ;
   
    glcdFillScreen(0);
    drawDeviceLogo();
    glcdUpdate();
    
    //delete
    	ucDebugEnableFlag=DEBUG_NIL;
	ucDebugRS485=0;
	ucDebugNetwork=0;
    
    ucDebugNetwork=0;
    ucDebugRS485=0;
    
    
    
    //1043
    if ( (ucDebugBaudRate>7) ||(ucDebugBaudRate<1) )
    	ucDebugBaudRate=7;
   // 	ucUartInit(UART_DEBUG,UART_BAUD_9600,UART_DATABIT_EIGHT,UART_PARITY_NONE,UART_STOPBIT_ONE);
  //  else
    	ucUartInit(UART_DEBUG,ucDebugBaudRate,UART_DATABIT_EIGHT,UART_PARITY_NONE,UART_STOPBIT_ONE);
    
    
	
	
    while (1U)
    {
	ucInitComplete=1;    
	    
        R_WDT_Restart();
	
	
//	//1013
//	if( (ucTimerNonComm>30) && (uiPendingFiles>0) )	
//	{
//		
//		storeRTCValue();
//		while(1);
//	}
//	
//	if(ucTimerNonComm > 300)
//	{
//		storeRTCValue();
//		while(1);
//	}
	
	
	if(ucUartDebugRecDataFlag==1)
	{
		ucUartDebugRecDataFlag=0;
 		checkBufferForCommands(ucDebugRecvBuffer,DEBUG_REC_BUFFER_SIZE,BUFFER_CLEAR);
	}
	
	
	
	if(sRTC.ucPreSeconds!=sRTC.ucSeconds)
	{
		displayScreen();
		
		getAllAnalogValues();
		sRTC.ucPreSeconds=sRTC.ucSeconds;
		processInterruptData();	
		
		if(ucCSVSendData==1)
		{
			ucCSVSendData=0;
			sendCSVData();
		}
		
		if( (sRTC.ucSeconds==0) && (ucDebugEnableFlag==DEBUG_ENABLE_ALL) )
			sendRtcViaDebug();
		
		
	}
	
	
	if(ucBaordTestInProgress!=0)
	{
		if(ucBoardTestSMFlag==1)
			uploadBoardTestReport();
	}
	else if(ucInComingCallFlag)
	{
		
	}
	else if(ucSystemRegistrationStatus!=1)
	{
		imieRegistration();	
	}
	else if(ucSystemDownloadUnitConfig==1)
	{
		if( (ulSystemID==0) || (ulSystemID>50000)  )
		{	
			ucSystemRegistrationStatus=0;
			delayMs(30000);	
		}
			
		downloadUnitConfig();
	}
	else if(ucSystemDownloadParaConfig==1)
	{
		
		//1013
		if(ulSystemNoOfParameters>200)
			ucSystemDownloadUnitConfig=1;
			
		downloadParaConfig();
	}
	else if(ucSystemDownloadRelayModuleConfig==1)
	{
		downloadRelayModuleConfig();
	}
	else
	{
		if(ucSendSMSFlag==1)
		{
			sendSMSStateMachine(&ucSendSMSFlag);	
		}
		else
		{
			sendStoredDataToserver();	
		}
	}
	relayModuleService();
	
	if(ucCheckRelayOperation==1)
	{
		ucCheckRelayOperation=0;
		checkRelay1Operation();
		checkRelay2Operation();
		checkRelay3Operation();
	}
	

	//acq data
	if( (ucAcqDataDisable==0) && (ucBaordTestInProgress==0) && (ucRS485BusFlag==RS485_CON_DATA_ACQ) &&(ucSystemRegistrationStatus==1) && (ucSystemDownloadUnitConfig==0) && (ucSystemDownloadParaConfig==0) && (ucSystemDownloadRelayModuleConfig==0)  )
		acquireData(&ucPraAcqFlag);
	
	receiveSMS();
	checkForIncomingCall();
	
	//1003
	checkResetButtonPress();

	//if(ucUartDebugRecDataFlagMODBUS==1)
	//{
		//ucUartDebugRecDataFlagMODBUS=0;
		//processModbusMasterRequest();
	//}
	
	
    }
    /* End user code. Do not edit comment generated here */
}

/***********************************************************************************************************************
* Function Name: R_MAIN_UserInit
* Description  : This function adds user code before implementing main function.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_MAIN_UserInit(void)
{
    /* Start user code. Do not edit comment generated here */
    EI();
    
    ucInitComplete=0;
    //start 1ms timer used for delay
    R_TAU0_Channel0_Start();
    dio_initLED();
    dio_initRelay();
    dio_blink(LED_PWR,100);
    dio_blink(LED_GPRS,100);
    dio_blink(LED_MSG,100);
    dio_blink(LED_ACQ,100);
    
    //display
    ucDispBackFlag=1;
    lcdInit();
    lcdClear();
    drawDeviceLogo();
    glcdUpdate();
    
    
    //going to fix the clock division one for low speed and one for high speed
    // bit 4-7, clk 1 for high speed, 38400, 57600 and 115200 baud, 2^2
    // bit 0-3, clk 0 for low  speed, 2400,4800,9600,19200, 2^6 
    // clock selection and counter value will be selected below
    SPS0 = _0006_SAU_CK00_FCLK_6 | _0020_SAU_CK01_FCLK_2;
    SPS1 = _0006_SAU_CK00_FCLK_6 | _0020_SAU_CK01_FCLK_2;
    
    
    //start uart1 used for debug
    ucUartInit(UART_DEBUG,UART_BAUD_115200,UART_DATABIT_EIGHT,UART_PARITY_NONE,UART_STOPBIT_ONE);
   
    
    
    //start uart3 for rs485
    ucUartInit(UART_RS485,UART_BAUD_9600,UART_DATABIT_EIGHT,UART_PARITY_NONE,UART_STOPBIT_ONE);
    
    //satar uart0 for gprs
    
    
    //delete 
   ucModemType=MODEM_TYPE_4G;
   ucUartInit(UART_GPRS,UART_BAUD_115200,UART_DATABIT_EIGHT,UART_PARITY_NONE,UART_STOPBIT_ONE);
  
  
    
    //start UART2 used for the second RS-485 bus
    ucUartInit(UART_ZIGBEE,UART_BAUD_9600,UART_DATABIT_EIGHT,UART_PARITY_NONE,UART_STOPBIT_ONE);
    
    //start ADC converter
    R_ADC_Start();
    
    //RTC
    R_RTC_Start();
    sRTC.ucSeconds=0;
    sRTC.ucMinutes=0;
    sRTC.ucHours=0;
    sRTC.ucDate=23;
    sRTC.ucMonth=10;
    sRTC.ucYear=14;
    writeToProcessorRTC();
    
    
    //i2c
    R_IICA0_Create();
    
    debugSendStr((UINT8*)"\r\r\r\rVANMEN IoT Controller");
    
  
    flashInit();
    flashReadID(1,&ucFlashID);
    
    if(ucFlashID!=0x8e)
    {
	    debugSendStr((UINT8*)"\rFlash failure");
	    glcdWriteStrCallibri(1,45,(UINT8*)"Flash Error ....",11,1);
    	    glcdUpdate();
	    for(ucTempA=0;ucTempA<10;ucTempA++)
	    {
		  DB_LED_PWR=!DB_LED_PWR;
		  flashInit();
    		  flashReadID(1,&ucFlashID);
		  if(ucFlashID==0x8e)
		  	ucTempA=100;
	    }
    }

	    
    //start external int
    R_INTC7_Start();
    
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */

//1003
void checkResetButtonPress(void)
{
	if(ucResetKeyCounter<=1)
	{
		ucResetKeyCounter=0;
		return;	
	}
	else if(ucResetKeyCounter==2)
	{
		
		ucPraAcqFlag=1;
		ucPraTraFlag=1;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rKey Cmd -> acq now ");
		
	}
	else if(ucResetKeyCounter==3)
	{
		ucInternetSource=0;
		UARTSendStr(UART_DEBUG, (UINT8*) "\rKey cmd -> GPRS selected ");
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_INTERNET_SOURCE,ucInternetSource);
	}
	else if(ucResetKeyCounter==4)
	{
//		UARTSendStr(UART_DEBUG, (UINT8*) "\rKey cmd -> resetting server parameters ");
//		storeStringToEeprom((UINT8*)"www.yantramicrosystems.com",EEPROM_LOC_DATA_SER_ADD,48);
//		storeStringToEeprom((UINT8*)"/device_backend/",EEPROM_LOC_DATA_SER_PATH,98);
//		storeStringToEeprom((UINT8*)"80",EEPROM_LOC_DATA_SER_PORT,8);
//		storeStringToEeprom((UINT8*)"IMIERegistrationPost.aspx",EEPROM_LOC_SER_APP_REG,38);
//		storeStringToEeprom((UINT8*)"systemConfigPost.aspx",EEPROM_LOC_SER_APP_SYS_CONFIG,38);
//		storeStringToEeprom((UINT8*)"paraconfigPost.aspx",EEPROM_LOC_SER_APP_PARA_CONFIG,38);
//		storeStringToEeprom((UINT8*)"realtimeDataPost.aspx",EEPROM_LOC_SER_APP_REC_DATA,38);
//				
//		
//		storeStringToEeprom((UINT8*)"airtelgprs.com",EEPROM_LOC_GPRS_APN,23);
//		storeStringToEeprom((UINT8*)"",EEPROM_LOC_GPRS_USERNAME,23);
//		storeStringToEeprom((UINT8*)"",EEPROM_LOC_GPRS_PASSWORD,23);
//		ulSIMBalCntr=0;
//		writeExtEEPROMLong(EEPROM_ADD,EEPROM_LOC_SIM_BAL_CNTR,ulSIMBalCntr);
//		
//		readStringFromEeprom(ucSerAdd,EEPROM_LOC_DATA_SER_ADD,48);
//		readStringFromEeprom(ucSerPath,EEPROM_LOC_DATA_SER_PATH,98);
//		readStringFromEeprom(ucSerPort,EEPROM_LOC_DATA_SER_PORT,8);
//		readStringFromEeprom(ucSerAppReg,EEPROM_LOC_SER_APP_REG,38);
//		readStringFromEeprom(ucSerAppSysConfig,EEPROM_LOC_SER_APP_SYS_CONFIG,38);
//		readStringFromEeprom(ucSerAppParaConfig,EEPROM_LOC_SER_APP_PARA_CONFIG,38);
//		readStringFromEeprom(ucSerAppRecData,EEPROM_LOC_SER_APP_REC_DATA,38);
		
		
	}//1005
	else if(ucResetKeyCounter==5)
	{
		writeExtEEPROMChar(EEPROM_ADD,EEPROM_LOC_EEPROM_MEM_STATUS,'x'); 
		while(1);
	}
	ucResetKeyCounter=0;
}


/* End user code. Do not edit comment generated here */
