#pragma once

#include "CCaptionManagementData.h"
#include "CCaptionData.h"

// データグループID
#define DATA_GRP_MNG_A				0x00
#define DATA_GRP_TXT_LANG1_A		0x01
#define DATA_GRP_TXT_LANG2_A		0x02
#define DATA_GRP_TXT_LANG3_A		0x03
#define DATA_GRP_TXT_LANG4_A		0x04
#define DATA_GRP_TXT_LANG5_A		0x05
#define DATA_GRP_TXT_LANG6_A		0x06
#define DATA_GRP_TXT_LANG7_A		0x07
#define DATA_GRP_TXT_LANG8_A		0x08
#define DATA_GRP_MNG_B				(0x20 | DATA_GRP_MNG_A)
#define DATA_GRP_TXT_LANG1_B		(0x20 | DATA_GRP_TXT_LANG1_A)
#define DATA_GRP_TXT_LANG2_B		(0x20 | DATA_GRP_TXT_LANG2_A)
#define DATA_GRP_TXT_LANG3_B		(0x20 | DATA_GRP_TXT_LANG3_A)
#define DATA_GRP_TXT_LANG4_B		(0x20 | DATA_GRP_TXT_LANG4_A)
#define DATA_GRP_TXT_LANG5_B		(0x20 | DATA_GRP_TXT_LANG5_A)
#define DATA_GRP_TXT_LANG6_B		(0x20 | DATA_GRP_TXT_LANG6_A)
#define DATA_GRP_TXT_LANG7_B		(0x20 | DATA_GRP_TXT_LANG7_A)
#define DATA_GRP_TXT_LANG8_B		(0x20 | DATA_GRP_TXT_LANG8_A)

//
// データグループクラス
//
class CDataGroup
{
public:
	CDataGroup();
//	CDataGroup(unsigned char* pData);
	int Decode(unsigned char* pData);
	virtual ~CDataGroup();
	// データ設定
	void SetData(unsigned char* pData);
	// データグループID取得
	unsigned char GetDataGroupId();
	// データグループID設定
	void SetDataGroupId(unsigned char id);
	// CRC16再計算
	void CalcCRC16();
	// データグループバイト取得
	unsigned char* GetDataGroupByte();
	// CRC16取得
	unsigned short GetCRC16();
	// データ有効検査
	bool IsEnable();
	// データグループヘッダー長取得
	unsigned int GetHeaderLen();

	CCaptionData* getCaptionData() { return &m_capData; };
	CCaptionManagementData* getManagementData(){ return &m_capManager; };

private:
	void _SetData();

	unsigned char* m_pDataTop;
	unsigned char m_data_group_id;
	unsigned char m_group_version;
	unsigned char m_data_group_link_number;
	unsigned char m_last_data_group_link_number;
	unsigned short m_data_group_size;
	unsigned char* m_pData_group_byte;
	unsigned short m_CRC_16;
	unsigned int m_headerLen;
	bool m_isEnable;

	CCaptionManagementData m_capManager;
	CCaptionData m_capData;
};

