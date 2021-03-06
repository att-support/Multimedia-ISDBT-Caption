#pragma once

#ifdef CAPTION_CONVERTER_EXPORTS

#define CAPTION_CONVERTER_API __declspec(dllexport)
#else
#define CAPTION_CONVERTER_API __declspec(dllimport)
#endif

//////////////////////////////////////////////////////////
// iLogNum : 1～
// iLogLevel : 0(none),1(fatal),2(error),3(warning),4(info),5(debug)
// resolution : 1(SD),2(HD),4(2K),8(4K)
// beginTime : 開始時間
// endTime : 終了時間
// iRomSound : -1:なし、-1以外:内臓音
// iAnimation : -1:なし、1：フラッシングあり、2：スクロールあり
// 90区の記号用SVGフォント対応：unsigned char* pMarkOutputBuff, int* iMarkOutputLen
//////////////////////////////////////////////////////////
CAPTION_CONVERTER_API int API_ConvJisToTtml(int iLogNum, int iLogLevel, int resolution, unsigned char* pInputBuff, int iInputLen, unsigned char* pOutputBuff, int* iOutputLen,
	unsigned char* pDrcsInputBuff1, int iDrcsInput1Len, int iDrcsInput1Type, unsigned char* pDrcsOutputBuff1, int* iDrcsOutput1Len,
	unsigned char* pDrcsInputBuff2, int iDrcsInput2Len, int iDrcsInput2Type, unsigned char* pDrcsOutputBuff2, int* iDrcsOutput2Len,
	unsigned char* pDrcsInputBuff3, int iDrcsInput3Len, int iDrcsInput3Type, unsigned char* pDrcsOutputBuff3, int* iDrcsOutput3Len,
	unsigned char* pDrcsInputBuff4, int iDrcsInput4Len, int iDrcsInput4Type, unsigned char* pDrcsOutputBuff4, int* iDrcsOutput4Len,
	long* beginTime, long* endTime, int* iRomSound, int* iAnimation, unsigned char* pMarkOutputBuff, int* iMarkOutputLen);

CAPTION_CONVERTER_API int API_SetLogPath(unsigned char* pInputBuff, int iInputLen);


