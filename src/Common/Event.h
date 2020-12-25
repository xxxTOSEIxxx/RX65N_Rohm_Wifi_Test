/*
 * Event.h
 *
 *  Created on: 2020/10/19
 *      Author: 99990045
 */

#ifndef EVENT_H_
#define EVENT_H_
#include "platform.h"

//------------------------------------------
// タスク種別
//------------------------------------------
typedef enum
{
	TASK_KIND_MAIN = 0,					// メインタスク
	TASK_KIND_COINMEC,					// コインメックタスク
	TASK_KIND_LAUNDRY,					// CL-5(GW)通信タスク
	TASK_KIND_AUDIO_PLAYBACK,			// 音声再生タスク
	TASK_KIND_FWUPDATE_REQ,				// Fwupdateタスク(要求用)
	TASK_KIND_FWUPDATE_RES,				// Fwupdateタスク(応答用)
	TASK_KIND_CONTROL,

	TASK_KIND_MAX						// === 終端 ===

} TASK_KIND_ENUM;

//------------------------------------------
// プライオリティ種別
//------------------------------------------
typedef enum
{
	PRIORITY_KIND_NORMAL = 0,			// 優先度：通常
	PRIORITY_KIND_HIGH					// 優先度：高

} PRIORITY_KIND_ENUM;



//#define EVENT_SYSTEM_BIT						( 0x80000000 )				// システム専用イベント
//#define EVENT_TASK_COINMEC_BIT					( 0x01000000 )				// コインメックタスク専用イベント
//#define EVENT_TASK_LAUNDRY_BIT					( 0x02000000 )				// Laundryタスク専用イベント
//#define EVENT_TASK_AUDIOPLAYBACK_BIT			( 0x03000000 )				// 音声再生専用イベント
//#define EVENT_TASK_FWUPDATE_REQ_BIT				( 0x04000000 )				// FWUPDATE専用イベント(要求用)
//#define EVENT_TASK_FWUPDATE_RES_BIT				( 0x05000000 )				// FWUPDATE専用イベント(応答用)
#define EVENT_TASK_CONTROL_BIT					( 0x06000000 )				// CONTROLタスク専用イベント

#if 0
//------------------------------------------
// システム専用イベント
//------------------------------------------
#define EVENT_LOW_VOLTAGE						( EVENT_SYSTEM_BIT | 0x00000001 )					// 低電圧イベント
#define EVENT_LOW_VOLTAGE_RECOVERY				( EVENT_SYSTEM_BIT | 0x00000002 )					// 低電圧復帰イベント


//------------------------------------------
// コインメックタスク専用イベント
//------------------------------------------
#define EVENT_COIN_100YEN						( EVENT_TASK_COINMEC_BIT | 0x00000001 )				// 100円投入イベント
#define EVENT_COIN_500YEN						( EVENT_TASK_COINMEC_BIT | 0x00000002 )				// 100円投入イベント
#define EVENT_PREPAID_100YEN					( EVENT_TASK_COINMEC_BIT | 0x00000004 )				// プリペイドカード100円投入イベント
#define EVENT_PREPAID_500YEN					( EVENT_TASK_COINMEC_BIT | 0x00000008 )				// プリペイドカード500円投入イベント
#define EVENT_FEE_PAYMENT_COMPLETED				( EVENT_TASK_COINMEC_BIT | 0x00000010 )				// 料金支払い完了イベント


//------------------------------------------
// Laundryタスク専用イベント
//------------------------------------------
#define EVENT_LAUNDRY_SERIAL_ANALYZED			( EVENT_TASK_LAUNDRY_BIT | 0x00000001 )				// シリアル解析完了イベント
#define EVENT_LAUNDRY_SEND_END					( EVENT_TASK_LAUNDRY_BIT | 0x00000002 )				// シリアル送信完了イベント

//------------------------------------------
// 音声再生タスク専用イベント
//------------------------------------------
#define EVENT_AUDIOPLAYBACK_START				( EVENT_TASK_AUDIOPLAYBACK_BIT | 0x00000001 )		// 音声再生開始イベント
#define EVENT_AUDIOPLAYBACK_STOP				( EVENT_TASK_AUDIOPLAYBACK_BIT | 0x00000002 )		// 音声再生停止イベント


//------------------------------------------
// FWUPDATE専用イベント(要求用)
//------------------------------------------
#define EVENT_FWUPDATE_REQ_FWUPDATE_MODE		( EVENT_TASK_FWUPDATE_REQ_BIT | 0x00000001 )		// Fwupdateモード要求イベント
#define EVENT_FWUPDATE_REQ_FWUPDATE_WRITE		( EVENT_TASK_FWUPDATE_REQ_BIT | 0x00000002 )		// Fwupdate書込み要求イベント

//------------------------------------------
// FWUPDATE専用イベント(応答用)
//------------------------------------------
#define EVENT_FWUPDATE_RES_SUCCESS				( EVENT_TASK_FWUPDATE_RES_BIT | 0x00000001 )		// 成功
#define EVENT_FWUPDATE_RES_ERROR				( EVENT_TASK_FWUPDATE_RES_BIT | 0x00000002 )		// 異常
#define EVENT_FWUPDATE_RES_ERROR_UNKNOWN_EVENT	( EVENT_TASK_FWUPDATE_RES_BIT | 0x00000003 )		// 不明なイベントを受信
#define EVENT_FWUPDATE_RES_ERROR_RECORD_NUMBER	( EVENT_TASK_FWUPDATE_RES_BIT | 0x00000004 )		// レコード番号エラー
#define EVENT_FWUPDATE_RES_ERROR_ERASE			( EVENT_TASK_FWUPDATE_RES_BIT | 0x00000005 )		// 消去エラー
#define EVENT_FWUPDATE_RES_ERROR_WRITE			( EVENT_TASK_FWUPDATE_RES_BIT | 0x00000006 )		// 書込みエラー
#define EVENT_FWUPDATE_RES_ERROR_SYSTEM			( EVENT_TASK_FWUPDATE_RES_BIT | 0x00999999 )		// システムエラー


#endif

//------------------------------------------
// Controlタスク専用イベント
//------------------------------------------
#define EVENT_CONTROL_SERIAL_ANALYZED			( EVENT_TASK_CONTROL_BIT | 0x00000001 )				// シリアル解析完了イベント
#define EVENT_CONTROL_SEND_END					( EVENT_TASK_CONTROL_BIT | 0x00000002 )				// シリアル送信完了イベント



//============================================================================================================
// イベント初期化処理（※システム側でコールすること）
//============================================================================================================
void InitEvent(void);

//============================================================================================================
// イベント生成
//============================================================================================================
int8_t CreateEvent(TASK_KIND_ENUM eTaskKind);

//============================================================================================================
// イベント送信
//============================================================================================================
BaseType_t SendEvent(TASK_KIND_ENUM eTaskKind, PRIORITY_KIND_ENUM ePriorityKind, uint32_t Event, TickType_t xTicksToWait);

//============================================================================================================
// イベント送信(ISR用)
//============================================================================================================
BaseType_t SendEventFromISR(TASK_KIND_ENUM eTaskKind, PRIORITY_KIND_ENUM ePriorityKind, uint32_t Event, BaseType_t* pxHigherPriorityTaskWoken);

//============================================================================================================
// イベント受信処理
//============================================================================================================
BaseType_t ReceiveEvent(TASK_KIND_ENUM eTaskKind, uint32_t *pEvent, TickType_t xTicksToWait);





#endif /* EVENT_H_ */
