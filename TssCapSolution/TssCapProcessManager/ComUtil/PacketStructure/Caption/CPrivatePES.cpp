//#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include "CPrivatePES.h"


CPrivatePES::CPrivatePES()
{
	m_pData = NULL;
	m_pPayload = NULL;
	m_pPESDataByte = NULL;
	m_headerLen = 0;
	m_isEnable = false;
	m_i64Timestamp = 0;
}

CPrivatePES::~CPrivatePES(void)
{
}

// PESペイロード取得
unsigned char* CPrivatePES::GetPayload()
{
	return m_pPayload;
}

unsigned char* CPrivatePES::GetPESDataByte()
{
	return m_pPESDataByte;
}

bool CPrivatePES::IsEnable()
{
	return m_isEnable;
}

unsigned int CPrivatePES::GetPESHeaderLen()
{
	return m_headerLen;
}

unsigned long long CPrivatePES::GetTimestamp()
{
	return m_i64Timestamp;
}

int CPrivatePES::Decode(unsigned char* pData)
{
	int stream_id = 0;
	unsigned char* ptr = NULL;
	unsigned char hdLen = 0;
	unsigned char PES_data_packet_header_length = 0;
	int pts_dts_flag = 0;
	unsigned char sTemp = 0;

	if (pData == NULL) {
		return -1;
	}
	m_pData = pData;
	ptr = m_pData;

	// packet_start_code_prefix
	if ((*ptr) != 0x00) { return -1; } ptr++;
	if ((*ptr) != 0x00) { return -1; } ptr++;
	if ((*ptr) != 0x01) { return -1; } ptr++;
	// stream_id
	stream_id = *ptr;
	if ((stream_id != 0xBD)&&(stream_id != 0xBF)) {
		// error
		return -1;
	}
	ptr += 1;
	// PES_packet_length
	ptr += 2;
	if(stream_id== 0xBD){
		// header1
		ptr++;
		// header2
		pts_dts_flag = ((*ptr)>>6)&0x03;
		ptr++;
		// PES_header_data_length
		hdLen = *ptr;
		ptr++;

		if (pts_dts_flag==2) {
			//hdLen += 9;
			m_i64Timestamp = 0;
			sTemp = *ptr;
			m_i64Timestamp |= ((((unsigned long long)((sTemp >> 1) & 0x07)) << 30) & 0x00000001C0000000); //0x00000001C0000000
			sTemp = *(ptr+1);
			m_i64Timestamp |= ((((unsigned long long)((sTemp >> 0) & 0xFF)) << 22) & 0x000000003FC00000); //0x000000003FC00000
			sTemp = *(ptr+2);
			m_i64Timestamp |= ((((unsigned long long)((sTemp >> 1) & 0x7F)) << 15) & 0x00000000003F8000); //0x00000000003F8000
			sTemp = *(ptr+3);
			m_i64Timestamp |= ((((unsigned long long)((sTemp >> 0) & 0xFF)) << 7 ) & 0x0000000000007F80); //0x0000000000007F80
			sTemp = *(ptr+4);
			m_i64Timestamp |= ((((unsigned long long)((sTemp >> 1) & 0x7F)) << 0 ) & 0x000000000000007F); //0x000000000000007F
		}
		else {
			m_i64Timestamp = 0;
		}

		m_headerLen = hdLen + 9;
		ptr += hdLen;
		m_pPayload = ptr;
	}
	else{
		m_headerLen = 6;
		m_pPayload = ptr;
	}

	// data_identifier					:8bit
	ptr++;
	// private_stream_id				:8bit
	ptr++;
	// reserved_future_use				:4bit
	// PES_data_packet_header_length	:4bit
	PES_data_packet_header_length = (*ptr & 0x0F);
	ptr++;

	if (PES_data_packet_header_length != 0x00) {
		ptr += PES_data_packet_header_length;
	}
	m_headerLen += (3 + PES_data_packet_header_length);
	m_pPESDataByte = ptr;
	m_isEnable = true;

	return 0;
}

