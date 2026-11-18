/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIESREGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
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
* Copyright (C) 2011, 2017 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name    : r_cg_timer_user.c
* Version      : CodeGenerator for RL78/G13 V2.04.00.03 [05 May 2017]
* Device(s)    : R5F100PL
* Tool-Chain   : CA78K0R
* Description  : This file implements device driver for TAU module.
* Creation Date: 11-09-2017
***********************************************************************************************************************/

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
#pragma interrupt INTTM00 r_tau0_channel0_interrupt
#pragma interrupt INTTM01 r_tau0_channel1_interrupt
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_timer.h"
/* Start user code for include. Do not edit comment generated here */
#include "delay.h"
#include "dio.h"
#include "global.h"
#include "uartGPRS.h"
#include "modbus.h"
#include "dio.h"
#include "controller.h"
#include "memory.h"
#include "gprs.h"
#include "ui.h"
#include "data.h"
#include "rtc.h"
#include "uart.h"
/* End user code. Do not edit comment generated here */
#include "r_cg_userdefine.h"

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* Start user code for global. Do not edit comment generated here */
UINT16 uiMsCounterLocal=0;
UINT16 uiMSCntrGPRSStatus=0;
UINT16 uiMSCntrGPRSStatus1=0;
UINT8 uiTempGPRSStat=0;
//1003
UINT8 ucResetKeyPreviousStat=RESET_BTN_NOT_PRESSED;
UINT8 ucRestKeyCntrTemp=0;

//1013
UINT16 ucEEPROMResetTimer = 0;

/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: r_tau0_channel0_interrupt
* Description  : This function is INTTM00 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_tau0_channel0_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
	if(uiMSDelayCntr>0)
		uiMSDelayCntr--;
		
	if(uiInComingCallTimer>0)	
		uiInComingCallTimer--;
		
	uiGPRSTimer++;
	uiMODBUSTimerReg++;
	uiGPMsCntr++;
	uiResetKeyTimer++;
	
		
		
	//1043
	if(uiUartDebugRecDataTimerMs<10000)
		uiUartDebugRecDataTimerMs++;
		
	if(uiUartDebugRecDataTimerMs>200)
	{
		uiDebugRecvDataCount=0;
		uiUartDebugRecDataTimerMs=200;
		
	}
	
	//1003
	//rest key counter 
	if( (ucResetKeyPreviousStat!=RESET_BTN) && (RESET_BTN==RESET_BTN_PRESSED))
	{
		ucRestKeyCntrTemp++;
		uiResetKeyTimer=0;
	}
	ucResetKeyPreviousStat=RESET_BTN;
	if(uiResetKeyTimer==3000)
	{
		ucResetKeyCounter=ucRestKeyCntrTemp;	
		ucRestKeyCntrTemp=0;
	}
	if(uiResetKeyTimer>10000)
		uiResetKeyTimer=10000;
	
	if(RESET_BTN==RESET_BTN_PRESSED)
	{
		ucEEPROMResetTimer++;
		if(ucEEPROMResetTimer>10000)	
		{
			ucResetKeyCounter=5;	
		}
	}
	else
		ucEEPROMResetTimer=0;
	
	
	uiMSCntrGPRSStatus++;
	if(uiMSCntrGPRSStatus>200)
	{
		if(GPRS_RESET==0)
		{
			DB_LED_MSG=!DB_LED_MSG;	
		}
		
		
		uiMSCntrGPRSStatus=0;
		uiMSCntrGPRSStatus1++;
		
		if(uiMSCntrGPRSStatus1>20)
		{
			uiMSCntrGPRSStatus1=0;	
		}
		
		
//		if(ucInternetSource==0)
		{
			if(ucGPRSSMState==0)
				uiTempGPRSStat=0;
			else if(ucGPRSSMState>=GPRS_STATE_EXTABLISH_CONNECTION_CLIENT)
				uiTempGPRSStat=10;
			else if(ucGPRSSMState>=GPRS_STATE_SIGNAL_STRENGTH)	// 4 blinks
				uiTempGPRSStat=8;
			else if(ucGPRSSMState>=GPRS_STATE_CHECK_CONNECTION)	// 3 blinks
				uiTempGPRSStat=6;
			else if(ucGPRSSMState>=GPRS_STATE_CHECK_SIM)		// 2 blink
				uiTempGPRSStat=4;
			else if(ucGPRSSMState>=GPRS_STATE_GET_IMIE)		// 1 blink
				uiTempGPRSStat=2;	
			else
				uiTempGPRSStat=0;
		}
//		else if(ucInternetSource==2)
//		{
//			if(ucGPRSSMStateSIM7600==0)
//				uiTempGPRSStat=0;
//			else if(ucGPRSSMStateSIM7600>=SIM7600_STATE_CONNECT_TO_SERVER)
//				uiTempGPRSStat=6;
//			else if(ucGPRSSMStateSIM7600>=SIM7600_STATE_SIGNAL_STRENGTH)
//				uiTempGPRSStat=4;
//			else if(ucGPRSSMStateSIM7600>=SIM7600_STATE_GET_IMIE)
//				uiTempGPRSStat=2;	
//		}
		
		if( (uiMSCntrGPRSStatus1<13) && (uiMSCntrGPRSStatus1<uiTempGPRSStat) )
		{
			if((uiMSCntrGPRSStatus1%2)==0)
				DB_LED_GPRS=DB_LED_ON;
			else
				DB_LED_GPRS=DB_LED_OFF;
		}
		else
			DB_LED_GPRS=DB_LED_OFF;
			
		if(ucGPRSSMState>GPRS_STATE_EXTABLISH_CONNECTION_CLIENT)	
			DB_LED_GPRS=DB_LED_ON;
		
		
			
	}
	
	if(ulIntTimeMs<100100)
	{
		ulIntTimeMs++;
	}
	
	if(RESET_BTN==RESET_BTN_PRESSED)
		ucFlagRstButtonPressed=SET;		
	
	uiMsCounterLocal++;
	if(uiMsCounterLocal>1000)
	{
		uiMsCounterLocal=0;	
		ucKeyTimer++;
		
		//1015
		rtcInterrupt();
		if(sRTC.ucSeconds==0)
			ucCSVSendData=1;
	}
	
	
	
	
    /* End user code. Do not edit comment generated here */
}
/***********************************************************************************************************************
* Function Name: r_tau0_channel1_interrupt
* Description  : This function is INTTM01 interrupt service routine.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
__interrupt static void r_tau0_channel1_interrupt(void)
{
    /* Start user code. Do not edit comment generated here */
    if(uiUSDelayCntr>0)
    	uiUSDelayCntr--;
	
    /* End user code. Do not edit comment generated here */
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
