#pragma once

#include "CDataUnit.h"

//
// 字幕文データクラス
//
class CCaptionData
{
public:
//	CCaptionData(unsigned char* pData);
	CCaptionData(void);
	virtual ~CCaptionData();
	int Decode(unsigned char* pData);
	// データ有効検査
	bool IsEnable();
	// PESヘッダー長取得
	unsigned int GetHeaderLen();

	int GetDataUnitCount() { return m_dataCount; };
	unsigned char* GetDataUnitBuffer(int index) { return m_dataUnit[index].GetDataBuffer(); };
	int GetDataUnitLength(int index) { return m_dataUnit[index].GetDataLength(); };
	int GetDataUnitType(int index) { return m_dataUnit[index].GetUnitParameter(); };

public:
	unsigned char m_TMD;
	unsigned char m_STM[9];
	unsigned long m_data_unit_loop_length;
	unsigned char* m_pData_unit;
	unsigned int m_headerLen;
	bool m_isEnable;

private:
	CDataUnit m_dataUnit[8];
	int m_dataCount;

};


