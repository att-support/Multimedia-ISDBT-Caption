#pragma once

#include "CPrivatePES.h"
#include "CDataGroup.h"
//#include "MyApi.h"

//
// 字幕PESクラス
//
class CCaptionPES : public CPrivatePES
{
public:
//	CCaptionPES(unsigned char* pData);
	CCaptionPES(void);
	virtual ~CCaptionPES(void);

	int Decode(unsigned char* pData);

	// データグループID取得
	unsigned char GetDataGroupId();
	// データグループID設定
	void SetDataGroupId(unsigned char id);
	// データグループバイト取得
	unsigned char* GetDataGroupByte();
	// CRC16取得
	unsigned short GetCRC16();
	// データグループヘッダー長取得
	unsigned int GetHeaderLen();
	// ログ出力設定
//	void SetLogger(CMyApi* pMyApi, int level, char* chProcess, int iOutputValue);
	// ログ出力 service: 0=HD/SD1, 1=携帯
	void OutputLog( int iService, unsigned int limitPESPayloadSize=0 );

	int GetUnitCount();
	unsigned char* GetUnitData(int index);
	int GetUnitLength(int index);
	int GetUnitType(int index);

private:
//	void OutputLogPageData( int iService, unsigned int limitPESPayloadSize );
//	void OutputLogManagementData( int iService );
//	void _Log(char* logMsg);


	CDataGroup m_DataGroup;
//	CMyApi* m_pMyApi;
//	int m_Log_level;
//	char* m_Log_pProcess;
//	int m_Log_OuputValue;
};

