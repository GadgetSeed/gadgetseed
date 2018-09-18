/** @file
    @brief	FatFs ファイルシステムAPI

    @date	2018.09.09
    @author	Takashi SHUDO
*/

#ifndef FATFS_H
#define FATFS_H

#include "ff.h"
#include "device.h"

struct st_fatfs {
	FATFS fatfs;	///< FATワーク
	struct st_device *device;
};

#endif // FATFS_H
