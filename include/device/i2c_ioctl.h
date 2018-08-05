/** @file
    @brief	I2Cドライバ ioctl 用マクロ定義

    推奨デバイス名 : "i2c"

    @date	2015.09.27
    @author	Takashi SHUDO
*/

#ifndef I2C_IOCTL_H
#define I2C_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_I2C	"i2c"	///< 標準I2Cコントローラデバイス名

struct st_i2c_config {
	int		speed;		///< ビットレート(Kbps)

#define I2C_ADDR_MODE_7BIT	0	///< I2C 7ビットアドレスモード
#define I2C_ADDR_MODE_11BIT	1	///< I2C 11ビットアドレスモード
	int		address_mode;	///< 未使用(予約) @todo I2C 11ビットアドレスデバイス対応

	unsigned int	slave_addr;	///< I2Cスレーブデバイスアドレス

#define I2C_MEM_ADDR_SIZE_8BIT	0	///< I2Cスレーブデバイスメモリアドレスサイズは8ビット
#define I2C_MEM_ADDR_SIZE_16BIT	1	///< I2Cスレーブデバイスメモリアドレスサイズは16ビット
	int		mem_addr_size;	///< I2Cスレーブデバイスメモリアドレスサイズ
};	///< I2Cコンフィグデータ


#define IOCMD_I2C_SPEED		STDIOCTL(DEV_I2C,0x00)	///< com : 通信速度を設定する, arg : 通信速度(bps)
#define IOCMD_I2C_SLAVE_ADDR7	STDIOCTL(DEV_I2C,0x01)	///< Set Save 7bit Address

#define IOCMD_I2C_ADDRMODE	STDIOCTL(DEV_I2C,0x05)	///< 0:7bit or 1:11bit address
#define IOCMD_I2C_MEMADDRSIZE	STDIOCTL(DEV_I2C,0x06)	///< Memory Address Size 8bit or 16bit etc
#define IOCMD_I2C_SETCONFIG	STDIOCTL(DEV_I2C,0x07)	///< Device Access configration

#endif // I2C_IOCTL_H
