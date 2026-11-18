#ifndef __EEPROM_H_
#define __EEPROM_H_

#include "r_cg_macrodriver.h"
#include "r_cg_cgc.h"
#include "r_cg_serial.h"
#include "delay.h"
#include "global.h"

#define EEPROM_ADD  0XA8	// eeprom address




UINT8 writeExtEEPROM( UINT8 ucDeviceAddress,UINT8 ucLocAddressH,UINT8 ucLocAddressL,UINT8 ucData);
UINT8 readExtEEPROM(UINT8 ucDeviceAddress,UINT8 ucLocAddressH,UINT8 ucLocAddressL,UINT8* ucReadData);

UINT8 writeExtEEPROMChar(UINT8 ucDeviceAddress,UINT16 uiLocAddress,UINT8 ucData);
UINT8 readExtEEPROMChar(UINT8 ucDeviceAddress,UINT16 uiLocAddress,UINT8* ucData);

UINT8 writeExtEEPROMInt(UINT8 ucDeviceAddress,UINT16 uiLocAddress,UINT16 ucData);
UINT8 readExtEEPROMInt(UINT8 ucDeviceAddress,UINT16 uiLocAddress,UINT16* ucData);

UINT8 writeExtEEPROMLong(UINT8 ucDeviceAddress,UINT16 uiLocAddress,UINT32 ucData);
UINT8 readExtEEPROMLong(UINT8 ucDeviceAddress,UINT16 uiLocAddress,UINT32* ucData);

UINT8 readExtEEPROMFloat(UINT8 ucDeviceAddress, UINT16 uiLocAddress, float *fData);
UINT8 writeExtEEPROMFloat(UINT8 ucDeviceAddress,UINT16 uiLocAddress,float fData);

void readFromEEPROM(void);



#endif