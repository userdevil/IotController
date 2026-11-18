#include "uart.h"
#include "generalfunctions.h"
#include "r_cg_serial.h"
#include "uartGPRS.h"
#include "uartRS485.h"
#include "delay.h"


unsigned char ucUartDebugSendFlag;
unsigned char ucUartDebugRecData;
unsigned char ucUartDebugRecDataFlag;
unsigned char ucDebugRecvBuffer[DEBUG_REC_BUFFER_SIZE];
unsigned int uiDebugRecvDataCount;

//1043
unsigned char ucUartDebugRecDataFlagMODBUS;
unsigned int uiUartDebugRecDataTimerMs;

unsigned char ucUartRS485SendFlag;
unsigned char ucUartRS485RecData;
unsigned char ucUartRS485RecDataFlag;
unsigned char ucRS485RecvBuffer[RS485_REC_BUFFER_SIZE];
unsigned int uiRS485RecvDataCount;


unsigned char ucUartZBSendFlag;
unsigned char ucUartZBRecData;
unsigned char ucUartZBRecDataFlag;
unsigned char ucZBRecvBuffer[ZB_REC_BUFFER_SIZE];
unsigned int uiZBRecvDataCount;

unsigned char ucUartGPRSSendFlag;
unsigned char ucUartGPRSRecData;
unsigned char ucUartGPRSRecDataFlag;
unsigned char ucGPRSRecvBuffer[GPRS_REC_BUFFER_SIZE];
unsigned char ucGPRSTransmitBuffer[GPRS_TRA_BUFFER_SIZE];
unsigned int uiGPRSRecvDataCount;



UINT8 ucUartInit(UINT8 ucUARTModuleNo, UINT8 ucBaud, UINT8 ucDataBits, UINT8 ucParity, UINT8 ucStopBits)
{
	unsigned int ucSDRValue=0x0000;
	unsigned int uiParityValue=0x0000;
	unsigned int uiStopBitValue=0x0000;
	unsigned int uiClockSource = 0x0000;
	unsigned int uiDataBits =0x0000;
	
	switch(ucBaud)
	{
		
		case UART_BAUD_2400:
			uiClockSource = _0000_SAU_CLOCK_SELECT_CK00;
			ucSDRValue = 0xCE00;
		break;
		
		case UART_BAUD_4800:
			ucSDRValue = 0x6700;
			uiClockSource = _0000_SAU_CLOCK_SELECT_CK00;
		break;
		
		
		case UART_BAUD_9600:
			ucSDRValue = 0x3300;
			uiClockSource = _0000_SAU_CLOCK_SELECT_CK00;
		break;
		
		case UART_BAUD_19200:
			ucSDRValue = 0x1900;
			uiClockSource = _0000_SAU_CLOCK_SELECT_CK00;
		break;
		
		case UART_BAUD_38400:
			ucSDRValue = 0xCE00;
			uiClockSource = _8000_SAU_CLOCK_SELECT_CK01;
		break;
		
		case UART_BAUD_57600:
			ucSDRValue = 0x8A00;
			uiClockSource = _8000_SAU_CLOCK_SELECT_CK01;
		break;
		
		case UART_BAUD_115200:
			ucSDRValue = 0x4400;
			uiClockSource = _8000_SAU_CLOCK_SELECT_CK01;
		break;
		
		default:
		
			ucSDRValue = 0x3300;
			uiClockSource = _0000_SAU_CLOCK_SELECT_CK00;
			
		break;
		
		
	}
	
	switch(ucParity)
	{
		//make both the bits high
		
		
		case UART_PARITY_NONE:
			uiParityValue =_0000_SAU_PARITY_NONE;
		break;
		
		case UART_PARITY_ODD:
			uiParityValue = _0300_SAU_PARITY_ODD;
		break;
		
		case UART_PARITY_EVEN:
			uiParityValue=_0200_SAU_PARITY_EVEN;
			
		break;
		
		default:
			uiParityValue=_0000_SAU_PARITY_NONE;
		break;
		
	}
        
	switch(ucStopBits)
	{
		case UART_STOPBIT_ONE:
			uiStopBitValue = _0010_SAU_STOP_1;
		break;
		
		case UART_STOPBIT_NO:
			uiStopBitValue = _0000_SAU_STOP_NONE;
		
		break;
		
		case UART_STOPBIT_TWO:
			uiStopBitValue = _0020_SAU_STOP_2;
		break;

		default:
		
		break;
	}
	
	switch(ucDataBits)
	{
		
		case UART_DATABIT_EIGHT:
			uiDataBits =_0007_SAU_LENGTH_8;
		break;
		
		case UART_DATABIT_NINE:
			uiDataBits =_0006_SAU_LENGTH_7;
		break;
		
		case UART_DATABIT_SEVEN:
			uiDataBits =_0006_SAU_LENGTH_7;
		break;
	
		default:
			uiDataBits =_0007_SAU_LENGTH_8; 
		break;
	}
	
	
	switch(ucUARTModuleNo)
	{
		case UART_DEBUG:
		{
		    	R_UART1_Stop();

				
			SMR02 = _0020_SAU_SMRMN_INITIALVALUE | uiClockSource | _0000_SAU_TRIGGER_SOFTWARE |
			    _0002_SAU_MODE_UART | _0000_SAU_TRANSFER_END;
			SCR02 = _8000_SAU_TRANSMISSION | _0000_SAU_INTSRE_MASK | uiParityValue | _0080_SAU_LSB | uiStopBitValue |
			    uiDataBits;
			SDR02 = ucSDRValue;
			
					
			SMR03 = _0020_SAU_SMRMN_INITIALVALUE | uiClockSource | _0100_SAU_TRIGGER_RXD | _0000_SAU_EDGE_FALL |
			    _0002_SAU_MODE_UART | _0000_SAU_TRANSFER_END;
			SCR03 = _4000_SAU_RECEPTION | _0000_SAU_INTSRE_MASK | uiParityValue | _0080_SAU_LSB | uiStopBitValue |
			    uiDataBits;
			SDR03 = ucSDRValue;
			
			
			R_UART1_Start();
		
			R_UART1_Receive(&ucUartDebugRecData,1);

		}
		break;
		
		case UART_GPRS :
		{
			R_UART0_Stop();

				
			SMR00 = _0020_SAU_SMRMN_INITIALVALUE | uiClockSource | _0000_SAU_TRIGGER_SOFTWARE |
			    _0002_SAU_MODE_UART | _0000_SAU_TRANSFER_END;
			SCR00 = _8000_SAU_TRANSMISSION | _0000_SAU_INTSRE_MASK | uiParityValue | _0080_SAU_LSB | uiStopBitValue |
			    _0007_SAU_LENGTH_8;
			SDR00 = ucSDRValue;
			
					
			SMR01 = _0020_SAU_SMRMN_INITIALVALUE | uiClockSource | _0100_SAU_TRIGGER_RXD | _0000_SAU_EDGE_FALL |
			    _0002_SAU_MODE_UART | _0000_SAU_TRANSFER_END;
			SCR01 = _4000_SAU_RECEPTION | _0000_SAU_INTSRE_MASK | uiParityValue | _0080_SAU_LSB | uiStopBitValue |
			    _0007_SAU_LENGTH_8;
			SDR01 = ucSDRValue;
			
			
			R_UART0_Start();
			R_UART0_Receive(&ucUartGPRSRecData,1);	//required for receiving data on uart 1
    			initGPRS();
			
		}
		break;
		
		case UART_RS485 :
		{
			R_UART3_Stop();

				
			SMR12 = _0020_SAU_SMRMN_INITIALVALUE | uiClockSource | _0000_SAU_TRIGGER_SOFTWARE |
			    _0002_SAU_MODE_UART | _0000_SAU_TRANSFER_END;
			SCR12 = _8000_SAU_TRANSMISSION | _0000_SAU_INTSRE_MASK | uiParityValue | _0080_SAU_LSB | uiStopBitValue |
			    _0007_SAU_LENGTH_8;
			SDR12 = ucSDRValue;
			
					
			SMR13 = _0020_SAU_SMRMN_INITIALVALUE | uiClockSource | _0100_SAU_TRIGGER_RXD | _0000_SAU_EDGE_FALL |
			    _0002_SAU_MODE_UART | _0000_SAU_TRANSFER_END;
			SCR13 = _4000_SAU_RECEPTION | _0000_SAU_INTSRE_MASK | uiParityValue | _0080_SAU_LSB | uiStopBitValue |
			    _0007_SAU_LENGTH_8;
			SDR13 = ucSDRValue;
			
			
			R_UART3_Start();
			R_UART3_Receive(&ucUartRS485RecData,1);	//required for receiving data on uart 1
    			initRS485();
			
		}
		break;
		
		case UART_ZIGBEE :
		{
			R_UART2_Stop();

				
			SMR10 = _0020_SAU_SMRMN_INITIALVALUE | uiClockSource | _0000_SAU_TRIGGER_SOFTWARE |
			    _0002_SAU_MODE_UART | _0000_SAU_TRANSFER_END;
			SCR10 = _8000_SAU_TRANSMISSION | _0000_SAU_INTSRE_MASK | uiParityValue | _0080_SAU_LSB | uiStopBitValue |
			    _0007_SAU_LENGTH_8;
			SDR10 = ucSDRValue;
			
					
			SMR11 = _0020_SAU_SMRMN_INITIALVALUE | uiClockSource | _0100_SAU_TRIGGER_RXD | _0000_SAU_EDGE_FALL |
			    _0002_SAU_MODE_UART | _0000_SAU_TRANSFER_END;
			SCR11 = _4000_SAU_RECEPTION | _0000_SAU_INTSRE_MASK | uiParityValue | _0080_SAU_LSB | uiStopBitValue |
			    _0007_SAU_LENGTH_8;
			SDR11 = ucSDRValue;
			
			
			R_UART2_Start();
			R_UART2_Receive(&ucUartZBRecData,1);	//required for receiving data on uart 1
    			//initRS485();
			
		}
		break;
	
		default:
		
		break;
		
	}

	
	return SUCCESS;
}


unsigned char UARTSendChar(unsigned char ucUARTModuleNo,unsigned char ucData)
{

    if(ucUARTModuleNo==UART_DEBUG)
    {
//	if(ucData=='+')
//		ucData='|';
//		
//	if(ucData=='a')	
//	    	ucData=224;
//	
//	if(ucData=='A')	
//	    	ucData=193;
		
        ucUartDebugSendFlag=0;
	R_UART1_Send(&ucData,1);
	while(ucUartDebugSendFlag==0);
	
    }
    else if(ucUARTModuleNo==UART_RS485)
    {
	//if(ucData==0x44)
	//ucData=0x44;	
	    
        ucUartRS485SendFlag=0;
	R_UART3_Send(&ucData,1);
	while(ucUartRS485SendFlag==0);
    }
    else if(ucUARTModuleNo==UART_ZIGBEE)
    {
        ucUartZBSendFlag=0;
	R_UART2_Send(&ucData,1);
	while(ucUartZBSendFlag==0);
	delayMs(10);
    }	
    else if(ucUARTModuleNo==UART_GPRS)
    {
        ucUartGPRSSendFlag=0;
	R_UART0_Send(&ucData,1);
	while(ucUartGPRSSendFlag==0);
    }	
    
    return SUCCESS;
}

unsigned char UARTSendStr(unsigned char ucUARTModuleNo,unsigned char const *ucStr)
{
	unsigned char ucTraData=0;
	ucTraData=*ucStr;
	while(*ucStr)
	{
		UARTSendChar(ucUARTModuleNo,*ucStr++);
	}
    return SUCCESS;
}

unsigned char UARTSendLong(unsigned char ucUARTModuleNo,unsigned long ulNumber,unsigned char ucNoOfDigits)
{
	unsigned char ucArray[11];
	unsigned char ucResult;
	ucResult = intToASCIIStr(ulNumber,ucNoOfDigits,11,ucArray);
	if(ucResult == FAILURE)
	{
		return FAILURE;
	}
	UARTSendStr(ucUARTModuleNo,ucArray);
	return SUCCESS;
}

unsigned char UARTSendFloat(unsigned char ucUARTModuleNo,float fData,unsigned char ucNoOfDigitsReal,unsigned char ucNoOfDigitFraction)
{
	unsigned char ucArray[15];
	floatToASCIIStr(fData,ucNoOfDigitsReal,ucNoOfDigitFraction,15,ucArray);
	UARTSendStr(ucUARTModuleNo,ucArray);
	return SUCCESS;
}


