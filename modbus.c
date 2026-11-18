#include "modbus.h"
#include "r_cg_macrodriver.h"
#include "r_cg_cgc.h"
#include "r_cg_serial.h"
#include "global.h"
#include "uart.h"
#include "uartDebug.h"
#include "generalfunctions.h"
#include "uartRS485.h"
#include "memory.h"

#include "memory.h"
#include "controller.h"


unsigned int uiMODBUSTimerReg;
UINT8 ucMODBUSMasterOperationInProgress;

unsigned char ucMobus1WaitForStartPacket=0;

unsigned char ucMODBUSIdleCounter=0;


UINT8 ucDebugIntTransmitBuffer[50];
UINT8 ucDebugIntBufferDataLength;
UINT8 ucDebugIntBufferTransCount;




UINT8 checkCRC16(UINT8 *pPacket, UINT8 ucNoOfBytes) 
{
    UINT16 CheckSum;
    UINT8 ucIndex, ucCntr;
    UINT8 lowCRC;
    UINT8 highCRC;

    CheckSum = 0xffff;
    for (ucIndex = 0; ucIndex < ucNoOfBytes; ucIndex++)
    {
        CheckSum = CheckSum^(UINT16) pPacket[ucIndex];
        for (ucCntr = 8; ucCntr > 0; ucCntr--)
            if ((CheckSum)&0x0001)
                CheckSum = (CheckSum >> 1)^0xa001;
            else
                CheckSum >>= 1;
    }
    highCRC = (UINT8)(CheckSum >> 8);
    CheckSum <<= 8;
    lowCRC = (UINT8)(CheckSum >> 8);

    if ((pPacket[ucNoOfBytes + 1] == highCRC) && (pPacket[ucNoOfBytes] == lowCRC))
        return 0; // sucess
    else
        return 1; // failure
}

void insertCRC16(UINT8 *pPacket, UINT8 ucNoOfBytes) 
{
    UINT16 CheckSum;
    UINT8 ucIndex, ucCntr;
    UINT8 lowCRC;
    UINT8 highCRC;

    CheckSum = 0xffff;
    for (ucIndex = 0; ucIndex < ucNoOfBytes; ucIndex++)
    {
        CheckSum = CheckSum^(UINT16) pPacket[ucIndex];
        for (ucCntr = 8; ucCntr > 0; ucCntr--)
            if ((CheckSum)&0x0001)
                CheckSum = (CheckSum >> 1)^0xa001;
            else
                CheckSum >>= 1;
    }
    highCRC = (UINT8)(CheckSum >> 8);
    CheckSum <<= 8;
    lowCRC = (UINT8)(CheckSum >> 8);
    pPacket[ucNoOfBytes+1]=highCRC;
    pPacket[ucNoOfBytes]=lowCRC;
    
}

UINT8 ucCheckLCR(UINT8 *pucPacket, UINT8 ucNoOfBytes)
{
	UINT8 ucLCR,ucCntr,ucTempF;
	ucLCR=0;
	pucPacket++;
	for(ucCntr=0;ucCntr<(ucNoOfBytes/2);ucCntr++)
	{

		ASCIIHexToNumber(pucPacket,1);
		ucTempF=uConvert.ucArray[0];
		ucLCR=ucLCR+ucTempF;
	//	UARTSendLong(UART_DEBUG,ucTempF,4);
	//	UARTSendStr(UART_DEBUG,(UINT8*)" ");
		pucPacket++;
		pucPacket++;
	}
// UARTSendStr(UART_DEBUG,(UINT8*) "\rLCR :");
//	UARTSendLong(UART_DEBUG,ucLCR,4);
	if(ucLCR!=0)
		return FAILURE;
	return SUCCESS;
}

UINT8 displayModusRecBuff(void) {
    UINT8 ucCntrF;

    if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
    {
	    UARTSendStr(UART_DEBUG, (UINT8*) "\r MODBUS BUFF, length ");
	    UARTSendLong(UART_DEBUG, uiRS485RecvDataCount, 3);
	    UARTSendStr(UART_DEBUG, (UINT8*) ", Rec Data : ");
	    for (ucCntrF = 0; ucCntrF < uiRS485RecvDataCount; ucCntrF++) 
	    {
	        UINT8 ucTempHexBuff[5];
	        intHexToHexBytes(ucRS485RecvBuffer[ucCntrF], ucTempHexBuff);
	        UARTSendChar(UART_DEBUG, ucTempHexBuff[0]);
	        UARTSendChar(UART_DEBUG, ucTempHexBuff[1]);
	        UARTSendStr(UART_DEBUG, (UINT8*) " ");
	    }
    }

    return SUCCESS;
}

UINT8 displayModusRecBuffASCII(void) {
    UINT8 ucCntrF;

    if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
    {
	    UARTSendStr(UART_DEBUG, (UINT8*) "\r ASCII MODBUS BUFF, length ");
	    UARTSendLong(UART_DEBUG, uiRS485RecvDataCount, 3);
	    UARTSendStr(UART_DEBUG, (UINT8*) ", Rec Data : ");
	    for (ucCntrF = 0; ucCntrF < uiRS485RecvDataCount; ucCntrF++) {
	        UARTSendChar(UART_DEBUG, ucRS485RecvBuffer[ucCntrF]);
	    }
    }
    return SUCCESS;
}


UINT8 clearModusRecBuff(void) {
    UINT8 ucCntrF;
    //	UARTSendStr(UART_DEBUG,(UINT8*)"\r Rec Data :::");
    for (ucCntrF = 0; ucCntrF < RS485_REC_BUFFER_SIZE; ucCntrF++) {
        ucRS485RecvBuffer[ucCntrF] = 0x00;
    }
    uiRS485RecvDataCount = 0;
    return SUCCESS;
}


UINT8 processRS485Packet(UINT8* pucPCF, UINT8 ucMeterType, UINT8 ucMode, UINT8 ucDataType, UINT8 ucBaudF, UINT8 ucDataBitsF, UINT8 ucParity, UINT8 ucStopBits,UINT16 uiTimeout, UINT8* pucPacket,UINT32 ulPacketLength, UINT8* pucRespStatus, F32* fVal, UINT8 ucNoOfRetries) {
	#define STATE_CHECK_IDLE_BUS		0
	#define STATE_WAIT_FOR_IDLE		1
	#define STATE_WAIT_ZIGBEE_PACKET	2
	#define STATE_PREPARE_PACKET 		3
	#define STATE_GET_RESPONSE 		4
	#define STATE_PROCESS_REC_PACKET 	5
		
	
    UINT8 ucCntrF = 0;
    UINT8 ucTempF = 0;
    static UINT8 ucRS485BaudF;
    static UINT8 ucRS485Dataype, ucRS485Databits, ucRS485Parity, ucRS485stopbits, ucRS485Packet[15];
    static UINT8 ucStateF = STATE_CHECK_IDLE_BUS;
    static F32 fFinalValue;
    static UINT8 ucFinalStatusFlag = RS485_RESP_STAT_NOP;
    static UINT8 ucRespLength;
    static UINT8 ucNoOfAttempts = 0;
    UINT32 ulTemp=0;
    UINT8 ucTraPktTemp[30];
     
    if (*pucPCF == 0) {
        ucStateF = STATE_CHECK_IDLE_BUS;
        ucNoOfAttempts = 0;
	uiMODBUSTimerReg=0;
        return SUCCESS;
    }

    switch (ucStateF) {
	    
        case STATE_CHECK_IDLE_BUS:
        {
            uiRS485RecvDataCount = 2;
            ucStateF = STATE_WAIT_FOR_IDLE;
            uiMODBUSTimerReg = 0;
        }
        break;

        case STATE_WAIT_FOR_IDLE:
        {
	    
            if (uiMODBUSTimerReg < 100)
                break;
            if (uiRS485RecvDataCount != 2)
	    {
                
		ucMODBUSIdleCounter++;
		if(ucMODBUSIdleCounter<20)
			ucStateF = STATE_CHECK_IDLE_BUS;
		else
		{
			ucStateF = STATE_PREPARE_PACKET;
			
		}
	    }
            else
	    {
		uiMODBUSTimerReg=0;
		if(uiTimeout>4000)
		{
			if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
				UARTSendStr(UART_DEBUG,(UINT8*)"\r wait zigbee packet:");	
		  ucStateF = STATE_WAIT_ZIGBEE_PACKET;
		}
		else
                  ucStateF = STATE_PREPARE_PACKET;
	    }
        }
            break;
	    //1002
	case STATE_WAIT_ZIGBEE_PACKET:
	{
		if (uiMODBUSTimerReg < uiTimeout)
                	break;
		ucStateF = STATE_PREPARE_PACKET;
	}
	break;
	
        case STATE_PREPARE_PACKET:
        {
		
		ucMODBUSIdleCounter=0;	
		
            ucRS485Dataype = ucDataType;

	    
	    for (ucCntrF = 0; ucCntrF < ulPacketLength; ucCntrF++) 
	    {
                    ucTraPktTemp[(ucCntrF*2)+0]=*pucPacket;
                    ucRS485Packet[ucCntrF] = hexToDecimal(*pucPacket++) << 4;
                    ucTraPktTemp[(ucCntrF*2)+1]=*pucPacket;
		    ucRS485Packet[ucCntrF] = ucRS485Packet[ucCntrF] | (hexToDecimal(*pucPacket++));
            }
	    
	    ucTraPktTemp[(ucCntrF*2)+0]=0x00;	
	     ucTraPktTemp[(ucCntrF*2)+1]=0x00;
	    
            if (ucMode == RS485_MODBUS_RTU) 
	    {
                if (ucRS485Dataype == RS485_DATATYPE_UINT8)
                    ucRespLength = 6;
                else if (ucRS485Dataype == RS485_DATATYPE_UINT16)
                    ucRespLength = 7;
                else if (ucRS485Dataype == RS485_DATATYPE_INT16)
                    ucRespLength = 7;
		else if (ucRS485Dataype == RS485_DATATYPE_COIL_RESP)
                    ucRespLength = 8;    
                else
                    ucRespLength = 9;
            } 
	    else 
	    {
                ucRespLength = (ucRS485Packet[5]*2)+5;
            }   
	    
            if (ucBaudF == UART_BAUD_2400) ucRS485BaudF = UART_BAUD_2400;
            else if (ucBaudF == UART_BAUD_4800) ucRS485BaudF = UART_BAUD_4800;
            else if (ucBaudF == UART_BAUD_9600) ucRS485BaudF = UART_BAUD_9600;
            else if (ucBaudF == UART_BAUD_19200) ucRS485BaudF = UART_BAUD_19200;
            else if (ucBaudF == UART_BAUD_38400) ucRS485BaudF = UART_BAUD_38400;
            else if (ucBaudF == UART_BAUD_57600) ucRS485BaudF = UART_BAUD_57600;
            else if (ucBaudF == UART_BAUD_115200) ucRS485BaudF = UART_BAUD_115200;
 
            if (ucDataBitsF == 0) ucRS485Databits = UART_DATABIT_EIGHT;
            else if (ucDataBitsF == 1) ucRS485Databits = UART_DATABIT_SEVEN;
            else if (ucDataBitsF == 2) ucRS485Databits = UART_DATABIT_NINE;
           
            ucRS485Parity = ucParity;
         
            if (ucStopBits == 0) ucRS485stopbits = UART_STOPBIT_ONE;
            else if (ucStopBits == 1) ucRS485stopbits = UART_STOPBIT_NO;
            else if (ucStopBits == 2) ucRS485stopbits = UART_STOPBIT_TWO;
          

		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
		{	
		    UARTSendStr(UART_DEBUG, (UINT8*) "\rTra MODBUS, Type =");
		    UARTSendLong(UART_DEBUG, ucMeterType, 1);
		    UARTSendStr(UART_DEBUG, (UINT8*) ", mode =");
		    UARTSendLong(UART_DEBUG, ucMode, 1);
		    UARTSendStr(UART_DEBUG, (UINT8*) ", DT =");
		    UARTSendLong(UART_DEBUG, ucRS485Dataype, 1);
		    UARTSendStr(UART_DEBUG, (UINT8*) ", RL =");
		    UARTSendLong(UART_DEBUG, ucRespLength, 2);
		    UARTSendStr(UART_DEBUG, (UINT8*) ", Baud =");
		    UARTSendLong(UART_DEBUG, ucRS485BaudF, 2);
		    UARTSendStr(UART_DEBUG, (UINT8*) ", Data bits =");
		    UARTSendLong(UART_DEBUG, ucRS485Databits, 1);
		    UARTSendStr(UART_DEBUG, (UINT8*) ", parity =");
		    UARTSendLong(UART_DEBUG, ucRS485Parity, 1);
		    UARTSendStr(UART_DEBUG, (UINT8*) ", stopbits =");
		    UARTSendLong(UART_DEBUG, ucRS485stopbits, 1);
		    UARTSendStr(UART_DEBUG, (UINT8*) ", Packet =");
		    UARTSendStr(UART_DEBUG,ucTraPktTemp);
		}
	     		
			
	     ucUartInit(UART_RS485,ucRS485BaudF,ucDataBitsF,ucRS485Parity,ucRS485stopbits);

		
          //  getStrLength(pucPacket,&ulTemp);
	  //  ulPacketLength = ulTemp/2;
	    if( (ulPacketLength<2) || (ulPacketLength>15) )
	    	ulPacketLength=8;
              
                
		clearModusRecBuff();
		uiRS485RecvDataCount = 0;
		ucUartInit(UART_RS485,ucRS485BaudF,ucDataBitsF,ucRS485Parity,ucRS485stopbits);;//RS485Init(ulRS485BaudF, ucRS485Databits, ucRS485Parity, ucRS485stopbits, 1, 1);
		for (ucCntrF = 0; ucCntrF < ulPacketLength; ucCntrF++) 
		{
		    RS485SendChar(ucRS485Packet[ucCntrF]);
		}

		ucMobus1WaitForStartPacket=ucRS485Packet[0];//V30_2
		
           

            clearModusRecBuff();
            uiRS485RecvDataCount = 0;
            uiMODBUSTimerReg = 0;
            ucStateF = STATE_GET_RESPONSE;
            fFinalValue = 0;
            ucFinalStatusFlag = 0;
        }
            break;

        case STATE_GET_RESPONSE:
        {
            //if (uiRS485RecvDataCount == ucRespLength)
	    //1014
	    if (uiRS485RecvDataCount >= ucRespLength)
	    {
              //  ClrWdt();
            } 
	      else if (uiMODBUSTimerReg < uiTimeout) {
                return FAILURE;
            }

            if (uiMODBUSTimerReg >= uiTimeout) 
	    {
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
		{
		
			
	                    displayModusRecBuff();
	          
		}
                if (uiRS485RecvDataCount == 0)
                    ucFinalStatusFlag = RS485_RESP_STAT_TIMEOUT;
                else if (uiRS485RecvDataCount < ucRespLength)
                    ucFinalStatusFlag = RS485_RESP_STAT_ERR_EXCP;
                else
                    ucFinalStatusFlag = RS485_RESP_STAT_ERR;

                ucStateF = STATE_PROCESS_REC_PACKET;

                return FAILURE;
            } else {
                UINT8 ucCRCStatus;
                

                if (ucMode == RS485_MODBUS_RTU) 
		{

			if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
				displayModusRecBuff();          

		    if(uiRS485RecvDataCount>ucRespLength)
		    {
		  	if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			{
				UARTSendStr(UART_DEBUG, (UINT8*) "\r RS485 Resp err, length mismatch");
				
			}
			uiRS485RecvDataCount=ucRespLength;
		    }

                    ucCRCStatus = checkCRC16(ucRS485RecvBuffer, (UINT8)(uiRS485RecvDataCount - 2));
                    if (ucCRCStatus == FAILURE) 
		    {
			if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
			{
	                        displayModusRecBuff();
	                        UARTSendStr(UART_DEBUG, (UINT8*) "\r RS485 Resp Received, ");
	                        UARTSendStr(UART_DEBUG, (UINT8*) ", CRC error ");
			}
                        ucFinalStatusFlag = RS485_RESP_STAT_ERR_CRC;
                        //	ucStateF=STATE_CHECK_IDLE_BUS;
                        //	* pucPCF=0;
                        //	return FAILURE;
                    } 
			else 
		    {
                     //   UARTSendStr(UART_DEBUG, (UINT8*) ", CRC ok ");
                        ucFinalStatusFlag = RS485_RESP_STAT_OK;
                    }
		    
		    
		    fFinalValue = getDataFromModbusPAcket(ucRS485Dataype,ucMeterType, &ucRS485RecvBuffer[3]);
		    
		    
		
		    
                    
                }
		else 
		{
                    if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
				displayModusRecBuff(); 
				
				
                    ucCRCStatus = checkCRC16(ucRS485RecvBuffer, (UINT8)(uiRS485RecvDataCount - 2));
                    if (ucCRCStatus == FAILURE) 
		    {
			 ucFinalStatusFlag = RS485_RESP_STAT_ERR_CRC;  
			 fFinalValue = 0;
                       
                    } else 
		    {
                   
                        ucFinalStatusFlag = RS485_RESP_STAT_OK;
                        fFinalValue = ucRespLength;
                    }
                }
            }
            ucStateF = STATE_PROCESS_REC_PACKET;
        }
            break;

        case STATE_PROCESS_REC_PACKET:
        {

            if (ucFinalStatusFlag != 1) 
	    {
		    
		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
		{
	                UARTSendStr(UART_DEBUG, (UINT8*) "\r RS485 No Of attempts ");
	                UARTSendLong(UART_DEBUG, ucNoOfAttempts, 2);
		}
                ucNoOfAttempts++;
                if (ucNoOfAttempts > ucNoOfRetries) 
		{
                    ucNoOfAttempts = 0;
                    *fVal = fFinalValue;
                    *pucRespStatus = ucFinalStatusFlag;
                    ucStateF = STATE_CHECK_IDLE_BUS;
                    * pucPCF = 0;
		    if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
		    {
	                    UARTSendStr(UART_DEBUG, (UINT8*) "\r RS485 response error, Value = ");
	                    UARTSendFloat(UART_DEBUG, fFinalValue, 5, 5);
	                    UARTSendStr(UART_DEBUG, (UINT8*) ",  Status = ");
	                    UARTSendLong(UART_DEBUG, ucFinalStatusFlag, 2);
		    }
                } else {
                    ucStateF = STATE_CHECK_IDLE_BUS;
                }
            } 
	    else 
	    {
                *fVal = fFinalValue;
                *pucRespStatus = ucFinalStatusFlag;
                ucStateF = STATE_CHECK_IDLE_BUS;
                * pucPCF = 0;

		if( (ucDebugEnableFlag==DEBUG_ENABLE_ALL) || (ucDebugRS485==1) )
		{
                UARTSendStr(UART_DEBUG, (UINT8*) "\r RS485 respose, Value = ");
                UARTSendFloat(UART_DEBUG, fFinalValue, 5, 5);
                UARTSendStr(UART_DEBUG, (UINT8*) ",  Status = ");
                UARTSendLong(UART_DEBUG, ucFinalStatusFlag, 2);

		}
            }



        }
            break;

        default:
            ucStateF = STATE_CHECK_IDLE_BUS;
            break;
    }


    return SUCCESS;
}


UINT8 processModbusMasterRequest()
{
	UINT8 ucCRCStatus;
	UINT8 ucCntrF;
	UINT8 ucCntrSend;
	UINT8 ucRegisterNo = 0x00;
	UINT8 ucRequestLength = 0x00;
	UINT8 ucResponsePaket[100];
	UINT8 ucReqLenCntr=0;
	
	ucMODBUSMasterOperationInProgress=1;
	
	if(ucInitComplete==0)
		return FAILURE;
	
	if(ucDebugRecvBuffer[1]!=0x03)
	{
		ucMODBUSMasterOperationInProgress=0;
		return FAILURE;
	}
	ucCRCStatus = checkCRC16(ucDebugRecvBuffer, 6);
	if (ucCRCStatus == SUCCESS) 
	{
	    uConvert.ucArray[1]=ucDebugRecvBuffer[2];
	    uConvert.ucArray[0]=ucDebugRecvBuffer[3];
	    ucRegisterNo=(UINT8)uConvert.uiIntVar;
	    ucRegisterNo=ucRegisterNo/2;
	    
	     uConvert.ucArray[1]=ucDebugRecvBuffer[4];
	    uConvert.ucArray[0]=ucDebugRecvBuffer[5];
	    ucRequestLength = (UINT8)uConvert.uiIntVar/2;
	    if(ucRegisterNo>=ulSystemNoOfParameters)
	    {
		    ucResponsePaket[0]=ucMasterModbusID;
		    ucResponsePaket[1]=0x83;
		    ucResponsePaket[2]=0x02;
		    insertCRC16(ucResponsePaket, 3);
		    transmitBufferInInterrupts(ucResponsePaket,5);
		   
	    }
	    else if(ucRequestLength>25)
	    {
		    ucResponsePaket[0]=ucMasterModbusID;
		    ucResponsePaket[1]=0x83;
		    ucResponsePaket[2]=0x03;
		    insertCRC16(ucResponsePaket, 3);
		    insertCRC16(ucResponsePaket, 3);
		    transmitBufferInInterrupts(ucResponsePaket,5);
		   
	    }
	   // else if(sParaConfig[ucRegisterNo].para.ucValueStatusBit==2)
	   // {
	//	    
	  //  }
	    else
	    {
		    ucResponsePaket[0]=ucMasterModbusID;
		    ucResponsePaket[1]=ucDebugRecvBuffer[1];
		    ucResponsePaket[2]=ucRequestLength*2;
		    for(ucReqLenCntr=0;ucReqLenCntr<ucRequestLength;ucReqLenCntr++)
		    {
			    ucCntrF=(ucReqLenCntr*4);
			    uConvert.fFloatVar=sParaConfig[ucRegisterNo+ucReqLenCntr].para.value.fValue;
			    ucResponsePaket[ucCntrF+3]=uConvert.ucArray[1];
			    ucResponsePaket[ucCntrF+4]=uConvert.ucArray[0];
			    ucResponsePaket[ucCntrF+5]=uConvert.ucArray[3];
			    ucResponsePaket[ucCntrF+6]=uConvert.ucArray[2];
		    }
		    insertCRC16(ucResponsePaket, ucCntrF+7);
		       
		    //1040
		    //for(ucCntrSend=0;ucCntrSend<ucCntrF+9;ucCntrSend++)
		    //	UARTSendChar(UART_DEBUG, ucResponsePaket[ucCntrSend]);
		    transmitBufferInInterrupts(ucResponsePaket,ucCntrF+9);
	    }
	}
	ucMODBUSMasterOperationInProgress=0;
	return SUCCESS;
}





UINT8 transmitBufferInInterrupts(UINT8* pucData,UINT8 ucDatalength)
{
	R_UART1_Send(pucData,ucDatalength);
	return SUCCESS;
}

