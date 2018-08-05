/** @file
    @brief	映像関連ドライバIO ioctl 用マクロ定義

    推奨デバイス名 : "vio"

    @date	2015.08.15
    @author	Takashi SHUDO
*/

#ifndef VIO_IOCTL_H
#define VIO_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_VIDEOIO	"videoio"	///< 標準ビデオIOデバイスドライバ名

// ビデオ出力IO制御 ioctl
#define IOCMD_VIO_LOCK_BUS	STDIOCTL(DEV_VIDEOIO,0x00)	///< バスをMUTEXロックする
#define IOCMD_VIO_UNLOCK_BUS	STDIOCTL(DEV_VIDEOIO,0x01)	///< バスをMUTEXアンロックする

#define IOCMD_VIO_SET_RESET	STDIOCTL(DEV_VIDEOIO,0x02)	///< デバイスのリセットを設定する
#define IOCMD_VIO_SET_CS	STDIOCTL(DEV_VIDEOIO,0x03)	///< CSを設定する

#define IOCMD_VIO_WRITE_COMMAND	STDIOCTL(DEV_VIDEOIO,0x10)	///< コントローラデバイスにコマンドを書き込む
#define IOCMD_VIO_WRITE_DATA8	STDIOCTL(DEV_VIDEOIO,0x11)	///< コントローラデバイスに8ビットデータを書き込む
#define IOCMD_VIO_WRITE_DATA16	STDIOCTL(DEV_VIDEOIO,0x12)	///< コントローラデバイスに16ビットデータを書き込む
#define IOCMD_VIO_WRITE_DATA24	STDIOCTL(DEV_VIDEOIO,0x13)	///< コントローラデバイスに24ビットデータを書き込む
#define IOCMD_VIO_WRITE_DATA32	STDIOCTL(DEV_VIDEOIO,0x14)	///< コントローラデバイスに32ビットデータを書き込む
#define IOCMD_VIO_NOLOCK_WRITE_DATA16	STDIOCTL(DEV_VIDEOIO,0x16)	///< コントローラデバイスに16ビットデータを書き込む(ロックは無視)

#define IOCMD_VIO_READ_DATA8	STDIOCTL(DEV_VIDEOIO,0x17)	///< コントローラデバイスから8ビットデータを読み出す

#define IOCMD_VIO_WRITE_REG8	STDIOCTL(DEV_VIDEOIO,0x20)	///< コントローラデバイスのレジスタに8ビットデータを書き込む
#define IOCMD_VIO_WRITE_REG16	STDIOCTL(DEV_VIDEOIO,0x21)	///< コントローラデバイスのレジスタに16ビットデータを書き込む
#define IOCMD_VIO_WRITE_REG24	STDIOCTL(DEV_VIDEOIO,0x22)	///< コントローラデバイスのレジスタに24ビットデータを書き込む
#define IOCMD_VIO_WRITE_REG32	STDIOCTL(DEV_VIDEOIO,0x23)	///< コントローラデバイスのレジスタに32ビットデータを書き込む

#define IOCMD_VIO_READ_REG8	STDIOCTL(DEV_VIDEOIO,0x30)	///< コントローラデバイスのレジスタより8ビットデータを読み出す
#define IOCMD_VIO_READ_REG16	STDIOCTL(DEV_VIDEOIO,0x31)	///< コントローラデバイスのレジスタより16ビットデータを読み出す
#define IOCMD_VIO_READ_REG24	STDIOCTL(DEV_VIDEOIO,0x32)	///< コントローラデバイスのレジスタより24ビットデータを読み出す
#define IOCMD_VIO_READ_REG32	STDIOCTL(DEV_VIDEOIO,0x33)	///< コントローラデバイスのレジスタより32ビットデータを読み出す

#define IOCMD_VIO_SET_WRITEDATA0	STDIOCTL(DEV_VIDEOIO,0x40)	///< 描画データ0を設定する
#define IOCMD_VIO_SET_WRITEDATA1	STDIOCTL(DEV_VIDEOIO,0x41)	///< 描画データ1を設定する
#define IOCMD_VIO_REPEAT_DATA	STDIOCTL(DEV_VIDEOIO,0x42)		///< 描画データ0で指定ドット数描画する
#define IOCMD_VIO_REPEAT_BITS	STDIOCTL(DEV_VIDEOIO,0x43)		///< 指定のビットデータを描画データ0、1で描画する

#endif // VIO_IOCTL_H
