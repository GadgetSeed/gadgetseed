/** @file
    @brief	ストレージデバイス

    ストレージデバイスのファイルシステムへのマウント、アンマウント等を行う。

    @date	2007.12.16
    @author	Takashi SHUDO

    @page storage_device ストレージデバイス

    GadgetSeedはストレージデバイスに対してファイルシステムへのマウント及びアンマウント操作を行うAPIがあります。


    ---
    @section storage_device_api ストレージデバイスAPI

    include ファイル : storage.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | mount_storage()		| @copybrief mount_storage		|
    | unmount_storage()		| @copybrief unmount_storage		|
    | register_storage_device()	| @copybrief register_storage_device	|
    | get_storage_device_name()	| @copybrief get_storage_device_name	|

    システムの初期化時に mount_storage() または register_storage_device() を実行し、ストレージデバイスをマウントして下さい。

    マウントされたデバイスは、ファイル操作関数によりアクセスすることができます。
*/

#include "storage.h"
#include "fs.h"

#include "device.h"
#include "diskio.h"
#include "tprintf.h"
#include "tkprintf.h"

struct st_storage_info storage[GSC_FS_VOLUME_NUM];	///< ストレージデバイステーブル

/**
   @brief	外部記憶装置管理初期化
*/
void init_storage(void)
{
	int i ;

	for(i=0; i<GSC_FS_VOLUME_NUM; i++) {
		storage[i].device = 0;
		storage[i].fs = 0;
	}
}

#define DEVNAME_LEN	2	// "X:"

/**
   @brief	ストレージデバイスをマウントする

   @param[in]	drvno	デバイス番号
   @param[in]	devname	デバイス名文字列ポインタ

   @return	エラーコード
*/
int mount_storage(int drvno, const char *devname, const char *fsname)
{
	int rtn;
	struct st_filesystem *fs = 0;
	struct st_device *dev = 0;

	if(drvno >= GSC_FS_VOLUME_NUM) {
		SYSERR_PRINT("Storage number %d too large.\n", drvno);
		return -1;
	}

	if(storage[drvno].device != 0) {
		SYSERR_PRINT("Storage %d: already mounted.\n", drvno);
		return -1;
	}

	fs = search_filesystem(fsname);
	if(fs == 0) {
		SYSERR_PRINT("Invalid file system \"%s\".\n", fsname);
		return -1;
	}
	storage[drvno].fs = fs;

	if(devname != 0) {
		dev = open_device((char *)devname);
		if(dev != 0) {
			storage[drvno].device = dev;
		} else {
			SYSERR_PRINT("Cannot open Device \"%s\" (Storage %d:).\n", devname, drvno);
			return -1;
		}
	}

	if(fs->mount != 0) {
		rtn = fs->mount(drvno, dev);
		if(rtn != 0) {
			eprintf("Device \"%s\" (Storage %d:) mount error.\n", devname, drvno);
			storage[drvno].device = 0;
			storage[drvno].fs = 0;
			return -1;
		} else {
			return 0;
		}
	}

	return 0;
}

/**
   @brief	ストレージデバイスをアンマウントする

   @param[in]	drvno	デバイス番号

   @return	エラーコード
*/
int unmount_storage(int drvno)
{
	int rtn;

	if(drvno >= GSC_FS_VOLUME_NUM) {
		SYSERR_PRINT("Storage number %d too large.\n", drvno);
		return -1;
	}

	if(storage[drvno].fs == 0) {
		SYSERR_PRINT("Storage %d: not mounted.\n", drvno);
		return -1;
	}

	if(storage[drvno].device != 0) {
		sync_device(storage[drvno].device);
	}

	rtn = storage[drvno].fs->unmount(drvno, storage[drvno].device);
	if(rtn != 0) {
		SYSERR_PRINT("Device (Storage %d:) unmount error.\n", drvno);
		return -1;
	} else {
		if(storage[drvno].device != 0) {
			close_device(storage[drvno].device);
		}

		storage[drvno].device = 0;
		storage[drvno].fs = 0;

		return 0;
	}
}

/**
   @brief	ストレージデバイスをリストでマウントする

   @param[in]	device_name	デバイス名リスト

   @return	0:成功, !=0:失敗
*/
int register_storage_device(const char * const device_name[])
{
	int i = 0;
	char *drvname = (char *)device_name[i];

	i = 0;
	while(drvname) {
		if(i < GSC_FS_VOLUME_NUM) {
			if(mount_storage(i, drvname, FSNAME_VFAT) == 0) {
				tprintf("Storage %d: \"%s\"\n", i, drvname);
			} else {
				tprintf("Storage %d: \"%s\" mount failed.\n",
					i, drvname);
			}
		} else {
			SYSERR_PRINT("Storage number %d too large.\n", i);
			return -1;
			break;
		}
		i++;
		drvname = (char *)device_name[i];
	}

	return 0;
}

/** 
    @brief	マウントされているデバイス名を取得する

    @param num		デバイス番号
    @param devneme	デバイス名
    @param fsneme	ファイルシステム名

    @return	0:成功, !=0:マウントされていないデバイス番号
*/
int get_storage_device_name(int drv, char **devname, char **fsname)
{
	if(drv >= GSC_FS_VOLUME_NUM) {
		return -1;
	}

	if(storage[drv].device != 0) {
		*devname = storage[drv].device->name;
		*fsname = storage[drv].fs->name;
		return 0;
	} else {
		return -1;
	}
}
