//#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "CDataUnit.h"

CDataUnit::CDataUnit()
{
	m_unit_separator = 0;
	m_data_unit_parameter = 0;
	m_data_unit_size = 0;
	m_pData_unit_data_byte = NULL;
	m_isEnable = false;
	m_pDataBuffer = NULL;
	m_iDataLength = 0;
}

CDataUnit::~CDataUnit()
{
	if (m_pDataBuffer) {
		delete[] m_pDataBuffer;
	}
}

bool CDataUnit::IsEnable()
{
	return m_isEnable;
}

unsigned int CDataUnit::GetHeaderLen()
{
	return 5;
}

int CDataUnit::Decode(unsigned char* pData)
{
	if (pData == NULL) {
		return -1;
	}
	m_unit_separator = *pData;
	if (m_unit_separator != 0x1F) {
		return -1;
	}
	pData++;

	m_data_unit_parameter = *pData;
	pData++;

	m_data_unit_size = (unsigned long)(((unsigned long)(*(pData + 0)) << (2 * 8))
		| ((unsigned long)(*(pData + 1)) << (1 * 8))
		| ((unsigned long)(*(pData + 2)) << (0 * 8)));
	pData += 3;

	m_pData_unit_data_byte = pData;
	{
		if (m_pDataBuffer == NULL) {
			m_pDataBuffer = new unsigned char[m_data_unit_size+1];
			memset(m_pDataBuffer, NULL, m_data_unit_size+1);
			memcpy(m_pDataBuffer, m_pData_unit_data_byte, m_data_unit_size);
			m_iDataLength = m_data_unit_size;
		}
	}
	m_isEnable = true;

	return 0;
}

