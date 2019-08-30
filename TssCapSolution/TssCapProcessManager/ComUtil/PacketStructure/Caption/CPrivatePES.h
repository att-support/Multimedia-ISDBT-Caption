#pragma once

//
// 独立PESクラス
//
class CPrivatePES
{
public:
//	CPrivatePES(unsigned char* pData);
	CPrivatePES(void);
	virtual ~CPrivatePES(void);

	int Decode(unsigned char* pData);

	// PESペイロード取得
	unsigned char* GetPayload();
	// PESデータバイト取得
	unsigned char* GetPESDataByte();
	// データ有効検査
	bool IsEnable();
	// PESヘッダー長取得
	unsigned int GetPESHeaderLen();
	// STC
	unsigned long long GetTimestamp();

protected:
	unsigned char* m_pData;
	unsigned char* m_pPayload;
	unsigned char* m_pPESDataByte;
	unsigned int m_headerLen;
	unsigned long long m_i64Timestamp;
	bool m_isEnable;

};

