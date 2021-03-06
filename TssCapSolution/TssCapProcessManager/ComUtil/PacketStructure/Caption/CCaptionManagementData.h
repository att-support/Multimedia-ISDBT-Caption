#pragma once

#include "CDataUnit.h"

//
// 字幕管理データクラス
//
class CCaptionManagementData
{
public:
//	CCaptionManagementData(unsigned char* pData);
	CCaptionManagementData(void);
	virtual ~CCaptionManagementData();
	int Decode(unsigned char* pData);
	// データ有効検査
	bool IsEnable();

	int GetDataUnitCount() { return m_dataCount; };
	unsigned char* GetDataUnitBuffer(int index) { return m_dataUnit[index].GetDataBuffer(); };
	int GetDataUnitLength(int index) { return m_dataUnit[index].GetDataLength(); };
	int GetDataUnitType(int index) { return m_dataUnit[index].GetUnitParameter(); };

public:
	unsigned char m_TMD;
	unsigned char m_OTM[9];
	unsigned char m_num_languages;
	struct {
		unsigned char language_tag;
		unsigned char DMF;
		unsigned char DC;
		unsigned char ISO_639_language_code[4];
		unsigned char Format;
		unsigned char TCS;
		unsigned char rollup_mode;
	} m_lang[8];
	unsigned long m_data_unit_loop_length;
	unsigned char* m_pData_unit;
	bool m_isEnable;

private:
	CDataUnit m_dataUnit[8];
	int m_dataCount;

};

