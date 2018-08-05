/** @file
    @brief	環境センサ(温度、湿度、気圧等)ドライバ ioctl 用マクロ定義

    推奨デバイス名 : "envsnsr"

    @date	2018.01.23
    @author	Takashi SHUDO
*/

#ifndef ENVSNSR_IOCTL_H
#define ENVSNSR_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_ENVSNSR	"envsnsr"	///< 標準環境センサデバイス名

#define IOCMD_ENVSNSR_GET_TEMP		STDIOCTL(DEV_ENVSNSR,0x00)	///< 温度を取得する(℃)
#define IOCMD_ENVSNSR_GET_HUM		STDIOCTL(DEV_ENVSNSR,0x01)	///< 湿度を取得する(%)
#define IOCMD_ENVSNSR_GET_PRESS		STDIOCTL(DEV_ENVSNSR,0x02)	///< 気圧を取得する(hPa)
#define IOCMD_ENVSNSR_GET_THP		STDIOCTL(DEV_ENVSNSR,0x03)	///< TEMP & HUM & PRES

#endif // ENVSNSR_IOCTL_H
