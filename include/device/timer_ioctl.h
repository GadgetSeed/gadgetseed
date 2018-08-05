/** @file
    @brief	タイマドライバ ioctl 用マクロ定義

    推奨デバイス名 : "timer"

    @date	2015.09.27
    @author	Takashi SHUDO
*/

#ifndef TIMER_IOCTL_H
#define TIMER_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_TIMER	"timer"	///< 標準タイマデバイス名

#define IOCMD_TIMER_GETTIME	STDIOCTL(DEV_TIMER,0x01)	///< タイマの値を取得する(msec)
#define IOCMD_TIMER_SETFUNC	STDIOCTL(DEV_TIMER,0x02)	///< タイマの割り込みハンドラ処理を登録する
#define IOCMD_TIMER_START	STDIOCTL(DEV_TIMER,0x03)	///< タイマのカウントを開始する
#define IOCMD_TIMER_STOP	STDIOCTL(DEV_TIMER,0x04)	///< タイマのカウントを停止する
#define IOCMD_TIMER_GETSYSTIME	STDIOCTL(DEV_TIMER,0x05)	///< タイマの値を取得する(usec)

#endif // TIMER_IOCTL_H
