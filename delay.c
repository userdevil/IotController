#include "delay.h"
#include "r_cg_wdt.h"
#include "global.h"
#include "r_cg_timer.h"

unsigned int uiMSDelayCntr;
unsigned int uiUSDelayCntr;
unsigned int uiGPMsCntr;

void delayMs(unsigned int uiDelay)
{
	uiMSDelayCntr=uiDelay;

	while(uiMSDelayCntr)
	{
		R_WDT_Restart();

	}
}

void delay10Us(unsigned int uiDelay)
{
	R_TAU0_Channel1_Start();
	uiUSDelayCntr=uiDelay;
	while(uiUSDelayCntr)
	{
		R_WDT_Restart();

	}
	R_TAU0_Channel1_Stop();
}

