/** @file
    @brief	ストレージデバイスドライバ ioctl 用マクロ定義

    推奨デバイス名 : "sd"

    @date	2018.02.04
    @author	Takashi SHUDO
*/

#ifndef SD_IOCTL_H
#define SD_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_SD		"sd"		///< 標準ストレージデバイス名(MMC,SD等)

#define IOCMD_SD_GET_SECTOR_COUNT	STDIOCTL(DEV_SD,1)	///< セクタ数を取得する
#define IOCMD_SD_GET_SECTOR_SIZE	STDIOCTL(DEV_SD,2)	///< 1セクタサイズを取得する
#define IOCMD_SD_GET_BLOCK_SIZE		STDIOCTL(DEV_SD,3)	///< 消去ブロックサイズを取得する

#endif // SD_IOCTL_H
