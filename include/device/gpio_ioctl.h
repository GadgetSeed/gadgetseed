/** @file
    @brief	GPIOドライバ ioctl 用マクロ定義

    推奨デバイス名 : "gpio"

    @date	2015.10.17
    @author	Takashi SHUDO
*/

#ifndef GPIO_IOCTL_H
#define GPIO_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_GPIO	"gpio"	///< 標準GPIOデバイス名

#define IOCMD_GPIO_DIRECTION		STDIOCTL(DEV_GPIO,0x00)	///< GPIO入出力設定
#define IOARG_GPIO_INPUT		(0)	///< GPIO入力設定(arg設定値)
#define IOARG_GPIO_OUTPUT		(1)	///< GPIO出力設定(arg設定値)
#define IOCMD_GPIO_SET_BITS		STDIOCTL(DEV_GPIO,0x01)	///< GPIO High(1)出力
#define IOCMD_GPIO_CLEAR_BITS		STDIOCTL(DEV_GPIO,0x02)	///< GPIO Low(0)出力

#endif // GPIO_IOCTL_H
