/** @file
    @brief	Etherドライバ ioctl 用マクロ定義

    推奨デバイス名 : "eth"

    @date	2012.01.10
    @author	Takashi SHUDO
*/

#ifndef ETHER_IOCTL_H
#define ETHER_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_ETHER	"eth"	///< 標準Etherデバイス名

#define IORTN_BIT_ETHER_LINK_UP		(0x01)	///< リンクアップ状態
#define IORTN_BIT_ETHER_FULLDUPLEX	(0x02)	///< 全二重状態
#define IORTN_BIT_ETHER_100M		(0x04)	///< 通信速度100Mbps
#define IORTN_BIT_ETHER_1000M		(0x08)	///< 通信速度1Gbps

// com : MACアドレスを取得する, arg : 取得アドレス(unsigned char addr[6])
#define IOCMD_ETHER_GET_MACADDR	STDIOCTL(DEV_ETHER,0x00)	///< MACアドレスを取得する
#define IOCMD_ETHER_SET_MACADDR	STDIOCTL(DEV_ETHER,0x01)	///< MACアドレスを設定する
#define IOCMD_ETHER_CLEAR_BUF	STDIOCTL(DEV_ETHER,0x02)	///< バッファをクリアする
#define IOCMD_ETHER_LINK_UP	STDIOCTL(DEV_ETHER,0x03)	///< リンクアップする
#define IOCMD_ETHER_LINK_DOWN	STDIOCTL(DEV_ETHER,0x04)	///< リンクダウンする
#define IOCMD_ETHER_GET_LINK_STATUS	STDIOCTL(DEV_ETHER,0x05)	///< リンク状態を取得する

#endif // ETHER_IOCTL_H
