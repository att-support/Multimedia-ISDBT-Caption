#if !defined _TSS_CAT_PROC_API_
#define _TSS_CAT_PROC_API_
//#######################################################################
//## TssCapProcessManagerApi.h
//#######################################################################

//**************************************************
// include
//**************************************************
#include "TssCapProcessManagerListenerIf.h"

//**************************************************
// define
//**************************************************
//IMPORT/EXPORT
#ifdef TSS_CAP_PROCESS_MANAGER_EXPORTS
#define TSSPROC_API __declspec(dllexport)
#else
#define TSSPROC_API __declspec(dllimport)
#endif
//TYPE
#define TSSLIB_TYPE_CAPTION_TR			   10
#define TSSLIB_TYPE_MTMEDIA_TR			   11
//RESOLUTION
#define TSSLIB_RESOKUTUION_2K			   0
#define TSSLIB_RESOKUTUION_4K			   1
#define TSSLIB_RESOKUTUION_8K			   2
//VALUE
#define TSSLIB_PTSVALUE_NONE				0xFFFFFFFFFFFFFFFF

//**************************************************
// error code
//**************************************************
#define TSSLIB_OK						   0
#define TSSLIB_ERROR					  -1
#define TSSLIB_ERROR_INSTANCEID			 -10
#define TSSLIB_ERROR_INPUT				-101

//**************************************************
// function
//**************************************************

//--------------------------------------------------
// int TssManApiGetVersion(int type)
// 説明：本DLLのバージョン値(10進数4桁=NXYZ)
// 　　：N=1～9(メインバージョン　 ：開発版は9,リリース版は1から)
// 　　：X=0～9(マイナーバージョン ：開発版は9,リリース版は0から)
// 　　：Y=0～9(ﾊﾞｸﾞﾌｨｯｸｽバージョン：開発版は9,リリース版は0から)
// 　　：Z=0～9(ビルドバージョン　 ：0～9 9を超える場合はZ=0,Y++)
// 引数：なし
// 戻値：int＜バージョン番号＞上記数値
// 分類：共通(字幕,データ放送)
//--------------------------------------------------
TSSPROC_API int TssCapManApiGetVersion();

//**************************************************
// 生成・破棄
//**************************************************
//--------------------------------------------------
// int TssCapManApiCreateInstancea(int type)
// 説明：インスタンスを生成します(種別指定)
// 引数：int resolution
//     ：＜解像度＞(0=2K,1=4K,2=8K) ここでは2K固定
// 戻値：int＜インスタンスID,正否＞(0=インスタンスID,マイナス値=エラーコード)
// 分類：共通(字幕,データ放送)
//--------------------------------------------------
TSSPROC_API int TssCapManApiCreateInstance(int resolution = 0);

//--------------------------------------------------
// int TssCapManApiDeleteInstance(int id)
// 説明：インスタンスを破棄します(インスタンスID指定)
// 引数：int id
//     ：＜インスタンスID＞
// 戻値：int＜正否＞(0=OK,マイナス値=エラーコード)
// 分類：共通(字幕,データ放送)
//--------------------------------------------------
TSSPROC_API int TssCapManApiDeleteInstance(int id);

//**************************************************
// 設定
//**************************************************
//--------------------------------------------------
// int TssCapManApiSetParameterInit(int id, unsigned long streamid, unsigned long serviceid)
// 説明：初期設定
// 引数：int id
//     ：＜インスタンスID＞
// 引数：unsigned long streamid
//     ：＜ストリームID＞(32bit値)
//     ：同じサービスのデータ放送を複数並べる場合も考慮して、サービスより上位の識別値として設定する。
//	　 ：この値は、上位側で管理する任意のIDを設定する。これにより同じTSの異なる時間帯を複数並べて表示が出来る想定。
// 引数：unsigned long serviceid
//     ：＜サービスID＞(16bit値)
//     ：TSのSID(16bit値)、MMTのMMT_package_id_byteの下位16bit
// 戻値：int＜正否＞(0=OK,マイナス値=エラーコード)
// 分類：字幕のみ（データ放送は別途BMLブラウザとのインタフェースで実現）
//--------------------------------------------------
TSSPROC_API int TssCapManApiSetParameterInit(int id, unsigned long streamid, unsigned long serviceid);

//--------------------------------------------------
// int TssCapManApiSetParameterInputAdd(int id, int pid, unsigned long type, int tag)
// 説明：PID設定(追加方式)
// 引数：int id
//     ：＜インスタンスID＞
//     ：int pid
//     ：＜PID＞入力PID設定（追加方式）必要な数だけ設定する
//     ：unsigned long type
//     ：＜データタイプ＞(32bit値)
//     ：MPTに書かれているasset_type(32bit)
//     ：int tag
//     ：＜タグID＞(8bit値)
//     ：MPTに含まれるMH-ストリーム識別記述子内のcomponent_tag値
// 戻値：int＜正否＞(0=OK,マイナス値=エラーコード)
// 分類：字幕のみ（データ放送は別途BMLブラウザとのインタフェースで実現）
//--------------------------------------------------
TSSPROC_API int TssCapManApiSetParameterInputAdd(int id, int pid, unsigned long type, int tag);

//--------------------------------------------------
// int TssCapManApiSetParameterInputClear(int id)
// 説明：PID設定(設定のクリア)
// 引数：int id
//     ：＜インスタンスID＞
// 戻値：int＜正否＞(0=OK,マイナス値=エラーコード)
// 分類：字幕のみ（データ放送は別途BMLブラウザとのインタフェースで実現）
//--------------------------------------------------
TSSPROC_API int TssCapManApiSetParameterInputClear(int id);

//--------------------------------------------------
// int TssCapManApiSetParameterOutput(int id, unsigned char *pointer, int size)
// 説明：出力設定(書込用メモリ設定)
// 引数：int id
//     ：＜インスタンスID＞
// 引数：unsigned char *pointer
//     ：＜書込ポインタ＞
//     ：書き込むデータ領域の先頭ポインタ(解像度に以上のバイト数とする)
// 引数：int size
//     ：＜書込サイズ＞
//     ：書き込むデータ領域のサイズ（書き込める最大サイズ）
// 戻値：int＜正否＞(0=OK,マイナス値=エラーコード)
// 分類：字幕のみ（データ放送は別途BMLブラウザとのインタフェースで実現）
//--------------------------------------------------
TSSPROC_API int TssCapManApiSetParameterOutput(int id, unsigned char *pointer, int size);

//--------------------------------------------------
// int TssManApiSetParameterInputLang(int id, int lang)
// 説明：言語設定
// 引数：int id
//     ：＜インスタンスID＞
// 引数：int lang
//     ：＜言語指定＞
//     ：0=第1言語、1=第2言語
// 戻値：int＜正否＞(0=OK,マイナス値=エラーコード)
// 分類：字幕のみ
//--------------------------------------------------
TSSPROC_API int TssCapManApiSetParameterInputLang( int id, int lang );

//**************************************************
// 動作開始停止
//**************************************************
//--------------------------------------------------
// int TssCapManApiStart(int id)
// 説明：動作開始
// 引数：int id
//     ：＜インスタンスID＞
// 引数：bool isClient
//     ：＜TCP/IPのクライアント有無＞
// 戻値：int＜正否＞(0=OK,マイナス値=エラーコード)
// 分類：字幕のみ
//--------------------------------------------------
TSSPROC_API int TssCapManApiStart(int id, bool isClient=0);

//--------------------------------------------------
// int TssCapManApiPause(int id)
// 説明：一時停止
// 引数：int id
//     ：＜インスタンスID＞
// 引数：int enable
//     ：＜一時停止制御＞(0=一時停止解除,1=一時停止)
//     ：enable＝1で一時停止 enable=0で一時停止解除
// 戻値：int＜正否＞(0=OK,マイナス値=エラーコード)
// 分類：字幕のみ
//--------------------------------------------------
//TSSPROC_API int TssCapManApiPause(int id, int enable);

//--------------------------------------------------
// int TssCapManApiStop(int id)
// 説明：動作停止
// 引数：int id
//     ：＜インスタンスID＞
// 戻値：int＜正否＞(0=OK,マイナス値=エラーコード)
// 分類：字幕のみ
//--------------------------------------------------
TSSPROC_API int TssCapManApiStop(int id);

//**************************************************
// STC
//**************************************************
TSSPROC_API int TssCapManApiSetTimestamp(int id, unsigned long long uTimestamp); //字幕のSTC設定用

//**************************************************
// DATA
//**************************************************
//--------------------------------------------------
// int TssCapInputApiPushTspData(int id, unsigned char *pointer, int size)
// 説明：TSPデータを処理します(インスタンスID指定)
// 引数：int id
//     ：＜インスタンスID＞
// 引数：unsigned char *pointer
//     ：＜データポインタ＞
//     ：データ領域の先頭ポインタ
// 引数：int size
//     ：＜データサイズ＞
//     ：データ領域のサイズ
// 戻値：int＜正否＞(0=OK,マイナス値=エラーコード)
// 分類：字幕のみ（ただしこちらは廃止予定）
//--------------------------------------------------
TSSPROC_API int TssCapInputApiPushTspData(int id, unsigned char *pointer, int size);

//--------------------------------------------------
// int TssCapInputApiPushPesData(int id, unsigned char *pointer, int size)
// 説明：PESデータを処理します(インスタンスID指定)
// 引数：int id
//     ：＜インスタンスID＞
// 引数：unsigned char *pointer
//     ：＜データポインタ＞
//     ：データ領域の先頭ポインタ
// 引数：int size
//     ：＜データサイズ＞
//     ：データ領域のサイズ
// 引数：int uTimestamp
//     ：＜表示タイムスタンプ＞
//     ：表示するPTSの指定 ※90kHz(=STC/300) ※0xFFFFFFFFFFFFFFFFの場合は不使用
// 戻値：int＜正否＞(0=OK,マイナス値=エラーコード)
// 分類：字幕のみ
//--------------------------------------------------
TSSPROC_API int TssCapInputApiPushPesData(int id, unsigned char *pointer, int size);
TSSPROC_API int TssCapInputApiPushPesDataWithPts(int id, unsigned char *pointer, int size, unsigned long long uTimestamp);

//**************************************************
// 通知登録
//**************************************************
TSSPROC_API int TssCapManApiAppendListener(int id, ITssCapCaptureListener* pListener);	//リスナー登録追加（字幕用）
TSSPROC_API int TssCapManApiRemoveListener(int id, ITssCapCaptureListener* pListener);	//リスナー登録削除（字幕用）


#endif //_TSS_CAT_PROC_API_
