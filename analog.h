#ifndef __ANALOG_H_
#define __ANALOG_H_

#include "global.h"

#define ADC_CH_I2	1 	
#define ADC_CH_I1	2
#define ADC_CH_V4	3
#define ADC_CH_V3	4
#define ADC_CH_V2	5
#define ADC_CH_V1	6
#define ADC_CH_LM35	7
#define ADC_CH_SUPPLY	8

#define ADC_V_MUL_FACTOR 0.00244

#define I_MULT_FACTOR		0.01
#define V1_MULT_FACTOR		2.4242
#define V234_MULT_FACTOR	15.2424
#define LM35_MULT_FACTOR	100
#define SUPPLY_MULT_FACTOR	13.192

#define ADC_REF		2.5


extern F32 fV1;
extern F32 fV2;
extern F32 fV3;
extern F32 fV4;
extern F32 fI1;
extern F32 fI2;
extern F32 fLM35;
extern F32 fSupplyVolts;

//get ADC value for the passed channel
unsigned long getADCValue(unsigned char ucChannel);

//get all input analog parameters
void getAllAnalogValues(void);

#endif