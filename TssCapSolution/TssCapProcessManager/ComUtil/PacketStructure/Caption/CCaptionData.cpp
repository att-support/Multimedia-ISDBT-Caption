//#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "CCaptionData.h"

CCaptionData::CCaptionData()
{
	m_TMD = 0;
	/* m_STM[9] */
	m_data_unit_loop_length=0;
	m_pData_unit=NULL;
	m_headerLen = 0;
	m_isEnable=false;
	m_dataCount = 0;
}

CCaptionData::~CCaptionData()
{
}

bool CCaptionData::IsEnable()
{
	return m_isEnable;
}

unsigned int CCaptionData::GetHeaderLen()
{
	return m_headerLen;
}

int CCaptionData::Decode(unsigned char* pData)
{
	if (pData == NULL) {
		return -1;
	}
	m_TMD = (unsigned char)(*pData >> 6) & 0x03;
	pData++;

	// オフセットタイムの場合
	if ((m_TMD == 0x01) || (m_TMD == 0x02)) {
		m_STM[0] = (unsigned char)(*pData >> 4);
		m_STM[1] = (unsigned char)(*pData & 0x0F);
		pData++;
		m_STM[2] = (unsigned char)(*pData >> 4);
		m_STM[3] = (unsigned char)(*pData & 0x0F);
		pData++;
		m_STM[4] = (unsigned char)(*pData >> 4);
		m_STM[5] = (unsigned char)(*pData & 0x0F);
		pData++;
		m_STM[6] = (unsigned char)(*pData >> 4);
		m_STM[7] = (unsigned char)(*pData & 0x0F);
		pData++;
		m_STM[8] = (unsigned char)(*pData >> 4);
		pData++;

		m_headerLen = 8;
	}
	else {
		memset(m_STM, 0x00, sizeof(m_STM));

		m_headerLen = 4;
	}
	m_data_unit_loop_length =
		(unsigned long)(((unsigned long)(*(pData + 0)) << (2 * 8))
			| ((unsigned long)(*(pData + 1)) << (1 * 8))
			| ((unsigned long)(*(pData + 2)) << (0 * 8)));

	pData += 3;

	m_pData_unit = pData;
	m_isEnable = true;

	int i = 0;
//	CDataUnit dataUnit;
	int unit_type = 0;
	unsigned char* pUnitBuffer = NULL;
	int iUnitLength = 0;
	int pointer = 0;
	m_dataCount = 0;
	for (i = 0; i < (int)m_data_unit_loop_length;) {
		m_dataUnit[m_dataCount].Decode(m_pData_unit + pointer);

		unit_type = m_dataUnit[m_dataCount].GetUnitParameter();
		if (m_dataUnit[m_dataCount].GetUnitSeparator() != 0x1F) {
			break;
		}
		if (unit_type == 0x20) {
			pUnitBuffer = m_dataUnit[m_dataCount].GetDataBuffer();
			iUnitLength = m_dataUnit[m_dataCount].GetDataLength();
		}
		else {
			pUnitBuffer = m_dataUnit[m_dataCount].GetDataBuffer();
			iUnitLength = m_dataUnit[m_dataCount].GetDataLength();
		}
		pointer += (5+iUnitLength);
		i += (5 + iUnitLength);
		m_dataCount++;
	}

	return 0;
}