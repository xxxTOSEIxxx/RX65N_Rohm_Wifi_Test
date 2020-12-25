/*
 * Task_Control_Global.h
 *
 *  Created on: 2020/12/25
 *      Author: 99990045
 */

#ifndef TASK_CONTROL_GLOBAL_H_
#define TASK_CONTROL_GLOBAL_H_
#include "platform.h"
#include "semphr.h"
#include "r_sci_rx_if.h"


#define CONTROL_SERIAL_BUFF_SIZE                    ( 1000 )
#define CONTROL_RESPONSE_BUFF_SIZE                  ( 512 )

// シリアル解析種別
typedef enum
{
    SERIAL_ANALYZE_STX = 0,             // STX待ち
    SERIAL_ANALYZE_ETX,                 // ETX待ち
    SERIAL_ANALYZE_PAUSE,               // 処理中のため、解析一時停止

} SERIAL_ANALYZE_ENUM;


typedef struct
{
    SemaphoreHandle_t                   MutexHandle;                                // ミューテックスハンドル

    // シリアル通信用設定情報
    uint8_t                             SciCh;                                      // チャンネル
    sci_mode_t                          eSciMode;                                   // 動作モード
    sci_cfg_t                           tSciCfg;                                    // 設定情報
    sci_hdl_t                           SciHandle;                                  // SCIハンドル

    SERIAL_ANALYZE_ENUM                 eSerialAnalyze;
    uint32_t                            SerialSize;                                 // シリアル通信用バッファに格納したデータサイズ
    uint8_t                             szSerialBuff[CONTROL_SERIAL_BUFF_SIZE + 1]; // シリアル通信用バッファ

    uint8_t								szResponseBuff[CONTROL_RESPONSE_BUFF_SIZE + 1];

} TASK_CONTROL_GLOBAL_TABLE;

#endif /* TASK_CONTROL_GLOBAL_H_ */
