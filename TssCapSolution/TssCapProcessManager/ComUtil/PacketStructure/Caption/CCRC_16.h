#pragma once

//
// CRC_16
//
class CCRC_16
{
public:
	CCRC_16();
	virtual ~CCRC_16();
	static unsigned short Calc(unsigned char *buf,unsigned long length);
};


