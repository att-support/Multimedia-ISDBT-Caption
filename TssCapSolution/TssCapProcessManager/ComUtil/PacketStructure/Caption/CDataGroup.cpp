//#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "CDataGroup.h"
#include "CCRC_16.h"

CDataGroup::CDataGroup(void)
{
	m_pDataTop = NULL;
	m_data_group_id=0xFF;
	m_group_version = 0xFF;
	m_data_group_link_number = 0xFF;
	m_last_data_group_link_number = 0xFF;
	m_data_group_size = 0;
	m_pData_group_byte=NULL;
	m_CRC_16=0xFFFF;
	m_isEnable=false;
}

CDataGroup::~CDataGroup(void)
{
}

int CDataGroup::Decode(unsigned char* pData)
{
	m_pDataTop = pData;
	m_data_group_id = 0xFF;
	m_group_version = 0xFF;
	m_data_group_link_number = 0xFF;
	m_last_data_group_link_number = 0xFF;
	m_data_group_size = 0;
	m_pData_group_byte = NULL;
	m_CRC_16 = 0xFFFF;
	m_isEnable = false;
	_SetData();

	return 0;
}

// データ設定
void CDataGroup::SetData(unsigned char* pData)
{
	m_pDataTop = pData;
	_SetData();
}

// データ設定
void CDataGroup::_SetData()
{
	if( m_pDataTop == NULL ){
		return;
	}
	unsigned char* ptr = m_pDataTop;
	m_data_group_id = (*ptr >> 2) & 0x3F;

	if ((m_data_group_id == DATA_GRP_MNG_A) || ((m_data_group_id == DATA_GRP_MNG_B))) {
		int aaa = 0;
	}
	else {
		int bbb = 0;
	}

	m_group_version = *ptr & 0x03;
	ptr++;
	m_data_group_link_number = *ptr;
	ptr++;
	m_last_data_group_link_number = *ptr;
	ptr++;
	m_data_group_size = (unsigned short)(((unsigned short)*ptr << 8) & 0xFF00) | (unsigned short)*(ptr + 1);
	ptr += 2;
	m_headerLen = 5;
	m_pData_group_byte = ptr;
	ptr += m_data_group_size;
	m_CRC_16 = (unsigned short)(((unsigned short)*ptr << 8) & 0xFF00) | (unsigned short)*(ptr + 1);
	{
		int group_id = m_data_group_id & 0x0F;
		if ((group_id== DATA_GRP_MNG_A)||(group_id== DATA_GRP_MNG_B)) {
			m_capManager.Decode(m_pData_group_byte);
		}
		else {
			m_capData.Decode(m_pData_group_byte);
		}
	}
	m_isEnable = true;
}

// データグループID取得
unsigned char CDataGroup::GetDataGroupId()
{
	return m_data_group_id;
}
// データグループID設定
void CDataGroup::SetDataGroupId(unsigned char id)
{
	m_data_group_id = id;
	if( m_pDataTop == NULL ) {
		return;
	}
	unsigned char* ptr = m_pDataTop;
	*ptr = (unsigned char)((*ptr & 0x03) | ((m_data_group_id << 2) & ~0x03));
	// CRC_16再計算
	CalcCRC16();
}

// CRC16再計算
void CDataGroup::CalcCRC16()
{
	unsigned long len = (m_data_group_size + 5);
	m_CRC_16 = CCRC_16::Calc(m_pDataTop, len );
	if( m_pDataTop == NULL ) {
		return;
	}
	unsigned char* ptr = m_pDataTop;
	ptr += len;
	*ptr = (unsigned char)(m_CRC_16 >> 8);
	ptr++;
	*ptr = (unsigned char)(m_CRC_16 & 0x00FF);
}
// データグループバイト取得
unsigned char* CDataGroup::GetDataGroupByte()
{
	return m_pData_group_byte;
}
// CRC16取得
unsigned short CDataGroup::GetCRC16()
{
	return m_CRC_16;
}

bool CDataGroup::IsEnable()
{
	return m_isEnable;
}

unsigned int CDataGroup::GetHeaderLen()
{
	return m_headerLen;
}

