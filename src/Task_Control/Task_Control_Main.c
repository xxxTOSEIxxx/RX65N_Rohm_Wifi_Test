/*
 * Task_Control_Main.c
 *
 *  Created on: 2020/12/25
 *      Author: 99990045
 */
#include "Task_Control_Global.h"
#include "Event.h"
#include "stdbool.h"
#include "parson.h"
#include "string.h"

#define CONTROL_SCI_CH              ( 11 )						// シリアル通信チャンネル
#define STX                         ( 0x02 )					// STX
#define ETX                         ( 0x03 )					// ETX
#define	SEND_TIMEOUT				( 2000 )					// 送信タイムアウト

// Controlタスク用グローバル変数
TASK_CONTROL_GLOBAL_TABLE               g_tControl;


// プロトタイプ宣言
bool Task_Control_Open(void);
bool Task_Control_Close(void);
void Task_Control_Callback(void* pArgs);
bool Task_Control_SerialRecv(void);
void Task_Control_SerialAnalyze(uint8_t ch);
void Task_Control_CommandCheck(const uint8_t* pSerialDate, uint32_t SerialDataSize);
sci_err_t Task_Control_SendResponse(uint8_t *p_src, uint16_t const length);


//====================================================================================================
// Controlタスクメイン処理
//====================================================================================================
void Task_Control_Main(void)
{
	bool					bRet = false;
    uint32_t                Event = 0;


    // LEDのPIN設定
    PORTA.PDR.BIT.B1 = 1;
    PORTA.PODR.BIT.B1 = 0;				// OFF

    // Controlタスク用のイベント生成
    CreateEvent(TASK_KIND_CONTROL);

    // ミューテックスハンドルを生成
	g_tControl.MutexHandle = xSemaphoreCreateMutex();
    if (g_tControl.MutexHandle == NULL)
    {
        printf("xSemaphoreCreateMutex Error.\n");
        while(1);
    }

    // Controlタスク用シリアルオープン
    bRet = Task_Control_Open();
    if (bRet == false)
    {
        printf("Task_Control_Open Error.\n");
        while(1);
    }

    while(1)
    {
        // シリアルデータ解析完了待ち
        ReceiveEvent(TASK_KIND_CONTROL, &Event, portMAX_DELAY);
        if (Event == EVENT_CONTROL_SERIAL_ANALYZED)
        {
        	// コマンドチェック
        	Task_Control_CommandCheck(g_tControl.szSerialBuff, strlen(g_tControl.szSerialBuff));

            // STX待ちにする
            g_tControl.eSerialAnalyze = SERIAL_ANALYZE_STX;
        }
    }
}


//====================================================================================================
// Controlタスク用シリアルオープン
//====================================================================================================
bool Task_Control_Open(void)
{
	sci_err_t               eSciResult = SCI_SUCCESS;

	g_tControl.SciCh = CONTROL_SCI_CH;
	g_tControl.eSciMode = SCI_MODE_ASYNC;
	g_tControl.tSciCfg.async.baud_rate = 9600;                  // ボーレート
	g_tControl.tSciCfg.async.clk_src = SCI_CLK_INT;             // クロックソース（内部クロック, 外部クロックx8, 外部クロックx16)
	g_tControl.tSciCfg.async.data_size = SCI_DATA_8BIT;         // データ長
	g_tControl.tSciCfg.async.parity_en = SCI_PARITY_OFF;        // パリティチェック有り or 無し
	g_tControl.tSciCfg.async.parity_type = SCI_EVEN_PARITY;     // パリティチェック(奇数 or 偶数)
	g_tControl.tSciCfg.async.stop_bits = SCI_STOPBITS_1;        // ストップビット
	g_tControl.tSciCfg.async.int_priority = 3;                  // 割込み優先度(1=最低値, 15=最高値)

    eSciResult = R_SCI_Open(g_tControl.SciCh,                   // 初期化するチャンネル
    		                g_tControl.eSciMode,                // 動作モード
                            &g_tControl.tSciCfg,                // 設定情報
							Task_Control_Callback,              // コールバック関数
                            &g_tControl.SciHandle);             // SCIハンドル
    if (eSciResult != SCI_SUCCESS)
    {
        printf("R_SCI_Open Error. [eSciResult:%d]\n",eSciResult);
        return false;
    }

    // SCIの端子を有効にする
    R_SCI_PinSet_SCI11();

    return true;
}


//====================================================================================================
// Controlタスク用シリアルクローズ
//====================================================================================================
bool Task_Control_Close(void)
{
    sci_err_t               eSciResult = SCI_SUCCESS;


	// シリアルハンドルを解放する
	if (g_tControl.SciHandle != NULL)
	{
		eSciResult = R_SCI_Close(g_tControl.SciHandle);
		if (eSciResult != SCI_SUCCESS)
		{
			printf("R_SCI_Close Error. [eSciResult:%d]\n",eSciResult);
			return false;
		}
		g_tControl.SciHandle = NULL;
	}

    return true;
}


//====================================================================================================
// シリアル通信のコールバック(*割込みルーチン)
//====================================================================================================
void Task_Control_Callback(void* pArgs)
{
    sci_cb_args_t*          ptArgs = (sci_cb_args_t*)pArgs;
    BaseType_t				pxHigherPriorityTaskWoken = pdFALSE;


    switch (ptArgs->event) {
    case SCI_EVT_RX_CHAR:
    case SCI_EVT_RX_CHAR_MATCH:
        // シリアル通信の受信処理
        Task_Control_SerialRecv();
        break;
    case SCI_EVT_TEI:           // 送信割込み発生
//      printf("--- SCI_EVT_TEI ---\n");
    	SendEventFromISR(TASK_KIND_CONTROL, PRIORITY_KIND_NORMAL, EVENT_CONTROL_SEND_END, &pxHigherPriorityTaskWoken);
    	portYIELD_FROM_ISR(pxHigherPriorityTaskWoken);

        break;
    case SCI_EVT_RXBUF_OVFL:    // RXキューバッファフロー
        printf("--- SCI_EVT_RXBUF_OVFL ---\n");
        break;
    case SCI_EVT_FRAMING_ERR:   // フレミングエラー
        printf("--- SCI_EVT_FRAMING_ERR ---\n");
        break;
    case SCI_EVT_PARITY_ERR:    // パリティエラー
        printf("--- SCI_EVT_PARITY_ERR ---\n");
        break;
    case SCI_EVT_OVFL_ERR:      // オーバーフローエラー
        printf("--- SCI_EVT_OVFL_ERR ---\n");
        break;
    default:                    // 意図していないエラー
        printf("--- Unknown SCI Event [%d] ---\n", ptArgs->event);
        break;
    }
}


//--------------------------------------------------------------------------------------------------
// シリアル通信の受信処理(*割込みルーチン)
//--------------------------------------------------------------------------------------------------
bool Task_Control_SerialRecv(void)
{
    sci_err_t                       eSciResult = SCI_SUCCESS;
    uint8_t                         ch = 0x00;


    // RXキューから1Byte分データを取得する
    eSciResult = R_SCI_Receive(g_tControl.SciHandle, &ch, 1);
    if (eSciResult != SCI_SUCCESS)
    {
        printf("R_SCI_Receive Error. [eSciResult:%d]\n",eSciResult);
        return false;
    }

    // シリアル受信データ解析処理
    Task_Control_SerialAnalyze(ch);

    return true;
}


//--------------------------------------------------------------------------------------------------
// シリアル受信データ解析処理(*割込みルーチン)
//--------------------------------------------------------------------------------------------------
void Task_Control_SerialAnalyze(uint8_t ch)
{
    // STX待ちの場合
    if (g_tControl.eSerialAnalyze == SERIAL_ANALYZE_STX)
    {
        // STXの場合
        if (ch == STX)
        {
            // シリアルデータとして受信を開始
        	g_tControl.eSerialAnalyze = SERIAL_ANALYZE_ETX;
        	g_tControl.SerialSize = 0;
            memset(&g_tControl.szSerialBuff, 0x00, sizeof(g_tControl.szSerialBuff));

            // STXを格納
            //g_tControl.szSerialBuff[g_tControl.SerialSize++] = ch;
        }
        else
        {
            // 読み捨て
        }
    }
    // ETX待ちの場合
    else if (g_tControl.eSerialAnalyze == SERIAL_ANALYZE_ETX)
    {
        // STXの場合
        if (ch == STX)
        {
            // 新たにシリアルデータとして受信を開始
        	g_tControl.eSerialAnalyze = SERIAL_ANALYZE_ETX;
        	g_tControl.SerialSize = 0;
            memset(&g_tControl.szSerialBuff, 0x00, sizeof(g_tControl.szSerialBuff));

            // STXを格納
            //g_tControl.szSerialBuff[g_tControl.SerialSize++] = ch;
        }
        // ETXの場合
        else if (ch == ETX)
        {
            // ETXを格納
        	//g_tControl.szSerialBuff[g_tControl.SerialSize++] = ch;

            // 解析一時停止とする
        	g_tControl.eSerialAnalyze = SERIAL_ANALYZE_PAUSE;

            // 【シリアル解析完了イベント】を送信する
            SendEvent(TASK_KIND_CONTROL, PRIORITY_KIND_NORMAL, EVENT_CONTROL_SERIAL_ANALYZED, portMAX_DELAY);
        }
        else
        {
            // シリアルデータとして格納する
        	g_tControl.szSerialBuff[g_tControl.SerialSize++] = ch;
        }
    }
    // 解析一時中断の場合
    else
    {
        // 読み捨て
    }
}


//====================================================================================================
// コマンドチェック
//====================================================================================================
void Task_Control_CommandCheck(const uint8_t* pSerialDate, uint32_t SerialDataSize)
{
	JSON_Value*             pJsonValue = NULL;
	JSON_Object*            pRoot_object = NULL;
	const char*             pString = NULL;


	memset(g_tControl.szResponseBuff, 0x00, sizeof(g_tControl.szResponseBuff));

    // JSON文字列を解析
    pJsonValue = json_parse_string(pSerialDate);
    if (pJsonValue == NULL)
    {
    	sprintf(g_tControl.szResponseBuff, "json_parse_string Error.\n");
    	Task_Control_SendResponse(g_tControl.szResponseBuff,strlen(g_tControl.szResponseBuff));
        goto Task_Control_CommandCheck_EndProc_Label;
    }

    // JSONオブジェクトなのかをチェックする
    if (json_value_get_type(pJsonValue) != JSONObject)
    {
    	sprintf(g_tControl.szResponseBuff, "json_value_get_type Error.\n");
    	Task_Control_SendResponse(g_tControl.szResponseBuff,strlen(g_tControl.szResponseBuff));
        goto Task_Control_CommandCheck_EndProc_Label;
    }

    // 第1階層解析
    pRoot_object = json_value_get_object(pJsonValue);
    if (pRoot_object == NULL)
    {
    	sprintf(g_tControl.szResponseBuff, "json_value_get_object Error.\n");
    	Task_Control_SendResponse(g_tControl.szResponseBuff,strlen(g_tControl.szResponseBuff));
        goto Task_Control_CommandCheck_EndProc_Label;
    }

    // 「SetLed」を探す
    pString = json_object_get_string(pRoot_object, "SetLed");
    if (pString != NULL)
    {
    	if ((strcmp(pString, "on") == 0) || (strcmp(pString, "On") == 0) || (strcmp(pString, "ON") == 0))
    	{
    		PORTA.PODR.BIT.B1 = 1;				// ON
    		sprintf(g_tControl.szResponseBuff, "'SetLed:%s' Done.\n", pString);
        	Task_Control_SendResponse(g_tControl.szResponseBuff,strlen(g_tControl.szResponseBuff));
            goto Task_Control_CommandCheck_EndProc_Label;
    	}
    	else if ((strcmp(pString, "off") == 0) || (strcmp(pString, "Off") == 0) || (strcmp(pString, "OFF") == 0))
    	{
    		PORTA.PODR.BIT.B1 = 0;				// OFF
        	sprintf(g_tControl.szResponseBuff, "'SetLed:%s' Done.\n", pString);
        	Task_Control_SendResponse(g_tControl.szResponseBuff,strlen(g_tControl.szResponseBuff));
            goto Task_Control_CommandCheck_EndProc_Label;
    	}
    	else
    	{
        	sprintf(g_tControl.szResponseBuff, "'SetLed' Illegal Control Command.\n");
        	Task_Control_SendResponse(g_tControl.szResponseBuff,strlen(g_tControl.szResponseBuff));
            goto Task_Control_CommandCheck_EndProc_Label;
    	}
    }

    // 「GetLed」を探す
    pString = json_object_get_string(pRoot_object, "GetLed");
    if (pString != NULL)
    {
    	if (PORTA.PIDR.BIT.B1 == 1)
    	{
    		sprintf(g_tControl.szResponseBuff, "Led : On\n");
    	}
    	else
    	{
    		sprintf(g_tControl.szResponseBuff, "Led : Off\n");
    	}
    	Task_Control_SendResponse(g_tControl.szResponseBuff,strlen(g_tControl.szResponseBuff));
        goto Task_Control_CommandCheck_EndProc_Label;
    }


    // コマンド不正
	sprintf(g_tControl.szResponseBuff, "Command invalid.n");
	Task_Control_SendResponse(g_tControl.szResponseBuff,strlen(g_tControl.szResponseBuff));

Task_Control_CommandCheck_EndProc_Label:

	// 解析終了(後片付け)
	if (pJsonValue != NULL)
	{
		json_value_free(pJsonValue);
		pJsonValue = NULL;
	}
}


//====================================================================================================
// シリアル送信処理（内部用)
//====================================================================================================
sci_err_t Task_Control_SendResponse(uint8_t *p_src, uint16_t const length)
{
	sci_err_t            				eSciResult = SCI_SUCCESS;
	BaseType_t							Result = pdFALSE;
	uint32_t 							Event = 0x00000000;


	// シリアル送信
	eSciResult = R_SCI_Send(g_tControl.SciHandle, p_src, length);
    if (eSciResult != SCI_SUCCESS)
    {
        printf("R_SCI_Send Error. [eSciResult:%d]\n",eSciResult);
        return eSciResult;
    }

    // シリアル送信完了待ち
    Result = ReceiveEvent(TASK_KIND_LAUNDRY, &Event, SEND_TIMEOUT);
    if (Result == pdFALSE)
    {
    	printf("ReceiveEvent Error. [Result:%d]\n",Result);
    	return SCI_ERR_XCVR_BUSY;
    }

    return SCI_SUCCESS;
}

