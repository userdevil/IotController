#ifndef __DISPLAY_H_
#define __DISPLAY_H_


#include "r_cg_macrodriver.h"
	#include "r_cg_cgc.h"
	#include "r_cg_port.h"

	#include "global.h"

#define  LCD_EN     P5.2
#define  LCD_RW     P5.1
#define  LCD_RS     P5.0		//data or command 1-data, 0 command
#define  LCD_RST    P3.0
#define  LCD_CS     P8.7

#define  LCD_D0     P5.3
#define  LCD_D1     P5.4
#define  LCD_D2     P5.5
#define  LCD_D3     P5.6
#define  LCD_D4     P5.7
#define  LCD_D5     P1.7
#define  LCD_D6     P1.6
#define  LCD_D7     P1.5

#define  LCD_BL     P15.6		

#define COL1      1
#define PAGE1     0xB0	//Page address is specify in data sheet i.e B, 64/8 = 8 pages is availabale 
#define PAGE2     0xB1
#define PAGE3     0xB2
#define PAGE4     0xB3
#define PAGE5     0xB4
#define PAGE6     0xB5
#define PAGE7     0xB6
#define PAGE8     0xB7

#define GLCD_DEBUG_LINE 6	/**< line 6 of the GLCD will be used as debug line*/
#define GLCD_SIDE_LEFT 0	/**< used to select left side chip of glcd */
#define GLCD_SIDE_RIGHT 1   /**< Used to select right side chip of glcd*/
#define GLCD_WIDTH   128	/**< glcd has 128 horizontal pixels*/
#define GLCD_MODE_ON 1		/**< GLCD mode on*/
#define GLCD_MODE_OFF 0		/**< GLCD mode off*/


#define GLCD_FILL_YES 1  	/**< GLCD fill screen yes ->*/
#define GLCD_FILL_NO 0		/**< GLCD fill screen no-> */

#define GLCD_PIXEL_ON 1		/**< 1-> turn ON  the pixel */
#define GLCD_PIXEL_OFF 0	/**< 0-> turn OFF the pixel*/
#define ON	1				/**< ON -> 1*/
#define OFF	0				/**< OFF-> 2*/

 extern unsigned char PAGE;
 extern unsigned char COL;
 

 extern unsigned char glcdBuffer[1024];
 extern unsigned char ucTempPixelAddress;
 
 void DispImage(unsigned char *Adata);
void displaySendData(unsigned char ucData);
void lcdInit(void);
void lcdWriteCmd(unsigned char cmd);
void lcdWriteData(unsigned char data);
void lcdSetPage(unsigned char page,unsigned char col);
void lcdClear(void);
void lcdFill(void);


void  displayWriteString(unsigned char lineNumber, unsigned char ColNo, unsigned char *pucMessage);
void lcdPrintFont(unsigned char data);
void LCD_PAGE_COL_CHECK_FONT1(void);


void glcdUpdate(void);
void glcdFillScreen(unsigned char ucPixelStatus);
unsigned char glcdPixel(unsigned char ucX,unsigned char ucY,unsigned char ucPixelStatus);
unsigned char glcdDrawLine(unsigned char ucX1,unsigned char ucY1,unsigned char ucX2,unsigned char ucY2,unsigned char ucPixelStatus);
unsigned char glcdDrawRectangle(unsigned char ucX1,unsigned char ucY1,unsigned char ucX2,unsigned char ucY2,unsigned char ucFill,unsigned char ucPixelStatus);
unsigned char glcdDrawCircle(unsigned char ucX,unsigned char ucY,unsigned char ucRadius,unsigned char ucFill,unsigned char ucPixelStatus);
unsigned char glcdWriteChar(unsigned char ucX, unsigned char ucY, unsigned char ucChar,unsigned char ucSize,unsigned char ucPixelStatus);
unsigned char glcdWriteCharCallibri(unsigned char ucX, unsigned char ucY, unsigned char ucChar,unsigned ucFontHeight,unsigned char ucPixelStatus);
unsigned char glcdWriteStrCallibri(unsigned char ucX, unsigned char ucY, unsigned char* pucStr,unsigned char ucFontHeight,unsigned char ucPixelStatus);
unsigned char glcdWriteStr(unsigned char ucX, unsigned char ucY, unsigned char* pucStr,unsigned char ucSize,unsigned char ucPixelStatus);
unsigned char glcdWriteLongCallibri(unsigned char ucX, unsigned char ucY,unsigned long ulNumber,unsigned char ucNoOfDigits, unsigned char ucFontHeight,unsigned char ucPixelStatus);
unsigned char glcdWriteLong(unsigned char ucX, unsigned char ucY,unsigned long ulNumber,unsigned char ucNoOfDigits,unsigned char ucSize,unsigned char ucPixelStatus);
unsigned char glcdWriteFloatCallibri(unsigned char ucX, unsigned char ucY,float fData,unsigned char ucNoOfDigitsReal,unsigned char ucNoOfDigitFraction, unsigned char ucFontHeight,unsigned char ucPixelStatus);
unsigned char glcdWriteFloat(unsigned char ucX, unsigned char ucY,float fData,unsigned char ucNoOfDigitsReal,unsigned char ucNoOfDigitFraction,unsigned char ucSize,unsigned char ucPixelStatus);


unsigned char drawdSenZlogo(void);

#endif