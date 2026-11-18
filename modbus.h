#ifndef __MODBUS_H_
#define __MODBUS_H_
#include "global.h"

#define RS485_RESP_STAT_NOP		0
#define RS485_RESP_STAT_OK		1
#define RS485_RESP_STAT_TIMEOUT		2
#define RS485_RESP_STAT_ERR		3
#define RS485_RESP_STAT_ERR_CRC		4
#define RS485_RESP_STAT_ERR_EXCP	5
#define RS485_RESP_LINE_BUYS		6


#define RS485_MODBUS_RTU 0
#define RS485_MODBUS_ASCII 1

#define RS485_DATATYPE_UINT8 0
#define RS485_DATATYPE_UINT16 1
#define RS485_DATATYPE_INT16 2
#define RS485_DATATYPE_UINT32 3
#define RS485_DATATYPE_INT32 4
#define RS485_DATATYPE_F32 5
#define RS485_DATATYPE_COIL_RESP 10


extern unsigned char ucMobus1WaitForStartPacket;// 00 -> start packet received, xx - > will hold the modbus address, rec func will wait for this char and set rec counter to zero, only once after request is sent
extern unsigned int uiMODBUSTimerReg;
extern UINT8 ucMODBUSMasterOperationInProgress;


extern UINT8 ucDebugIntTransmitBuffer[50];
extern UINT8 ucDebugIntBufferDataLength;
extern UINT8 ucDebugIntBufferTransCount;


UINT8 checkCRC16(UINT8 *pPacket, UINT8 ucNoOfBytes);
void insertCRC16(UINT8 *pPacket, UINT8 ucNoOfBytes);
UINT8 ucCheckLCR(UINT8 *pucPacket, UINT8 ucNoOfBytes);
UINT8 displayModusRecBuff(void);
UINT8 displayModusRecBuffASCII(void);
UINT8 clearModusRecBuff(void);
UINT8 processRS485Packet(UINT8* pucPCF, UINT8 ucMeterType, UINT8 ucMode, UINT8 ucDataType, UINT8 ucBaudF, UINT8 ucDataBitsF, UINT8 ucParity, UINT8 ucStopBits,UINT16 uiTimeout, UINT8* pucPacket,UINT32 ulPacketLength, UINT8* pucRespStatus, F32* fVal, UINT8 ucNoOfRetries);
UINT8 processModbusMasterRequest();
UINT8 transmitBufferInInterrupts(UINT8* pucData,UINT8 ucDatalength);

#endif
