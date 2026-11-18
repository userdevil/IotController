#ifndef __RTC_H_
#define __RTC_H_

#include "global.h"

struct RTC
{
	unsigned char ucDate;
	unsigned char ucMonth;
	unsigned char ucYear;
	unsigned char ucDay;
	unsigned char ucHours;
	unsigned char ucMinutes;
	unsigned char ucSeconds;
	unsigned char ucPreSeconds;
	unsigned char ucCounts;
	unsigned char ucOffset;
};

extern struct RTC sRTC;

extern UINT8 ucRTCReadFlag;

void readFromRTC(void);
void writeToProcessorRTC(void);
UINT8 BCDToInteger(UINT8 ucNumber);
UINT8 intergerToBCD(UINT8 ucNumber);
void sendRtcViaDebug(void);
void rtcInterrupt(void);//1015
void incrementRTCDate();

#endif