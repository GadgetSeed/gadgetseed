/** @file
    @brief	inputドライバ ioctl 用マクロ定義

    推奨デバイス名 : "input"

    @date	2019.11.11
    @author	Takashi SHUDO

    キーボード、ボタン等入力デバイス
*/

#ifndef INPUT_IOCTL_H
#define INPUT_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_INPUT	"input"	///< 標準GPIOデバイス名

#define IOCMD_INPUT_SCAN_LINE		STDIOCTL(DEV_INPUT,0x00)	///< キー状態のスキャン

#endif // INPUT_IOCTL_H
