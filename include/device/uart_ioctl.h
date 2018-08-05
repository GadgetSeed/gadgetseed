/** @file
    @brief	UARTドライバ ioctl 用マクロ定義

    推奨デバイス名 : "uart"

    @date	2014.12.30
    @author	Takashi SHUDO
*/

#ifndef UART_IOCTL_H
#define UART_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_DEBUG	"debug"	///< 標準エラー出力用UARTデバイス名
#define DEF_DEV_NAME_UART	"uart"	///< 標準入出力用UARTデバイス名

// com : 通信速度を設定する, arg : 通信速度(bps)
#define IOCMD_UART_SPEED	STDIOCTL(DEV_UART,0x00)	///< 通信速度設定 @info arg:設定する通信速度(bps)

#endif // UART_IOCTL_H
