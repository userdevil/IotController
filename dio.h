#ifndef __LED_H_
#define __LED_H_

	#include "r_cg_macrodriver.h"
	#include "r_cg_cgc.h"
	#include "r_cg_port.h"

	#include "global.h"
		
	#define DB_LED_ON 	1
	#define DB_LED_OFF	0
	
	#define RESET_BTN_PRESSED	0
	#define RESET_BTN_NOT_PRESSED	1	//1003
	
	#define LED_PWR		1
	#define LED_GPRS	2
	#define LED_MSG		3
	#define LED_ACQ		4
	#define DB_LED_PWR	P14.7	
	#define DB_LED_GPRS	P14.6	
	#define DB_LED_MSG	P11.1	
	#define DB_LED_ACQ	P11.0	
	//1007 REMAPPING LED PINTO DUMMY PIN
//	#define DB_LED_PWR	P15.4
//	#define DB_LED_GPRS	P15.4
//	#define DB_LED_MSG	P15.4	
//	#define DB_LED_ACQ	P15.4	
	
	
	#define RESET_BTN	P10.0

	#define KP_SW_1		P8.2
	#define KP_SW_2		P8.3
	#define KP_SW_3		P8.4
	#define KP_SW_4		P8.5
	#define KP_SW_5		P8.6
	
	#define KP_KEY_UP	3
	#define KP_KEY_DOWN	4
	#define KP_KEY_LEFT	1
	#define KP_KEY_RIGHT	5
	#define KP_KEY_ENTER	2
	#define KP_KEY_MENU	9
	
	#define RELAY_1		P10.2
	#define RELAY_2		P0.4
	#define RELAY_3		P13.0
	#define REALY_ON	1
	#define REALY_OFF	0
	
	#define ZB_EXTRA	P7.6
	#define ZB_EXTRA_1	P7.7
	
	//1007 LED PINS USED FOR RELAY CONTTROL
	
	#define EXT_PIN_8	P14.7
	#define EXT_PIN_7	P14.6	
	#define EXT_PIN_6	P11.1	
	#define EXT_PIN_5	P11.0	
	
	#define EXT_PIN_4	P6.4
	#define EXT_PIN_3	P3.1
	#define EXT_PIN_2	P6.2
	#define EXT_PIN_1	P6.3
	
	#define EXT_PIN_ON	1
	#define EXT_PIN_OFF	0
	
	#define SPI_CLK 	P7.0
	#define SPI_SI 		P7.1
	#define SPI_SO 		P7.2
		
	extern UINT32 ulFreqPulsesPerSecond;
	
	extern UINT32 ulPreviousIntCntr;
	extern UINT32 ulIntCntr;
	
	extern UINT32 ulIntTimeMs;
	extern UINT32 ulIntTimeMsPre;
	
	extern UINT8 ucFlagRstButtonPressed;
	extern UINT8 ucKeyTimer;
	
	extern F32 fFreq;
	
	extern UINT8 uckeyPreTestUP;
	extern UINT8 uckeyPreTestDOWN;
	extern UINT8 uckeyPreTestRIGHT;
	extern UINT8 uckeyPreTestLEFT;
	extern UINT8 uckeyPreTestENTER;
	
	//1003
	extern UINT16 uiResetKeyTimer;
	extern UINT8 ucResetKeyCounter;
	
	
	//TURN OFF ALL THE RELAY
	UINT8 dio_initRelay();
	
	//TURN OFF ALL THE LED
	UINT8 dio_initLED(void);
	
	//TURN ON AND OFF THE LED,
	// ucLEDN0 - led number, refer dio.h
	// ucDelay - ON and OFF delay
	UINT8 dio_blink(UINT8 ucLEDNo,UINT16 ucDelay);
	
	// RETURN SUCCESS IF RESET EEPROM BUTTON PRESSED
	UINT8 dio_checkResetBtnPressed(void);

	// RETURN THE KEY PRESSED ON KEYPAD
	UINT8 dio_checkKeyPressed(void);
	
	
	void processInterruptData(void);
	
#endif