#include <string>
#include <stdlib.h>

#include "windows.h"
#include "shellapi.h"
#pragma comment(lib ,"shell32.lib")

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <memory.h>

#include <Util/StringUtil.h>
#include <Util/Path.h>
#include <Util/DbgPrint.h>
#include <Util/Conv.h>
#include <Util/SysTime.h>

#include <App/AppLogger.h>
#include <PacketStructure/Caption/CCaptionPES.h>

#include "CInputTsFile.h"
#include "TssCapProcessManager.h"

#pragma comment(lib ,"CaptionConverter.lib")
#include "CaptionConverter.h"

#include <ZipUtil/zutil.h>

#define TSS_INTERFACE_TYPE 1 //1:memory 0:file

#if 1
#define TSS_INTERFACE_SAVE 1 //1:save 0:nosave
#define TSS_CONVERT_LOG 0	 //0:none 1:log
#define TSS_CONV_LOG_LEVEL 0 //0:none 5:save
#define TSS_OUTPUT_IMAGE_FILE_TYPE 0 // 0:argb 1:png 2:bmp
#define TSS_TIMESTAMP_NOCHECK 0
#define TSS_TIMESTAMP_DELTA 120 //msec
#else
#define TSS_INTERFACE_SAVE 1 //1:save 0:nosave
#define TSS_CONVERT_LOG 1	 //0:none 1:log
#define TSS_CONV_LOG_LEVEL 5 //0:none 5:save
#define TSS_OUTPUT_IMAGE_FILE_TYPE 1 // 0:argb 1:png 2:bmp
#define TSS_TIMESTAMP_NOCHECK 1
#define TSS_TIMESTAMP_DELTA 120 //msec
#endif

#define TSS_PROCESS_STATUS_STOP		0
#define TSS_PROCESS_STATUS_START	1

#define TSS_PROCESS_TYPE_CAPTION_MAXCOUNT	8
#define TSS_PROCESS_TYPE_MTMEDIA_MAXCOUNT	4

#define TSS_RESOLUTION_2K_SIZE 8294400   //1920*1080*4
#define TSS_RESOLUTION_4K_SIZE 33177600  //1920*1080*4*4
#define TSS_RESOLUTION_8K_SIZE 132710400 //1920*1080*4*4*4

//#define TSS_INITIAL_NAME	"TssCapProcessManager.ini"
//#define TSS_INITIAL_FILE	"C:\\work\\app\\TssCapProcessManager.ini"
#define TSS_LOG_PATH		"C:\\work\\app\\Log\\TssCap"


TssCapProcessManager::TssCapProcessManager(std::string InitFile, std::string LogPath, int iLogLevel)
{
	int i = 0;

	m_id = 0;
	m_type = 0;
	m_number = 0;
	m_resolution = 0;
	m_streamid = 0;
	m_serviceid = 0;
	m_timestamp = 0;

	m_INI_FILENAME = InitFile;

	m_LogPath = LogPath;

	m_LogLevel = iLogLevel;

	if (m_LogLevel) {
		//create
		Path::MakeDir(m_LogPath);
		//Path::MakeDir(TSS_LOG_PATH);

		if (TSS_INTERFACE_SAVE) {
			//create
			//std::string sFileSavePath;
			m_FileSavePath = m_LogPath;
			m_FileSavePath += "\\data";
			Path::MakeDir(m_FileSavePath);
			//Path::MakeDir(TSS_SAVE_BASE_PATH);
		}

		if (TSS_CONVERT_LOG) {
			m_ConvertLogPath = m_LogPath;
			m_ConvertLogPath += "\\conv";
			Path::MakeDir(m_ConvertLogPath);

			if (m_LogLevel) {
				//API_SetLogPath((unsigned char*)m_LogPath.c_str(), m_LogPath.size());
				API_SetLogPath((unsigned char*)m_ConvertLogPath.c_str(), (int)m_ConvertLogPath.size());
				APPLOG("[MAN],INF,%d,TssCapProcessManager,TssCapProcessManager(),Coverter Log Path = %s", m_id, m_ConvertLogPath.c_str());
			}
		}
	}

	if (m_LogLevel) {
		APPLOG("[MAN],INF,%d,TssCapProcessManager,TssCapProcessManager(),Start Instance ===============", m_id);
	}

	for (i = 0; i < TSS_PROCESS_INPUT_COUNT; i++) {
		m_input_pid[i] = -1;
		m_input_type[i] = 0;
		m_input_tag[i] = -1;
		m_input_cc[i] = -1;
	}
	m_input_lang = 0;
	m_input_count = 0;

	m_pOutputPointer = NULL;
	m_output_size = 0;
	m_status = TSS_PROCESS_STATUS_STOP;
	m_pOutInteface = NULL;
	m_out_inteface_size = 0;
	m_pOutInteface = new unsigned char[TSS_RESOLUTION_2K_SIZE];
	if (m_pOutInteface == NULL) {
		return;
	}
	memset(m_pOutInteface, NULL, TSS_RESOLUTION_2K_SIZE);
	m_out_inteface_size = TSS_RESOLUTION_2K_SIZE;
	m_resolution = 1;

	if (m_LogLevel) {
		APPLOG("[MAN],INF,%d,TssCapProcessManager,TssCapProcessManager(),new m_pOutInteface[%d]", m_id, TSS_RESOLUTION_2K_SIZE);
	}

	m_vecListener.clear();

	m_ThreadFlag = 0;
	m_StartFlag = 0;
	m_DataTriggerFlag = 0;
	m_UpadteOutputFlag = 0;

	m_iGetStatus = -1;
	m_payloadMpuSequenceNumber = 0;
	m_mfuSubsampleNumber = 0;
	m_mfuLastSubsampleNumber = 0;

	m_sNumber.clear();
	m_sInputDir.clear();
	m_sInputTriggerFile.clear();
	m_sInputPageZipFile.clear();
	m_sInputTempDir.clear();
	m_sInputTempPageDir.clear();
	m_sInputTempTriggerFile.clear();
	m_sInputTempPageZipFile.clear();
	m_sInputTempCommandFile.clear();
	m_sOutputDir.clear();
	m_sOutputTriggerFile.clear();
	m_sOutputImageFile.clear();
	m_OutputImageFileType = 0;
	m_DemoMode = 0;
	m_sVbsPath.clear();

	m_output_count = 0;

	//m_pMyApi = NULL;
	//m_pMyApi = new CMyApi;
	//if (m_pMyApi == NULL) {
	//	return;
	//}
	//m_pMyApi->ApiLogMessage(LOG_LEBEL_INFO, "INIT", "起動処理=======================================================", OUTPUT_LOGFILE, "CapLog");

	m_ManLastCrc16 = 0xFFFF;

	memset(m_ManUnitBuffer0, NULL, TSS_CAPTION_LENGTH);
	m_iManUnitLength0 = 0;
	memset(m_ManUnitBuffer1, NULL, TSS_CAPTION_LENGTH_MORE);
	m_iManUnitLength1 = 0;
	memset(m_ManUnitBuffer2, NULL, TSS_CAPTION_LENGTH_MORE);
	m_iManUnitLength2 = 0;

	m_iManUnitType0 = -1;
	m_iManUnitType1 = -1;
	m_iManUnitType2 = -1;

	memset(m_CapUnitBuffer0, NULL, TSS_CAPTION_LENGTH_MORE);
	m_iCapUnitLength0 = 0;
	memset(m_CapUnitBuffer1, NULL, TSS_CAPTION_LENGTH);
	m_iCapUnitLength1 = 0;
	memset(m_CapUnitBuffer2, NULL, TSS_CAPTION_LENGTH);
	m_iCapUnitLength2 = 0;
	memset(m_CapUnitBuffer3, NULL, TSS_CAPTION_LENGTH);
	m_iCapUnitLength3 = 0;
	memset(m_CapUnitBuffer4, NULL, TSS_CAPTION_LENGTH);
	m_iCapUnitLength4 = 0;

	m_iCapUnitType0 = -1;
	m_iCapUnitType1 = -1;
	m_iCapUnitType2 = -1;
	m_iCapUnitType3 = -1;
	m_iCapUnitType4 = -1;

	memset(m_CapUnitBuffer0_Man, NULL, TSS_CAPTION_LENGTH);
	m_CapUnitBuffer0_ManLength = 0;

	m_CaptionDataRomSound = -1;

	m_iDgCount = 0;

	m_pClockTime = NULL;
	m_UpdateStop = 0;
}

void TssCapProcessManager::InitPreview(std::string sLogPath)
{
#if TSS_PROCESS_MMCTRL
	if (Path::IsDir(sLogPath) == 1) {
		ApiInit(m_LogLevel, (char*)(sLogPath.c_str()));
		if (m_LogLevel) {
			APPLOG("[MAN],INF,%d,TssCapProcessManager,InitPreview(),ApiInit(%d %s)", m_id, m_LogLevel, sLogPath.c_str());
		}
	}
	else {
		ApiInit(m_LogLevel, TSS_LOG_PATH);
		if (m_LogLevel) {
			APPLOG("[MAN],INF,%d,TssCapProcessManager,InitPreview(),ApiInit(%d %s)", m_id, m_LogLevel, TSS_LOG_PATH);
		}
	}

	if (m_LogLevel) {
		APPLOG("[MAN],INF,%d,TssCapProcessManager,InitPreview(),ApiInit(%d %s)", m_id, m_LogLevel, TSS_LOG_PATH);
	}
#endif

	//if (m_LogLevel) {
	//	API_SetLogPath((unsigned char*)m_LogPath.c_str(), m_LogPath.size());
	//	APPLOG("[MAN],INF,%d,TssCapProcessManager,InitPreview(),Coverter Log Path = %s", m_id, m_LogPath.c_str());
	//}
}

void TssCapProcessManager::ExitPreview()
{
#if TSS_PROCESS_MMCTRL
	ApiDestory();
	if (m_LogLevel) {
		APPLOG("[MAN],INF,%d,TssCapProcessManager,ExitPreview(),ApiDestory()", m_id);
	}
#endif
}

TssCapProcessManager::~TssCapProcessManager()
{
	m_id = 0;
	m_type = 0;
	m_number = 0;
	m_resolution = 0;
	m_streamid = 0;
	m_serviceid = 0;

	m_vecListener.clear();

	m_ThreadFlag = 0;

	if (m_pOutInteface) {
		delete[] m_pOutInteface;
		m_pOutInteface = NULL;
	}

	if (m_pClockTime) {
		delete m_pClockTime;
		m_pClockTime = NULL;
	}

	if (m_LogLevel) {
		APPLOG("[MAN],INF,%d,TssCapProcessManager,~TssCapProcessManager(),delete m_pDataSection", m_id);
	}
}

unsigned long long TssCapProcessManager::GetTimestamp()
{
	if (m_pClockTime) {
		return m_pClockTime->GetTimeStampSTC();
	}
	return 0;
}

int TssCapProcessManager::SetTimestamp(unsigned long long uTimestamp) 
{ 
	int iUpdateFlag = 0;
	unsigned long long uNowTimestamp = 0;
	unsigned long long uTimestampRange = 270000; //10msec
	unsigned long long uNowTimestampUp = 0;
	unsigned long long uNowTimestampDn = 0;

	if (m_pClockTime==NULL) {
		return -1;
	}
	uNowTimestamp = m_pClockTime->GetTimeStampSTC();
	uNowTimestampUp = uNowTimestamp + uTimestampRange;
	uNowTimestampDn = uNowTimestamp - uTimestampRange;

	if (uTimestamp < uNowTimestampDn) {
		iUpdateFlag++;
	}

	if (uTimestamp > uNowTimestampUp) {
		iUpdateFlag++;
	}

	//±10msecを超える場合
	if (iUpdateFlag) {
		m_pClockTime->InputTimeStampSTC(uTimestamp);
	}

	m_timestamp = uTimestamp;

	return 0;
}

int TssCapProcessManager::SetParameterInputAdd(int pid, unsigned long type, int tag)
{
	int i = 0;

	if (m_input_count >= TSS_PROCESS_INPUT_COUNT) {
		return -1;
	}

	i = m_input_count;

	m_input_pid[i] = pid;
	m_input_type[i] = type;
	m_input_tag[i] = tag;

	m_input_count++;

	return 0;
}

int TssCapProcessManager::SetParameterInputClear()
{
	int i = 0;
	for (i = 0; i < TSS_PROCESS_INPUT_COUNT; i++) {
		m_input_pid[i] = -1;
		m_input_type[i] = 0;
		m_input_tag[i] = -1;
	}
	m_input_count = 0;

	return 0;
}

int TssCapProcessManager::SetParameterInputLang(int lang)
{
	if (lang == 0) {
		m_input_lang = 0;
	}
	else {
		m_input_lang = 1;
	}

	return 0;
}

int TssCapProcessManager::SetParameterOutput(unsigned char *pointer, int size)
{
	m_pOutputPointer = pointer;
	m_output_size = size;

	return 0;
}

long TssCapProcessManager::Create(long number, long type, int resolution)
{
	int ret = 0;
	std::string sParam;
	std::string sGroupName;
	std::string sParamName;
	std::string sTemp;
	char sBuff[64];
	int iTemp;


	//number
	m_number = number;
	if (m_LogLevel) {
		APPLOG("[MAN],INF,%d,TssCapProcessManager,Create(),number=%d", m_id, m_number);
	}

	//type
	m_type = type;
	if (m_LogLevel) {
		APPLOG("[MAN],INF,%d,TssCapProcessManager,Create(),type=%d", m_id, m_type);
	}

	if (m_type == TSSLIBMAN_TYPE_CAPTION) {
		if (m_number == 1) {}
		else if ((m_number >= 2) && (m_number <= TSS_PROCESS_TYPE_CAPTION_MAXCOUNT)) {
			m_sNumber = Conv::IntToDecStr(m_number);
		}
		else {
			return -1;
		}
	}
	else if (m_type == TSSLIBMAN_TYPE_MTMEDIA) {
		if (m_number == 1) {
			m_sNumber = "";
		}
		else if ((m_number >= 2) && (m_number <= TSS_PROCESS_TYPE_MTMEDIA_MAXCOUNT)) {
			m_sNumber = Conv::IntToDecStr(m_number);
		}
		else {
			return -1;
		}
	}

	//resolution
	SetResolution(resolution);

#if TSS_PROCESS_MMCTRL
	if (m_type == TSSLIBMAN_TYPE_CAPTION) {

		ret = ApiCreate(number, type-10, resolution);
		if (ret < 0) {
			if (m_LogLevel) {
				APPLOG("[MAN],ERR,%d,TssCapProcessManager,Create(),ApiCreate(%d,%d,%d) Err=%d", m_id, number, type-10, resolution, ret);
			}
			Release(number, type);
			if (ret == -302) {
			}
			return ret;
		}
		if (m_LogLevel) {
			APPLOG("[MAN],INF,%d,TssCapProcessManager,Create(),ApiCreate(%d,%d,%d)", m_id, number, type-10, resolution);
		}

		//[INPUT]
		sGroupName = "Caption";
		//=====================
		//StartupType = 0
		{
			//StartupType
			sParamName = "StartupType";
			iTemp = GetPrivateProfileIntA((LPCTSTR)sGroupName.c_str(), (LPCTSTR)sParamName.c_str(), -1, (LPCTSTR)m_INI_FILENAME.c_str());
			if (iTemp == 0) {
				iTemp = 0;
			}
			else {
				//iTemp = 1;
				iTemp = 0;
			}
		}
		//sParam = StringUtil::Format("%s#%d", "StartupType", 0);
		sParam = StringUtil::Format("%s#%d", "StartupType", iTemp);
		ret = ApiSetParam(number, type-10, (char*)sParam.c_str(), (int)sParam.size());
		if (ret < 0) {
			Release(number, type-10);
			return ret;
		}
		if (m_LogLevel) {
			APPLOG("[MAN],INF,%d,TssCapProcessManager,Create(),ApiSetParam(%d,%d,%s)", m_id, number, type-10, sParam.c_str());
		}
		//=====================
		//ErrorTtmlPath = C:\WORK\RealTimeProcessTss\error\page.zip
		{
			sParamName = "ErrorTtmlPath";
			memset(sBuff, NULL, 64);
			iTemp = ::GetPrivateProfileStringA((LPCTSTR)sGroupName.c_str(), (LPCTSTR)sParamName.c_str(), "", (LPSTR)sBuff, 64, (LPCTSTR)m_INI_FILENAME.c_str());
			sTemp = sBuff;
			if (Path::IsExist(sTemp) == 0) {
				APPLOG("[MAN],ERR,%d,TssCapProcessManager,Create(),ErrorTtmlPath is nothing (%s)", m_id, sTemp.c_str());
				return -1;
			}
		}
		sParam = StringUtil::Format("%s#%s", "ErrorTtmlPath", sTemp.c_str());
		ret = ApiSetParam(number, type-10, (char*)sParam.c_str(), (int)sParam.size());
		if (ret < 0) {
			Release(number, type-10);
			return ret;
		}
		if (m_LogLevel) {
			APPLOG("[MAN],INF,%d,TssCapProcessManager,Create(),ApiSetParam(%d,%d,%s)", m_id, number, type-10, sParam.c_str());
		}
		//=====================
		//OutputImageFileType
		// 0:argb 1:png 2:bmp
		m_OutputImageFileType = TSS_OUTPUT_IMAGE_FILE_TYPE;
		if (m_LogLevel) {
			APPLOG("[MAN],INF,%d,TssCapProcessManager,Create(),OutputImageFileType=%d", m_id, m_OutputImageFileType);
		}
		//=====================
		//FontFamily
		{
			sParamName = "FontFamily";
			memset(sBuff, NULL, 64);
			iTemp = ::GetPrivateProfileStringA((LPCTSTR)sGroupName.c_str(), (LPCTSTR)sParamName.c_str(), "", (LPSTR)sBuff, 64, (LPCTSTR)m_INI_FILENAME.c_str());
			sTemp = sBuff;
			if (sTemp.size() == 0) {
				sTemp = "MS Gothic";
			}
		}
		//sParam = StringUtil::Format("%s#%s", "FontFamily", "MS Gothic");
		//sParam = StringUtil::Format("%s#%s", "FontFamily", "メイリオ");
		sParam = StringUtil::Format("%s#%s", "FontFamily", sTemp.c_str());
		ret = ApiSetParam(number, type-10, (char*)sParam.c_str(), (int)sParam.size());
		if (ret < 0) {
			if (m_LogLevel) {
				APPLOG("[MAN],ERR,%d,TssCapProcessManager,Create(),ApiSetParam(%d,%d,%s)=%d", m_id, number, type-10, sParam.c_str(), ret);
			}
			Release(number, type-10);
			return ret;
		}
		if (m_LogLevel) {
			APPLOG("[MAN],INF,%d,TssCapProcessManager,Create(),ApiSetParam(%d,%d,%s)", m_id, number, type-10, sParam.c_str());
		}

		ret = ApiStart(number, type - 10);
		if (ret < 0) {
			if (m_LogLevel) {
				APPLOG("[MAN],ERR,%d,TssCapProcessManager,Create(),ApiStart(%d,%d) Err=%d", m_id, number, type - 10, ret);
			}
			Release(number, type - 10);
			return ret;
		}
		if (m_LogLevel) {
			APPLOG("[MAN],INF,%d,TssCapProcessManager,Create(),ApiStart(%d,%d)", m_id, number, type - 10);
		}

		//clock
		if (m_pClockTime) {
			delete m_pClockTime;
		}
		m_pClockTime = new ClockTime(m_id);
		if (m_pClockTime == NULL) {
			return -1;
		}

	}
#else
#endif

	return 0;
}


long TssCapProcessManager::Release(long number, long type)
{
	int ret = 0;

	return 0;
}


int TssCapProcessManager::PushTspData(unsigned char *pointer, int size)
{
	int i = 0;
	int ret = 0;
	int pid = 0;
	int cc = 0;
	unsigned char *pesBuff = NULL;
	int pesLength = 0;
	int complete = 0;
	int cc_error = 0;

	pid = CInputTsFile::GetPid(pointer);
	cc  = CInputTsFile::GetCc(pointer);

	for (i = 0; i < m_input_count; i++) {
		if (pid == m_input_pid[i]) {

			//CCチェック
			//--初回
			if (m_input_cc[i]<0) {
				//--更新
				//m_input_cc[i] = cc;
			}
			//--2回目以降
			else {
				//--区切り
				if (m_input_cc[i] == 0x0F) {
					//--０のはず
					if (cc == 0) {
						//--更新
						//m_input_cc[i] = cc;
					}
					//--０以外はエラー
					else {
						cc_error++;
					}
				}
				//--通常
				else {
					//--前回+1のはず
					if ((m_input_cc[i]+1) == cc) {
						///--更新
						//m_input_cc[i] = cc;
					}
					//--前回+1以外はエラー
					else {
						cc_error++;
					}
				}
			}
			//CCエラーの場合
			if (cc_error > 0) {
				if (m_LogLevel) {
					APPLOG("[MAN],ERR,%d,TssCapProcessManager,PushTspData(),CC=%d(%d)", m_id, pid, cc, m_input_cc[i]);
				}
				//clear
				PesBufferClear();
				if (m_LogLevel) {
					APPLOG("[MAN],INF,%d,TssCapProcessManager,PushTspData(),PesBufferClear()", m_id, pid);
				}
			}
			else {
				//--更新
				if (m_LogLevel) {
					APPLOG("[MAN],INF,%d,TssCapProcessManager,PushTspData(),CC=%d(%d)", m_id, pid, cc, m_input_cc[i]);
				}
				m_input_cc[i] = cc;
			}

			if (m_LogLevel) {
				APPLOG("[MAN],INF,%d,TssCapProcessManager,PushTspData(),PID=%d(0x%04X)", m_id, pid, pid);
			}
			ret = CapturePesPacket(pointer,188);
			if (ret < 0) {
				if (m_LogLevel) {
					APPLOG("[MAN],ERR,%d,TssCapProcessManager,PushTspData(),CapturePesPacket() Err=%d", m_id, ret);
				}
				return ret;
			}
			if (m_LogLevel) {
				//APPLOG("[MAN],INF,%d,TssCapProcessManager,PushTspData(),CapturePesPacket()", m_id);
			}
			//取得完了
			if (ret == 100) {
				complete = 1;
				break;
			}
		}
	}

	if (complete) {
		pesBuff = GetPesBuffer();
		pesLength = GetPesLength();

		ret = ParsePesData(pesBuff, pesLength, 0);
		if (ret < 0) {
			return ret;
		}
	}

	return 0;
}

int TssCapProcessManager::CapturePesPacket(unsigned char *pointer, int size)
{
	int ptr = 0;
	int pusi = 0;
	int plen = 0;
	int pes_len = 0;
	int comp = 0;
	unsigned char *pdata = NULL;
	unsigned char *tspBuff = NULL;

	tspBuff = pointer;
	pusi = CInputTsFile::GetPusi(tspBuff);

	if (m_iCapPesFlag == 0) {
		if (pusi == 1) {
			//取得開始
			PesBufferClear();
			ptr = CInputTsFile::GetPalyload(tspBuff);
			pdata = tspBuff + ptr;
			plen = 188 - ptr;
			pes_len = ((unsigned long)tspBuff[ptr + 4]) * 256;
			pes_len += (unsigned long)tspBuff[ptr + 5];
			pes_len += 6;
			PesBufferInit(pes_len);
			m_iCapPesFlag = 1;
			comp = PesBufferSet(pdata, plen);
			if (comp != 100) {
				return 0;
			}
		}
		else {
			return 0;
		}
	}
	//m_iCapPesFlag:1
	else {
		if (pusi == 1) {
			//取得開始
			PesBufferClear();
			ptr = CInputTsFile::GetPalyload(tspBuff);
			pdata = tspBuff + ptr;
			plen = 188 - ptr;
			pes_len = ((unsigned long)tspBuff[ptr + 4]) * 256;
			pes_len += (unsigned long)tspBuff[ptr + 5];
			PesBufferInit(pes_len);
			m_iCapPesFlag = 1;
			comp = PesBufferSet(pdata, plen);
			if (comp != 100) {
				return 0;
			}
		}
		else {
			ptr = CInputTsFile::GetPalyload(tspBuff);
			pdata = tspBuff + ptr;
			plen = 188 - ptr;
			comp = PesBufferSet(pdata, plen);
			if (comp != 100) {
				return 0;
			}
		}
	}
	//取得完了
	if (comp == 100) {

		m_iCapPesFlag = 0;
		return 100;
	}
	else {
		return 0;
	}
}

int TssCapProcessManager::PushPesData(unsigned char *pointer, int size)
{
	int ret = 0;

	ret = ParsePesData(pointer, size, m_input_lang);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

int TssCapProcessManager::ParsePesData(unsigned char *pointer, int size, int lang)
{
	int ret = 0;
	int i = 0;
	unsigned char *pesBuff = NULL;
	int pesLength = 0;

	CCaptionPES* pCaptionPES = new CCaptionPES;
	if (pCaptionPES == NULL) {
		return -1;
	}
	pesBuff = pointer;
	pesLength = size;
	ret = pCaptionPES->Decode(pesBuff);
	if (ret < 0) {
		return -1;
	}

	int iUnitCount = 0;
	iUnitCount = pCaptionPES->GetUnitCount();
	int iUnitIndex = 0;
	unsigned char* pUnitBuffer = NULL;
	int iUnitLength = 0;
	int iUnitType = -1;

	int image_interval = 0;
	int iDuration = 0;
	int iBegin = 0;
	int iEnd = 0;
	int iAnimation = -1;

	unsigned long long i64PageTimestap = pCaptionPES->GetTimestamp();


	//management_data
	if ((pCaptionPES->GetDataGroupId() == DATA_GRP_MNG_A) || ((pCaptionPES->GetDataGroupId() == DATA_GRP_MNG_B))) {

		int iUpdateFlag = 0;
		int iUnitFirst = 0;
		unsigned short sNowCrc16 = pCaptionPES->GetCRC16();

		if (m_iPesCount==6) {
			ret = pCaptionPES->Decode(pesBuff);
		}

		//初回時
		if (m_ManLastCrc16 == 0xFFFF) {
			if (m_LogLevel) {
				APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[update] management_data=%d ManLastCrc16=%d", m_id, pCaptionPES->GetDataGroupId(), m_ManLastCrc16);
			}
			m_ManLastCrc16 = sNowCrc16;
			iUpdateFlag++;
		}
		//2回目以降
		else {
			if (m_LogLevel) {
				APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[update] management_data=%d ManLastCrc16=%d NowCrc16=%d", m_id, pCaptionPES->GetDataGroupId(), m_ManLastCrc16, sNowCrc16);
			}
			//更新確認
			if (m_ManLastCrc16 != sNowCrc16) {
				m_ManLastCrc16 = sNowCrc16;
				iUpdateFlag++;
			}
			//何もしない
			else {
				iUpdateFlag = 0;
			}
		}
		if (m_LogLevel) {
			APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[update] management_data=%d UpdateFlag=%d", m_id, pCaptionPES->GetDataGroupId(), iUpdateFlag);
		}

		//初回時または更新がある場合
		if (iUpdateFlag) {

			//メモリ初期化
			//memset(m_ManUnitBuffer0, NULL, TSS_CAPTION_LENGTH);
			//m_iManUnitLength0 = 0;
			memset(m_ManUnitBuffer1, NULL, TSS_CAPTION_LENGTH_MORE);
			m_iManUnitLength1 = 0;
			memset(m_ManUnitBuffer2, NULL, TSS_CAPTION_LENGTH_MORE);
			m_iManUnitLength2 = 0;

			//UnitDataの初め
			iUnitFirst = 0;

			if (m_LogLevel) {
				APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[MAN] UnitCount=%d", m_id, iUnitCount);
			}

			//UnitData
			for (iUnitIndex = 0; iUnitIndex < iUnitCount; iUnitIndex++) {

				pUnitBuffer = pCaptionPES->GetUnitData(iUnitIndex);
				iUnitLength = pCaptionPES->GetUnitLength(iUnitIndex);
				iUnitType = pCaptionPES->GetUnitType(iUnitIndex);
				if (m_LogLevel) {
					APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[MAN] UnitLength=%d UnitType=%d", m_id, iUnitLength, iUnitType);
				}

				if (TSS_INTERFACE_SAVE)
				{
					std::string saveName;
					std::string savePath = m_FileSavePath;

					if (iUnitType == 0x20) {
						//本文
						saveName = StringUtil::Format("%s\\CManage_%04d_man_%d_20.bin", savePath.c_str(), m_iPesCount, iUnitIndex);
					}
					else if (iUnitType == 0x30) {
						//１バイトDRCS
						saveName = StringUtil::Format("%s\\CManage_%04d_man_%d_30.bin", savePath.c_str(), m_iPesCount, iUnitIndex);
					}
					else if (iUnitType == 0x31) {
						//２バイトDRCS
						saveName = StringUtil::Format("%s\\CManage_%04d_man_%d_31.bin", savePath.c_str(), m_iPesCount, iUnitIndex);
					}
					else if (iUnitType == 0x35) {
						//ビットマップ
						saveName = StringUtil::Format("%s\\CManage_%04d_man_%d_35.bin", savePath.c_str(), m_iPesCount, iUnitIndex);
					}
					else {
						saveName = StringUtil::Format("%s\\CManage_%04d_man_%d_%02X.bin", savePath.c_str(), m_iPesCount, iUnitIndex, iUnitType);
					}

					FILE* fpt;
					fpt = fopen(saveName.c_str(), "w+b");
					if (fpt == NULL) {
						return -1;
					}
					fseek(fpt, 0, SEEK_SET);
					fwrite(pUnitBuffer, iUnitLength, 1, fpt);
					fclose(fpt);
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),CManage Unit=%s", m_id, saveName.c_str());
					}
				}

				if (iUnitLength > 0) {
					//Type=0x20かつ初回時のみデータを取得
					if ((iUnitFirst == 0) && (iUnitType == 0x20)) {

						//初期化
						memset(m_ManUnitBuffer0, NULL, TSS_CAPTION_LENGTH);
						m_iManUnitLength0 = 0;

						//データコピー
						for (i = 0; i < iUnitLength; i++) {
							m_ManUnitBuffer0[m_iManUnitLength0] = pUnitBuffer[i];
							m_iManUnitLength0++;
						}
						m_iManUnitType0 = iUnitType;

						if (m_LogLevel) {
							APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[MAN]<%d>UnitType=%d UnitLength=%d", m_id, iUnitIndex, iUnitType, m_iManUnitLength0);
						}

						iUnitFirst++;
					}
					//管理データで2バイトDRCSが来た場合（1個のみ対応）
					else if (iUnitType == 0x30) {

						//初回時
						if (m_iManUnitLength1 == 0) {

							//想定する長さを超える場合、終了
							if (iUnitLength > TSS_CAPTION_LENGTH_MORE) {
								if (m_LogLevel) {
									APPLOG("[MAN],ERR,%d,TssCapProcessManager,ParsePesData(),[MAN]<%d>UnitType=0x%02X UnitLength=%d > &d", m_id, iUnitIndex, iUnitType, iUnitLength, TSS_CAPTION_LENGTH_MORE);
								}
								return -1;
							}
							memset(m_ManUnitBuffer1, NULL, TSS_CAPTION_LENGTH_MORE);
							m_iManUnitLength1 = 0;

							for (i = 0; i < iUnitLength; i++) {
								m_ManUnitBuffer1[m_iManUnitLength1] = pUnitBuffer[i];
								m_iManUnitLength1++;
							}
							m_iManUnitType1 = iUnitType;

							if (m_LogLevel) {
								APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[MAN]<%d>UnitType=0x%02X UnitLength=%d", m_id, iUnitIndex, iUnitType, m_iManUnitLength1);
							}
						}

						iUnitFirst++;
					}
					//管理データで2バイトDRCSが来た場合（1個のみ対応）
					else if (iUnitType == 0x31) {

						//初回時
						if (m_iManUnitLength2 == 0) {

							//想定する長さを超える場合
							if (iUnitLength > TSS_CAPTION_LENGTH_MORE) {
								if (m_LogLevel) {
									APPLOG("[MAN],ERR,%d,TssCapProcessManager,ParsePesData(),[MAN]<%d>UnitType=0x%02X UnitLength=%d > &d", m_id, iUnitIndex, iUnitType, iUnitLength, TSS_CAPTION_LENGTH_MORE);
								}
								return -1;
							}

							memset(m_ManUnitBuffer2, NULL, TSS_CAPTION_LENGTH_MORE);
							m_iManUnitLength2 = 0;

							for (i = 0; i < iUnitLength; i++) {
								m_ManUnitBuffer2[m_iManUnitLength2] = pUnitBuffer[i];
								m_iManUnitLength2++;
							}
							m_iManUnitType2 = iUnitType;

							if (m_LogLevel) {
								APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[MAN]<%d>UnitType=0x%02X UnitLength=%d", m_id, iUnitIndex, iUnitType, m_iManUnitLength2);
							}
						}

						iUnitFirst++;
					}
					else {
						//別タイプの場合
						if (m_LogLevel) {
							APPLOG("[MAN],WAR,%d,TssCapProcessManager,ParsePesData(),<x>ManUnitType=%d", m_id, iUnitType);
						}
					}
				}
				else {
					//iUnitLength=0の場合
					if (m_LogLevel) {
						APPLOG("[MAN],WAR,%d,TssCapProcessManager,ParsePesData(),<x>ManUnitLength=%d", m_id, iUnitLength);
					}
				}

			}

		}
		//更新しない場合
		else {
			//何もしない
		}
	}
	//caption_data
	else {

		if (TSS_INTERFACE_SAVE) {
			std::string saveName;
			std::string savePath = m_FileSavePath;
			saveName = StringUtil::Format("%s\\PesData_%04d_pes.bin", savePath.c_str(), m_iPesCount, iUnitIndex);

			FILE* fpt;
			fpt = fopen(saveName.c_str(), "w+b");
			if (fpt == NULL) {
				return -1;
			}
			fseek(fpt, 0, SEEK_SET);
			fwrite(pesBuff, pesLength, 1, fpt);
			fclose(fpt);
			if (m_LogLevel) {
				APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),Pes Data =%s", m_id, saveName.c_str());
			}
		}

		//lang:第1言語チェック
		if (lang == 0) {
			if ((pCaptionPES->GetDataGroupId() == DATA_GRP_TXT_LANG1_A) || ((pCaptionPES->GetDataGroupId() == DATA_GRP_TXT_LANG1_B))) {
				//OK
			}
			else {
				//対象言語でないためここで終了
				if (m_LogLevel) {
					APPLOG("[MAN],WAR,%d,TssCapProcessManager,ParsePesData(),Lang=%d DataGroupId=%d", m_id, lang, (int)pCaptionPES->GetDataGroupId());
				}
				return 0;
			}
		}
		//lang:第2言語チェック
		else {
			if ((pCaptionPES->GetDataGroupId() == DATA_GRP_TXT_LANG2_A) || ((pCaptionPES->GetDataGroupId() == DATA_GRP_TXT_LANG2_B))) {
				//OK
			}
			else {
				//対象言語でないためここで終了
				if (m_LogLevel) {
					APPLOG("[MAN],WAR,%d,TssCapProcessManager,ParsePesData(),Lang=%d DataGroupId=%d", m_id, lang, (int)pCaptionPES->GetDataGroupId());
				}
				return 0;
			}
		}

		//init
		memset(m_CapUnitBuffer0, NULL, TSS_CAPTION_LENGTH_MORE);
		m_iCapUnitLength0 = 0;
		memset(m_CapUnitBuffer1, NULL, TSS_CAPTION_LENGTH);
		m_iCapUnitLength1 = 0;
		memset(m_CapUnitBuffer2, NULL, TSS_CAPTION_LENGTH);
		m_iCapUnitLength2 = 0;
		memset(m_CapUnitBuffer3, NULL, TSS_CAPTION_LENGTH);
		m_iCapUnitLength3 = 0;
		memset(m_CapUnitBuffer4, NULL, TSS_CAPTION_LENGTH);
		m_iCapUnitLength4 = 0;
	
		m_iCapUnitType0 = 0;
		m_iCapUnitType1 = 0;
		m_iCapUnitType2 = 0;
		m_iCapUnitType3 = 0;
		m_iCapUnitType4 = 0;

		//PesCount
		if (iUnitCount > 0) {
			m_iPesCount++;
			if (m_LogLevel) {
				APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),PesCount=%d(0x%04X) Unit=%d", m_id, m_iPesCount, m_iPesCount, iUnitCount);
			}
		}
		if (m_LogLevel) {
			APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),caption_data=%d UnitCount=%d", m_id, pCaptionPES->GetDataGroupId(), iUnitCount);
		}

		int iFirstCaption = 0;
		int iCaptionDataIndex = 0;

		//UnitData
		for (iUnitIndex = 0; iUnitIndex < iUnitCount; iUnitIndex++) {
			pUnitBuffer = pCaptionPES->GetUnitData(iUnitIndex);
			iUnitLength = pCaptionPES->GetUnitLength(iUnitIndex);
			iUnitType = pCaptionPES->GetUnitType(iUnitIndex);

			if (TSS_INTERFACE_SAVE) {
				std::string saveName;
				//std::string savePath = TSS_SAVE_BASE_PATH;
				std::string savePath = m_FileSavePath;

				if (iUnitType == 0x20) {
					//本文
					saveName = StringUtil::Format("%s\\Caption_%04d_cap_%d_20.bin", savePath.c_str(), m_iPesCount, iUnitIndex);
				}
				else if (iUnitType == 0x30) {
					//１バイトDRCS
					saveName = StringUtil::Format("%s\\Caption_%04d_cap_%d_30.bin", savePath.c_str(), m_iPesCount, iUnitIndex);
				}
				else if (iUnitType == 0x31) {
					//２バイトDRCS
					saveName = StringUtil::Format("%s\\Caption_%04d_cap_%d_31.bin", savePath.c_str(), m_iPesCount, iUnitIndex);
				}
				else if (iUnitType == 0x35) {
					//ビットマップ
					saveName = StringUtil::Format("%s\\Caption_%04d_cap_%d_35.bin", savePath.c_str(), m_iPesCount, iUnitIndex);
				}
				else {
					saveName = StringUtil::Format("%s\\Caption_%04d_cap_%d_%02X.bin", savePath.c_str(), m_iPesCount, iUnitIndex, iUnitType);
				}

				FILE* fpt;
				fpt = fopen(saveName.c_str(), "w+b");
				if (fpt == NULL) {
					return -1;
				}
				fseek(fpt, 0, SEEK_SET);
				fwrite(pUnitBuffer, iUnitLength, 1, fpt);
				fclose(fpt);
				if (m_LogLevel) {
					APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),Caption Unit=%s", m_id, saveName.c_str());
				}

			}

			unsigned char* pCapUnitBuffer = NULL;
			int* iCapUnitLength = 0;

			//本文
			if (iUnitType == 0x20) {

				if (iUnitLength>0) {

					//本文合成
					for (int i = 0; i < iUnitLength; i++) {
						if (m_iCapUnitLength0 >= TSS_CAPTION_LENGTH_MORE) {
							break;
						}
						if (iUnitLength > 1) {
							if ((i == 0) && (pUnitBuffer[0] == 0x0C)) {
								continue;
							}
						}
						m_CapUnitBuffer0[m_iCapUnitLength0] = pUnitBuffer[i];
						m_iCapUnitLength0++;
					}
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),CapUnitLength0=%d", m_id, m_iCapUnitLength0);
					}

				}

			}
			//その他
			else {
				if (iCaptionDataIndex == 0) {
					pCapUnitBuffer = m_CapUnitBuffer1;
					//iCapUnitLength = m_iCapUnitLength1;
					iCapUnitLength = &m_iCapUnitLength1;
					m_iCapUnitType1 = iUnitType;
					iCaptionDataIndex++;
				}
				else if (iCaptionDataIndex == 1) {
					pCapUnitBuffer = m_CapUnitBuffer2;
					//iCapUnitLength = m_iCapUnitLength2;
					iCapUnitLength = &m_iCapUnitLength2;
					m_iCapUnitType2 = iUnitType;
					iCaptionDataIndex++;
				}
				else if (iCaptionDataIndex == 2) {
					pCapUnitBuffer = m_CapUnitBuffer3;
					//iCapUnitLength = m_iCapUnitLength3;
					iCapUnitLength = &m_iCapUnitLength3;
					m_iCapUnitType3 = iUnitType;
					iCaptionDataIndex++;
				}
				else if (iCaptionDataIndex == 3) {
					pCapUnitBuffer = m_CapUnitBuffer4;
					//iCapUnitLength = m_iCapUnitLength4;
					iCapUnitLength = &m_iCapUnitLength4;
					m_iCapUnitType4 = iUnitType;
					iCaptionDataIndex++;
				}

				if (iUnitLength > 0) {
					memset(pCapUnitBuffer, NULL, TSS_CAPTION_LENGTH);
					(*iCapUnitLength) = 0;

					for (i = 0; i < iUnitLength; i++) {
						pCapUnitBuffer[i] = pUnitBuffer[i];
						(*iCapUnitLength)++;
					}
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),CapUnitLength=%d", m_id, iUnitLength);
					}
				}
				else {
					if (m_LogLevel) {
						APPLOG("[MAN],WAR,%d,TssCapProcessManager,ParsePesData(),CapUnitLength=%d", m_id, iUnitLength);
					}
				}

			}
			if (m_LogLevel) {
				APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),iUnitType=%d FirstCaption=%d CaptionDataIndex=%d", m_id, iUnitType, iFirstCaption, iCaptionDataIndex);
			}

		}

		//文字コード処理
		//字幕本文がある場合
		if (m_LogLevel) {
			APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),m_iCapUnitLength0=%d", m_id, m_iCapUnitLength0);
		}

		if(m_iCapUnitLength0 >0){

			std::string sCaptionDataUtf8;
			std::string sCaptionDataTtml;

			if(TSS_INTERFACE_SAVE){
				std::string saveName;
				std::string savePath = m_FileSavePath;

				saveName = StringUtil::Format("%s\\Caption_%04d_8TaniCap_Cap.bin", savePath.c_str(), m_iPesCount);

				FILE* fpt;
				fpt = fopen(saveName.c_str(), "w+b");
				if (fpt == NULL) {
					return -1;
				}
				fseek(fpt, 0, SEEK_SET);
				fwrite(m_CapUnitBuffer0, m_iCapUnitLength0, 1, fpt);
				fclose(fpt);
				if (m_LogLevel) {
					APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),8Tani=%s", m_id, saveName.c_str());
				}
			}

			unsigned char* pUtf8Buffer = NULL;
			pUtf8Buffer = new unsigned char[TSS_CAPTION_LENGTH_MORE];
			memset(pUtf8Buffer, NULL, TSS_CAPTION_LENGTH_MORE);
			int iUtf8BuffLen = 0;
			iUtf8BuffLen = TSS_CAPTION_LENGTH_MORE;
			
			//
			//管理データのUnitを初期値として追加処理
			//
			memset(m_CapUnitBuffer0_Man, NULL, TSS_CAPTION_LENGTH_MORE);
			m_CapUnitBuffer0_ManLength = 0;
			int pointer = 0;

			for (i = 0; i < m_iManUnitLength0; i++) {

				if (i == 0) {
					//CSを飛ばす
					if (m_ManUnitBuffer0[i] == 0x0C) {
						continue;
					}
					//APRを飛ばす
					else if (m_ManUnitBuffer0[i] == 0x0D) {
						continue;
					}
				}
				//データコピー
				m_CapUnitBuffer0_Man[pointer] = m_ManUnitBuffer0[i];
				pointer++;
				m_CapUnitBuffer0_ManLength++;
			}
			if (m_LogLevel) {
				APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),pUnitBuffer2a=%d", m_id, pointer);
			}


			int iUnitPointer = 0;
			{
				for (i = 0; i < m_iCapUnitLength0; i++) {
					m_CapUnitBuffer0_Man[pointer] = m_CapUnitBuffer0[i];
					pointer++;
					m_CapUnitBuffer0_ManLength++;
					iUnitPointer++;
				}
				if (m_LogLevel) {
					APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),CapUnitBuffer0ManLength=%d", m_id, m_CapUnitBuffer0_ManLength);
				}
				iUnitLength = pointer;
			}


			if (TSS_INTERFACE_SAVE)
			{
				std::string saveName;
				std::string savePath = m_FileSavePath;

				saveName = StringUtil::Format("%s\\Caption_%04d_8TaniCap_Man.bin", savePath.c_str(), m_iPesCount);
				FILE* fpt;
				fpt = fopen(saveName.c_str(), "w+b");
				if (fpt == NULL) {
					delete[] pUtf8Buffer;
					return -1;
				}
				fseek(fpt, 0, SEEK_SET);
				fwrite(m_CapUnitBuffer0_Man, m_CapUnitBuffer0_ManLength, 1, fpt);
				fclose(fpt);
				if (m_LogLevel) {
					APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),8Tani=%s", m_id, saveName.c_str());
				}
			}


			//管理データのDRCS1をコピー
			if (m_iManUnitLength1 > 0) {
				if (m_iCapUnitLength1 == 0) {
					memset(m_CapUnitBuffer1, NULL, TSS_CAPTION_LENGTH);
					m_iCapUnitLength1 = 0;
					for (i = 0; i < m_iManUnitLength1; i++) {
						m_CapUnitBuffer1[i] = m_ManUnitBuffer1[i];
						m_iCapUnitLength1++;
					}
					m_iCapUnitType1 = m_iManUnitType1;
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),COPY ManUnit1->CapUnit1", m_id);
					}
				}
				else if (m_iCapUnitLength2 == 0) {
					memset(m_CapUnitBuffer2, NULL, TSS_CAPTION_LENGTH);
					m_iCapUnitLength2 = 0;
					for (i = 0; i < m_iManUnitLength1; i++) {
						m_CapUnitBuffer2[i] = m_ManUnitBuffer1[i];
						m_iCapUnitLength2++;
					}
					m_iCapUnitType2 = m_iManUnitType1;
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),COPY ManUnit1->CapUnit2", m_id);
					}
				}
				else if (m_iCapUnitLength3 == 0) {
					memset(m_CapUnitBuffer3, NULL, TSS_CAPTION_LENGTH);
					m_iCapUnitLength3 = 0;
					for (i = 0; i < m_iManUnitLength1; i++) {
						m_CapUnitBuffer3[i] = m_ManUnitBuffer1[i];
						m_iCapUnitLength3++;
					}
					m_iCapUnitType3 = m_iManUnitType1;
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),COPY ManUnit1->CapUnit3", m_id);
					}
				}
				else if (m_iCapUnitLength4 == 0) {
					memset(m_CapUnitBuffer4, NULL, TSS_CAPTION_LENGTH);
					m_iCapUnitLength1 = 0;
					for (i = 0; i < m_iManUnitLength1; i++) {
						m_CapUnitBuffer4[i] = m_ManUnitBuffer1[i];
						m_iCapUnitLength4++;
					}
					m_iCapUnitType4 = m_iManUnitType1;
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),COPY ManUnit1->CapUnit4", m_id);
					}
				}
			}

			//管理データのDRCS2をコピー
			if (m_iManUnitLength2 > 0) {
				if (m_iCapUnitLength1 == 0) {
					memset(m_CapUnitBuffer1, NULL, TSS_CAPTION_LENGTH);
					m_iCapUnitLength1 = 0;
					for (i = 0; i < m_iManUnitLength2; i++) {
						m_CapUnitBuffer1[i] = m_ManUnitBuffer2[i];
						m_iCapUnitLength1++;
					}
					m_iCapUnitType1 = m_iManUnitType2;
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),COPY ManUnit2->CapUnit1", m_id);
					}
				}
				else if (m_iCapUnitLength2 == 0) {
					memset(m_CapUnitBuffer2, NULL, TSS_CAPTION_LENGTH);
					m_iCapUnitLength2 = 0;
					for (i = 0; i < m_iManUnitLength2; i++) {
						m_CapUnitBuffer2[i] = m_ManUnitBuffer2[i];
						m_iCapUnitLength2++;
					}
					m_iCapUnitType2 = m_iManUnitType2;
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),COPY ManUnit2->CapUnit2", m_id);
					}
				}
				else if (m_iCapUnitLength3 == 0) {
					memset(m_CapUnitBuffer3, NULL, TSS_CAPTION_LENGTH);
					m_iCapUnitLength3 = 0;
					for (i = 0; i < m_iManUnitLength2; i++) {
						m_CapUnitBuffer3[i] = m_ManUnitBuffer2[i];
						m_iCapUnitLength3++;
					}
					m_iCapUnitType3 = m_iManUnitType2;
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),COPY ManUnit2->CapUnit3", m_id);
					}
				}
				else if (m_iCapUnitLength4 == 0) {
					memset(m_CapUnitBuffer4, NULL, TSS_CAPTION_LENGTH);
					m_iCapUnitLength1 = 0;
					for (i = 0; i < m_iManUnitLength2; i++) {
						m_CapUnitBuffer4[i] = m_ManUnitBuffer2[i];
						m_iCapUnitLength4++;
					}
					m_iCapUnitType4 = m_iManUnitType2;
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),COPY ManUnit2->CapUnit4", m_id);
					}
				}
			}

			//if (m_LogLevel) {
			//	API_SetLogPath((unsigned char*)m_LogPath.c_str(), m_LogPath.size());
			//	APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),Coverter Log Path = %s", m_id, m_LogPath.c_str());
			//}

			//=================================================
			// JIS -> UTF8
			if (m_LogLevel) {
				APPLOG("[MAN],INF,%d,TssCapProcessManager,API_ConvJisToUtf8() #S Len=%d", m_id, m_CapUnitBuffer0_ManLength);
			}
			//ret = API_ConvJisToUtf8(pUnitBuffer + 1, iUnitLength - 1, pUtf8Buffer, &iUtf8BuffLen);
#if 0
			ret = API_ConvJisToUtf8(m_CapUnitBuffer0_Man, m_CapUnitBuffer0_ManLength, pUtf8Buffer, &iUtf8BuffLen);
			if (m_LogLevel) {
				APPLOG("[MAN],INF,%d,TssCapProcessManager,API_ConvJisToUtf8() #E Ret=%d", m_id, ret);
			}

			if (TSS_INTERFACE_SAVE)
			{
				std::string saveName;
				std::string savePath = m_FileSavePath;

				saveName = StringUtil::Format("%s\\Caption_%04d_8TaniCap_Utf8.bin", savePath.c_str(),m_iPesCount);
				FILE* fpt;
				fpt = fopen(saveName.c_str(), "w+b");
				if (fpt == NULL) {
					delete[] pUtf8Buffer;
					return -1;
				}
				fseek(fpt, 0, SEEK_SET);
				fwrite(pUtf8Buffer, iUtf8BuffLen, 1, fpt);
				fclose(fpt);
				if (m_LogLevel) {
					APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),Utf8=%s", m_id, saveName.c_str());
				}
			}
#endif
			unsigned char* pTtmlBuffer0 = NULL;
			int iTtmlBuffLen0 = 0;
			if (m_iCapUnitLength0 > 0) {
				pTtmlBuffer0 = new unsigned char[TSS_CAPTION_LENGTH_MORE];
				if (pTtmlBuffer0) {
					memset(pTtmlBuffer0, NULL, TSS_CAPTION_LENGTH_MORE);
					iTtmlBuffLen0 = TSS_CAPTION_LENGTH_MORE;
				}
				else { return -1; }
			}
			unsigned char* pTtmlBuffer1 = NULL;
			int iTtmlBuffLen1 = 0;
			if (m_iCapUnitLength1 > 0) {
				pTtmlBuffer1 = new unsigned char[TSS_CAPTION_LENGTH_MORE];
				if (pTtmlBuffer1) {
					memset(pTtmlBuffer1, NULL, TSS_CAPTION_LENGTH_MORE);
					iTtmlBuffLen1 = TSS_CAPTION_LENGTH_MORE;
				}
				else { return -1; }
			}
			unsigned char* pTtmlBuffer2 = NULL;
			int iTtmlBuffLen2 = 0;
			if (m_iCapUnitLength2 > 0) {
				pTtmlBuffer2 = new unsigned char[TSS_CAPTION_LENGTH_MORE];
				if (pTtmlBuffer2) {
					memset(pTtmlBuffer2, NULL, TSS_CAPTION_LENGTH_MORE);
					iTtmlBuffLen2 = TSS_CAPTION_LENGTH_MORE;
				}
				else { return -1; }
			}
			unsigned char* pTtmlBuffer3 = NULL;
			int iTtmlBuffLen3 = 0;
			if (m_iCapUnitLength3 > 0) {
				pTtmlBuffer3 = new unsigned char[TSS_CAPTION_LENGTH_MORE];
				if (pTtmlBuffer3) {
					memset(pTtmlBuffer3, NULL, TSS_CAPTION_LENGTH_MORE);
					iTtmlBuffLen3 = TSS_CAPTION_LENGTH_MORE;
				}
				else { return -1; }
			}
			unsigned char* pTtmlBuffer4 = NULL;
			int iTtmlBuffLen4 = 0;
			if (m_iCapUnitLength4 > 0) {
				pTtmlBuffer4 = new unsigned char[TSS_CAPTION_LENGTH_MORE];
				if (pTtmlBuffer4) {
					memset(pTtmlBuffer4, NULL, TSS_CAPTION_LENGTH_MORE);
					iTtmlBuffLen4 = TSS_CAPTION_LENGTH_MORE;
				}
				else { return -1; }
			}

			//std::string saveNameTtml;
			std::string saveNameTtml0 = "";
			std::string saveNameTtml1 = "";
			std::string saveNameTtml2 = "";
			std::string saveNameTtml3 = "";
			std::string saveNameTtml4 = "";
			int iTmp=0;
			long beginTime=0;
			long endTime=0;
			int iRomSound=0;
			int iAnimation = 0;


			//==============================================================================================================
			// 8単位符号からTTMLへ変換
			//==============================================================================================================
			//DRCSタイプ変換
			int iDataType1 = -1;
			int iDataType2 = -1;
			int iDataType3 = -1;
			int iDataType4 = -1;
			{
				if ((m_iCapUnitType1 == 0x30) || (m_iCapUnitType1 == 0x31)) {
					iDataType1 = 0;
				}
				if ((m_iCapUnitType2 == 0x30) || (m_iCapUnitType2 == 0x31)) {
					iDataType2 = 0;
				}
				if ((m_iCapUnitType3 == 0x30) || (m_iCapUnitType3 == 0x31)) {
					iDataType3 = 0;
				}
				if ((m_iCapUnitType4 == 0x30) || (m_iCapUnitType4 == 0x31)) {
					iDataType4 = 0;
				}
			}
			if (m_LogLevel) {
				APPLOG("[MAN],INF,%d,TssCapProcessManager,API_ConvJisToTtml(),[TSS] <IN0=%d:%02X> <IN1=%d:%02X> <IN2=%d:%02X> <IN3=%d:%02X> <IN2=%d:%02X>", m_id,
					m_CapUnitBuffer0_ManLength, 0x20,
					m_iCapUnitLength1, m_iCapUnitType1,
					m_iCapUnitLength2, m_iCapUnitType2,
					m_iCapUnitLength3, m_iCapUnitType3,
					m_iCapUnitLength4, m_iCapUnitType4
				);
			}

			ret = API_ConvJisToTtml(m_number, TSS_CONV_LOG_LEVEL, 2,
				m_CapUnitBuffer0_Man, m_CapUnitBuffer0_ManLength, pTtmlBuffer0, &iTtmlBuffLen0,
				m_CapUnitBuffer1, m_iCapUnitLength1, iDataType1, pTtmlBuffer1, &iTtmlBuffLen1,
				m_CapUnitBuffer2, m_iCapUnitLength2, iDataType2, pTtmlBuffer2, &iTtmlBuffLen2,
				m_CapUnitBuffer3, m_iCapUnitLength3, iDataType3, pTtmlBuffer3, &iTtmlBuffLen3,
				m_CapUnitBuffer4, m_iCapUnitLength4, iDataType4, pTtmlBuffer4, &iTtmlBuffLen4,
				&beginTime, &endTime, &iRomSound, &iAnimation
			);

			if (iRomSound >= 0) {
				m_CaptionDataRomSound = iRomSound;
				if (m_CaptionDataRomSound >= 16) {
					m_CaptionDataRomSound = m_CaptionDataRomSound - 6;
				}
			}
			else {
				m_CaptionDataRomSound = -1;
			}

			if (m_LogLevel) {
				if (m_CapUnitBuffer0_ManLength > 0) {
					APPLOG("[MAN],INF,%d,TssCapProcessManager,API_ConvJisToTtml(),[TSS] <IN0> IN=%d OUT=%d TYPE=%d", m_id, m_CapUnitBuffer0_ManLength, iTtmlBuffLen0, 0x20);
				}
				if (m_iCapUnitLength1 > 0) {
					APPLOG("[MAN],INF,%d,TssCapProcessManager,API_ConvJisToTtml(),[TSS] <IN1> IN=%d OUT=%d TYPE=%d", m_id, m_iCapUnitLength1, iTtmlBuffLen1, iDataType1);
				}
				if (m_iCapUnitLength2 > 0) {
					APPLOG("[MAN],INF,%d,TssCapProcessManager,API_ConvJisToTtml(),[TSS] <IN2> IN=%d OUT=%d TYPE=%d", m_id, m_iCapUnitLength2, iTtmlBuffLen2, iDataType2);
				}
				if (m_iCapUnitLength3 > 0) {
					APPLOG("[MAN],INF,%d,TssCapProcessManager,API_ConvJisToTtml(),[TSS] <IN3> IN=%d OUT=%d TYPE=%d", m_id, m_iCapUnitLength3, iTtmlBuffLen3, iDataType3);
				}
				if (m_iCapUnitLength4 > 0) {
					APPLOG("[MAN],INF,%d,TssCapProcessManager,API_ConvJisToTtml(),[TSS] <IN4> IN=%d OUT=%d TYPE=%d", m_id, m_iCapUnitLength4, iTtmlBuffLen4, iDataType4);
				}
				APPLOG("[MAN],INF,%d,TssCapProcessManager,API_ConvJisToTtml(),[TSS] <RET> BT=%d ET=%d RS=%d", m_id, beginTime, endTime, m_CaptionDataRomSound);
			}


			if (TSS_INTERFACE_SAVE)
			{
				std::string saveName;
				//std::string savePath = TSS_SAVE_BASE_PATH;
				std::string savePath = m_FileSavePath;

				if (iTtmlBuffLen0 > 0) {
					//saveName = StringUtil::Format("C:\\WORK\\RealTimeCaption\\RealTimeAppRelease\\temp\\Caption_%04d_Ttml.ttml", m_iPesCount);
					saveName = StringUtil::Format("%s\\Caption_%04d_Ttml.ttml", savePath.c_str(), m_iPesCount);
					//saveNameTtml = saveName;
					saveNameTtml0 = saveName;
					FILE* fpt;
					fpt = fopen(saveName.c_str(), "w+b");
					if (fpt == NULL) {
						return -1;
					}
					fseek(fpt, 0, SEEK_SET);
					fwrite(pTtmlBuffer0, iTtmlBuffLen0, 1, fpt);
					fclose(fpt);
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),Ttml=%s", m_id, saveName.c_str());
					}
				}

				if (iTtmlBuffLen1 > 0) {
					saveName = StringUtil::Format("%s\\Caption_%04d_Ttml_1.bin", savePath.c_str(), m_iPesCount);
					saveNameTtml1 = saveName;
					FILE* fpt;
					fpt = fopen(saveName.c_str(), "w+b");
					if (fpt == NULL) {
						return -1;
					}
					fseek(fpt, 0, SEEK_SET);
					fwrite(pTtmlBuffer1, iTtmlBuffLen1, 1, fpt);
					fclose(fpt);
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),Ttml_Bin1=%s", m_id, saveName.c_str());
					}
				}

				if (iTtmlBuffLen2 > 0) {
					saveName = StringUtil::Format("%s\\Caption_%04d_Ttml_2.bin", savePath.c_str(), m_iPesCount);
					saveNameTtml2 = saveName;
					FILE* fpt;
					fpt = fopen(saveName.c_str(), "w+b");
					if (fpt == NULL) {
						return -1;
					}
					fseek(fpt, 0, SEEK_SET);
					fwrite(pTtmlBuffer2, iTtmlBuffLen2, 2, fpt);
					fclose(fpt);
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),Ttml_Bin2=%s", m_id, saveName.c_str());
					}
				}

				if (iTtmlBuffLen3 > 0) {
					saveName = StringUtil::Format("%s\\Caption_%04d_Ttml_3.bin", savePath.c_str(), m_iPesCount);
					saveNameTtml3 = saveName;
					FILE* fpt;
					fpt = fopen(saveName.c_str(), "w+b");
					if (fpt == NULL) {
						return -1;
					}
					fseek(fpt, 0, SEEK_SET);
					fwrite(pTtmlBuffer3, iTtmlBuffLen3, 3, fpt);
					fclose(fpt);
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),Ttml_Bin3=%s", m_id, saveName.c_str());
					}
				}

				if (iTtmlBuffLen4 > 0) {
					saveName = StringUtil::Format("%s\\Caption_%04d_Ttml_4.bin", savePath.c_str(), m_iPesCount);
					saveNameTtml4 = saveName;
					FILE* fpt;
					fpt = fopen(saveName.c_str(), "w+b");
					if (fpt == NULL) {
						return -1;
					}
					fseek(fpt, 0, SEEK_SET);
					fwrite(pTtmlBuffer4, iTtmlBuffLen4, 2, fpt);
					fclose(fpt);
					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),Ttml_Bin4=%s", m_id, saveName.c_str());
					}
				}

			}

			//================================================================================================================================
			{
				std::string sCaptionCapFileame	= "page.ttml";
				std::string sCaptionDatFileame1	= "1";
				std::string sCaptionDatFileame2 = "2";
				std::string sCaptionDatFileame3 = "3";
				std::string sCaptionDatFileame4 = "4";
				int iAddFileCount = 0;
				int iZipRt = 0;
				int iZipAddSize = 512;
				///////////////////////////////////
				// 初期化
				///////////////////////////////////
				fileCompInit();
				///////////////////////////////////
				// ZIPするファイルを追加
				///////////////////////////////////
				if (m_CapUnitBuffer0_ManLength > 0) {

					//TTMLファイル
					if (iTtmlBuffLen0 > 0) {
						fileCompAdd(sCaptionCapFileame.c_str(), (int)sCaptionCapFileame.size(), pTtmlBuffer0, iTtmlBuffLen0);
						iZipAddSize += iTtmlBuffLen0;
						iZipAddSize += 256;
						iAddFileCount++;
					}
					//1ファイル
					if (m_iCapUnitLength1 > 0) {
						if (iTtmlBuffLen1 > 0) {
							fileCompAdd(sCaptionDatFileame1.c_str(), (int)sCaptionDatFileame1.size(), pTtmlBuffer1, iTtmlBuffLen1);
							iZipAddSize += iTtmlBuffLen1;
							iZipAddSize += 256;
							iAddFileCount++;
						}
					}
					//2ファイル
					if (m_iCapUnitLength2 > 0) {
						if (iTtmlBuffLen2 > 0) {
							fileCompAdd(sCaptionDatFileame2.c_str(), (int)sCaptionDatFileame2.size(), pTtmlBuffer2, iTtmlBuffLen2);
							iZipAddSize += iTtmlBuffLen2;
							iZipAddSize += 256;
							iAddFileCount++;
						}
					}
					//3ファイル
					if (m_iCapUnitLength3 > 0) {
						if (iTtmlBuffLen3 > 0) {
							fileCompAdd(sCaptionDatFileame3.c_str(), (int)sCaptionDatFileame3.size(), pTtmlBuffer3, iTtmlBuffLen3);
							iZipAddSize += iTtmlBuffLen3;
							iZipAddSize += 256;
							iAddFileCount++;
						}
					}
					//4ファイル
					if (m_iCapUnitLength4 > 0) {
						if (iTtmlBuffLen4 > 0) {
							fileCompAdd(sCaptionDatFileame4.c_str(), (int)sCaptionDatFileame4.size(), pTtmlBuffer4, iTtmlBuffLen4);
							iZipAddSize += iTtmlBuffLen4;
							iZipAddSize += 256;
							iAddFileCount++;
						}
					}
				}

				///////////////////////////////////
				// ZIPのアーカイブ化
				///////////////////////////////////
				unsigned char* pZipDst = NULL;
				int iZipSize = iZipAddSize;
				pZipDst = new unsigned char[iZipSize];
				if (pZipDst) {
					memset(pZipDst, NULL, iZipSize);
					iZipRt = fileCompExec(pZipDst, iZipSize, 0);
					if (iZipRt < 0) {
						//ERROR
					}
				}

				{
					//字幕用内部インタフェース（入力＆出力）
					m_out_inteface_size = TSS_RESOLUTION_2K_SIZE;
					printf("[INF] ApiCaptionUpdate(%d) IN Data = %d, OUT Data = %d\n", m_number, iZipSize, m_out_inteface_size);

					int ctrl_type = 0;
					//long image_interval = 0;
					//long iDuration = 0;
					//long iBegin = 0;
					//long iEnd = 0;
					//return 0;
					image_interval = 0;
					iDuration = 0;
					iBegin = 0;
					iEnd = 0;
					iAnimation = -1;

					m_OutputImageFileType = TSS_OUTPUT_IMAGE_FILE_TYPE;

					if (m_LogLevel) {
						APPLOG("[MAN],DBG,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #ApiCaptionUpdate(%d) Start", m_id, m_iPesCount, m_number);
						APPLOG("[MAN],DBG,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #ApiCaptionUpdate(%d) m_number=%d", m_id, m_iPesCount, m_number, m_number);
						APPLOG("[MAN],DBG,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #ApiCaptionUpdate(%d) DataBuffer=0x%x", m_id, m_iPesCount, m_number, pZipDst);
						APPLOG("[MAN],DBG,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #ApiCaptionUpdate(%d) DataSize=0x%02X", m_id, m_iPesCount, m_number, iZipSize);
						APPLOG("[MAN],DBG,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #ApiCaptionUpdate(%d) m_pOutInteface=%d", m_id, m_iPesCount, m_number, m_pOutInteface);
						APPLOG("[MAN],DBG,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #ApiCaptionUpdate(%d) m_OutputImageFileType=%d", m_id, m_iPesCount, m_number, m_OutputImageFileType);
						APPLOG("[MAN],DBG,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #ApiCaptionUpdate(%d) ctrl_type=%d", m_id, m_iPesCount, m_number, ctrl_type);
						APPLOG("[MAN],DBG,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #ApiCaptionUpdate(%d) image_interval=%d", m_id, m_iPesCount, m_number, image_interval);
						APPLOG("[MAN],DBG,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #ApiCaptionUpdate(%d) iDuration=%d", m_id, m_iPesCount, m_number, iDuration);
						APPLOG("[MAN],DBG,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #ApiCaptionUpdate(%d) iBegin=%d", m_id, m_iPesCount, m_number, iBegin);
						APPLOG("[MAN],DBG,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #ApiCaptionUpdate(%d) iEnd=%d", m_id, m_iPesCount, m_number, iEnd);
					}

					//int ret = ApiCaptionUpdate(m_number, pZipDst, iZipSize, m_pOutInteface, m_out_inteface_size, m_OutputImageFileType, &ctrl_type, &image_interval, &iDuration, &iBegin, &iEnd);
					int ret = ApiCaptionUpdate(m_number, pZipDst, iZipSize, m_pOutInteface, m_out_inteface_size, m_OutputImageFileType, &ctrl_type, &image_interval, &iDuration, &iBegin, &iEnd, iAnimation);
					if (ret < 0) {
						//printf("[ERR] ApiCaptionUpdate(%d) = %d\n", m_number, ret);
						if (m_LogLevel) {
							APPLOG("[MAN],ERR,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #ApiCaptionUpdate(%d) = %d", m_id, m_iPesCount, m_number, ret);
						}
						delete[] pZipDst;
						return -1;
					}

					if (iAnimation == -1) {
						//何もしない
					}
					else if ((iAnimation == 0) || (iAnimation == 1)) {
						if (m_LogLevel) {
							APPLOG("[MAN],ERR,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #ApiCaptionUpdate(%d) = %d (Flash=%d)", m_id, m_iPesCount, m_number, ret, iAnimation);
						}
					}
					else {
						//何もしない
					}

					//delete[] pZipDst;

					if (m_pOutputPointer == NULL) {
						delete[] pZipDst;
						return -1;
					}
					m_out_inteface_size = ret;

					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #COPY ST size=%d", m_id, m_iPesCount, m_out_inteface_size);
					}

					//memcpy(m_pOutputPointer, m_pOutInteface, m_out_inteface_size);
					{
						unsigned char* pOutInterfaceBuffer = NULL;
						unsigned long long i64OutTimestamp = 0;
						pOutInterfaceBuffer = new unsigned char[TSS_RESOLUTION_2K_SIZE+2];
						if (pOutInterfaceBuffer == NULL) {
							return -1;
						}
						//init
						memset(pOutInterfaceBuffer, NULL, TSS_RESOLUTION_2K_SIZE+2);
						//DATA COPY
						memcpy(pOutInterfaceBuffer, m_pOutInteface, m_out_inteface_size);
						//ROMS COPY
						pOutInterfaceBuffer[TSS_RESOLUTION_2K_SIZE] = (unsigned char)m_CaptionDataRomSound&0xFF;
						//PES NO
						pOutInterfaceBuffer[TSS_RESOLUTION_2K_SIZE+1] = (unsigned char)m_iPesCount & 0xFF;

						i64OutTimestamp = i64PageTimestap;

						//std::map<unsigned char*, unsigned long long> fifoCaption;
						int iCaptionCountBefore = (int)m_fifoCaption.size();
						m_fifoCaption.insert(map<unsigned char*, unsigned long long>::value_type(pOutInterfaceBuffer, i64OutTimestamp));
						int iCaptionCountAfter = (int)m_fifoCaption.size();
						if (m_LogLevel) {
							APPLOG("[MAN],INF,%d,TssCapProcessManager,ThreadProc(),[MAP] #INS Count=%d->%d", m_id, iCaptionCountBefore, iCaptionCountAfter);
						}
						if (m_LogLevel) {
							APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[CAP-IN] PES=%04d MAP-INS STC=%016X", m_id, m_iPesCount, i64OutTimestamp);
						}
					}


					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #COPY EN size=%d", m_id, m_iPesCount, m_out_inteface_size);
					}

					if (m_LogLevel) {
						APPLOG("[MAN],DBG,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #ApiCaptionUpdate(%d) End", m_id, m_iPesCount, m_number);
					}

					if (TSS_INTERFACE_SAVE) {
						FILE *fpt = NULL;
						std::string savePath = m_FileSavePath;
						std::string sPageZipTemp = m_sInputTempPageZipFile;
						std::string saveName = sPageZipTemp + ".argb";
						saveName = StringUtil::Format("%s\\Caption_%04d_Image", savePath.c_str(), m_iPesCount);

						if (m_LogLevel) {
							int packetId = 0;
							APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #SAVE path=%s", m_id, m_iPesCount, saveName.c_str());
						}

						//saveName += ".argb";
						if (m_OutputImageFileType == 0) {
							//saveName += Conv::IntToDecStr(m_output_count);
							saveName += ".argb";
						}
						else if (m_OutputImageFileType == 1) {
							//saveName += Conv::IntToDecStr(m_output_count);
							saveName += ".png";
						}
						else if (m_OutputImageFileType == 2) {
							//saveName += Conv::IntToDecStr(m_output_count);
							saveName += ".bmp";
						}

						if (m_LogLevel) {
							int packetId = 0;
							APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #SAVE name=%s", m_id, m_iPesCount, saveName.c_str());
						}

						fpt = fopen(saveName.c_str(), "w+b");
						if (fpt == NULL) {
							return -1;
						}
						fseek(fpt, 0, SEEK_SET);
						fwrite(m_pOutInteface, m_out_inteface_size, 1, fpt);
						fclose(fpt);

						if (m_LogLevel) {
							int packetId = 0;
							APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #SAVE=%s", m_id, m_iPesCount, saveName.c_str());
						}
					}

					//廃止
					//m_DataTriggerFlag = 1;

					if (m_LogLevel) {
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #OUT ApiCaptionUpdate(NUMBER=%d,IN-SIZE=%d,IMG-TYPE=%d)", m_id, m_iPesCount, m_number, iZipSize, m_OutputImageFileType);
					}

					m_output_count++;

					if (m_LogLevel) {
						int packetId = 0;
						APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #IF ED -----", m_id, m_iPesCount);
					}
				}

				///////////////////////////////////
				// 後処理
				///////////////////////////////////
				fileCompDist();

				free(pZipDst);
			}
			//================================================================================================================================

			//delete[] pTtmlBuffer;
			if (pTtmlBuffer0) {
				delete[] pTtmlBuffer0;
			}
			if (pTtmlBuffer1) {
				delete[] pTtmlBuffer1;
			}
			if (pTtmlBuffer2) {
				delete[] pTtmlBuffer2;
			}
			if (pTtmlBuffer3) {
				delete[] pTtmlBuffer3;
			}
			if (pTtmlBuffer4) {
				delete[] pTtmlBuffer4;
			}

			if (pUtf8Buffer) {
				delete[] pUtf8Buffer;
			}

		}
	}

	image_interval = 0;
	iDuration = 0;
	iBegin = 0;
	iEnd = 0;

	return 0;
}

int TssCapProcessManager::PesBufferClear() {
	if (m_pPesBuffer) {
		delete[] m_pPesBuffer;
		m_pPesBuffer = NULL;
	}
	m_iPesBufferLength = 0;
	m_iPesTargetLength = 0;
	return 0;
}

int TssCapProcessManager::PesBufferInit(int size) {
	PesBufferClear();

	m_pPesBuffer = new unsigned char[size + 1];
	if (m_pPesBuffer == NULL) {
		return -1;
	}
	memset(m_pPesBuffer, NULL, size + 1);
	m_iPesTargetLength = size;
	return 0;
}

int TssCapProcessManager::PesBufferSet(unsigned char* pdata, int size) {
	if ((m_iPesBufferLength + size) > m_iPesTargetLength) {
		return -1;
	}
	unsigned char* pBuffer = m_pPesBuffer + m_iPesBufferLength;
	memcpy(pBuffer, pdata, size);
	m_iPesBufferLength += size;
	if (m_iPesBufferLength >= m_iPesTargetLength) {
		return 100;
	}
	return 0;
}

int TssCapProcessManager::AppendListener(ITssCapCaptureListener* pListener)
{
	if (pListener == NULL) {
		return -1;
	}

	m_vecListener.push_back(pListener);

	if (m_LogLevel) {
		APPLOG("[MAN],INF,%d,TssCapProcessManager,AppendListener(),Listener Add", m_id);
	}

	return 0;
}

int TssCapProcessManager::RemoveListener(ITssCapCaptureListener* pListener)
{
	int i = 0;
	if (pListener == NULL) {
		return -1;
	}

	// 全要素に対するループ
	for (auto itr = m_vecListener.begin(); itr != m_vecListener.end(); ) {
		if ((*itr) == (pListener)) {
			itr = m_vecListener.erase(itr);
		}
		else {
			++itr;
		}
	}

	if (m_LogLevel) {
		APPLOG("[MAN],INF,%d,TssCapProcessManager,RemoveListener(),Listener Remove", m_id);
	}

	return 0;
}

int TssCapProcessManager::EventError(unsigned long ErrorCode)
{
	int id = GetId();

	LISTENER_VECTOR::iterator ite;
	for (ite = m_vecListener.begin(); ite != m_vecListener.end(); ite++)
	{
		//エラーイベント通知
		(*ite)->ManApiEventError(id, ErrorCode);
	}

	return 0;
}

int TssCapProcessManager::EventUpdateCaptionPlane()
{
	int id = GetId();

	if (m_LogLevel) {
		APPLOG("[MAN],INF,%d,TssCapProcessManager,EventUpdateCaptionPlane() #S", m_id);
	}

	LISTENER_VECTOR::iterator ite;
	for (ite = m_vecListener.begin(); ite != m_vecListener.end(); ite++)
	{
		//画像出力通知
		(*ite)->OutputApiEventUpdateCaptionPlane(id);
		if (m_LogLevel) {
			APPLOG("[MAN],INF,%d,TssCapProcessManager,EventUpdateCaptionPlane()", m_id);
		}

		//内蔵音通知
		if (m_CaptionDataRomSound >= 0) {
			(*ite)->OutputApiEventUpdateCaptionSound(id, m_CaptionDataRomSound);
			if (m_LogLevel) {
				APPLOG("[MAN],INF,%d,TssCapProcessManager,OutputApiEventUpdateCaptionSound(%d)", m_id, m_CaptionDataRomSound);
			}
			m_CaptionDataRomSound = -1;
		}
	}

	if (m_LogLevel) {
		APPLOG("[MAN],INF,%d,TssCapProcessManager,EventUpdateCaptionPlane() #E", m_id);
	}

	return 0;
}

void TssCapProcessManager::UpdateStop(int flag)
{
	if (flag == 1) {
		m_UpdateStop = 1;
	}
	else {
		m_UpdateStop = 0;
	}
}

void TssCapProcessManager::PauseStop()
{
	if (m_pClockTime) {
		m_pClockTime->UpdateStop(1);
	}
	UpdateStop(1);
	return;
}

void TssCapProcessManager::PauseStart()
{
	if (m_pClockTime) {
		m_pClockTime->UpdateStop(0);
	}
	UpdateStop(0);
	return;
}

int TssCapProcessManager::ThreadInitProc() {
	m_StartFlag = 1;
	m_ThreadFlag = 1;
	if (m_pClockTime) {
		m_pClockTime->StartThread();
	}
	return 0;
}

int TssCapProcessManager::ThreadProc() {

	int ret = 0;
	int iCount = 0;
	m_ThreadFlag = 1;
	m_DataTriggerFlag = 0;
	int iCaptionFifoCount = 0;
	int iCaptionDisplayTimeCheck = 0;

	unsigned char* pOutputBuffer = NULL;
	pOutputBuffer = new unsigned char[TSS_RESOLUTION_2K_SIZE];
	if (pOutputBuffer == NULL) {
		return -1;
	}
	//init
	memset(pOutputBuffer, NULL, TSS_RESOLUTION_2K_SIZE);


	if (m_LogLevel) {
		APPLOG("[MAN],INF,%d,TssCapProcessManager,ThreadProc() DataTriggerFlag=%d type=%d", m_id, m_DataTriggerFlag, m_type);
	}

	do {

		if (m_ThreadFlag==0) {
			break;
		}

		if (m_UpdateStop) {
			Sleep(50); //50msec
			continue;
		}

		if (m_type == TSSLIBMAN_TYPE_CAPTION) {

			iCaptionFifoCount = (int)m_fifoCaption.size();

			//if (m_DataTriggerFlag) {
			{

				if (iCaptionFifoCount > 0) {

					unsigned char* pOutInterfaceBuffer = NULL;
					unsigned long long i64OutTimestampSTC = 0;
					int iRomSound = 0;
					int iPrsCountOutput = 0;

					if (iCaptionDisplayTimeCheck == 0) {

						//初めの項目を取得
						map<unsigned char*, unsigned long long, int>::iterator it = m_fifoCaption.begin();
						//std::map<unsigned char*, unsigned long long> fifoCaption;
						//fifoCaption.insert(map<unsigned char*, unsigned long long>::value_type(pOutInterfaceBuffer, i64OutTimestamp));

						//値を取得
						pOutInterfaceBuffer = it->first;
						memset(pOutputBuffer, NULL, TSS_RESOLUTION_2K_SIZE);
						memcpy(pOutputBuffer, pOutInterfaceBuffer, TSS_RESOLUTION_2K_SIZE);
						i64OutTimestampSTC = (it->second) * 300;
						//i64OutTimestampSTC += TSS_TIMESTAMP_DELTA * 27000;

						iRomSound = pOutInterfaceBuffer[TSS_RESOLUTION_2K_SIZE];
						if (iRomSound == 0xFF) {
							iRomSound = -1;
						}
						iPrsCountOutput = pOutInterfaceBuffer[TSS_RESOLUTION_2K_SIZE + 1];

						//mapから削除
						int iCaptionCountBefore = (int)m_fifoCaption.size();
						m_fifoCaption.erase(it);
						int iCaptionCountAfter = (int)m_fifoCaption.size();
						if (m_LogLevel) {
							APPLOG("[MAN],INF,%d,TssCapProcessManager,ThreadProc(),[MAP] #DEL Count=%d->%d", m_id, iCaptionCountBefore, iCaptionCountAfter);
						}
						//メモリを破棄
						if (pOutInterfaceBuffer) {
							delete[] pOutInterfaceBuffer;
						}

						iCaptionDisplayTimeCheck = 1;
					}

					//ここで表示時刻のチェック
					if (iCaptionDisplayTimeCheck) {
#if	TSS_TIMESTAMP_NOCHECK
						ret = 100;
#else
						if (m_pClockTime) {
							ret = m_pClockTime->CheckTimeStamp(i64OutTimestampSTC, (TSS_TIMESTAMP_DELTA * 27000));
						}
						else {
							ret = 1001;
						}
						if (m_LogLevel) {
							APPLOG("[MAN],INF,%d,TssCapProcessManager,ThreadProc(),CheckTimeStamp(%016X)=%d", m_id, i64OutTimestampSTC, ret);
						}
#endif
						//字幕を破棄
						if ((ret == 1000)||(ret == 1001)) {
							//廃止
							//m_DataTriggerFlag = 0;
							if (m_LogLevel) {
								APPLOG("[MAN],INF,%d,TssCapProcessManager,ThreadProc(),[CAP-OT] SKEP STC=%016X RS=%d", m_id, iPrsCountOutput, i64OutTimestampSTC, m_CaptionDataRomSound);
							}
							iCaptionDisplayTimeCheck = 0;
						}
						//字幕を表示
						else if (ret == 100) {
							//廃止
							//m_DataTriggerFlag = 0;

							//メモリにコピー							
							//memcpy(m_pOutputPointer, pOutInterfaceBuffer, TSS_RESOLUTION_2K_SIZE);
							memcpy(m_pOutputPointer, pOutputBuffer, TSS_RESOLUTION_2K_SIZE);

							//ROMサウンド
							m_CaptionDataRomSound = iRomSound;

							//更新処理
							EventUpdateCaptionPlane();

							if (m_LogLevel) {
								APPLOG("[MAN],INF,%d,TssCapProcessManager,ThreadProc(),[CAP-OT] PES=%04d STC=%016X RS=%d", m_id, iPrsCountOutput, i64OutTimestampSTC, m_CaptionDataRomSound);
							}

							iCaptionDisplayTimeCheck = 0;

#if 0
							if (TSS_INTERFACE_SAVE) {
								FILE *fpt = NULL;
								std::string savePath = m_FileSavePath;
								std::string sPageZipTemp = m_sInputTempPageZipFile;
								std::string saveName = sPageZipTemp + ".argb";
								saveName = StringUtil::Format("%s\\Caption_%04d_ImageCopy", savePath.c_str(), m_iPesCount);

								if (m_LogLevel) {
									int packetId = 0;
									APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #SAVE path=%s", m_id, m_iPesCount, saveName.c_str());
								}

								//saveName += ".argb";
								if (m_OutputImageFileType == 0) {
									//saveName += Conv::IntToDecStr(m_output_count);
									saveName += ".argb";
								}
								else if (m_OutputImageFileType == 1) {
									//saveName += Conv::IntToDecStr(m_output_count);
									saveName += ".png";
								}
								else if (m_OutputImageFileType == 2) {
									//saveName += Conv::IntToDecStr(m_output_count);
									saveName += ".bmp";
								}

								if (m_LogLevel) {
									APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #SAVE name=%s", m_id, m_iPesCount, saveName.c_str());
								}

								fpt = fopen(saveName.c_str(), "w+b");
								if (fpt == NULL) {
									return -1;
								}
								fseek(fpt, 0, SEEK_SET);
								fwrite(m_pOutInteface, m_out_inteface_size, 1, fpt);
								fclose(fpt);

								if (m_LogLevel) {
									int packetId = 0;
									APPLOG("[MAN],INF,%d,TssCapProcessManager,ParsePesData(),[TSS] PES=%04d #SAVE=%s", m_id, m_iPesCount, saveName.c_str());
								}
							}
#endif
							//更新処理
							//EventUpdateCaptionPlane();
						}
						//字幕を待機
						else {
							//Sleep(50); //50msec
							//continue;
						}
					}
					else {
						//Sleep(50); //50msec
						//continue;
					}
				}
			}
		}

		Sleep(50); //50msec

	} while (1);

	m_ThreadFlag = 0;

	delete[] pOutputBuffer;
	pOutputBuffer = NULL;

	return 0;
}

int TssCapProcessManager::ThreadTermProc() {
	m_ThreadFlag = 0;
	if (m_pClockTime) {
		m_pClockTime->ThreadStop();
	}
	return 0;
}

int TssCapProcessManager::UpdateDataOutput()
{
	if (m_LogLevel) {
		APPLOG("[MAN],INF,%d,TssCapProcessManager,UpdateDataOutput(),ApiMmDataUpdate()", m_id);
	}

	if (m_pOutputPointer == NULL) {
		printf("[ERR] m_pOutputPointer = NULL\n");
		return -1;
	}
	m_out_inteface_size = ret;


	if (TSS_INTERFACE_SAVE) {
		FILE *fpt = NULL;
		std::string saveName = m_sOutputImageFile;
		if (m_OutputImageFileType == 0) {
			saveName += Conv::IntToDecStr(m_output_count);
			saveName += ".argb";
		}
		else if (m_OutputImageFileType == 1) {
			saveName += Conv::IntToDecStr(m_output_count);
			saveName += ".png";
		}
		else if (m_OutputImageFileType == 2) {
			saveName += Conv::IntToDecStr(m_output_count);
			saveName += ".bmp";
		}
		fpt = fopen(saveName.c_str(), "w+b");
		if (fpt == NULL) {
			return -1;
		}
		fseek(fpt, 0, SEEK_SET);
		fwrite(m_pOutInteface, m_out_inteface_size, 1, fpt);
		fclose(fpt);
		m_Time.Now(); COUT("%s [TSS] IMAGE = %s", m_Time.GetHHMMSSFFF().c_str(), saveName.c_str());
		if (m_LogLevel) {
			APPLOG("[MAN],INF,%d,TssCapProcessManager,UpdateDataOutput(),SeveFile=%s", m_id, saveName.c_str());
		}
	}

	memcpy(m_pOutputPointer, m_pOutInteface, m_out_inteface_size);

	m_output_count++;

	return 0;
}


