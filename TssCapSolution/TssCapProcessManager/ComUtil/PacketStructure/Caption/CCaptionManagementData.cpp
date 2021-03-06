//#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "CCaptionManagementData.h"

CCaptionManagementData::CCaptionManagementData()
{	
	m_TMD = 0;
	/* m_OTM[9] */
	m_num_languages = 0;
	/* m_lang[8] */
	m_data_unit_loop_length = 0;
	m_pData_unit = NULL;
	m_isEnable = false;
}

CCaptionManagementData::~CCaptionManagementData()
{
}

bool CCaptionManagementData::IsEnable()
{
	return m_isEnable;
}

int CCaptionManagementData::Decode(unsigned char* pData)
{
	if (pData == NULL) {
		return -1;
	}
	m_TMD = (unsigned char)(*pData >> 6) & 0x03;
	pData++;

	// オフセットタイムの場合
	if (m_TMD == 0x02) {
		m_OTM[0] = (unsigned char)(*pData >> 4);
		m_OTM[1] = (unsigned char)(*pData & 0x0F);
		pData++;
		m_OTM[2] = (unsigned char)(*pData >> 4);
		m_OTM[3] = (unsigned char)(*pData & 0x0F);
		pData++;
		m_OTM[4] = (unsigned char)(*pData >> 4);
		m_OTM[5] = (unsigned char)(*pData & 0x0F);
		pData++;
		m_OTM[6] = (unsigned char)(*pData >> 4);
		m_OTM[7] = (unsigned char)(*pData & 0x0F);
		pData++;
		m_OTM[8] = (unsigned char)(*pData >> 4);
		pData++;
	}
	else {
		memset(m_OTM, 0x00, sizeof(m_OTM));
	}
	m_num_languages = *pData;
	pData++;

	for (int i = 0; i < m_num_languages; i++) {
		m_lang[i].language_tag = (unsigned char)(*pData >> 5);
		m_lang[i].DMF = (unsigned char)(*pData & 0x0F);
		pData++;

		if ((m_lang[i].DMF == 0x0C) || (m_lang[i].DMF == 0x0D) || (m_lang[i].DMF == 0x0E)) {
			m_lang[i].DC = *pData;
			pData++;
		}
		else {
			m_lang[i].DC = 0x00;
		}

		m_lang[i].ISO_639_language_code[0] = *pData;
		pData++;
		m_lang[i].ISO_639_language_code[1] = *pData;
		pData++;
		m_lang[i].ISO_639_language_code[2] = *pData;
		pData++;
		m_lang[i].ISO_639_language_code[3] = 0; // \0

		m_lang[i].Format = (unsigned char)(*pData >> 4);
		m_lang[i].TCS = (unsigned char)((*pData >> 2) & 0x03);
		m_lang[i].rollup_mode = (unsigned char)(*pData & 0x03);
		pData++;
	}
	m_data_unit_loop_length =
		(unsigned long)(((unsigned long)(*(pData + 0)) << (2 * 8))
			| ((unsigned long)(*(pData + 1)) << (1 * 8))
			| ((unsigned long)(*(pData + 2)) << (0 * 8)));

	pData += 3;

	m_pData_unit = pData;
	m_isEnable = true;

	int i = 0;
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
		pointer += (5 + iUnitLength);
		i += pointer;
		m_dataCount++;
	}

	return 0;
}
