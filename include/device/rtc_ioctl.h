/** @file
    @brief	RTCドライバ ioctl 用マクロ定義

    推奨デバイス名 : "rtc"

    @date	2015.09.27
    @author	Takashi SHUDO
*/

#ifndef RTC_IOCTL_H
#define RTC_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_RTC	"rtc"	///< 標準リアルタイムクロックデバイス名

#define IOCMD_RTC_SET		STDIOCTL(DEV_RTC,0x00)	///< 時刻を設定する
#define IOCMD_RTC_GET		STDIOCTL(DEV_RTC,0x01)	///< 時刻を取得する

#endif // RTC_IOCTL_H
