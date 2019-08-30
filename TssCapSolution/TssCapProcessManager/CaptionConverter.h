#pragma once

//#include <string>
//#include "global_cnv.h"
//#include "TtmlConverter.h"
//#include "Common/plog/Log.h"
//#include <sstream>

using namespace std;

#ifdef CAPTION_CONVERTER_EXPORTS

#define CAPTION_CONVERTER_API __declspec(dllexport)
#else
#define CAPTION_CONVERTER_API __declspec(dllimport)
#endif



CAPTION_CONVERTER_API int API_ConvJisToUtf8(unsigned char* pInputBuff, int iInputLen, unsigned char* pOutputBuff, int* iOutputLen);

CAPTION_CONVERTER_API int API_ConvUtf8ToJis(unsigned char* pInputBuff, int iInputLen, unsigned char* pOutputBuff, int* iOutputLen);

//////////////////////////////////////////////////////////
//
// iRomSound : -1:なし、-1以外:内臓音
//////////////////////////////////////////////////////////
CAPTION_CONVERTER_API int API_ConvJisToTtml(int iLogNum, int iLogLevelAPI_ConvJisToTtml, int resolution, unsigned char* pInputBuff, int iInputLen, unsigned char* pOutputBuff, int* iOutputLen,
	unsigned char* pDrcsInputBuff1, int iDrcsInput1Len, int iDrcsInput1Type, unsigned char* pDrcsOutputBuff1, int* iDrcsOutput1Len,
	unsigned char* pDrcsInputBuff2, int iDrcsInput2Len, int iDrcsInput2Type, unsigned char* pDrcsOutputBuff2, int* iDrcsOutput2Len,
	unsigned char* pDrcsInputBuff3, int iDrcsInput3Len, int iDrcsInput3Type, unsigned char* pDrcsOutputBuff3, int* iDrcsOutput3Len,
	unsigned char* pDrcsInputBuff4, int iDrcsInput4Len, int iDrcsInput4Type, unsigned char* pDrcsOutputBuff4, int* iDrcsOutput4Len,
	long* beginTime, long* endTime, int* iRomSound, int* iAnimation);
 
CAPTION_CONVERTER_API int API_ConvUtf8ToJisToTtml(int iLogNum, int iLogLevel, int resolution, unsigned char* pInputBuff, int iInputLen, unsigned char* pOutputBuff, int* iOutputLen,
	unsigned char* pDrcsInputBuff1, int iDrcsInput1Len, int iDrcsInput1Type, unsigned char* pDrcsOutputBuff1, int* iDrcsOutput1Len,
	unsigned char* pDrcsInputBuff2, int iDrcsInput2Len, int iDrcsInput2Type, unsigned char* pDrcsOutputBuff2, int* iDrcsOutput2Len,
	unsigned char* pDrcsInputBuff3, int iDrcsInput3Len, int iDrcsInput3Type, unsigned char* pDrcsOutputBuff3, int* iDrcsOutput3Len,
	unsigned char* pDrcsInputBuff4, int iDrcsInput4Len, int iDrcsInput4Type, unsigned char* pDrcsOutputBuff4, int* iDrcsOutput4Len);

CAPTION_CONVERTER_API int API_SetLogPath(unsigned char* pInputBuff, int iInputLen);

CAPTION_CONVERTER_API int API_ConvUtf8ToTtml(int iLogNum, int iLogLevel, int resolution, unsigned char* pInputBuff, int iInputLen, unsigned char* pOutputBuff, int* iOutputLen);

