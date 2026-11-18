#ifndef __DEVICE_DATA__
#define __DEVICE_DATA__

#include "global.h"

#define RS485_CON_DATA_ACQ 	0


extern UINT8 ucCSVWriteHeader;
extern UINT8 ucCSVSendData;

void acquireData(UINT8* pucPCF);
void storeParameterToFlash(void);
void storeModbusDataToFlash(UINT32 ulParamNumber);

void sendStoredDataToserver(void);
UINT8 checkParaForErrorAndSendAlert(UINT8 ulParaCntrF);
UINT8 convertVoltageToResistance(F32 fSupplyVolts, F32 fSensorVolts, F32 fpullUpresistor, F32* pfResistance);
UINT8 convert10KTheristerResitanceToTemp(F32 fResistance, F32* fTemperaure);
void sendCSVData(void);



extern UINT16 uiParaAcqSeconds;
extern UINT8 ucAcqDataDisable;
extern UINT16 uiFlashStorageSize;
extern UINT16 uiFlashMaxReadCntr;
extern UINT16 uiFlashMaxWriteCntr;

extern UINT8 ucRS485BusFlag;

extern UINT16 ucTimerNonComm;

extern UINT8 ucCumilativeResetFlag;

extern F32 fInverterPower;

extern UINT8 ucParaBeingAcquired;

#endif
