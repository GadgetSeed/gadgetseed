/** @file
    @brief	SPIドライバ ioctl 用マクロ定義

    推奨デバイス名 : "spi"

    @date	2011.12.24
    @author	Takashi SHUDO
*/

#ifndef SPI_IOCTL_H
#define SPI_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_SPI	"spi"	///< 標準SPIマスターコントローラデバイス名


#define IOCMD_SPI_SPEED		STDIOCTL(DEV_SPI,0x00)	///< com : 転送速度を設定する, arg : 転送速度(bps)

// com : write アクセス終了時の CS 状態を設定する
#define IOCMD_SPI_CS0ASSERT	STDIOCTL(DEV_SPI,0x10)	///< CS0をアサートしたままにする
#define IOCMD_SPI_CS0NEGATE	STDIOCTL(DEV_SPI,0x11)	///< CS0をネゲートする

#define IOCMD_SPI_CS1ASSERT	STDIOCTL(DEV_SPI,0x12)	///< CS1をアサートしたままにする
#define IOCMD_SPI_CS1NEGATE	STDIOCTL(DEV_SPI,0x13)	///< CS1をネゲートする

#define IOCMD_SPI_CS2ASSERT	STDIOCTL(DEV_SPI,0x14)	///< CS2をアサートしたままにする
#define IOCMD_SPI_CS2NEGATE	STDIOCTL(DEV_SPI,0x15)	///< CS2をネゲートする

#define IOCMD_SPI_CS3ASSERT	STDIOCTL(DEV_SPI,0x16)	///< CS3をアサートしたままにする
#define IOCMD_SPI_CS3NEGATE	STDIOCTL(DEV_SPI,0x17)	///< CS3をネゲートする

#define IOCMD_SPI_FORCE_UNLOCK	STDIOCTL(DEV_SPI,0x20)	///< 強制的にアンロック

#define IOCMD_SPI_WRITE_BYTE	STDIOCTL(DEV_SPI,0x30)	///< 1バイトデータを書き込む
#define IOCMD_SPI_WRITE_WORD	STDIOCTL(DEV_SPI,0x31)	///< 2バイトデータを書き込む
#define IOCMD_SPI_WRITE_LONG	STDIOCTL(DEV_SPI,0x32)	///< 4バイトデータを書き込む

#define IOCMD_SPI_WRITE_CONT_BYTE	STDIOCTL(DEV_SPI,0x40)	///< 1バイトデータを読み出す
#define IOCMD_SPI_WRITE_CONT_WORD	STDIOCTL(DEV_SPI,0x41)	///< 2バイトデータを読み出す
#define IOCMD_SPI_WRITE_CONT_LONG	STDIOCTL(DEV_SPI,0x42)	///< 4バイトデータを読み出す

#endif // SPI_IOCTL_H
