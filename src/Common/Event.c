/*
 * Event.c
 *
 *  Created on: 2020/10/19
 *      Author: 99990045
 */
#include "Event.h"
#include "queue.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"


#define QUEUE_LENGTH						( 20 )										// キューの長さ


typedef struct
{
	QueueHandle_t								QueueHandle[TASK_KIND_MAX];				// キューハンドル



} EVENT_GLOBAL_TABLE;

EVENT_GLOBAL_TABLE								g_tEvent;



//============================================================================================================
// イベント初期化処理（※システム側でコールすること）
//============================================================================================================
void InitEvent(void)
{
	// グローバル変数の初期化
	memset(g_tEvent.QueueHandle,0x00,sizeof(g_tEvent.QueueHandle));
}


//============================================================================================================
// イベント生成
//============================================================================================================
int8_t CreateEvent(TASK_KIND_ENUM eTaskKind)
{
	// 既に作成している場合
	if (g_tEvent.QueueHandle[eTaskKind] != NULL)
	{
		return 0;
	}

	// キューハンドルを生成
	g_tEvent.QueueHandle[eTaskKind] = xQueueCreate(QUEUE_LENGTH, sizeof(uint32_t));

	return 0;
}


//============================================================================================================
// イベント送信
//============================================================================================================
BaseType_t SendEvent(TASK_KIND_ENUM eTaskKind, PRIORITY_KIND_ENUM ePriorityKind, uint32_t Event, TickType_t xTicksToWait)
{
	BaseType_t					Result = pdFALSE;
	QueueHandle_t				QueueHandle = NULL;


	// 指定したタスクのハンドルチェック
	QueueHandle = g_tEvent.QueueHandle[eTaskKind];
	if (QueueHandle == NULL)
	{
		return pdFALSE;
	}

	// イベントを送信
	if (ePriorityKind == PRIORITY_KIND_HIGH)
	{
		Result = xQueueSendToFront(QueueHandle, &Event, xTicksToWait);
	}
	else
	{
		Result = xQueueSend(QueueHandle, &Event, xTicksToWait);
	}

	return Result;
}


//============================================================================================================
// イベント送信(ISR用)
//============================================================================================================
BaseType_t SendEventFromISR(TASK_KIND_ENUM eTaskKind, PRIORITY_KIND_ENUM ePriorityKind, uint32_t Event, BaseType_t* pxHigherPriorityTaskWoken)
{
	BaseType_t					Result = pdFALSE;
	QueueHandle_t				QueueHandle = NULL;


	// 引数チェック
	if (pxHigherPriorityTaskWoken == NULL)
	{
		return pdFALSE;
	}

	// 指定したタスクのハンドルチェック
	QueueHandle = g_tEvent.QueueHandle[eTaskKind];
	if (QueueHandle == NULL)
	{
		return pdFALSE;
	}

	// イベントを送信
	if (ePriorityKind == PRIORITY_KIND_HIGH)
	{
		Result = xQueueSendToFrontFromISR(QueueHandle, &Event, pxHigherPriorityTaskWoken);
	}
	else
	{
		Result = xQueueSendFromISR(QueueHandle, &Event, pxHigherPriorityTaskWoken);
	}

	return Result;
}


//============================================================================================================
// イベント受信処理
//============================================================================================================
BaseType_t ReceiveEvent(TASK_KIND_ENUM eTaskKind, uint32_t *pEvent, TickType_t xTicksToWait)
{
	BaseType_t					Result = pdFALSE;
	QueueHandle_t				QueueHandle = NULL;


	// 引数チェック
	if (pEvent == NULL)
	{
		return pdFALSE;
	}

	// 指定したタスクのハンドルチェック
	QueueHandle = g_tEvent.QueueHandle[eTaskKind];
	if (QueueHandle == NULL)
	{
		return pdFALSE;
	}

	// イベントを受信
	Result = xQueueReceive(QueueHandle, pEvent, xTicksToWait);

	return Result;
}







