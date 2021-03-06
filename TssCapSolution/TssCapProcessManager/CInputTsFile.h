#if !defined _INPUT_TSFILE_
#define _INPUT_TSFILE_

//**************************************************
// include
//**************************************************
//#include "SectionTsp.h"
//#include "JimakuMan.h"

//**************************************************
// define
//**************************************************
#define TSP_BUFFER_UNIT 1024*64*10
#define TYPE_FILE_START	0
#define TYPE_FILE_END	1
#define TSP_PACKET_SIZE 204

//**************************************************
// class CInputTsFile
//**************************************************
class CInputTsFile {
 
public:
	CInputTsFile();
	~CInputTsFile();

	//クラス初期化
	void Initial();
	void Reset();
	int Alloc();
	void Free();
	int GetTsPacket();

	//パケット長
	int m_iTsPacketLength;

	//入力ファイル
	char m_pFilename[4096];
	__int64 m_i64FileLength;
	__int64 m_i64FilePointer;
	int GetFileLength(char *filename,__int64 *length);

	//入力バッファ
	unsigned char* m_pInBuffer;
	unsigned char* GetBuffer() { return m_pInBuffer; };
	int m_iInBufferLength;
	int m_iInBufferTspCount;
	int m_iInBufferTspPointer;
	__int64 m_i64ReadPointer;

	//字幕パラメータ
	unsigned long m_CaptionPid;
	char m_pCaptionFilename[4096];
	int m_iCaptionFileType;
	int m_iStartOffsetTime;
	void SetStartOffsetTime(int iStartOffsetTime);

	//バッファ読み込み
	int SetBuffer();
	int SetBuffer(unsigned char *buff,int length);
	int SetBuffer(unsigned char *buff,int length,int type,__int64* addr);

	//変換関数
	int CheckTsp(int count);
	static int GetPid(unsigned char *buff);
	static int GetPusi(unsigned char *buff);
	static int GetCc(unsigned char *buff);
	static int GetPalyload(unsigned char *buff);
	int m_iCapPesFlag;

	int PesBufferClear();
	int PesBufferInit(int size);
	int PesBufferSet(unsigned char* pdata, int size);
	unsigned char* m_pPesBuffer;
	int m_iPesBufferLength;
	int m_iPesTargetLength;
	unsigned char* GetPesBuffer() { return m_pPesBuffer; };
	int GetPesLength() { return m_iPesBufferLength; };

	//PTS初期値
	__int64 m_i64InitStc;

	//停止フラグ
	int m_iBreakFlag;

//API
public:
	void Stop(){ m_iBreakFlag=true; };
	int SetFilename(char *filename);
	void SetCaptionPid(unsigned long pid) { m_CaptionPid = pid; };
	__int64 GetFileLength(){ return m_i64FileLength; };

	bool m_bFinishFlag;
	void SetFinish(){ m_bFinishFlag=true; };
	bool GetFinish(){ return m_bFinishFlag; };
};	


#endif //_INPUT_TSFILE_