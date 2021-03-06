/*
//#######################################################################
//## TssCapProcessManagerApi.cpp
//#######################################################################
*/

//**************************************************
// include
//**************************************************
#include <stdio.h>
#include <Util/Path.h>
#include <Util/Conv.h>
#include <App/AppLogger.h>
#include "TssCapProcessManagerApi.h"
#include "TssCapProcessManager.h"

//**************************************************
// define
//**************************************************
//VERSION
#define TSSLIB_TYPE_CAPTION_TR_VERSION		1000
#define TSSLIB_TYPE_CAPTION_TR_MAXCOUNT		8
#define MPM_OUTPUT_PUTIMAGE 1
#define TSS_PROCESS_INTERFACE_DEBUG 0
#define TSSLIB_INITIAL_NAME	".\\TssCapProcessManager.ini"
#define TSSLIB_INITIAL_FILE	"C:\\work\\app\\TssCapProcessManager.ini"
#define TSSLIB_LOG_PATH		"C:\\work\\app\\Log\\TssCap"
#define TSS_API_DEBUG_LOG 0


//**************************************************
// global parameter
//**************************************************
TssCapProcessManager *gObjectTssCaption[TSSLIB_TYPE_CAPTION_TR_MAXCOUNT];
int gObjectTssCount = 0;
int gObjectTssCaptionCount = 0;
std::string gIniTssFilename = TSSLIB_INITIAL_NAME;
int gObjectTssLogLevel = 1;
std::string gObjectTssLogPath = TSSLIB_LOG_PATH;

int gObjectTssCapCreateFlag = 0;
int gObjectTssCapInitialSettingFlag = 0;


//**************************************************
// function
//**************************************************
int ManFuncGetNewNumber(int type)
{
	TssCapProcessManager *pObject = NULL;

	if (type == TSSLIB_TYPE_CAPTION_TR) {

		for (int i = 0; i < TSSLIB_TYPE_CAPTION_TR_MAXCOUNT; i++) {
			if (gObjectTssCaption[i] == NULL) {
				return (i + 1);
			}
		}
	}

	return -1;
}

TssCapProcessManager *ManFuncGetObject(int type, int number)
{
	TssCapProcessManager *pObject = NULL;

	if (type == TSSLIB_TYPE_CAPTION_TR) {
		if (number < 1) {
			return NULL;
		}
		if (number > TSSLIB_TYPE_CAPTION_TR_MAXCOUNT) {
			return NULL;
		}
		pObject = gObjectTssCaption[number - 1];
	}
	else { pObject = NULL; }

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" ManFuncGetObject()                 :type=%d munber=%d pObject=%p \n", type, number, pObject);
#endif

	return pObject;
}

TssCapProcessManager *ManFuncGetObjectFromId(int id)
{
	TssCapProcessManager *pObject = NULL;

	int type = (id >> 12) - 3 + 10;
	int number = id & 0x000F;

	if (type == TSSLIB_TYPE_CAPTION_TR) {
		if (number < 1) {
			return NULL;
		}
		if (number > TSSLIB_TYPE_CAPTION_TR_MAXCOUNT) {
			return NULL;
		}
		pObject = gObjectTssCaption[number - 1];
	}
	else { pObject = NULL; }

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" ManFuncGetObjectFromId()           :id=%d pObject=%p \n", id, pObject);
#endif

	return pObject;
}

int ManFuncGetInstanceId(int type, int number)
{
	int iInstanceId = 0;

	iInstanceId = (type-10+1) * 512 * 8 + (512 * 16) + number;

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" ManFuncGetInstanceId()             :type=%d munber=%d id=%d(0x%08X) \n", type, number, iInstanceId, (unsigned int)iInstanceId);
#endif

	return iInstanceId;
}

int ManFuncGetTypeFromId(int id)
{
	int iType = 0;
	if (     id == 0x3001) { iType = TSSLIB_TYPE_CAPTION_TR; }
	else if (id == 0x3002) { iType = TSSLIB_TYPE_CAPTION_TR; }
	else if (id == 0x3003) { iType = TSSLIB_TYPE_CAPTION_TR; }
	else if (id == 0x3004) { iType = TSSLIB_TYPE_CAPTION_TR; }
	else if (id == 0x3005) { iType = TSSLIB_TYPE_CAPTION_TR; }
	else if (id == 0x3006) { iType = TSSLIB_TYPE_CAPTION_TR; }
	else if (id == 0x3007) { iType = TSSLIB_TYPE_CAPTION_TR; }
	else if (id == 0x3008) { iType = TSSLIB_TYPE_CAPTION_TR; }
	else if (id == 0x4001) { iType = TSSLIB_TYPE_MTMEDIA_TR; }
	else if (id == 0x4002) { iType = TSSLIB_TYPE_MTMEDIA_TR; }
	else if (id == 0x4003) { iType = TSSLIB_TYPE_MTMEDIA_TR; }
	else if (id == 0x4004) { iType = TSSLIB_TYPE_MTMEDIA_TR; }
	else { iType = -1; }

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" ManFuncGetTypeFromId()             :id=%d type=%d \n", id, iType);
#endif

	return iType;
}

int ManFuncInitialSetting(int iInstanceId)
{
	std::string m_INI_FILENAME;
	std::string sGroupName;
	std::string sParamName;
	std::string sTemp;
	char sBuff[64];
	int iTemp;

	//INIファイル
	m_INI_FILENAME = gIniTssFilename;

	if (Path::IsExist(m_INI_FILENAME) == 0) {
		printf("[API],ERR,%d,TssCapProcessManagerApi,ManFuncInitialSetting(),Ini File Error Path=%s", iInstanceId, m_INI_FILENAME.c_str());
		return TSSLIB_ERROR;
	}

	//[System]
	sGroupName = "System";

	//LogLevel
	sParamName = "Log";
	iTemp = GetPrivateProfileIntA((LPCTSTR)sGroupName.c_str(), (LPCTSTR)sParamName.c_str(), -1, (LPCTSTR)m_INI_FILENAME.c_str());
	if (iTemp == 1) {
		gObjectTssLogLevel = 1;
	}
	else {
		gObjectTssLogLevel = 0;
	}

	//m_LogPath
	//ChannelName
	memset(sBuff, NULL, 64);
	sParamName = "LogPath";
	iTemp = ::GetPrivateProfileStringA((LPCTSTR)sGroupName.c_str(), (LPCTSTR)sParamName.c_str(), "", (LPSTR)sBuff, 64, (LPCTSTR)m_INI_FILENAME.c_str());
	sTemp = sBuff;
	if (Path::IsDir(sTemp) == 1) {
		gObjectTssLogPath = sTemp;
	}
	else {
		Path::MakeDir(sTemp);

		if (Path::IsDir(sTemp) == 1) {
			gObjectTssLogPath = sTemp;
		}
		else {
			printf("[API],ERR,%d,TssCapProcessManagerApi,ManFuncInitialSetting(),Log Path Error Path=%s", iInstanceId, sTemp.c_str());
			if (gObjectTssLogLevel == 1) {
				gObjectTssLogPath = TSSLIB_LOG_PATH;
				if (Path::IsDir(gObjectTssLogPath) == 0) {
					gObjectTssLogLevel = 0;
					//return TSSLIB_ERROR;
				}
			}
		}
	}

	//LogSetting
	AppLogger::Init(gObjectTssLogPath, "DllTssCap");

	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,ManFuncInitialSetting(),Log Level=%d", iInstanceId, gObjectTssLogLevel);
	}

	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,ManFuncInitialSetting(),Log Path =%s", iInstanceId, gObjectTssLogPath.c_str());
	}

	return 0;
}

//--------------------------------------------------
// TssCapManApiGetVersion()
//--------------------------------------------------
int TssCapManApiGetVersion()
{
	int version=0;
	int type = TSSLIB_TYPE_CAPTION_TR;
	version = TSSLIB_TYPE_CAPTION_TR_VERSION;

	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiCreateInstance(),Type=%d Version=%d", 0, type, version);
	}

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" TssCapManApiCreateInstance()                 :type=%d vresion=%d\n", type, version);
#endif
	return version;
}

//--------------------------------------------------
// TssCapManApiCreateInstance()
//--------------------------------------------------
//int TssCapManApiCreateInstance(
int TssCapManApiCreateInstance(
//	int type,
	int resolution
){
	int iObjectCount=-1;
	TssCapProcessManager *pObject = NULL;
	int iCount = 0;
	int iInstanceId = 0;
	int ret = 0;
	int iWaiteCount = 0;
	int type = TSSLIB_TYPE_CAPTION_TR;
	int iNumber = -1;

	if (gObjectTssCapCreateFlag) {
		do {
			if (gObjectTssCapCreateFlag == 0) {
				break;
			}
			iWaiteCount++;
			Sleep(100);
			if (iWaiteCount > 100) {
				gObjectTssCapCreateFlag = 0;
				break;
			}
		} while (1);
	}
	gObjectTssCapCreateFlag = 1;

	if (type == TSSLIB_TYPE_CAPTION_TR) {
		iCount = gObjectTssCaptionCount;
	}
	else {
		gObjectTssCapCreateFlag = 0;
		return TSSLIB_ERROR;
	}

	//init
	if (iCount == 0) {
		if (type == TSSLIB_TYPE_CAPTION_TR) {
			for (int i = 0; i < TSSLIB_TYPE_CAPTION_TR_MAXCOUNT; i++) {
				gObjectTssCaption[i] = NULL;
			}
		}
	}

	if (type == TSSLIB_TYPE_CAPTION_TR) {
		if (iCount < TSSLIB_TYPE_CAPTION_TR_MAXCOUNT) {
		}
		else {
			gObjectTssCapCreateFlag = 0;
			return TSSLIB_ERROR;
		}
	}
	else {
		gObjectTssCapCreateFlag = 0;
		return TSSLIB_ERROR;
	}

	if (type == TSSLIB_TYPE_CAPTION_TR) {

		if (gObjectTssCapInitialSettingFlag == 0) {

			//IniFile
			gIniTssFilename = TSSLIB_INITIAL_NAME;

			if (Path::IsExist(gIniTssFilename) == 0) {
				printf("[API],WAR,%d,TssCapProcessManagerApi,TssCapProcessManager(),Ini File Error Path=%s", iInstanceId, gIniTssFilename.c_str());

				gIniTssFilename = TSSLIB_INITIAL_FILE;
				if (Path::IsExist(gIniTssFilename) == 0) {
					printf("[API],ERR,%d,TssCapProcessManagerApi,TssCapProcessManager(),Ini File Error Path=%s", iInstanceId, gIniTssFilename.c_str());
					gObjectTssCapCreateFlag = 0;
					return TSSLIB_ERROR;
				}
			}

			//LogPath/LogLevel
			iInstanceId = ManFuncGetInstanceId(type, (gObjectTssCaptionCount+1));

			ret = ManFuncInitialSetting(iInstanceId);
			if (ret < 0) {
				printf("[API],ERR,%d,TssDatProcessManagerApi,TssDatProcessManager(),ManFuncInitialSetting()=%d", iInstanceId, ret);
				gObjectTssCapCreateFlag = 0;
				return TSSLIB_ERROR;
			}
			if (gObjectTssLogLevel) {
				APPLOG("[API/DLL],Level(INF/WAR/DBG),id,Param1,Param2,FreeText");
			}
			if (gObjectTssLogLevel) {
				APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiCreateInstance(),Start Instance #################################################", iInstanceId);
				APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiCreateInstance(%d, %d) Log=%d", iInstanceId, type, resolution, gObjectTssLogLevel);
			}
		}

		pObject = new TssCapProcessManager(gIniTssFilename, gObjectTssLogPath, gObjectTssLogLevel);
		if (pObject == NULL) {
			gObjectTssCapCreateFlag = 0;
			return TSSLIB_ERROR;
		}
		if (gObjectTssLogLevel) {
			APPLOG("[API],INF,%d,TssCapProcessManagerApi,new TssCapProcessManager(%s, %s, %d)", iInstanceId, gIniTssFilename.c_str(), gObjectTssLogPath.c_str(), gObjectTssLogLevel);
		}

		gObjectTssCount++;
	}
	else {
		gObjectTssCapCreateFlag = 0;
		return TSSLIB_ERROR;
	}
	
	if (type == TSSLIB_TYPE_CAPTION_TR) {
		//gObjectTssCaptionCount++;
		//iCount = gObjectTssCaptionCount;

		iNumber = ManFuncGetNewNumber(type);
		if (gObjectTssLogLevel) {
			APPLOG("[API],INF,%d,TssCapProcessManagerApi,ManFuncGetNewNumber(%d) Number=%d", iInstanceId, type, iNumber);
		}
		if (iNumber < 0) {
			gObjectTssCapCreateFlag = 0;
			return TSSLIB_ERROR;
		}

		//gObjectTssCapCreateFlag++;
		gObjectTssCaptionCount++;
		iCount = iNumber;
	}
	else {
		gObjectTssCapCreateFlag = 0;
		return TSSLIB_ERROR;
	}

	//SetPreviewLog
	if (gObjectTssCapInitialSettingFlag == 0) {
#if TSS_PROCESS_MMCTRL
		//PreviewLogSetting
		pObject->InitPreview(gObjectTssLogPath);
#endif
	}

	//Version
	int version = TssCapManApiGetVersion();
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiCreateInstance(),TssDatManApiGetVersion()=%d", iInstanceId, version);
	}

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" TssCapManApiCreateInstance()             :Create(%d,%d) START\n", iCount, type);
	if (gObjectTssLogLevel) { 
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiCreateInstance(),Create(%d,%d)", iInstanceId, iCount, type);
	}
#endif

	if (type == TSSLIB_TYPE_CAPTION_TR) {
		ret = pObject->Create(iCount, type, resolution);
		if (ret < 0) {
			if (gObjectTssLogLevel) {
				APPLOG("[API],ERR,%d,TssCapProcessManagerApi,TssCapManApiCreateInstance(),Create()=%d", iInstanceId, ret);
			}
			gObjectTssCapCreateFlag = 0;
			return ret;
		}
	}
	else {
		gObjectTssCapCreateFlag = 0;
		return TSSLIB_ERROR;
	}

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" TssCapManApiCreateInstance()             :Create(%d,%d) END\n", iCount, type);
	if (gObjectTssLogLevel) { 
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiCreateInstance(),Create(%d,%d) END", iInstanceId, iCount, type);
	}
#endif

	if (type == TSSLIB_TYPE_CAPTION_TR) {

		iInstanceId = ManFuncGetInstanceId(type, iCount);

		pObject->SetId(iInstanceId);
		if (gObjectTssLogLevel) {
			APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiCreateInstance(),SetId(%d)", iInstanceId, iInstanceId);
		}

		pObject->SetType(type);
		if (gObjectTssLogLevel) {
			APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiCreateInstance(),SetType(%d)", iInstanceId, type-10);
		}

		pObject->SetNumber(iCount);
		if (gObjectTssLogLevel) {
			APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiCreateInstance(),SetNumber(%d)", iInstanceId, iCount);
		}
	}
	else {
		gObjectTssCapCreateFlag = 0;
		return TSSLIB_ERROR;
	}

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" TssCapManApiCreateInstance()             :type=%d munber=%d id=%d(0x%08X) \n", type, iCount, iInstanceId, (unsigned int)iInstanceId);
	if (gObjectTssLogLevel) { 
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiCreateInstance(),type=%d munber=%d id=%d(0x%08X)", iInstanceId, type-10, iCount, iInstanceId, (unsigned int)iInstanceId);
	}
#endif

#if TSS_PROCESS_INTERFACE_DEBUG
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiCreateInstance(),index=%d", iInstanceId, (iCount-1));
		if (type == TSSLIB_TYPE_CAPTION_TR) {
			APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiCreateInstance(),gObjectTssCaption(%d)=%p", iInstanceId, (iCount - 1), gObjectTssCaption[iCount - 1]);
		}
	}
#endif

	if (type == TSSLIB_TYPE_CAPTION_TR) {
		gObjectTssCaption[iCount - 1] = pObject;
	}
	else {
		gObjectTssCapCreateFlag = 0;
		return TSSLIB_ERROR;
	}

#if TSS_PROCESS_INTERFACE_DEBUG
	if (type == TSSLIB_TYPE_CAPTION_TR) {
		printf(" TssCapManApiCreateInstance()             :pObject=%p \n", pObject);
		if (gObjectTssLogLevel) {
			APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiCreateInstance(),pObject=%p", iInstanceId, pObject);
		}
	}
	else {
		gObjectTssCapCreateFlag = 0;
		return TSSLIB_ERROR;
	}
#endif

	if (gObjectTssCapInitialSettingFlag == 0) {
		gObjectTssCapInitialSettingFlag++;
	}

	gObjectTssCapCreateFlag = 0;

	return iInstanceId;
}

//--------------------------------------------------
// TssCapManApiDeleteInstance()
//--------------------------------------------------
int TssCapManApiDeleteInstance(
	int id
){
	TssCapProcessManager *pObject = NULL;
	int iType = 0;
	int iCount = 0;
	int iNumber = 0;
	int ret = 0;

#if TSS_PROCESS_INTERFACE_DEBUG
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiDeleteInstance(),Release(%d,%d) START", id, iNumber, iType);
	}
#endif

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		return TSSLIB_ERROR_INSTANCEID;
	}
	iNumber = pObject->GetNumber();
	iType = pObject->GetType();

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" TssCapManApiDeleteInstance()             :Release(%d,%d) START\n", iNumber, iType);
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiDeleteInstance(),Release(%d,%d) START", id, iNumber, iType);
	}
#endif

#if TSS_PROCESS_MMCTRL
	ret = pObject->Release(iNumber, iType);
	if (ret < 0) {
		if (gObjectTssLogLevel) {
			APPLOG("[API],ERR,%d,TssCapProcessManagerApi,TssCapManApiDeleteInstance(),Release(%d,%d) =%d", id, iNumber, iType, ret);
		}
		//return ret;
	}
	//if (gObjectTssCount == 0) {
	//	pObject->ExitPreview();
	//}
#endif

	if (gObjectTssCaption[(iNumber - 1)]) {
		delete gObjectTssCaption[(iNumber - 1)];
		gObjectTssCaption[(iNumber - 1)] = NULL;
	}

	//if (pObject) {
	//	delete pObject;
	//	pObject = NULL;
	//}

#if TSS_PROCESS_INTERFACE_DEBUG
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiDeleteInstance(),Release(%d,%d) END", id, iNumber, iType);
	}
#endif

	iType = ManFuncGetTypeFromId(id);

	if (iType == TSSLIB_TYPE_CAPTION_TR) {
		gObjectTssCaptionCount--;
		if (gObjectTssCaptionCount < 0) { gObjectTssCaptionCount = 0; }
		iCount = gObjectTssCaptionCount;
	}

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" TssCapManApiDeleteInstance()             :type=%d munber=%d id=%d(0x%08X) \n", iType, iNumber, id, (unsigned int)id);
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiDeleteInstance(),type=%d munber=%d id=%d(0x%08X)", id, iType, iNumber, id, (unsigned int)id);
	}
#endif

	return 0;
}


//--------------------------------------------------
// TssCapManApiSetParameterInit()
//--------------------------------------------------
int TssCapManApiSetParameterInit(
	int id,
//	int resolution,
	unsigned long streamid,
	unsigned long serviceid
){
	TssCapProcessManager *pObject = NULL;
	int iType = 0;

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		return TSSLIB_ERROR_INSTANCEID;
	}

	pObject->SetStreamId(streamid);
	pObject->SetServiceId(serviceid);
	pObject->SetResolution(TSSLIB_RESOKUTUION_2K);

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" TssCapManApiSetParameterInit()           :id=%d(0x%08X) \n", id,(unsigned int)id);
	printf(" TssCapManApiSetParameterInit()           :streamid=%d(0x%08X) serviceid=%d(0x%08X) \n", streamid, streamid, (unsigned int)serviceid, (unsigned int)serviceid);
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiSetParameterInit(),id=%d(0x%08X) streamid=%d(0x%08X) serviceid=%d(0x%08X)", id, id, (unsigned int)id, streamid, streamid, (unsigned int)serviceid, (unsigned int)serviceid);
	}
#endif

	return 0;
}

//--------------------------------------------------
// TssCapManApiSetParameterInputAdd()
//--------------------------------------------------
int TssCapManApiSetParameterInputAdd(
	int id,
	int pid,
	unsigned long type,
	int tag
){
	int ret = 0;
	TssCapProcessManager *pObject = NULL;
	int iType = 0;
	int iCount = 0;

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		return TSSLIB_ERROR_INSTANCEID;
	}

	ret = pObject->SetParameterInputAdd(pid, type, tag);
	if (ret < 0) {
		return ret;
	}

	iCount = pObject->GetParameterInputCount();

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" TssCapManApiSetParameterInputAdd()       :id=%d(0x%08X) \n", id,(unsigned int)id);
	printf(" TssCapManApiSetParameterInputAdd()       :pid=%d type=%d tag=%d \n", pid, type, tag );
	printf(" TssCapManApiSetParameteTssCapManApiSetParameterInputLangrInputAdd()       :count=%d \n", iCount);
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiSetParameterInputAdd(),id=%d(0x%08X) pid=%d type=%d tag=%d count=%d", id, id, (unsigned int)id, pid, type, tag, iCount);
	}
#endif

	return 0;
}

//--------------------------------------------------
// int TssCapManApiSetParameterInputLang()
//--------------------------------------------------
int TssCapManApiSetParameterInputLang(
	int id,
	int lang
){
	int ret = 0;
	TssCapProcessManager *pObject = NULL;
	int iType = 0;
	int iCount = 0;

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		return TSSLIB_ERROR_INSTANCEID;
	}

	ret = pObject->SetParameterInputLang(lang);
	if (ret < 0) {
		return ret;
	}

	return 0;
}

//--------------------------------------------------
// TssCapManApiSetParameterInputClear()
//--------------------------------------------------
int TssCapManApiSetParameterInputClear(
	int id
){
	int ret = 0;
	TssCapProcessManager *pObject = NULL;
	int iType = 0;
	int iCount = 0;

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		return TSSLIB_ERROR_INSTANCEID;
	}

	ret = pObject->SetParameterInputClear();
	if (ret < 0) {
		return ret;
	}

	iCount = pObject->GetParameterInputCount();

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" TssCapManApiSetParameterInputClear()     :id=%d(0x%08X) \n", id,(unsigned int)id);
	printf(" TssCapManApiSetParameterInputClear()     :count=%d \n", iCount);
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiSetParameterInputClear(),id=%d(0x%08X) count=%d", id, id, (unsigned int)id, iCount);
	}
#endif

	return 0;
}

//--------------------------------------------------
// TssCapManApiSetParameterOutput()
//--------------------------------------------------
int TssCapManApiSetParameterOutput(
	int id,
	unsigned char *pointer,
	int size
){
	int ret = 0;
	TssCapProcessManager *pObject = NULL;
	int iType = 0;

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		return TSSLIB_ERROR_INSTANCEID;
	}

	ret = pObject->SetParameterOutput(pointer, size);
	if (ret < 0) {
		return ret;
	}

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" TssCapManApiSetParameterOutput()         :id=%d(0x%08X) \n", id,(unsigned int)id);
	printf(" TssCapManApiSetParameterOutput()         :pointer=%p size=%d \n", pointer, size);
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiSetParameterOutput(),id=%d(0x%08X) pointer=%p size=%d", id, id, (unsigned int)id, pointer, size);
	}
#endif

	return 0;
}

//--------------------------------------------------
// TssCapManApiStart()
//--------------------------------------------------
int TssCapManApiStart(
	int id,
	bool isClien
) { 
	int rv = 0;
	TssCapProcessManager *pObject = NULL;
	int iType = 0;

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		return TSSLIB_ERROR_INSTANCEID;
	}

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" TssCapManApiStart()                      :id=%d(0x%08X) \n", id,(unsigned int)id);
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiStart(),id=%d(0x%08X)", id, id, (unsigned int)id);
	}
#endif

#if MPM_OUTPUT_PUTIMAGE
	pObject->StartThread();
#endif

	return 0;
}

//--------------------------------------------------
// TssCapManApiPause()
//--------------------------------------------------
#if 0
int TssCapManApiPause(
	int id,
	int enable
) { 
	int rv = 0;
	TssCapProcessManager *pObject = NULL;
	int iType = 0;

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		return TSSLIB_ERROR_INSTANCEID;
	}

	//一時停止解除
	if (enable == 0) {
		//pObject->StartThread();
		pObject->PauseStop();
	}
	//一時停止
	else {
		//pObject->ThreadFlagStop();
		//pObject->StopThread();
		pObject->PauseStart();
	}

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" TssCapManApiPause()                      :id=%d(0x%08X) enable=%d\n", id, (unsigned int)id, enable);
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiPause(),id=%d(0x%08X) enable=%d", id, id, (unsigned int)id, enable);
	}
#endif

	return 0;
}
#endif

//--------------------------------------------------
// TssCapManApiStop()
//--------------------------------------------------
int TssCapManApiStop(
	int id
) {
	int rv = 0;
	TssCapProcessManager *pObject = NULL;
	int iType = 0;

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		return TSSLIB_ERROR_INSTANCEID;
	}

#if TSS_PROCESS_INTERFACE_DEBUG
	printf(" TssCapManApiStop()                       :id=%d(0x%08X)\n", id, (unsigned int)id);
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiStop(),id=%d(0x%08X)", id, id, (unsigned int)id);
	}
#endif

#if MPM_OUTPUT_PUTIMAGE
	pObject->ThreadFlagStop();
	pObject->StopThread();
#endif

	return 0;
}

//--------------------------------------------------
// TssCapInputApiPushTspData()
//--------------------------------------------------
int TssCapInputApiPushTspData(
	int id, 
	unsigned char *pointer, 
	int size
){
	int rv = 0;
	TssCapProcessManager *pObject = NULL;
	int iType = 0;

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		APPLOG("[API],ERR,%d,TssCapProcessManagerApi,TssCapInputApiPushTspData(),ManFuncGetObjectFromId() id=%d(0x%08X)", id, id, (unsigned int)id);
		return TSSLIB_ERROR_INSTANCEID;
	}
	
	rv = pObject->PushTspData(pointer, size);
	if ( rv < 0) {
		return TSSLIB_ERROR;
	}

#if TSS_PROCESS_INTERFACE_DEBUG
	if (gObjectTssLogLevel&&TSS_API_DEBUG_LOG) {
		if (size >= 8) {
			APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapInputApiPushTspData(),size=%04d data=%02X%02X%02X%02X %02X%02X%02X%02X", id, size,
				pointer[0], pointer[1], pointer[2], pointer[3], pointer[4], pointer[5], pointer[6], pointer[7] );
		}
	}
#endif

	return rv;
}

//--------------------------------------------------
// TssCapInputApiPushPesData()
//--------------------------------------------------
int TssCapInputApiPushPesData(
	int id,
	unsigned char *pointer,
	int size
) {
	int rv = 0;
	TssCapProcessManager *pObject = NULL;
	int iType = 0;

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		APPLOG("[API],ERR,%d,TssCapProcessManagerApi,TssCapInputApiPushPesData(),ManFuncGetObjectFromId() id=%d(0x%08X)", id, id, (unsigned int)id);
		return TSSLIB_ERROR_INSTANCEID;
	}

	rv = pObject->PushPesData(pointer, size);
	if (rv < 0) {
		return TSSLIB_ERROR;
	}

#if TSS_PROCESS_INTERFACE_DEBUG
	if (gObjectTssLogLevel&&TSS_API_DEBUG_LOG) {
		if (size >= 8) {
			APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapInputApiPushPesData(),size=%04d data=%02X%02X%02X%02X %02X%02X%02X%02X", id, size,
				pointer[0], pointer[1], pointer[2], pointer[3], pointer[4], pointer[5], pointer[6], pointer[7]);
		}
	}
#endif

	return rv;
}

int TssCapInputApiPushPesDataWithPts(
	int id,
	unsigned char *pointer,
	int size,
	unsigned long long uTimestamp
) {
	int rv = 0;
	TssCapProcessManager *pObject = NULL;
	int iType = 0;

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		APPLOG("[API],ERR,%d,TssCapProcessManagerApi,TssCapInputApiPushPesDataWithPts(),ManFuncGetObjectFromId() id=%d(0x%08X)", id, id, (unsigned int)id);
		return TSSLIB_ERROR_INSTANCEID;
	}

	rv = pObject->PushPesDataWithPts(pointer, size, uTimestamp);
	if (rv < 0) {
		return TSSLIB_ERROR;
	}

#if TSS_PROCESS_INTERFACE_DEBUG
	if (gObjectTssLogLevel&&TSS_API_DEBUG_LOG) {
		if (size >= 8) {
			APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapInputApiPushPesDataWithPts(),size=%04d PTS=0x%llX", id, size, uTimestamp);
		}
	}
#endif

	return rv;
}




//--------------------------------------------------
// TssCapManApiAppendListener()
//--------------------------------------------------
int TssCapManApiAppendListener(
	int id,
	ITssCapCaptureListener* pListener
) {
	int rv = 0;
	TssCapProcessManager *pObject = NULL;
	int iType = 0;
	int iCount = 0;

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		return TSSLIB_ERROR_INSTANCEID;
	}

	if (pObject->AppendListener(pListener) < 0) {
		return TSSLIB_ERROR;
	}

	iCount = pObject->GetListenerCount();

#if TSS_PROCESS_INTERFACE_DEBUG
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiAppendListener() Count=%d", id, iCount);
	}
#endif

	return 0;
}


//--------------------------------------------------
// TssCapManApiRemoveListener()
//--------------------------------------------------
int TssCapManApiRemoveListener(
	int id,
	ITssCapCaptureListener* pListener
) {
	int rv = 0;
	TssCapProcessManager *pObject = NULL;
	int iType = 0;
	int iCount = 0;

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		return TSSLIB_ERROR_INSTANCEID;
	}

	if (pObject->RemoveListener(pListener) < 0) {
		return TSSLIB_ERROR;
	}

	iCount = pObject->GetListenerCount();

#if TSS_PROCESS_INTERFACE_DEBUG
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiRemoveListener() Count=%d", id, iCount);
	}
#endif

	return 0;
}


//--------------------------------------------------
// TssCapManApiSetTimestamp()
//--------------------------------------------------
int TssCapManApiSetTimestamp(
	int id,
	unsigned long long uTimestamp
) {
	int rv = 0;
	TssCapProcessManager *pObject = NULL;
	int iType = 0;
	unsigned long long uNowTimestamp = 0;

	pObject = ManFuncGetObjectFromId(id);
	if (pObject == NULL) {
		return TSSLIB_ERROR_INSTANCEID;
	}

	iType = ManFuncGetTypeFromId(id);

	if (iType == TSSLIB_TYPE_CAPTION_TR) {

		uNowTimestamp = pObject->GetTimestamp();

#if TSS_PROCESS_INTERFACE_DEBUG
		if (gObjectTssLogLevel) {
			APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiSetTimestamp() Timestamp=0x%llX [0x%llX]", id, uTimestamp, uNowTimestamp);
		}
#endif

		if (pObject->SetTimestamp(uTimestamp) < 0) {
			return TSSLIB_ERROR;
		}
	}

#if TSS_PROCESS_INTERFACE_DEBUG
	if (gObjectTssLogLevel) {
		APPLOG("[API],INF,%d,TssCapProcessManagerApi,TssCapManApiSetTimestamp() END", id);
	}
#endif

	return 0;
}




