/** @file
    @brief	Buzzerドライバ ioctl 用マクロ定義

    推奨デバイス名 : "buzzer"

    @date	2015.09.27
    @author	Takashi SHUDO
*/

#ifndef BUZZER_IOCTL_H
#define BUZZER_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_BUZZER	"buzzer"	///< 標準ブザーデバイス名

#define IOCMD_BUZZER_ON		STDIOCTL(DEV_BUZZER,0x00)	///< ブザー音声出力開始
#define IOCMD_BUZZER_OFF	STDIOCTL(DEV_BUZZER,0x01)	///< ブザー音声出力停止

#endif // BUZZER_IOCTL_H
