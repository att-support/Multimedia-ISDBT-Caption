#pragma once

#ifdef TSS_CAP_PROCESS_LISTERNER_EXPORTS

#define TSS_CAP_PROCESS_LISTERNER_API __declspec(dllexport)
#else
#define TSS_CAP_PROCESS_LISTERNER_API __declspec(dllimport)
#endif

__interface TSS_CAP_PROCESS_LISTERNER_API ITssCapCaptureListener
{
  	public:
//		ITssCapCaptureListener(){};
//		virtual ~ITssCapCaptureListener(){};
  	
	//画像出力通知
	//
	//OutputApiEventUpdateCaptionPlane() 字幕画像更新イベント
	virtual int OutputApiEventUpdateCaptionPlane(int id) = 0;
	//
	//OutputApiEventUpdateDataPlane() データ放送画像更新イベント
	//virtual int OutputApiEventUpdateDataPlane(int id, int x, int y, int width, int height) = 0;
	//
	//OutputApiEventUpdateCaptionSound() 内蔵音の通知
	//int sound ページ内に音声が指定している場合、その番号を返す（音声無し=-1、音声あり=0～）
	virtual int OutputApiEventUpdateCaptionSound(int id, int sound) = 0;

  	//エラー通知イベント
    virtual int ManApiEventError(int id, unsigned long ErrorCode) = 0;
    
    //他にもいろいろ通知イベントのメソッドを定義します…
    
};


