//#######################################################################
// CloclTime.h
//#######################################################################
#ifndef TSSTIME_CLOCKTIME_H_
#define TSSTIME_CLOCKTIME_H_

//**************************************************
// include
//**************************************************
#include <string>
#include <Util/SysTime.h>
#include <Util/Thread.h>
//#include "MuxOutDefine.h"
#include <chrono>

//**************************************************
// define
//**************************************************

//
// 初期値=0
// 初期時刻(ミリ秒精度)
// オフセット値(外部PCR設定)
// 一時停止でタイムスタンプ保持＆再開
// →ただし初期時刻は再開時刻とする
//
// INITVALUE = IN(t)
// STARTTIME = ST(t)
// 現在時刻  = NT(t)
// TIMESTAMP = IN(t) + (NT(t)-ST(t))
// 
// 起動時 IN(t)=0 ST(t)セット
// 同期時 IN(t)=X ST(t)セット
// 停止時 タイムスタンプ更新停止=Y
// 再開時 IN(t)=Y ST(t)セット
//

// -----------------------------------------------
// ClockBase
// -----------------------------------------------
class ClockTime : public Thread
{
public:
	ClockTime(int id);
	virtual ~ClockTime();
	void Init();
	void Reset();
	int ThreadStop();

public:
	int m_id;
	unsigned long long m_TimeStamp;

protected:
	std::string m_name;
	std::string m_nameForLog;
	int m_UpdateStop;
	int m_ThreadStop;
	virtual int ThreadProc();
	virtual int ThreadInitProc();
	virtual int ThreadTermProc();
	std::chrono::system_clock::time_point  m_start_time;
	int m_time_flag;
	int m_iFirstFlag;

public:
	void Update(int msec);
	void UpdateStop(int flag);
	unsigned long long GetTimeStamp();
	unsigned long long GetTimeStampSTC();
	void InputTimeStampSTC(unsigned long long inputSTC);
	int CheckTimeStamp(unsigned long long displaySTC, int delta);
};

#endif//  TSSTIME_CLOCKTIME_H_

