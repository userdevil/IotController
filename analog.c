#include "analog.h"
#include "global.h"


F32 fV1;
F32 fV2;
F32 fV3;
F32 fV4;
F32 fI1;
F32 fI2;
F32 fLM35;
F32 fSupplyVolts;

unsigned long getADCValue(unsigned char ucChannel)
{
 	unsigned int uiADCValue=0,uiTemp=0;
	unsigned char ucCntr=0;
	for(ucCntr=0;ucCntr<25;ucCntr++)
	{
		ADS = ucChannel;
		ADIF = 0;
		while(ADIF == 0);
		uiADCValue += (uint16_t)(ADCR >> 6U);
		
	}
	uiADCValue=uiADCValue/25;
	return uiADCValue;
}

void getAllAnalogValues()
{
	fV1=(float)getADCValue(ADC_CH_V1)*ADC_V_MUL_FACTOR*V1_MULT_FACTOR;
	fV2=(float)getADCValue(ADC_CH_V2)*ADC_V_MUL_FACTOR*V234_MULT_FACTOR;
	fV3=(float)getADCValue(ADC_CH_V3)*ADC_V_MUL_FACTOR*V234_MULT_FACTOR;
	fV4=(float)getADCValue(ADC_CH_V4)*ADC_V_MUL_FACTOR*V234_MULT_FACTOR;
	fI1=(float)getADCValue(ADC_CH_I1)*ADC_V_MUL_FACTOR*I_MULT_FACTOR;
	fI2=(float)getADCValue(ADC_CH_I2)*ADC_V_MUL_FACTOR*I_MULT_FACTOR;
	fLM35=(float)getADCValue(ADC_CH_LM35)*ADC_V_MUL_FACTOR*LM35_MULT_FACTOR;
	fSupplyVolts=(float)getADCValue(ADC_CH_SUPPLY)*ADC_V_MUL_FACTOR*SUPPLY_MULT_FACTOR;
	
}