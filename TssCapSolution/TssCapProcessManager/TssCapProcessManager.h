#pragma once

//============================================================
#define TSS_PROCESS_CAPTION 0
#define TSS_PROCESS_MMCTRL 1
//============================================================

#include <Util/Thread.h>
#include <Util/SysTime.h>
#include <map>
#include "ClockTime.h"

#if TSS_PROCESS_MMCTRL
#include "MmtMultimediaControl.h"
#pragma comment(lib, "Modules/MmtMultimediaControl.lib")
#endif

#include <vector>
#include "TssCapProcessManagerListenerIf.h"

//#ifdef TSS_CAP_PROCESS_MANAGER_EXPORTS
//
//#define TSS_CAP_PROCESS_MANAGER_API __declspec(dllexport)
//#else
//#define TSS_CAP_PROCESS_MANAGER_API __declspec(dllimport)
//#endif

using namespace std;

//TYPE
#define TSSLIBMAN_TYPE_CAPTION		   10
#define TSSLIBMAN_TYPE_MTMEDIA		   11

#define TSS_PROCESS_INPUT_COUNT 8

#define TSS_CAPTION_LENGTH 1024*10
#define TSS_CAPTION_LENGTH_MORE 1024*100
#define TSS_CAPTION_LENGTH_MORE90KU 1024*360

//class TSS_CAP_PROCESS_MANAGER_API TssCapProcessManager : public Thread
class TssCapProcessManager : public Thread
{
private:
	int m_id;
	int m_type;
	int m_number;
	int m_resolution;
	int m_streamid;
	int m_serviceid;
	unsigned long long m_timestamp;
	ClockTime* m_pClockTime;
	int m_busy;
	int m_release_flag;

public:
	TssCapProcessManager(std::string InitFile, std::string LogPath, int iLogLevel=0);
	~TssCapProcessManager();

	void InitPreview(std::string sLogPath);
	void ExitPreview();

	long Create(long number, long type, int resolution);
	long Release(long number, long type);

	void SetId(int param) {			m_id = param; };
	void SetType(int param) {		m_type = param; };
	void SetNumber(int param) {		m_number = param; };
	void SetResolution(int param) { m_resolution = param; };
	void SetStreamId(int param) {	m_streamid = param; };
	void SetServiceId(int param) {	m_serviceid = param; };

	int GetId() { return m_id; };
	int GetType() { return m_type; };
	int GetNumber() { return m_number; };

private:
	int m_ThreadFlag;
	int m_StartFlag;
	int ThreadProc();
	int ThreadInitProc();
	int ThreadTermProc();
public:
	void ThreadFlagStop() { m_ThreadFlag = 0; };

public:
	int SetParameterInputAdd(int pid, unsigned long type, int tag);
	int SetParameterInputClear();
	int SetParameterInputLang(int lang);
	int SetParameterOutput(unsigned char *pointer, int size);
	int GetParameterInputCount() {	return m_input_count; };

public:
	int m_UpadteOutputFlag;
	//int UpdateOutput();
	int m_DataTriggerFlag;
	int m_CaptionDataRomSound;
	int UpdateDataOutput();

public:
	int SetTimestamp(unsigned long long uTimestamp);
	void PauseStop();
	void PauseStart();
	int m_UpdateStop;
	void UpdateStop(int flag);

public:
	int PushTspData(unsigned char *pointer, int size);
	int CapturePesPacket(unsigned char *pointer, int size);
	int PushPesData(unsigned char *pointer, int size);
	int PushPesDataWithPts(unsigned char *pointer, int size, unsigned long long uTimestamp);
//	int ParsePesData(unsigned char *pointer, int size, int lang);
	int ParsePesData(unsigned char *pointer, int size, int lang, unsigned long long uTimestamp);

	int m_pDataSectionLength;
	int m_iGetStatus;
	int m_payloadMpuSequenceNumber;
	int m_mfuSubsampleNumber;
	int m_mfuLastSubsampleNumber;
	int m_mfuSubsampleDataType;
	int m_mfuSubsampleDataSize;
	SysTime m_Time;

	int AppendListener(ITssCapCaptureListener* pListener);	//リスナー登録追加  
	int RemoveListener(ITssCapCaptureListener* pListener);	//リスナー登録削除
	int GetListenerCount() { return(int) m_vecListener.size(); };

	int EventError(unsigned long ErrorCode);
	int EventUpdateCaptionPlane();

	std::string GetVbsPath() { return m_sVbsPath; };

	unsigned long long GetTimestamp();

private:
	std::string m_sNumber;
	std::string m_sInputDir;
	std::string m_sInputTriggerFile;
	std::string m_sInputPageZipFile;
	std::string m_sInputTempDir;
	std::string m_sInputTempPageDir;
	std::string m_sInputTempTriggerFile;
	std::string m_sInputTempPageZipFile;
	std::string m_sInputTempCommandFile;
	std::string m_sOutputDir;
	std::string m_sOutputTriggerFile;
	std::string m_sOutputImageFile;
	std::string m_sVbsPath;

private:
	int m_input_pid[TSS_PROCESS_INPUT_COUNT];
	unsigned long m_input_type[TSS_PROCESS_INPUT_COUNT];
	int m_input_tag[TSS_PROCESS_INPUT_COUNT];
	int m_input_cc[TSS_PROCESS_INPUT_COUNT];
	int m_input_lang;
	int m_input_count;
	unsigned char *m_pOutputPointer;
	int m_output_size;
	unsigned char *m_pOutInteface;
	int m_out_inteface_size;
	int m_status;
	int m_output_count;
	int m_OutputImageFileType;
	int m_DemoMode;
	//std::map<unsigned char*, unsigned long long> m_fifoCaption;	//字幕メモリップ
	std::map<unsigned long long, unsigned char*> m_fifoCaption;	//字幕メモリップ
	unsigned char *m_pOutInteface2;
	int m_out_inteface_size2;

private:
	typedef std::vector<ITssCapCaptureListener*> LISTENER_VECTOR;
	LISTENER_VECTOR	m_vecListener;
	std::string m_INI_FILENAME;
	int m_LogLevel;
	std::string m_LogPath;
	std::string m_FileSavePath;
	std::string m_ConvertLogPath;

private:
	int m_iCapPesFlag;
	int PesBufferClear();
	int PesBufferInit(int size);
	int PesBufferSet(unsigned char* pdata, int size);
	unsigned char* m_pPesBuffer;
	int m_iPesBufferLength;
	int m_iPesTargetLength;
	unsigned char* GetPesBuffer() { return m_pPesBuffer; };
	int GetPesLength() { return m_iPesBufferLength; };
	int m_iPesCount;
	int m_iDgCount;

	unsigned short m_ManLastCrc16;
	unsigned char m_ManUnitBuffer0[TSS_CAPTION_LENGTH];
	unsigned char m_ManUnitBuffer1[TSS_CAPTION_LENGTH_MORE]; //DRCS1
	unsigned char m_ManUnitBuffer2[TSS_CAPTION_LENGTH_MORE]; //DRCS2
	int m_iManUnitLength0;
	int m_iManUnitLength1;
	int m_iManUnitLength2;
	int m_iManUnitType0;
	int m_iManUnitType1;
	int m_iManUnitType2;

	unsigned char m_CapUnitBuffer0[TSS_CAPTION_LENGTH_MORE];
	unsigned char m_CapUnitBuffer1[TSS_CAPTION_LENGTH];
	unsigned char m_CapUnitBuffer2[TSS_CAPTION_LENGTH];
	unsigned char m_CapUnitBuffer3[TSS_CAPTION_LENGTH];
	unsigned char m_CapUnitBuffer4[TSS_CAPTION_LENGTH];
	unsigned char m_CapUnitBuffer5[TSS_CAPTION_LENGTH_MORE90KU];
	int m_iCapUnitLength0;
	int m_iCapUnitLength1;
	int m_iCapUnitLength2;
	int m_iCapUnitLength3;
	int m_iCapUnitLength4;
	int m_iCapUnitLength5;
	int m_iCapUnitType0;
	int m_iCapUnitType1;
	int m_iCapUnitType2;
	int m_iCapUnitType3;
	int m_iCapUnitType4;
	int m_iCapUnitType5;
	unsigned char m_CapUnitBuffer0_Man[TSS_CAPTION_LENGTH_MORE];
	int m_CapUnitBuffer0_ManLength;

	std::string m_ClearTtml;
	int m_ClearTtmlFlag;
	unsigned char *m_pOutputClearPointer;
	int m_output_clear_size;
	int m_iClearTimeMsec;
	int m_iClearTimeCount;
	int m_iClearUpdateFlag;
	int UpdateClearScreen();

	int m_iFlashTimeMsec;
	int m_iFlashTimeCount;
	int m_iFlashUpdateFlag;
	int m_iFlashOnOffFlag;
	int UpdateFlashScreen();
};

//class __declspec(dllimport) VecWrapper : vector<TssCapProcessManager *> {};   // C4251

