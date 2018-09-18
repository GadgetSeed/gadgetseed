/** @file
    @brief	外部記憶装置管理

    @date	2007.12.16
    @author	Takashi SHUDO
*/

#ifndef DISK_H
#define DISK_H

#include "device.h"
#include "file.h"

#ifndef GSC_FS_VOLUME_NUM
#define GSC_FS_VOLUME_NUM	1	///< $gsc 最大ストレージデバイスボリューム数
#endif

struct st_storage_info {
	struct st_device *device;	///< ストレージデバイスドライバ
	struct st_filesystem *fs;	///< ファイルシステム
	void *private_data;
}; ///< ストレージデバイス

extern void init_storage(void);
extern int mount_storage(int drvno, const char *devname, const char *fsname);
extern int unmount_storage(int drvno);
extern int register_storage_device(const char * const device_name[]);
extern int get_storage_device_name(int drv, char **devname, char **fsname);

#endif // DISK_H
