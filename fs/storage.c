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
#include "ff.h"

#include "device.h"
#include "diskio.h"
#include "tprintf.h"
#include "tkprintf.h"

static FATFS fatfs[FF_VOLUMES];	///< FATワーク
struct st_storage_info storage[FF_VOLUMES];	///< ストレージデバイステーブル

/**
   @brief	外部記憶装置管理初期化
*/
void init_storage(void)
{
	int i ;

	for(i=0; i<FF_VOLUMES; i++) {
		storage[i].device = 0;
	}
}

#define DEVNAME_LEN	2	// "X:"

/**
   @brief	ストレージデバイスをマウントする

   @param[in]	drv	デバイス番号
   @param[in]	devname	デバイス名文字列ポインタ

   @return	エラーコード
*/
int mount_storage(int drv, const char *devname)
{
	FRESULT result;
	char dev_name[DEVNAME_LEN+1];

	if(drv >= FF_VOLUMES) {
		SYSERR_PRINT("Storage number %d too large.\n", drv);
		return -1;
	}

	if(storage[drv].device != 0) {
		SYSERR_PRINT("Storage %d: already mounted.\n", drv);
		return -1;
	}

	storage[drv].device = open_device((char *)devname);

	if(storage[drv].device == 0) {
		//SYSERR_PRINT("Cannot open Device \"%s\" (Storage %d:).\n", devname, (int)drv);
		return -1;
	}

	tsnprintf(dev_name, DEVNAME_LEN, "%1d:", drv);
	result = f_mount(&fatfs[drv], dev_name, 1);

	if(result != FR_OK) {
		SYSERR_PRINT("Device \"%s\" (Storage %d:) mount error.\n",
			     devname, drv);
		return -1;
	} else {
		return 0;
	}
}

/**
   @brief	ストレージデバイスをアンマウントする

   @param[in]	drv	デバイス番号

   @return	エラーコード
*/
int unmount_storage(int drv)
{
	FRESULT result;
	char dev_name[DEVNAME_LEN+1];

	if(drv >= FF_VOLUMES) {
		SYSERR_PRINT("Storage number %d too large.\n", drv);
		return -1;
	}

	if(storage[drv].device == 0) {
		SYSERR_PRINT("Storage %d: not mounted.\n", drv);
		return -1;
	}

	sync_device(storage[drv].device);

	close_device(storage[drv].device);

	storage[drv].device = 0;

	tsnprintf(dev_name, DEVNAME_LEN, "%d:", drv);

	result = f_mount(&fatfs[drv], dev_name, 0);

	if(result != FR_OK) {
		SYSERR_PRINT("Device \"%s\" (Storage %d:) unmount error.\n",
			     dev_name, drv);
		return -1;
	} else {
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
		if(i < FF_VOLUMES) {
			if(mount_storage(i, drvname) == 0) {
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

    @param num	デバイス番号
    @param neme	デバイス名

    @return	0:成功, !=0:マウントされていないデバイス番号
*/
int get_storage_device_name(int drv, char **name)
{
	if(drv >= FF_VOLUMES) {
		return -1;
	}

	if(storage[drv].device != 0) {
		*name = storage[drv].device->name;
		return 0;
	} else {
		return -1;
	}
}
