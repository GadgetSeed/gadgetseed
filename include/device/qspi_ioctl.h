/** @file
    @brief	QSPI FLASH ROMデバイスドライバ ioctl 用マクロ定義

    推奨デバイス名 : "qspi"

    @date	2019.11.22
    @author	Takashi SHUDO
*/

#ifndef QSPI_IOCTL_H
#define QSPI_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_QSPI		"qspi"		///< 標準ストレージデバイス名(QSPI FLASH ROM)

#define QSPISTAT_OK		0x00
#define QSPISTAT_ERROR		0x01
#define QSPISTAT_BUSY		0x02
#define QSPISTAT_NOT_SUPPORTED	0x04
#define QSPISTAT_SUSPENDED	0x08

struct st_qspi_info {
	unsigned int	flash_size;
	unsigned int	erase_sector_size;
	unsigned int	erase_sectors_number;
	unsigned int	prog_page_size;
	unsigned int	prog_pages_number;
};	///< QSPI FLASH ROMデバイス情報

#define IOCMD_QSPI_GET_DEVICE_INFO	STDIOCTL(DEV_QSPI,1)	///< デバイス情報を取得する
#define IOCMD_QSPI_ERASE_BLOCK		STDIOCTL(DEV_QSPI,2)	///< ブロック消去
#define IOCMD_QSPI_ERASE_CHIP		STDIOCTL(DEV_QSPI,3)	///< チップ消去
#define IOCMD_QSPI_GET_STATUS		STDIOCTL(DEV_QSPI,4)	///< 状態(QSPISTAT_*))
#define IOCMD_QSPI_INDIRECT_MODE	STDIOCTL(DEV_QSPI,6)	///< 関節アクセスモード
#define IOCMD_QSPI_MEMORYMAP_MODE	STDIOCTL(DEV_QSPI,7)	///< メモリマップモード

#endif // QSPI_IOCTL_H
