#ifndef _GENERAL_FUNCTIONS_H
#define _GENERAL_FUNCTIONS_H

#include "global.h"

#define GENERAL_PURPOSE_BUFF_SIZE	2200

typedef union{
    long lLongVar;
    unsigned  long ulLongVar;
    float fFloatVar;
    int iIntVar;
    unsigned  int uiIntVar;
    unsigned char ucArray[4];
    unsigned char ucCharVal;
}CONVERSION;

extern CONVERSION uConvert; // used throught the code
extern UINT8 ucGeneralPurposeBuffer[GENERAL_PURPOSE_BUFF_SIZE];



void clearGeneralPurposeBuffer(void);

/** @brief This function will conver interger to ASCII string
* preconditions :
* @pre configClock();
* @param ulData         (data type:unsigned  long)  -> long data that has to be converted to string
* @param ucNoOfDigits   (datatype:unsigned char)    -> No of digits
* @param ucSizeOfStr    (datatype:unsigned char)    -> Size of output string
* @param pucStr         (datatype:unsigned char*)   -> address location where the converted ASCII string is to be stored
* @return               (datatype : unsigned char)  -> will return 0 in case of sucess and 1 in case of failure
*/
unsigned char intToASCIIStr(unsigned  long ulData,unsigned char ucNoOfDigits,unsigned char ucSizeOfStr, unsigned char* pucStr);

/** @brief This function will conver interger to ASCII array
* preconditions :
* @pre configClock();
* @param ulData         (data type:unsigned  long)  -> long data that has to be converted to string
* @param ucNoOfDigits   (datatype:unsigned char)    -> No of digits
* @param ucSizeOfArray    (datatype:unsigned char)    -> Size of output array
* @param pucArray         (datatype:unsigned char*)   -> address location where the converted ASCII array will be stored
* @return               (datatype : unsigned char)  -> will return 0 in case of sucess and 1 in case of failure
*/
unsigned char intToArray(unsigned  long ulData,unsigned char ucNoOfDigits,unsigned  int ucSizeOfArray,unsigned char* pucArray);

/** @brief This function will conver float to ASCII string
* preconditions :
* @pre configClock();
* @param fData         (data type:float)  -> long data that has to be converted to string
* @param ucNoOfDigitsReal   (datatype:unsigned char)    -> No of digits before decimal point
* @param ucNoOfDigitsFraction   (datatype:unsigned char)    -> No of digits after decimal point
* @param ucSizeOfArray    (datatype:unsigned char)    -> Size of output array
* @param pucArray         (datatype:unsigned char*)   -> address location where the converted ASCII string will be stored
* @return               (datatype : unsigned char)  -> will return 0 in case of sucess and 1 in case of failure
*/
unsigned char floatToASCIIStr(float fData,unsigned char ucNoOfDigitsReal, unsigned char ucNoOfDigitsFraction,unsigned char ucSizeOfStr, unsigned char* pucStr);

/** @brief This function will conver float to ASCII array
* preconditions :
* @pre configClock();
* @param fData         (data type:float)  -> long data that has to be converted to string
* @param ucNoOfDigitsReal   (datatype:unsigned char)    -> No of digits before decimal point
* @param ucNoOfDigitsFraction   (datatype:unsigned char)    -> No of digits after decimal point
* @param ucSizeOfArray    (datatype:unsigned char)    -> Size of output array
* @param pucArray         (datatype:unsigned char*)   -> address location where the converted ASCII array will be stored
* @return               (datatype : unsigned char)  -> will return 0 in case of sucess and 1 in case of failure
*/
unsigned char floatToArray(float fData,unsigned char ucNoOfDigitsReal, unsigned char ucNoOfDigitsFraction,unsigned char ucSizeOfArray, unsigned char* pucArray);



/** @brief This function will conver float to ASCII array
* preconditions :
* @pre configClock();
* @param ulData         (data type:unsigned  long)  -> long data that has to be converted to hex
* @param ucDataType    (datatype:unsigned char)    -> data type of the passed number, 1->unsigned char , 2->unsigned  int, 3-> unsigned  long
* @param ucSizeOfArray    (datatype:unsigned char)    -> Size of output array
* @param pucArray         (datatype:unsigned char*)   -> address location where the hex array will be stored
* @return               (datatype : unsigned char)  -> will return 0 in case of sucess and 1 in case of failure
*/
unsigned char intToArrayInHex(unsigned  long ulData,unsigned char ucDataType,unsigned char ucSizeOfArray,unsigned char* pucArray);

/** @brief This function will convert hex array into int
* preconditions :
* @pre configClock();
* @param pucArray         (datatype: unsigned char*)     -> hex array address
 * * @param puiNum         (datatype: unsigned  int*)     -> address location where interger value to be stored,
 * @return               (datatype : unsigned char)  -> will return 0 in case of sucess and 1 in case of failure
 *
*/
unsigned char hexToInt(unsigned char* pucArray,unsigned  int *puiNum);

/** @brief This function will convert hex array into long
* preconditions :
* @pre configClock();
* @param pucArray         (datatype: unsigned char*)     -> hex array address
* @param puiNum         (datatype: unsigned  long*)     -> address location where interger value to be stored,
 * @return               (datatype : unsigned char)  -> will return 0 in case of sucess and 1 in case of failure
 *
*/
unsigned char hexToLong(unsigned char* pucArray,unsigned  long *pulNum);

/** @brief This function will convert hex array into float
* preconditions :
* @pre configClock();
* @param pucArray         (datatype: unsigned char*)     -> hex array address
* @param pfNum         (datatype: float*)     -> address location where float value to be stored,
 * @return               (datatype : unsigned char)  -> will return 0 in case of sucess and 1 in case of failure
 *
*/
unsigned char hexToFloat(unsigned char* pucArray,float *pfNum);

/** @brief This function will convert float to hex array
* preconditions :
* @pre configClock();
* @param fData          (datatype:float)        -> float data to be cnverted to hex array
* @param ucSizeOfArray  (datatype: unsigned char)     -> size of array
* @param pucArray         (datatype: unsigned char*)     -> address location where hex array should be stored
 * @return               (datatype : unsigned char)  -> will return 0 in case of sucess and 1 in case of failure
 *
*/
unsigned char floatToArrayInHex(float fData,unsigned char ucSizeOfArray, unsigned char* pucArray);

/** @brief This function will convert ASCII hex char to decimal
* preconditions :
* @pre configClock();
* @param ucChar          (datatype:F8)        -> convert ASCII char
 * @return               (datatype : unsigned char)  -> decimal in ascii char was valid else 0
 *
*/
unsigned char hexToDecimal(unsigned char ucChar);

UINT8 searchStr(UINT8 *pucSource,UINT8 *pucSubStr,UINT32 ulSourceSize,UINT32 *pucLocation);
UINT8 getStrLength( UINT8 *pucStr,UINT32 *pulLen);
UINT8 nStrnCmp(UINT8 *pucSource,UINT32 ucSLocation,UINT32 ulSourceSize,UINT8 *pucSubStr,UINT32 ulSubStrLen);

unsigned char searchString(unsigned char* ucSourceString,unsigned  int uiSourceLength,unsigned char* ucDestStr,unsigned  int uiDestStrLength,unsigned  int* uiLoc);

UINT8 insertStrAt(UINT8 *pucStr1,UINT16 ucInsertAt,UINT16 ucStr1Length,const UINT8 *pucStr2,UINT16 ucStr2Length);

UINT16 convAsciiIntToInt (unsigned char * pucStr,unsigned char ucNoOfDigits);

UINT8 hexToAsciiHexByte(UINT8 ucInchar, UINT8* pucOutBuffer);
UINT8 intHexToHexBytes(UINT16 uiNumber,UINT8* pucOutBufferF);
UINT8 ASCIIHexToNumber(UINT8* inputStr,UINT8 ucDataType);

UINT8 concatStirngsToStr1(UINT8* pucStr1,UINT8* pucStr2,UINT16 ucSizeOfString1);


UINT8 getValueFromBuff(UINT8* pucSourceStr,UINT8* pucStartStr, UINT8* pucEndStr, UINT8* pucValue, UINT32 ulMaxStrLength);
UINT8 longHexToHexBytes(UINT32 ulNumber,UINT8* pucOutBufferF);
UINT8 floatHexToHexBytes(F32 fNumber,UINT8* pucOutBufferF);
void fillBufferWithValue(UINT8* pucValue, UINT16 uiBufferSize, UINT8 ucValue);
float getDataFromModbusPAcket(UINT8 ucRS485Dataype,UINT8 ucMeterType, UINT8* ucModbusData);

UINT8 getParameterFromJsonString(UINT8* pucJsonString,UINT8* pucParameter,UINT8* pucValue,UINT16 uiMaxLengthOfParaValue);
unsigned char hexToASCIIIntStr(unsigned char* pucArray,unsigned char ucArraySize,unsigned char ucDataType,unsigned char ucNoOfBytes,unsigned char ucSizeOfStr, unsigned char* ulDataStr);
unsigned char uLongToHexAsciiStr(unsigned long ulValue,
                               unsigned char ucHexDigits,
                               unsigned char *pucStrOut);
			       
UINT8 jsonBegin(UINT8 *pucJsonBuf, UINT16 uiMaxLen, UINT8 *pucFirstFieldFlag);
UINT8 jsonEnd(UINT8 *pucJsonBuf, UINT16 uiMaxLen);
UINT8 jsonAddHexField(UINT8  *pucJsonBuf,UINT16 uiMaxLen,UINT8  *pucKey,unsigned long ulValue,UINT8  ucDigits,UINT8  *pucFirstFieldFlag);
UINT8 jsonAddStrField(UINT8  *pucJsonBuf,UINT16 uiMaxLen,UINT8  *pucKey,UINT8  *pucVal,UINT8  *pucFirstFieldFlag);


#endif