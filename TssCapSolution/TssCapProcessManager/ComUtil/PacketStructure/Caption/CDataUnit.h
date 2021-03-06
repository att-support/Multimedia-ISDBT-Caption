#pragma once

// データユニットの種類
#define DUTYP_STATEMENT_BODY		0x20 //本文
#define DUTYP_GEOMETRIC				0x28 //ジオメトリック
#define DUTYP_SYNTHESIZED_SOUND		0x2C //付加音
#define DUTYP_DRCS_1BYTE			0x30 //１バイトDRCS
#define DUTYP_DRCS_2BYTE			0x31 //２バイトDRCS
#define DUTYP_COLOR_MAP				0x34 //カラーマップ
#define DUTYP_BIT_MAP				0x35 //ビットマップ

//
// データユニットクラス
//
class CDataUnit
{
public:
//	CDataUnit(unsigned char* pData);
	CDataUnit(void);
	virtual ~CDataUnit();
	int Decode(unsigned char* pData);
	// データ有効検査
	bool IsEnable();
	static unsigned int GetHeaderLen();
	int GetUnitSeparator() { return m_unit_separator; };
	int GetUnitParameter() { return m_data_unit_parameter; };
	unsigned char* GetUnitData() { return m_pData_unit_data_byte; };
	int GetUnitSize() { return m_data_unit_size; };
	unsigned char* GetDataBuffer() { return m_pDataBuffer; };
	int GetDataLength() { return m_iDataLength; };

private:
	unsigned char m_unit_separator;
	unsigned char m_data_unit_parameter;
	unsigned long m_data_unit_size;
	unsigned char* m_pData_unit_data_byte;
	bool m_isEnable;

private:
	unsigned char* m_pDataBuffer;
	int m_iDataLength;

};

