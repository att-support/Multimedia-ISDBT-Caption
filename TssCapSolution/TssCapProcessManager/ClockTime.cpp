//#######################################################################
//## ClockTime.c
//#######################################################################

//**************************************************
// include
//**************************************************
#include <stdio.h>
#include <stdlib.h>
//#include <sys/time.h>
#include <App/AppLogger.h>
#include <time.h>
#include <memory.h>
#include <string>
#include <Util/SysTime.h>
#include "ClockTime.h"


//**************************************************
// define
//**************************************************
//#define TIMESHIFT_TIMESTAMP -9*60*60 (sec)
#define TIMESHIFT_TIMESTAMP -32400
//#define TIMESTAMP_SEC_COUNT 1000000
#define TIMESTAMP_SEC_COUNT 27000000
#define TIMESTAMP_UPDATE 10 //msec
#define TIMESTAMP_RANGE 10 //msec
#define TIMESTAMP_RANGE_DISPLAY 750 //msec
#define TIMESTAMP_DEBUG_LOG 0


//==================================================
// ClockTime
//==================================================

#define NTP_EPOCH            (86400U * (365U * 70U + 17U))

ClockTime::ClockTime(int id)
{
	char nameBuf[256];
	sprintf(nameBuf,"送出STC管理");
	m_name = std::string(nameBuf);
	m_nameForLog = "[" + m_name + "]";
	m_id = id;
	m_UpdateStop = 0;
	m_iFirstFlag = 0;

	Init();
}

ClockTime::~ClockTime()
{
	Reset();
}

void ClockTime::Init()
{
	m_TimeStamp = 0;
	m_ThreadStop=0;
	m_time_flag = 0;
}

void ClockTime::Reset()
{
	//ThreadStop();
	m_TimeStamp=0;
}

unsigned long long ClockTime::GetTimeStamp()
{
	return m_TimeStamp;
}

unsigned long long ClockTime::GetTimeStampSTC()
{
	unsigned long long i64Timestamp = 0;

	//42bitでマスク
	i64Timestamp = m_TimeStamp & 0x000003FFFFFFFFFF;

	return i64Timestamp;
}

void ClockTime::Update(int msec)
{
	unsigned long long i64TempstampDelta = 0;
	i64TempstampDelta = msec * 27000;
	std::chrono::system_clock::time_point  m_now_time;
	double diff_msec = 0;

	if (m_UpdateStop) {
		return;
	}

	if (m_time_flag == 0) {
		m_time_flag = 1;
		m_start_time = std::chrono::system_clock::now();
		m_now_time = std::chrono::system_clock::now();
		diff_msec = (double)std::chrono::duration_cast<std::chrono::milliseconds>(m_now_time - m_start_time).count();
	}
	else {
		m_now_time = std::chrono::system_clock::now();
		diff_msec = (double)std::chrono::duration_cast<std::chrono::milliseconds>(m_now_time - m_start_time).count();
		m_start_time = std::chrono::system_clock::now();
	}

	i64TempstampDelta = (int)(diff_msec * 27000);


	if (TIMESTAMP_DEBUG_LOG) {
		APPLOG("[CLK],INF,%d,ClockTime,Update(),#ST TIME=%016X STC=%016X DLT=%d", m_id, GetTimeStamp(), GetTimeStampSTC(), i64TempstampDelta);
	}

//	Lock();
	//Timestamp
	m_TimeStamp = m_TimeStamp + i64TempstampDelta;
//	Unlock();

	if (TIMESTAMP_DEBUG_LOG) {
		APPLOG("[CLK],INF,%d,ClockTime,Update(),#ST TIME=%016X STC=%016X", m_id, GetTimeStamp(), GetTimeStampSTC());
	}
}

void ClockTime::UpdateStop(int flag)
{
	if (flag==1) {
		m_UpdateStop = 1;
	}
	else {
		m_UpdateStop = 0;
	}
}

void ClockTime::InputTimeStampSTC(unsigned long long inputSTC)
{
	int iUpdateFlag = 0;
	unsigned long long uInputTimestamp = inputSTC;
	unsigned long long uNowTimestamp = 0;
	unsigned long long uTimestampRange = TIMESTAMP_RANGE*27000;
	unsigned long long uNowTimestampUp = 0;
	unsigned long long uNowTimestampDn = 0;

	uNowTimestamp = GetTimeStampSTC();

	uNowTimestampUp = uNowTimestamp + uTimestampRange;
	uNowTimestampDn = uNowTimestamp - uTimestampRange;

	//±10msecを超える場合
	if (uInputTimestamp < uNowTimestampDn) {
		iUpdateFlag++;
	}
	//±10msecを超える場合
	if (uInputTimestamp > uNowTimestampUp) {
		iUpdateFlag++;
	}

	//更新する場合
	if (iUpdateFlag) {
		UpdateStop(1);
		if (TIMESTAMP_DEBUG_LOG) {
			APPLOG("[CLK],INF,%d,ClockTime,InputSTC,#ST PREV=%016X AFT=%016X", m_id, uNowTimestamp, uInputTimestamp);
		}
	//	Lock();
		m_TimeStamp = uInputTimestamp;
	//	Unlock();
		UpdateStop(0);
	}

	m_iFirstFlag = 1;

	return;
}

int ClockTime::CheckTimeStamp(unsigned long long displaySTC, int delta)
{
	int iUpdateFlag = 0;
	int SabunTime = 0;
	unsigned long long uInputTimestamp = (displaySTC + delta);
	unsigned long long uNowTimestamp = 0;
	unsigned long long uTimestampRange = TIMESTAMP_RANGE_DISPLAY * 27000;
	unsigned long long uNowTimestampUp = 0;
	unsigned long long uNowTimestampDn = 0;

	uNowTimestamp = GetTimeStampSTC();

	uNowTimestampUp = uNowTimestamp + uTimestampRange;
	uNowTimestampDn = uNowTimestamp - uTimestampRange;
	if (uNowTimestamp < uTimestampRange) {
		uNowTimestampDn = 0;
	}

	//SabunTime = displaySTC - uNowTimestamp;
	//if (SabunTime < 0) {
	//	iUpdateFlag++;
	//}

	//表示範囲を超える場合
	//if (uInputTimestamp < uNowTimestampDn) {
	//	APPLOG("[CLK],INF,%d,ClockTime,InputSTC,#ST NOW=%016X >PTS=%016X [X]", m_id, uNowTimestamp, uInputTimestamp);
	//	iUpdateFlag++;
	//}
	//表示範囲を超える場合
	//if (uInputTimestamp > uNowTimestampUp) {
	//	APPLOG("[CLK],INF,%d,ClockTime,InputSTC,#ST NOW=%016X <PTS=%016X [X]", m_id, uNowTimestamp, uInputTimestamp);
	//	iUpdateFlag++;
	//}

	//if (m_iFirstFlag == 0) {
	//	iUpdateFlag++;
	//}

	if (iUpdateFlag) {
		//字幕を破棄
		return 1000;
	}

	//表示時刻を超えている場合
	if (uNowTimestamp >= uInputTimestamp) {
		//字幕を表示
		if (TIMESTAMP_DEBUG_LOG) {
			APPLOG("[CLK],INF,%d,ClockTime,InputSTC,#ST NOW=%016X>=PTS=%016X [O]", m_id, uNowTimestamp, uInputTimestamp);
		}
		return 100;
	}

	//字幕を待機
	if (TIMESTAMP_DEBUG_LOG) {
		APPLOG("[CLK],INF,%d,ClockTime,InputSTC,#ST NOW=%016X  PTS=%016X [-]", m_id, uNowTimestamp, uInputTimestamp);
	}
	return 0;
}


int ClockTime::ThreadStop(){

	m_ThreadStop = 1;
	return 0;

#if 0
	int iEndFlag = 0;
	int iCount = 0;

	do{
		if(m_ThreadStop==0){
			iEndFlag=0;
			break;
		}
		iCount++;
		if(iCount>1000){ //1000msec=1sec
			iEndFlag=1;
			break;
		}
		//usleep(1000); //1msec
		Sleep(1); //1msec
	}while(1);

	if(iEndFlag){
		return -1;
	}
	return 0;
#endif
}

int ClockTime::ThreadProc()
{
	int rv=0;
	int iIntervalMsec = TIMESTAMP_UPDATE; //10msec

	rv =  0;

	///初期化
	Init();

	//定期更新
	do{
		if(m_ThreadStop){
			m_ThreadStop = 0;
			return -1;
		}

		//-----------------------
		//タイムスタンプの更新
		//-----------------------
		Update(iIntervalMsec);

		Sleep(iIntervalMsec);

	}while(1);

	return 0; //スレッド終了
}

int ClockTime::ThreadInitProc() {
	if (TIMESTAMP_DEBUG_LOG) {
		APPLOG("[CLK],INF,%d,ClockTime,ThreadInitProc(),時刻スレッド開始(%s)", m_id, m_name.c_str());
	}
	return 0;
}

int ClockTime::ThreadTermProc() {
	if (TIMESTAMP_DEBUG_LOG) {
		APPLOG("[CLK],INF,%d,ClockTime,ThreadInitProc(),時刻スレッド終了(%s)", m_id, m_name.c_str());
	}
	return 0;
}
