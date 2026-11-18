#include "generalfunctions.h"
#include "modbus.h"

CONVERSION uConvert;
UINT8 ucGeneralPurposeBuffer[GENERAL_PURPOSE_BUFF_SIZE];

UINT8 getParameterFromJsonString(UINT8* pucJsonString,UINT8* pucParameter,UINT8* pucValue,UINT16 uiMaxLengthOfParaValue)
{
	UINT8 ucStatus =0;
	UINT8 ucData =0;
	UINT32 ulCounter =0;
	UINT16 uiCounterLength =0;
	UINT32 ulSourceStringLength =0;
	UINT32 ulParameterLength =0;
	UINT16 uiLocation =0;
	getStrLength( pucJsonString,&ulSourceStringLength);
	getStrLength( pucParameter,&ulParameterLength);
	ucStatus = searchString(pucJsonString,(UINT16)ulSourceStringLength,pucParameter,(UINT16)ulParameterLength,&uiLocation);
	if(ucStatus==SUCCESS)
	{
		//loop till you get the fist "
		for(ulCounter=uiLocation+2+ulParameterLength;ulCounter<ulSourceStringLength;ulCounter++)
		{
			ucData= pucJsonString[ulCounter];	
			if(ucData=='"')
			{
				break;
			}
		}
		for(ulCounter=ulCounter+1;ulCounter<ulSourceStringLength;ulCounter++)
		{
			ucData= pucJsonString[ulCounter];	
			if(ucData=='"')
			{
				*pucValue = 0x00;	
				return SUCCESS;
			}
			*pucValue++ = ucData;
			
			
			uiCounterLength++;
			if(uiCounterLength>=uiMaxLengthOfParaValue)
			{
				return FAILURE;
			}
		}
	}
	return FAILURE;
}

void clearGeneralPurposeBuffer(void)
{
	UINT16 uiCntrF=0;
	for(uiCntrF=0;uiCntrF<GENERAL_PURPOSE_BUFF_SIZE;uiCntrF++)
		ucGeneralPurposeBuffer[uiCntrF]=0x00;
}

unsigned char intToASCIIStr(unsigned  long ulData,unsigned char ucNoOfDigits,unsigned char ucSizeOfStr, unsigned char* pucStr)
{
  	if(ucNoOfDigits == 0 )
  		return FAILURE;

  	if( ucSizeOfStr <= ucNoOfDigits )	// check whether array is long enough to contain any possible value (unsigned long)
  		return FAILURE;

	intToArray(ulData, ucNoOfDigits, ucSizeOfStr, pucStr);

	pucStr += (ucNoOfDigits);			// Increment pointer to 'ucNoOfDigits' of bytes
	* pucStr = '\0';					// Null terminate the string

	return SUCCESS;
}



unsigned char floatToASCIIStr(float fData,unsigned char ucNoOfDigitsReal, unsigned char ucNoOfDigitsFraction,unsigned char ucSizeOfStr, unsigned char* pucStr)
{
	if(ucNoOfDigitsReal == 0)
		return FAILURE;

	// Check whether array is long enough to hold any possible value
	if(ucNoOfDigitsFraction > 0)
	{
	 	if( ucSizeOfStr <= (ucNoOfDigitsReal+ucNoOfDigitsFraction+1) )
  			return FAILURE;
 	}
	else if(ucNoOfDigitsFraction == 0)
	{
	 	if( ucSizeOfStr <= (ucNoOfDigitsReal+ucNoOfDigitsFraction) )
  			return FAILURE;
 	}

	// Convert float to ascii string
	floatToArray(fData, ucNoOfDigitsReal, ucNoOfDigitsFraction, ucSizeOfStr, pucStr);

	if(ucNoOfDigitsFraction >0)
		pucStr += (ucNoOfDigitsReal + ucNoOfDigitsFraction+1) ;
	else
		pucStr += (ucNoOfDigitsReal + ucNoOfDigitsFraction) ;

	// null terminate the string
	*pucStr='\0';

	return SUCCESS;
}

unsigned char intToASCIIHexStr(unsigned  long ulData, unsigned char ucDataType,unsigned char ucLengthOfString, unsigned char* pucStr)
{
	unsigned char pucHexData[10] = {'0'};
	int ucCntr,ucHexByteCnt;
	unsigned char ucNum,ucTempNo;

	// check whether array in memory is long enough to hold any possible value
	if(ucLengthOfString < 13)
		return FAILURE;

	// check whether ucDatatype is 1/2/3
	if( (ucDataType == 0 )  || ( ucDataType > 3 ))
		return FAILURE;

	intToArrayInHex(ulData,ucDataType,ucLengthOfString,pucHexData); // convert int to hex byte array

	if(ucDataType == 3)			// if datatype is unsigned long
		ucHexByteCnt = 4;
	else if(ucDataType == 2)	// if datatype is unsigned int
		ucHexByteCnt = 2;
	else if(ucDataType == 1)	// if datatype is unsigned char
		ucHexByteCnt = 1;

	//convert hex byte array to hex ascii string and null terminate the string
	for(ucCntr = ucHexByteCnt-1 ; ucCntr >= 0; ucCntr--)
	{
		ucNum = pucHexData[ucCntr];
		ucTempNo = ucNum & 0xF0;
		ucTempNo = ucNum >> 4;
		if(ucTempNo <= 9)
			ucTempNo = ucTempNo+0x30;
		else
			ucTempNo = ucTempNo+0x37;
		*pucStr++ = ucTempNo;

		ucTempNo = ucNum & 0x0F;
		if(ucTempNo <= 9)
			ucTempNo = ucTempNo+0x30;
		else
			ucTempNo = ucTempNo+0x37;
		*pucStr++ = ucTempNo;
	}
	*pucStr = '\0';

	return SUCCESS;
}

unsigned char floatToASCIIHexStr(float fData,unsigned char ucLengthOfString, unsigned char* pucStr)
{
	unsigned char pucHexData[4] = {'0'};
	int ucCntr;
	unsigned char ucNum,ucTempNo;

	if(ucLengthOfString < 13)
		return FAILURE;

	floatToArrayInHex(fData,ucLengthOfString,pucHexData); // convert float to hex byte array

	// convert hex byte array to hex ascii string and null terminate the string
	for(ucCntr = 3 ; ucCntr >= 0; ucCntr--)
	{
		ucNum = pucHexData[ucCntr];
		ucTempNo = ucNum & 0xF0;
		ucTempNo = ucNum>>4;
		if(ucTempNo <= 9)
			ucTempNo = ucTempNo+0x30;
		else
			ucTempNo = ucTempNo+0x37;
		*pucStr++ = ucTempNo;

		ucTempNo = ucNum & 0x0F;
		if(ucTempNo <= 9)
			ucTempNo = ucTempNo+0x30;
		else
			ucTempNo = ucTempNo+0x37;
		*pucStr++ = ucTempNo;

		*pucStr++ = ' ';
	}

	*pucStr = '\0';
	return SUCCESS;
}




unsigned char hexToASCIIIntStr(unsigned char* pucArray,unsigned char ucArraySize,unsigned char ucDataType,unsigned char ucNoOfBytes,unsigned char ucSizeOfStr, unsigned char* ulDataStr)
{
	unsigned char ucResult;
	unsigned  long ulIntNum;

	// check whether ucDatatype = 1/2/3
	if( (ucDataType == 0 )  || ( ucDataType > 3 ))
		return FAILURE;

	// check whether hex array size is 4
	if(ucArraySize < 4)
		return FAILURE;

	if (ucNoOfBytes == 0)
		return FAILURE;

	if(ucSizeOfStr < ucNoOfBytes+1)
		return FAILURE;

	ucResult = hexToLong(pucArray,&ulIntNum);    // converts hex value to long

	intToASCIIStr(ulIntNum,ucNoOfBytes,ucSizeOfStr,ulDataStr); // convert long value to ascii string
	return SUCCESS;
}

unsigned char hexToASCIIFloatStr(unsigned char* pucArray,unsigned char ucNoOfDigitsReal, unsigned char ucNoOfDigitsFraction,unsigned char ucSizeOfStr,unsigned char* pucStr)
{
	float *pfData;
	float fData;

	if(ucNoOfDigitsReal == 0)
		return FAILURE;

  	// check whether array is long enough to hold any possible value
	if(ucNoOfDigitsFraction > 0)
	{
	 	if( ucSizeOfStr <= (ucNoOfDigitsReal+ucNoOfDigitsFraction+2) )
  			return FAILURE;
 	}
	else if(ucNoOfDigitsFraction == 0)
	{
	 	if( ucSizeOfStr <= (ucNoOfDigitsReal+ucNoOfDigitsFraction) )
  			return FAILURE;
 	}

	pfData = (float *)pucArray;		// convert hex value to float
	fData = *pfData;
	floatToASCIIStr(fData,ucNoOfDigitsReal,ucNoOfDigitsFraction,ucSizeOfStr,pucStr); // convert float to ascii string

	return SUCCESS;
}

unsigned char intToArray(unsigned  long ulData,unsigned char ucNoOfDigits,unsigned  int ucSizeOfArray,unsigned char* pucArray)
{
	unsigned  long ulDataCopy;
	unsigned char ucIndex,ucCntr;
	unsigned  long ucStrLen;
  	unsigned char ulDataArray[11] = {0};

  	// check whether array is long enough to contain any possible value
  	if( ucSizeOfArray < ucNoOfDigits )
  		return FAILURE;
	// return failure if no. of digits is 0
	if ( ucNoOfDigits == 0 )
		return FAILURE;
	ulDataCopy = ulData;
	// convert number to string and copy to data array
	ucIndex = 0;
	while(ulDataCopy)
	{
		ulDataArray[ucIndex++] = (unsigned char)(ulDataCopy % 10) +  '0';
		ulDataCopy /= 10;
	}
	// get string length
	 ucStrLen = ucIndex;
	// pad left of the string with zeroes if required
	if (ucNoOfDigits > ucStrLen)
	{
		for(ucIndex = 0; ucIndex < (ucNoOfDigits - ucStrLen) ; ucIndex++)
		{
			*pucArray++ = '0';
		}
	}
	// reverse string and copy to memory array
	if (ucNoOfDigits > ucStrLen)
		ucIndex = (unsigned char)ucStrLen;
	else if (ucNoOfDigits <= ucStrLen)
		ucIndex = ucNoOfDigits;
 	for(ucCntr = 0; ucCntr < (ucNoOfDigits) && (ucIndex>0); ucCntr++)
	{
		ucIndex -=1;
		*pucArray++ = (char)ulDataArray[ucIndex];
	}
	return SUCCESS;
}

unsigned char floatToArray(float fData,unsigned char ucNoOfDigitsReal,unsigned char ucNoOfDigitsFraction,unsigned char ucSizeOfArray, unsigned char* pucArray)
{
	unsigned  long ulIntegerPart = 0;
	unsigned  long ulDecimalPart = 1;
	unsigned char ucIndex;

	if(ucNoOfDigitsReal == 0)
		return FAILURE;

	// check whether array is long enough to hold any possible value
	if(ucNoOfDigitsFraction > 0)
	{
	 	if( ucSizeOfArray < (ucNoOfDigitsReal+ucNoOfDigitsFraction+1) )
  			return FAILURE;
 	}
	else if(ucNoOfDigitsFraction == 0)
	{
	 	if( ucSizeOfArray < (ucNoOfDigitsReal+ucNoOfDigitsFraction) )
  			return FAILURE;
 	}

	// get integer part of floating point number
	ulIntegerPart = (unsigned  long) fData;

	// get decimal part of floating point number
	fData = fData - ulIntegerPart;
	for( ucIndex = 0; ucIndex < ucNoOfDigitsFraction; ucIndex++ )
			ulDecimalPart = ulDecimalPart * 10;
	ulDecimalPart = (unsigned  long)(fData * ulDecimalPart);

	// convert integer part to string and copy to memory array
	intToArray( ulIntegerPart, ucNoOfDigitsReal, ucSizeOfArray, pucArray);

	//Increment pointer to 'ucNoOfDigitsReal' of bytes
	pucArray += ucNoOfDigitsReal;

	// insert decimal point at loation ( pucStr + ucNoOfDigitsReal)
	if(ucNoOfDigitsFraction >0)
	{
		*pucArray = '.';
		pucArray++;

		// convert decimal part to string and copy to memory array
		intToArray( ulDecimalPart, ucNoOfDigitsFraction, ucSizeOfArray, pucArray);
	}

	return SUCCESS;
}

unsigned char intToArrayInHex(unsigned  long ulData,unsigned char ucDataType,unsigned char ucSizeOfArray,unsigned char* pucArray)
{
	unsigned char *pulData;

	// check whether ucDatatype = 1,2,3
	if( (ucDataType == 0 )  || ( ucDataType >3 ))
		return FAILURE;
	if(ucSizeOfArray == 0)
		return FAILURE;

	if((ucDataType == 1) && (ucSizeOfArray <1))
		return FAILURE;
	if((ucDataType == 2) && (ucSizeOfArray <2))
		return FAILURE;
	if((ucDataType==3) && (ucSizeOfArray <4))
		return FAILURE;

	pulData = (unsigned char *)&ulData;

	if(ucDataType == 3)					// Gets each byte of log type data in hex
	{
		*pucArray++ = *pulData++;
		*pucArray++ = *pulData++;
		*pucArray++ = *pulData++;
		*pucArray = *pulData;
	}
	else if(ucDataType == 2)			// Get each byte of integer type data in hex
	{
		*pucArray++ = *pulData++;
		*pucArray = *pulData;
	}
	else if(ucDataType == 1)		  // Get each byte of char type data in hex
	{
		*pucArray = *pulData;
	}

	return SUCCESS;
}

unsigned char floatToArrayInHex(float fData,unsigned char ucSizeOfArray, unsigned char* pucArray)
{
	unsigned char *pulData;

	if(ucSizeOfArray < 4)
		return FAILURE;

	pulData = (unsigned char *)&fData;

	*pucArray++ = *pulData++;
	*pucArray++ = *pulData++;
	*pucArray++ = *pulData++;
	*pucArray = *pulData;

	return SUCCESS;
}

unsigned char hexToFloat(unsigned char* pucArray,float *pfNum)
{
	float *pfData;

	pfData = (float *)pucArray;
	*pfNum = *pfData;
	return SUCCESS;
}

unsigned char hexToLong(unsigned char* pucArray,unsigned  long *pulNum)
{
	unsigned  long *puldata;

	puldata = (unsigned  long *)pucArray;

	*pulNum = *puldata;
	return SUCCESS;
}

unsigned char hexToInt(unsigned char* pucArray,unsigned  int *puiNum)
{
	unsigned  int *puidata;

	puidata = (unsigned  int *)pucArray;

	*puiNum = *puidata;
	return SUCCESS;
}


unsigned char getStrLength( unsigned char *pucStr,unsigned  long *pulLen)
{
	unsigned char *pucStrCopy;

	pucStrCopy = pucStr;
	while(*pucStr)
	{
		pucStr++;
	}
	*pulLen = pucStr - pucStrCopy;
	return SUCCESS;
}


unsigned char searchString(unsigned char* ucSourceString,unsigned  int uiSourceLength,unsigned char* ucDestStr,unsigned  int uiDestStrLength,unsigned  int* uiLoc)
{
	unsigned  int uiSCntr = 0;
	unsigned  int uiDCntr = 0;
	unsigned char  ucFlag  = 0;

	uiDCntr = 0;
	ucFlag = FAILURE;

	for(uiSCntr = 0; uiSCntr<uiSourceLength; uiSCntr++)
	{
	
		if(ucSourceString[uiSCntr] == ucDestStr[uiDCntr])
		{
			if( uiDCntr+1 < uiDestStrLength )
			{
				ucFlag = FAILURE;
				uiDCntr++;

			}
			else
			{
				*uiLoc = uiSCntr-uiDestStrLength+1;
			    ucFlag = SUCCESS;
			    break;
			}
		}
		else
		{
			uiDCntr = 0;
			ucFlag = FAILURE;
		}
	}

	return ucFlag;
}





unsigned char hexToDecimal(unsigned char ucChar)
{
	 if (ucChar>='0' && ucChar<='9')
 		return ucChar-'0';
 	 if (ucChar>='A' && ucChar<='F')
 	 	return ucChar-'A'+10;
 	 if (ucChar>='a' && ucChar<='f')
 	 	return ucChar-'a'+10;
 	 return ucChar=0;        //return 0 if not hex value
}

unsigned char nibbleToAscii(unsigned char ucNibble)			// which function will convert input to
{

	if(ucNibble<10)
		return (ucNibble+'0');
	else if(ucNibble<16)
		return (ucNibble+'A'-10);
	else
		return 0;

}

UINT8 searchStr(UINT8 *pucSource,UINT8 *pucSubStr,UINT32 ulSourceSize,UINT32 *pucLocation)
{
	// check for null string
	UINT8 ucResult = FAILURE;
	UINT32 ulSubStrLen;
	UINT8 ucDChar;
	UINT16 uiIndex;

	//validate for null string
//	if(!(*pucSource) == 1)
//		return FAILURE;

	//validate for null string
	ucDChar = *pucSubStr;
//	if(!(ucDChar) == 1)
//		return FAILURE;

	//get string length of destinaiton
	ucResult = getStrLength(pucSubStr,&ulSubStrLen);

	ucResult = FAILURE;
	for(uiIndex=0;uiIndex<ulSourceSize;uiIndex++)
	{
		if((pucSubStr[0] == pucSource[uiIndex]))
		{
			if( uiIndex == (ulSourceSize - 1) )
				ucResult = nStrnCmp(pucSource,0,ulSourceSize,pucSubStr+1,ulSubStrLen);
			else
				ucResult = nStrnCmp(pucSource,uiIndex+1,ulSourceSize, pucSubStr+1, ulSubStrLen);

			if(ucResult == SUCCESS)
			{
				*pucLocation = uiIndex;
				break;
			}
		}
	}

	return ucResult;
}


UINT8 nStrnCmp(UINT8 *pucSource,UINT32 ucSLocation,UINT32 ulSourceSize,UINT8 *pucSubStr,UINT32 ulSubStrLen)
{
	UINT8 ucIndex;
	UINT8 temp1,temp2;

	for(ucIndex = 0; ucIndex < ulSubStrLen-1 ; ucIndex++)
	{
		if(ucSLocation == ulSourceSize)
			ucSLocation  = 0;

		temp1 = pucSource[ucSLocation] - 0x30 ;
		temp2 = pucSubStr[ucIndex] - 0x30;
		if(pucSource[ucSLocation] != pucSubStr[ucIndex])
			return FAILURE;
		ucSLocation++;
	}
	return SUCCESS;
}


UINT8 insertStrAt(UINT8 *pucStr1,UINT16 ucInsertAt,UINT16 ucStr1Length,const UINT8 *pucStr2,UINT16 ucStr2Length)
{
	UINT16 ucStr2Index,ucStr1Index;
	//check whether index is out of range
	if(ucInsertAt >= ucStr1Length)
	   return FAILURE;
	// check whether sting 2 has enough space to insert in string 1
 	if((ucStr1Length - ucInsertAt) < ucStr2Length)
 		return FAILURE;
 		
 	ucStr1Index = ucInsertAt;
 	for(ucStr2Index =0 ; ucStr2Index<=ucStr2Length; ucStr2Index++)
 	{
		pucStr1[ucStr1Index] =  pucStr2[ucStr2Index];
		ucStr1Index++;
 	}
 	
 	return SUCCESS;	
}

UINT16 convAsciiIntToInt (unsigned char * pucStr,unsigned char ucNoOfDigits)
{
	UINT16 uiTempValue=0;
	UINT16 uiMulFactor=1;
	UINT8  ucCntr;
	
	for(ucCntr=0;ucCntr<(ucNoOfDigits-1);ucCntr++)
	{
		uiMulFactor=uiMulFactor*10;
	}
	
	for(ucCntr=0;ucCntr<(ucNoOfDigits);ucCntr++)
	{
		uiTempValue=uiTempValue+((*pucStr-0x30) * uiMulFactor);
		pucStr++;
		uiMulFactor=uiMulFactor/10;
	}
	return uiTempValue;
}
UINT8 hexToAsciiHexByte(UINT8 ucInchar, UINT8* pucOutBuffer)	// convert the hex value to assic hex, ex: 0xB2 ->  'B' and '2' as output
{
	UINT8 ucUpperNibble,ucLowerNibble;

	ucUpperNibble=ucInchar & 0xF0;
	ucUpperNibble=ucUpperNibble>>4;
	ucLowerNibble=ucInchar & 0x0F;

	*pucOutBuffer=nibbleToAscii(ucUpperNibble);
	pucOutBuffer++;		// go to next location in buffer
	*pucOutBuffer=nibbleToAscii(ucLowerNibble);

	return SUCCESS;
}

UINT8 intHexToHexBytes(UINT16 uiNumber,UINT8* pucOutBufferF)
{
	union
	{
		UINT16 uinumber;
		UINT8 ucArray[2];
	}sNumber;

	sNumber.uinumber=uiNumber;

	hexToAsciiHexByte(sNumber.ucArray[0],pucOutBufferF);
	pucOutBufferF=pucOutBufferF+2;
	hexToAsciiHexByte(sNumber.ucArray[1],pucOutBufferF);
	return SUCCESS;
}

UINT8 ASCIIHexToNumber(UINT8* inputStr,UINT8 ucDataType)
{
	uConvert.ucArray[0]=0;
	uConvert.ucArray[1]=0;
	uConvert.ucArray[2]=0;
	uConvert.ucArray[3]=0;

	uConvert.ucArray[0]=hexToDecimal(*inputStr++)<<4;
	uConvert.ucArray[0]=uConvert.ucArray[0]|(hexToDecimal(*inputStr++));
	if(ucDataType==0)
		return SUCCESS;

	uConvert.ucArray[1]=hexToDecimal(*inputStr++)<<4;
	uConvert.ucArray[1]=uConvert.ucArray[1]|(hexToDecimal(*inputStr++));
	if(ucDataType==1)
		return SUCCESS;


	uConvert.ucArray[2]=hexToDecimal(*inputStr++)<<4;
	uConvert.ucArray[2]=uConvert.ucArray[2]|(hexToDecimal(*inputStr++));

	uConvert.ucArray[3]=hexToDecimal(*inputStr++)<<4;
	uConvert.ucArray[3]=uConvert.ucArray[3]|(hexToDecimal(*inputStr++));

	return SUCCESS;
}

UINT8 concatStirngsToStr1(UINT8* pucStr1,UINT8* pucStr2,UINT16 ucSizeOfString1)
{
	UINT32 ucStrLen1=0;	
	UINT32 ucStrLen2=0;
	UINT32 ucCntrF=0;
	getStrLength(pucStr1,&ucStrLen1);
	getStrLength(pucStr2,&ucStrLen2);
	if((ucStrLen1+ucStrLen2)>=ucSizeOfString1)
		return FAILURE;
	
	for(ucCntrF=0;ucCntrF<ucStrLen2;ucCntrF++)
	{
		pucStr1[ucStrLen1+ucCntrF]=pucStr2[ucCntrF];
	}
	pucStr1[ucCntrF+ucStrLen1]=0x00;	
		
	return SUCCESS;
	
	
}

UINT8 getValueFromBuff(UINT8* pucSourceStr,UINT8* pucStartStr, UINT8* pucEndStr, UINT8* pucValue, UINT32 ulMaxStrLength)
{
	UINT32 ulSourceLength;
	UINT8 ucStatusF;
	UINT32 ulStartLoc=0;
	UINT32 ulEndLoc=0;
	UINT32 ulCntr=0;

	getStrLength(pucStartStr,&ulSourceLength);

	ucStatusF=searchStr(pucSourceStr,pucStartStr,ulMaxStrLength,&ulStartLoc);
	if(ucStatusF==FAILURE)
		return FAILURE;

	for(ulCntr=0;ulCntr<(ulStartLoc+ulSourceLength);ulCntr++)
	{
		pucSourceStr++;
	}

	ucStatusF=searchStr(pucSourceStr,pucEndStr,ulMaxStrLength,&ulEndLoc);
	if(ucStatusF==FAILURE)
		return FAILURE;

	for(ulCntr=0;ulCntr<(ulEndLoc);ulCntr++)
	{
		*pucValue++=*pucSourceStr++;
	}

	*pucValue=0x00;

	return SUCCESS;
}

UINT8 longHexToHexBytes(UINT32 ulNumber,UINT8* pucOutBufferF)
{
	/*union
	{
		struct
		{
			UINT32 ulnumber;
		};
		UINT8 ucArray[4];
	}sNumber;

	sNumber.ulnumber=ulNumber;*/
	uConvert.ulLongVar=ulNumber;

	hexToAsciiHexByte(uConvert.ucArray[0],pucOutBufferF);
	pucOutBufferF=pucOutBufferF+2;
	hexToAsciiHexByte(uConvert.ucArray[1],pucOutBufferF);
	pucOutBufferF=pucOutBufferF+2;
	hexToAsciiHexByte(uConvert.ucArray[2],pucOutBufferF);
	pucOutBufferF=pucOutBufferF+2;
	hexToAsciiHexByte(uConvert.ucArray[3],pucOutBufferF);
	return SUCCESS;
}

UINT8 floatHexToHexBytes(F32 fNumber,UINT8* pucOutBufferF)
{
	union 
	{
		struct
		{
			F32 fnumber;
		}st;
		UINT8 ucArray[4];
	}sNumber;

	sNumber.st.fnumber=fNumber;
	
	hexToAsciiHexByte(sNumber.ucArray[0],pucOutBufferF);
	pucOutBufferF=pucOutBufferF+2;
	hexToAsciiHexByte(sNumber.ucArray[1],pucOutBufferF);
	pucOutBufferF=pucOutBufferF+2;
	hexToAsciiHexByte(sNumber.ucArray[2],pucOutBufferF);
	pucOutBufferF=pucOutBufferF+2;
	hexToAsciiHexByte(sNumber.ucArray[3],pucOutBufferF);
	return SUCCESS;
}

void fillBufferWithValue(UINT8* pucValue, UINT16 uiBufferSize, UINT8 ucValue)
{
	while(uiBufferSize)
	{
		*pucValue++=ucValue;
		uiBufferSize--;
	}
}


float getDataFromModbusPAcket(UINT8 ucRS485Dataype,UINT8 ucMeterType, UINT8* ucModbusData)
{
	float fFinalValue = 0;
	if (ucRS485Dataype == RS485_DATATYPE_UINT8) 
	{
		uConvert.ucArray[0] = ucModbusData[0];
                fFinalValue = (float) uConvert.ucArray[0];
        }
	else if (ucRS485Dataype == RS485_DATATYPE_UINT16) 
	{
	        uConvert.ucArray[0] = ucModbusData[1];
	        uConvert.ucArray[1] = ucModbusData[0];
	        fFinalValue = (float) uConvert.uiIntVar;
	}
	else if (ucRS485Dataype == RS485_DATATYPE_INT16) 
	{
                uConvert.ucArray[0] = ucModbusData[1];
                uConvert.ucArray[1] = ucModbusData[0];
                fFinalValue = (float) uConvert.iIntVar;
	}
	else if (ucRS485Dataype == RS485_DATATYPE_UINT32) 
	{
                if (ucMeterType == 0) {
                    uConvert.ucArray[0] = ucModbusData[3];
                    uConvert.ucArray[1] = ucModbusData[2];
                    uConvert.ucArray[2] = ucModbusData[1];
                    uConvert.ucArray[3] = ucModbusData[0];
                } else {
                    uConvert.ucArray[0] = ucModbusData[1];
                    uConvert.ucArray[1] = ucModbusData[0];
                    uConvert.ucArray[2] = ucModbusData[3];
                    uConvert.ucArray[3] = ucModbusData[2];
                }
                fFinalValue = (float) uConvert.ulLongVar;
        }
	else if (ucRS485Dataype == RS485_DATATYPE_INT32) 
	{
                if (ucMeterType == 0) {
                    uConvert.ucArray[0] = ucModbusData[3];
                    uConvert.ucArray[1] = ucModbusData[2];
                    uConvert.ucArray[2] = ucModbusData[1];
                    uConvert.ucArray[3] = ucModbusData[0];
                } else {
                    uConvert.ucArray[0] = ucModbusData[1];
                    uConvert.ucArray[1] = ucModbusData[0];
                    uConvert.ucArray[2] = ucModbusData[3];
                    uConvert.ucArray[3] = ucModbusData[2];
                }
                fFinalValue = (float) uConvert.lLongVar;
        }
	else if (ucRS485Dataype == RS485_DATATYPE_F32) 
	{
                if (ucMeterType == 0) {
                    uConvert.ucArray[0] = ucModbusData[3];
                    uConvert.ucArray[1] = ucModbusData[2];
                    uConvert.ucArray[2] = ucModbusData[1];
                    uConvert.ucArray[3] = ucModbusData[0];
                } else {
                    uConvert.ucArray[0] = ucModbusData[1];
                    uConvert.ucArray[1] = ucModbusData[0];
                    uConvert.ucArray[2] = ucModbusData[3];
                    uConvert.ucArray[3] = ucModbusData[2];
                }
                fFinalValue = (float) uConvert.fFloatVar;
        }
	return fFinalValue;
}

unsigned char uLongToHexAsciiStr(unsigned long ulValue,
                               unsigned char ucHexDigits,
                               unsigned char *pucStrOut)
{
    unsigned char i;
    unsigned long nibble;

    if (ucHexDigits == 0 || ucHexDigits > 8)
        return FAILURE;

    for (i = 0; i < ucHexDigits; i++)
    {
        nibble = (ulValue >> (4 * (ucHexDigits - 1 - i))) & 0x0FUL;

        if (nibble < 10)
            pucStrOut[i] = (unsigned char)('0' + nibble);
        else
            pucStrOut[i] = (unsigned char)('A' + (nibble - 10));
    }

    pucStrOut[ucHexDigits] = '\0';   // null terminate
    return SUCCESS;
}


UINT8 jsonBegin(UINT8 *pucJsonBuf, UINT16 uiMaxLen, UINT8 *pucFirstFieldFlag)
{
    if (uiMaxLen == 0)
        return FAILURE;

    pucJsonBuf[0] = 0x00;              // empty string
    *pucFirstFieldFlag = 1;            // next field is first

    if (concatStirngsToStr1(pucJsonBuf, (UINT8*)"{", uiMaxLen) == FAILURE)
        return FAILURE;

    return SUCCESS;
}

UINT8 jsonEnd(UINT8 *pucJsonBuf, UINT16 uiMaxLen)
{
    if (concatStirngsToStr1(pucJsonBuf, (UINT8*)"}", uiMaxLen) == FAILURE)
        return FAILURE;

    return SUCCESS;
}

UINT8 jsonAddHexField(UINT8  *pucJsonBuf,UINT16 uiMaxLen,UINT8  *pucKey,unsigned long ulValue,UINT8  ucDigits,UINT8  *pucFirstFieldFlag)
{
    UINT8 tempStr[10];

    // Comma if NOT first field
    if (*pucFirstFieldFlag == 0)
    {
        if (concatStirngsToStr1(pucJsonBuf, (UINT8*)",", uiMaxLen) == FAILURE)
            return FAILURE;
    }
    else
    {
        *pucFirstFieldFlag = 0;
    }

    // "KEY":"VALUE"
    if (concatStirngsToStr1(pucJsonBuf, (UINT8*)"\"", uiMaxLen) == FAILURE)
        return FAILURE;
    if (concatStirngsToStr1(pucJsonBuf, pucKey, uiMaxLen) == FAILURE)
        return FAILURE;
    if (concatStirngsToStr1(pucJsonBuf, (UINT8*)"\":\"", uiMaxLen) == FAILURE)
        return FAILURE;

    if (uLongToHexAsciiStr(ulValue, ucDigits, tempStr) == FAILURE)
        return FAILURE;
    if (concatStirngsToStr1(pucJsonBuf, tempStr, uiMaxLen) == FAILURE)
        return FAILURE;

    if (concatStirngsToStr1(pucJsonBuf, (UINT8*)"\"", uiMaxLen) == FAILURE)
        return FAILURE;

    return SUCCESS;
}


UINT8 jsonAddStrField(UINT8  *pucJsonBuf,UINT16 uiMaxLen,UINT8  *pucKey,UINT8  *pucVal,UINT8  *pucFirstFieldFlag)
{
    // Comma if NOT first field
    if (*pucFirstFieldFlag == 0)
    {
        if (concatStirngsToStr1(pucJsonBuf, (UINT8*)",", uiMaxLen) == FAILURE)
            return FAILURE;
    }
    else
    {
        *pucFirstFieldFlag = 0;
    }

    // "KEY":"VALUE"
    if (concatStirngsToStr1(pucJsonBuf, (UINT8*)"\"", uiMaxLen) == FAILURE)
        return FAILURE;
    if (concatStirngsToStr1(pucJsonBuf, pucKey, uiMaxLen) == FAILURE)
        return FAILURE;
    if (concatStirngsToStr1(pucJsonBuf, (UINT8*)"\":\"", uiMaxLen) == FAILURE)
        return FAILURE;
    if (concatStirngsToStr1(pucJsonBuf, pucVal, uiMaxLen) == FAILURE)
        return FAILURE;
    if (concatStirngsToStr1(pucJsonBuf, (UINT8*)"\"", uiMaxLen) == FAILURE)
        return FAILURE;

    return SUCCESS;
}
