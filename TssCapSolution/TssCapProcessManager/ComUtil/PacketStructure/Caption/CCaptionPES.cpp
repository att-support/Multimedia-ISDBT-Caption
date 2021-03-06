//#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "CCaptionPES.h"
#include "CCaptionData.h"
#include "CCaptionManagementData.h"
#include "CDataUnit.h"
//#include "C8bitCharCode.h"

CCaptionPES::CCaptionPES()
{
}

CCaptionPES::~CCaptionPES(void)
{
}

// データグループID取得
unsigned char CCaptionPES::GetDataGroupId()
{
	return m_DataGroup.GetDataGroupId();
}
// データグループID設定
void CCaptionPES::SetDataGroupId(unsigned char id)
{
	m_DataGroup.SetDataGroupId(id);
}
// データグループバイト取得
unsigned char* CCaptionPES::GetDataGroupByte()
{
	return m_DataGroup.GetDataGroupByte();
}
// CRC16取得
unsigned short CCaptionPES::GetCRC16()
{
	return m_DataGroup.GetCRC16();
}
// データグループヘッダー長取得
unsigned int CCaptionPES::GetHeaderLen()
{
	return (GetPESHeaderLen() + m_DataGroup.GetHeaderLen());
}
#if 0
// ログ出力設定
void CCaptionPES::SetLogger(CMyApi* pMyApi, int level, char* chProcess, int iOutputValue)
{
	m_pMyApi = pMyApi;
	m_Log_level = level;
	m_Log_pProcess = chProcess;
	m_Log_OuputValue = iOutputValue;
}
#endif

#if 0
// ログ出力
void CCaptionPES::OutputLog( int iService, unsigned int limitPESPayloadSize )
{
	if ( !IsEnable() ) {
		return;
	}
	// グループID取得
	unsigned char data_group_id = GetDataGroupId();
	// 管理データ
	if ( (data_group_id & ~0x20) == 0x00 ) {
		OutputLogManagementData( iService );
	}
	// 字幕文データ
	else {
		OutputLogPageData( iService, limitPESPayloadSize );
	}
}
#endif

#if 0
// ログ出力(字幕文データ)
void CCaptionPES::OutputLogPageData( int iService, unsigned int limitPESPayloadSize )
{
	char logbuf[512] = {0};
	// グループID取得
	unsigned char data_group_id = GetDataGroupId();
	// 組
	char ansClsStr[2] = {0};
	sprintf_s(ansClsStr, sizeof(ansClsStr), ((data_group_id&0x20)?"B":"A"));
	// 字幕文データインスタンス生成
	CCaptionData capData(GetDataGroupByte());
	sprintf_s( logbuf,sizeof(logbuf), "SV[%s] %s PAGE DGI=0x%02x TMD=0x%x STM=%d%d%d%d%d%d%d%d%d",
		((iService==0)?"HD":"MB"), ansClsStr, data_group_id, capData.m_TMD,
		capData.m_STM[0],capData.m_STM[1],capData.m_STM[2],capData.m_STM[3],capData.m_STM[4],
		capData.m_STM[5],capData.m_STM[6],capData.m_STM[7],capData.m_STM[8] );
	_Log(logbuf);
	unsigned char* pDataUnit = capData.m_pData_unit;
	unsigned long unit_loop_len = capData.m_data_unit_loop_length;
	if ( limitPESPayloadSize > 0 ) {
		if ( limitPESPayloadSize < capData.GetHeaderLen() ) {
			return;
		}
		limitPESPayloadSize -= capData.GetHeaderLen();
		if ( unit_loop_len > limitPESPayloadSize ) {
			unit_loop_len = limitPESPayloadSize;
		}
	}
	// 最大何個かわからないけどとりあえず10まで(無限ループ対策)
	int unitCountMax = 10;
	unsigned int dataUnitHeaderLen = CDataUnit::GetHeaderLen();
	for ( int unitCount = 1; unitCount <= unitCountMax; unitCount++ ) {
		if ( unit_loop_len < dataUnitHeaderLen ) {
			break;
		}
		// データユニット取得
		CDataUnit dataUnit(pDataUnit);
		if ( !dataUnit.IsEnable() ) {
			break;
		}
		if ( dataUnit.m_data_unit_parameter != 0x20 ) {
			char paramStr[128] = {0};
			switch (dataUnit.m_data_unit_parameter) {
			case 0x28 :
				sprintf_s(paramStr, sizeof(paramStr), "<ジオメトリック>");
				break;
			case 0x2c :
				sprintf_s(paramStr, sizeof(paramStr), "<付加音>");
				break;
			case 0x30 :
				sprintf_s(paramStr, sizeof(paramStr), "<1バイトDRSC>");
				break;
			case 0x31 :
				sprintf_s(paramStr, sizeof(paramStr), "<2バイトDRSC>");
				break;
			case 0x34 :
				sprintf_s(paramStr, sizeof(paramStr), "<カラーマップ>");
				break;
			case 0x35 :
				sprintf_s(paramStr, sizeof(paramStr), "<ビットマップ>");
				break;
			default:
				sprintf_s(paramStr, sizeof(paramStr), "<データユニット種別不明(0x%02x)>", dataUnit.m_data_unit_parameter);
				break;
			}
			sprintf_s( logbuf,sizeof(logbuf), "SV[%s] %s PAGE (%d) len=%03d %s",
				((iService==0)?"HD":"MB"), ansClsStr, unitCount, dataUnit.m_data_unit_size, paramStr );
			_Log(logbuf);
			break;
		}
		// 字幕データの内容を簡易表示用
		unsigned char unitByteSJIS[256] = {0};
		C8bitCharCode coder;
		// HD/SD1
		if ( iService == 0 ) {
			coder.LoadGL(0);
			coder.LoadGR(2);
		}
		// 携帯
		else {
			coder.SetCodeSet(1,0x41,true); // G1にDRCS-1を展開
			coder.LoadGL(1);
			coder.LoadGR(0);
		}

		unsigned int dataUnitSize = dataUnit.m_data_unit_size + dataUnitHeaderLen;
		int srcSize;
		if( dataUnitSize > unit_loop_len ) {
			srcSize = unit_loop_len - dataUnitHeaderLen;
		}
		else {
			srcSize = dataUnit.m_data_unit_size;
		}
		coder.Decode(dataUnit.m_pData_unit_data_byte, srcSize, unitByteSJIS, sizeof(unitByteSJIS));

		// ログ出力
		sprintf_s( logbuf,sizeof(logbuf), "SV[%s] %s PAGE (%d) len=%03d <本文> %s",
			((iService==0)?"HD":"MB"), ansClsStr, unitCount, dataUnit.m_data_unit_size, unitByteSJIS );
		_Log(logbuf);
		// データ長減算
		if ( unit_loop_len >= dataUnitSize ) {
			unit_loop_len -= dataUnitSize;
		}
		else {
			unit_loop_len = 0;
		}
		if ( unit_loop_len == 0 ) {
			break;
		}
		// 次のデータユニット
		pDataUnit += dataUnitSize;
	}
}
#endif

#if 0
// ログ出力(管理データ)
void CCaptionPES::OutputLogManagementData( int iService )
{
	char logbuf[256] = {0};
	// グループID取得
	unsigned char data_group_id = GetDataGroupId();
	// 組
	char ansClsStr[2] = {0};
	sprintf_s(ansClsStr, sizeof(ansClsStr), ((data_group_id&0x20)?"B":"A"));

	CCaptionManagementData capMngData(GetDataGroupByte());
	sprintf_s(logbuf,sizeof(logbuf),"SV[%s] %s MANT DGI=0x%02x TMD=0x%x OTM=%d%d%d%d%d%d%d%d%d num_languages=%d",
		((iService==0)?"HD":"MB"),ansClsStr,data_group_id,
		capMngData.m_TMD, 
		capMngData.m_OTM[0],capMngData.m_OTM[1],capMngData.m_OTM[2],capMngData.m_OTM[3],capMngData.m_OTM[4],
		capMngData.m_OTM[5],capMngData.m_OTM[6],capMngData.m_OTM[7],capMngData.m_OTM[8],
		capMngData.m_num_languages );
	_Log(logbuf);

	for ( int langCount = 0; langCount < capMngData.m_num_languages; langCount++ ) {
		sprintf_s(logbuf,sizeof(logbuf),"SV[%s] %s MANT (%d) 第%d言語 DMF=0x%x DC=0x%02x code='%s' Format=0x%x TSC=0x%x rollup_mode=0x%x",
			((iService==0)?"HD":"MB"), ansClsStr, (langCount+1),
			capMngData.m_lang[langCount].language_tag + 1,
			capMngData.m_lang[langCount].DMF,
			capMngData.m_lang[langCount].DC,
			capMngData.m_lang[langCount].ISO_639_language_code,
			capMngData.m_lang[langCount].Format,
			capMngData.m_lang[langCount].TCS,
			capMngData.m_lang[langCount].rollup_mode );
		_Log(logbuf);
	}
}
#endif

#if 0
// ログ出力
void CCaptionPES::_Log(char* logMsg)
{
	if ( m_pMyApi != NULL ) {
		m_pMyApi->ApiLogMessage(m_Log_level,m_Log_pProcess,logMsg,m_Log_OuputValue);
	}
}
#endif

int CCaptionPES::Decode(unsigned char* pData)
{
	int ret = 0;
	unsigned char* ptr = NULL;

	ret = CPrivatePES::Decode(pData);
	if (ret < 0) {
		return ret;
	}

	ptr = GetPESDataByte();
	if (ptr == NULL) {
		return -1;
	}

	m_DataGroup.SetData(ptr);

	return 0;
}

int CCaptionPES::GetUnitCount()
{ 
	int iCount = 0;

	if ((GetDataGroupId() == DATA_GRP_MNG_A) || ((GetDataGroupId() == DATA_GRP_MNG_B))) {
		iCount = m_DataGroup.getManagementData()->GetDataUnitCount();
	}
	else {
		iCount = m_DataGroup.getCaptionData()->GetDataUnitCount();
	}

	return iCount;
}


unsigned char* CCaptionPES::GetUnitData(int index)
{ 
	unsigned char* pBuffer = NULL;

	if ((GetDataGroupId() == DATA_GRP_MNG_A) || ((GetDataGroupId() == DATA_GRP_MNG_B))) {
		pBuffer = m_DataGroup.getManagementData()->GetDataUnitBuffer(index);
	}
	else {
		pBuffer = m_DataGroup.getCaptionData()->GetDataUnitBuffer(index);
	}

	return pBuffer;
}

int CCaptionPES::GetUnitLength(int index)
{ 
	int pBufLen = 0;

	if ((GetDataGroupId() == DATA_GRP_MNG_A) || ((GetDataGroupId() == DATA_GRP_MNG_B))) {
		pBufLen = m_DataGroup.getManagementData()->GetDataUnitLength(index);
	}
	else {
		pBufLen = m_DataGroup.getCaptionData()->GetDataUnitLength(index);
	}

	return pBufLen;
}

int CCaptionPES::GetUnitType(int index)
{
	int pBufType = 0;

	if ((GetDataGroupId() == DATA_GRP_MNG_A) || ((GetDataGroupId() == DATA_GRP_MNG_B))) {
		pBufType = m_DataGroup.getManagementData()->GetDataUnitType(index);
	}
	else {
		pBufType = m_DataGroup.getCaptionData()->GetDataUnitType(index);
	}

	return pBufType;
}




