#ifndef __DEVICE_UI__
#define __DEVICE_UI__

#include "global.h"

#define SCREEN_NONE 0
	#define SCREEN_IDLE 1
	#define SCREEN_PARA 2
	#define SCREEN_DEBUG 3

extern unsigned char ucMenuTimerMin;	
extern unsigned char ucDispBackFlag;


void displayScreen(void);
UINT8 displayNetworkStatus(UINT8 ucX, UINT8 ucY, UINT8 ucPixel);
UINT8 ucDisplayParaValue(UINT8 ucPara, UINT8 ucRow);
	
#endif
