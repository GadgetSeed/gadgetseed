/** @file
    @brief	外部割り込み(IRQ)ドライバ ioctl 用マクロ定義

    推奨デバイス名 : "irq"

    @date	2015.10.13
    @author	Takashi SHUDO
*/

#ifndef IRQ_IOCTL_H
#define IRQ_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_IRQ	"irq"	///< 標準外部割り込み(IRQ)ドライバ名

#define IOCMD_IRQ_REGISTER	STDIOCTL(DEV_IRQ,0x00)	///< 割込処理関数を登録する
#define IOCMD_IRQ_UNREGISTER	STDIOCTL(DEV_IRQ,0x01)	///< 割込処理関数を登録解除する
#define IOCMD_IRQ_ENABLE	STDIOCTL(DEV_IRQ,0x02)	///< 割込を有効にする
#define IOCMD_IRQ_DISABLE	STDIOCTL(DEV_IRQ,0x03)	///< 割込を無効にする
#define IOCMD_IRQ_SET_EDGE	STDIOCTL(DEV_IRQ,0x04)	///< 割込エッジを設定する[TODO]
#define IOCMD_IRQ_GET_LEVEL	STDIOCTL(DEV_IRQ,0x05)	///< 割込端子のレベルを取得する
#define IOCMD_IRQ_GET_INT	STDIOCTL(DEV_IRQ,0x06)	///< 割込状態を取得する

#endif // IRQ_IOCTL_H
