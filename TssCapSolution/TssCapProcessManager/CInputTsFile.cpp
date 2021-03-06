//**************************************************
// include
//**************************************************
#include "stdafx.h"
//#include "../../Public/ConvertTsp/ConvertTs.h"
#include "CInputTsFile.h"

//**************************************************
// define
//**************************************************
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <fcntl.h>      /* Needed only for _O_RDWR definition */
#include <io.h>
#include <share.h>
#include <sys/types.h>
#include <sys/stat.h>

#define ADDITIONAL_CODE1 1
#define TS_AFCONTROL_UNUSE		0x00
#define TS_AFCONTROL_PL_ONLY	0x01
#define TS_AFCONTROL_AF_ONLY	0x02
#define TS_AFCONTROL_BOTH		0x03


////////////////////////////////////////////////////
//　コンストラクタ
////////////////////////////////////////////////////
CInputTsFile::CInputTsFile()
{
	//パケット長
	m_iTsPacketLength= TSP_PACKET_SIZE;

	//入力バッファ
	m_pInBuffer=NULL;
	m_iInBufferLength=0;
	m_iInBufferTspCount=0;
	m_iInBufferTspPointer = 0;

	//初期化
	Initial();
}

////////////////////////////////////////////////////
//　デコンストラクタ
////////////////////////////////////////////////////
CInputTsFile::~CInputTsFile()
{
	Free();
}

////////////////////////////////////////////////////
//　初期化
////////////////////////////////////////////////////
void CInputTsFile::Initial()
{
	//入力ファイル
	memset(m_pFilename,NULL,4096);
	m_i64FileLength=0;
	m_i64FilePointer=0;

	//バッファ確保
	if(Alloc()<0){
		//error
		return;
	}

	//初期PCR
	m_i64InitStc=0;

	//開始時刻
	m_iStartOffsetTime=100;

	//停止フラグ
	m_iBreakFlag=1;

	//終了フラグ
	m_bFinishFlag=false;

	//取得フラグ
	m_iCapPesFlag = 0;
}

////////////////////////////////////////////////////
//　再初期化
////////////////////////////////////////////////////
void CInputTsFile::Reset()
{
	Free();

	Initial();

//	m_cSection.Initial();
}

////////////////////////////////////////////////////
//　バッファ確保
////////////////////////////////////////////////////
int CInputTsFile::Alloc()
{
	//入力バッファ
	if(m_pInBuffer){
		return -1;
	}
	m_pInBuffer = (unsigned char*)new unsigned char[TSP_BUFFER_UNIT*m_iTsPacketLength];
	if(m_pInBuffer){
		memset(m_pInBuffer,NULL,(TSP_BUFFER_UNIT*m_iTsPacketLength));
	}
	m_iInBufferLength=0;
	m_i64ReadPointer=0;
	m_iInBufferTspCount=0;

	return true;
}

////////////////////////////////////////////////////
//　バッファ確保
////////////////////////////////////////////////////
void CInputTsFile::Free()
{
	//入力バッファ
	if(m_pInBuffer){
		delete m_pInBuffer;
		m_pInBuffer=NULL;
		m_iInBufferLength=0;
		m_iInBufferTspCount=0;
		m_iInBufferTspPointer = 0;
	}
}

int CInputTsFile::GetTsPacket()
{
	if (m_iBreakFlag == 1) {
		//-----------------------
		//バッファをセット
		//-----------------------
		if (SetBuffer()<0) {
			return -1000;
		}
		m_iBreakFlag = 0;
	}

	int i = 0;
	int pid = 0;
	unsigned char *tspBuff = NULL;
	__int64 i64StartStc = 0;
	__int64 i64NowStc = 0;

	do {

		for (i = m_iInBufferTspPointer; i < m_iInBufferTspCount; i++) {

			//buff
			tspBuff = &m_pInBuffer[m_iTsPacketLength*i];

			//pid
			pid = GetPid(tspBuff);

			//-----------------------------------------------------
			// 字幕挿入削除(字幕パケット→NULLポケット)
			//-----------------------------------------------------
			if (pid == m_CaptionPid) {
				return (m_iTsPacketLength*i);
			}
		}

		if (m_iInBufferLength < (TSP_BUFFER_UNIT*m_iTsPacketLength)) {
			return 1000;
		}

		//-----------------------
		//バッファをセット
		//-----------------------
		if (SetBuffer() < 0) {
			return -1000;
		}

	} while (1);

	return 0;
}


#if 1
////////////////////////////////////////////////////
//　字幕変換処理
//
//  戻り値		：true=正常、-1=エラー
////////////////////////////////////////////////////
int CInputTsFile::CheckTsp(int count)
{
	if(m_pInBuffer==NULL){
		return -1;
	}

	int i=0;
	int ptr=0;
	int pid = 0;
	int pusi = 0;
	int plen = 0;
	int pes_len = 0;
	int comp = 0;
	unsigned char *pdata = NULL;
	unsigned char *tspBuff=NULL;
	__int64 i64StartStc=0;
	__int64 i64NowStc=0;

	for(i=m_iInBufferTspPointer;i<m_iInBufferTspCount;i++){

		//buff
		tspBuff = &m_pInBuffer[m_iTsPacketLength*i];

		//pid
		pid = GetPid(tspBuff);

		//-----------------------------------------------------
		// 字幕挿入削除(字幕パケット→NULLポケット)
		//-----------------------------------------------------
		if(pid==m_CaptionPid){

			//pusi
			pusi = GetPusi(tspBuff);

			//PESを取得
			//<未取得>
			//TSSI=1を確認
			//Payloadをコピー
			//PESサイズ確認->完了？
			//<取得中>
			//TSSI=0を確認
			//Payloadをコピー
			//PESサイズ確認->完了？
			//<<完了>>
			//PESを分析
			//-->管理データ？
			//-->字幕データ？
			//字幕データの場合、データユニットを8単位符号としてTTMLに変換

			if (m_iCapPesFlag == 0) {
				if (pusi == 1) {
					//取得開始
					PesBufferClear();
					ptr = GetPalyload(tspBuff);
					pdata = tspBuff + ptr;
					plen = 188 - ptr;
					pes_len = ((unsigned long)tspBuff[ptr + 4]) * 256;
					pes_len += (unsigned long)tspBuff[ptr + 5];
					pes_len += 6;
					PesBufferInit(pes_len);
					m_iCapPesFlag = 1;
					comp = PesBufferSet(pdata, plen);
					if (comp != 100) {
						continue;
					}
				}
				else {
					continue;
				}
			}
			//m_iCapPesFlag:1
			else {
				if (pusi == 1) {
					//取得開始
					PesBufferClear();
					ptr = GetPalyload(tspBuff);
					pdata = tspBuff + ptr;
					plen = 188 - ptr;
					pes_len = ((unsigned long)tspBuff[ptr + 4]) * 256;
					pes_len += (unsigned long)tspBuff[ptr + 5];
					PesBufferInit(pes_len);
					m_iCapPesFlag = 1;
					comp = PesBufferSet(pdata, plen);
					if (comp != 100) {
						continue;
					}
				}
				else {
					ptr = GetPalyload(tspBuff);
					pdata = tspBuff + ptr;
					plen = 188 - ptr;
					comp = PesBufferSet(pdata, plen);
					if (comp != 100) {
						continue;
					}
				}
			}

		}

		//取得完了
		if (comp == 100) {

			//PESを分析
			//-->管理データ？
			//-->字幕データ？
			//字幕データの場合、データユニットを8単位符号としてTTMLに変換

			m_iCapPesFlag = 0;
			m_iInBufferTspPointer = i;
			return 100;
		}
	}

	return true;
}
#endif


int CInputTsFile::PesBufferClear() {
	if (m_pPesBuffer) {
		delete[] m_pPesBuffer;
		m_pPesBuffer = NULL;
	}
	m_iPesBufferLength = 0;
	m_iPesTargetLength = 0;
	return 0;
}

int CInputTsFile::PesBufferInit(int size) {
	PesBufferClear();

	m_pPesBuffer = new unsigned char[size+1];
	if (m_pPesBuffer == NULL) {
		return -1;
	}
	memset(m_pPesBuffer, NULL, size + 1);
	m_iPesTargetLength = size;
	return 0;
}

int CInputTsFile::PesBufferSet(unsigned char* pdata, int size) {
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


int CInputTsFile::GetPid(unsigned char *buff){
	int pid=0;

	if(buff[0]!=0x47){
		return -1;
	}

	pid  = ((((unsigned long)buff[1])<<8)&0x1F00);
	pid |= (((unsigned long)buff[2])&0x00FF);

	return pid;
}

int CInputTsFile::GetCc(unsigned char *buff) {
	int cc = 0;

	if (buff[0] != 0x47) {
		return -1;
	}

	cc = (((unsigned long)buff[3]) & 0x0F);

	return cc;
}

int CInputTsFile::GetPusi(unsigned char *buff) {
	int pusi = 0;

	if (buff[0] != 0x47) {
		return -1;
	}

	pusi = ((((unsigned long)buff[1]) >> 6) & 0x0001);

	return pusi;
}

int CInputTsFile::GetPalyload(unsigned char *buff) {
	int afc = 0;
	int aflen = 0;
	int offset = 0;

	if (buff[0] != 0x47) {
		return -1;
	}

	afc = ((((unsigned long)buff[3]) >> 4) & 0x0003);

	offset = 4;

	if ((afc == TS_AFCONTROL_AF_ONLY) || (afc == TS_AFCONTROL_BOTH)) {
		aflen = (unsigned long)buff[4];
		offset++;
		offset += aflen;
	}

	return offset;
}

#if 1
////////////////////////////////////////////////////
//　字幕画音ファイル
//
//　char *filename：ファイル名
//  戻り値		：true=正常、-1=エラー
////////////////////////////////////////////////////
int CInputTsFile::SetFilename(char *filename)
{
	int i=0;
	int fnlen = (int)strlen(filename);
	//int fdlen = (int)strlen(m_pOutFolder);

	if(fnlen==0){
		return -1;
	}
	//if(fdlen==0){
	//	return -1;
	//}

	memset(m_pFilename,NULL,4096);
	memcpy(m_pFilename,filename,strlen(filename));

	//file size
	if(GetFileLength(m_pFilename,&m_i64FileLength)<0){
		memset(m_pFilename,NULL,4096);
		m_i64FileLength=0;
		m_i64FilePointer=0;
		return -1;
	}

	char pFilename[4096];
	memset(pFilename,NULL,4096);
	int j=0;
	for(i=0;i<fnlen;i++){
		if(m_pFilename[i]=='\\'){
			memset(pFilename,NULL,4096);
			j=0;
			continue;
		}
		pFilename[j] = m_pFilename[i];
		j++;
	}

	//char pFilenameSave[4096];
	//memset(pFilenameSave,NULL,4096);
	//sprintf(pFilenameSave,"%s\\%s",m_pOutFolder,pFilename);

	return true;
}
#endif

////////////////////////////////////////////////////
//　字幕画音ファイルサイズ
//
//　char *filename：ファイル名
//　__int64 *length：サイズ
//  戻り値		：true=正常、-1=エラー
////////////////////////////////////////////////////
int CInputTsFile::GetFileLength(char *filename,__int64 *length)
{
	__int64 filelength = 0;

	int _fp = _open(filename, _O_RDONLY | _O_BINARY);
	if(_fp<0){
		return -1;
	}

	_lseeki64(_fp,0,SEEK_END);
	filelength = _telli64(_fp);
	_lseeki64(_fp,0,SEEK_SET);

	_close(_fp);

	*length = filelength;

	return true;
}

////////////////////////////////////////////////////
//　開始時刻設定
//
//　int iStartOffsetTime：開始時刻[msec]
//  戻り値		：なし
////////////////////////////////////////////////////
void CInputTsFile::SetStartOffsetTime(int iStartOffsetTime)
{
	m_iStartOffsetTime = iStartOffsetTime;
}

////////////////////////////////////////////////////
//　ファイルデータをバッファに読み込み
//
//  戻り値		：true=正常、-1=エラー
////////////////////////////////////////////////////
int CInputTsFile::SetBuffer()
{
	int _fp=0;
	int rlen=0;
	__int64 ptr64=0;

	if(m_pInBuffer==NULL){
		return -1;
	}

	_fp = _open(m_pFilename,_O_RDONLY|_O_BINARY);
	if(_fp<0){
		return -1;
	}

	ptr64 = _lseeki64(_fp,m_i64FilePointer,SEEK_SET);
	if(ptr64<0){
		return -1;
	}

	memset(m_pInBuffer,NULL,(TSP_BUFFER_UNIT*m_iTsPacketLength));
	//m_i64FileLength=0;
	//m_i64FilePointer=0;

	rlen = _read(_fp,m_pInBuffer,(TSP_BUFFER_UNIT*m_iTsPacketLength));
	if(rlen<0){
		return -1;
	}
	//m_i64FileLength=rlen;
	m_iInBufferTspCount=rlen/m_iTsPacketLength;
	m_i64FilePointer += rlen;
	m_i64ReadPointer += rlen;
	m_iInBufferLength = rlen;

	_close(_fp);

	m_iInBufferTspPointer = 0;

	return true;
}

////////////////////////////////////////////////////
//　ファイルデータを一時バッファに読み込み
//
//　char *buff	：バッファポインタ
//　__int64 *length：バッファサイズ
//  戻り値		：取得サイズ、-1=エラー
////////////////////////////////////////////////////
int CInputTsFile::SetBuffer(unsigned char *buff,int length)
{
	int _fp=0;
	int rlen=0;
	__int64 ptr64=0;

	if(buff==NULL){
		return -1;
	}

	_fp = _open(m_pFilename,_O_RDONLY|_O_BINARY);
	if(_fp<0){
		return -1;
	}

	ptr64 = _lseeki64(_fp,0,SEEK_SET);
	if(ptr64<0){
		return -1;
	}

	memset(buff,NULL,length);

	rlen = _read(_fp,buff,length);
	if(rlen<0){
		return -1;
	}

	_close(_fp);

	return rlen;
}

////////////////////////////////////////////////////
//　ファイルデータを一時バッファに読み込み
//
//　char *buff	：バッファポインタ
//　__int64 *length：バッファサイズ
//  int type	：TYPE_FILE_START、TYPE_FILE_END
//__int64* addr	：読み込み先頭アドレスを格納
//  戻り値		：取得サイズ、-1=エラー
////////////////////////////////////////////////////
int CInputTsFile::SetBuffer(unsigned char *buff,int length,int type,__int64* addr)
{
	int _fp=0;
	int rlen=0;
	__int64 ptr64=0;

	if(buff==NULL){
		return -1;
	}

	_fp = _open(m_pFilename,_O_RDONLY|_O_BINARY);
	if(_fp<0){
		return -1;
	}

	//TYPE
	if(type==TYPE_FILE_START){
		ptr64 = _lseeki64(_fp,0,SEEK_SET);
	}
	else if(type==TYPE_FILE_END){
		ptr64 = _lseeki64(_fp,(m_i64FileLength-length),SEEK_SET);
	}
	if(ptr64<0){
		return -1;
	}
	*addr = ptr64;

	memset(buff,NULL,length);

	rlen = _read(_fp,buff,length);
	if(rlen<0){
		return -1;
	}

	_close(_fp);

	return rlen;
}

//#######################################################################
//## END
//#######################################################################
