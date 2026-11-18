#ifndef _DELAY_H
#define _DELAY_H


#include "r_cg_macrodriver.h"
#include "r_cg_cgc.h"
#include "r_cg_port.h"
#include "r_cg_wdt.h"

	extern unsigned int uiMSDelayCntr;
	extern unsigned int uiUSDelayCntr;
	extern unsigned int uiGPMsCntr;
	
	void delayMs(unsigned int uiDelay);
	void delay10Us(unsigned int uiDelay);

	
	
#endif