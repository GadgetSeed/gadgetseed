/** @file
    @brief	外部記憶装置管理

    @date	2007.12.16
    @author	Takashi SHUDO
*/

#ifndef DISK_H
#define DISK_H

#include "device.h"

struct st_storage_info {
	struct st_device *device;	///< ストレージデバイスドライバ
}; ///< ストレージデバイス

extern void init_storage(void);
extern int mount_storage(int drv, const char *devname);
extern int unmount_storage(int drv);
extern int register_storage_device(const char * const device_name[]);
extern int get_storage_device_name(int drv, char **name);

#endif // DISK_H
