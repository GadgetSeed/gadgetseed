/** @file
    @brief	FatFs用下位レイヤI/FディスクIO

    @date	2013.03.03
    @attach	Takashi SHUDO
*/

/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "sysconfig.h"
#include "ff.h"
#include "ffconf.h"
#include "diskio.h"
#include "device.h"
#include "device/sd_ioctl.h"
#include "tkprintf.h"
#include "fatfs.h"

//#define DEBUG
#ifdef DEBUG
#define DBG	tprintf
#else
#define DBG(x, ...)
#endif

/*-----------------------------------------------------------------------*/
/* Correspondence between drive number and physical drive                */
/* Note that Tiny-FatFs supports only single drive and always            */
/* accesses drive number 0.                                              */

extern struct st_fatfs fatfs[FF_VOLUMES];

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
		BYTE drv	/* Physical drive nmuber (0..) */
		)
{
	DBG("disk_initialize(%d)\n", (int)drv);
	if(drv >= FF_VOLUMES) {
		SYSERR_PRINT("Over max disk number %d.\n", (int)drv);
		return STA_NODISK;
	}

	if(fatfs[drv].device == 0) {
		return STA_NOINIT;
	}

	//(void)open_device((char *)fatfs[drv].device->name);

	return 0;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
)
{
	DBG("disk_status(%d) =", (int)drv);
	if(drv >= FF_VOLUMES) {
		DBG("STA_NODISK\n");
		return STA_NODISK;
	}

	if(fatfs[drv].device == 0) {
		DBG("STA_NOINIT\n");
		return STA_NOINIT;
	}

	DBG("OK\n");
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector number (LBA) */
	UINT count		/* Sector count (1..255) */
)
{
	long rt;
	DBG("disk_read(%d) sect=%08x count=%d\n", (int)pdrv, sector, (int)count);

	if(pdrv >= FF_VOLUMES) {
		return RES_PARERR;
	}

	rt = block_read_device(fatfs[pdrv].device, buff, sector, count);
	if(rt != count) {
		return RES_ERROR;
	}

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector number (LBA) */
	UINT count			/* Sector count (1..255) */
)
{
	long rt;
	DBG("disk_write(%d) sect=%08x count=%d\n", (int)pdrv, sector, (int)count);

	if(pdrv >= FF_VOLUMES) {
		return RES_PARERR;
	}

	rt = block_write_device(fatfs[pdrv].device, (unsigned char *)buff, sector, count);
	if(rt != count) {
		DBG("disk_write(%d) write error.\n", pdrv);
		return RES_ERROR;
	}

	DBG("disk_write(%d) result=%d\n", pdrv, rt);
	return RES_OK;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	int rt;
	DBG("disk_ioctl(%d)\n", (int)drv);

	if(drv >= FF_VOLUMES) {
		return RES_PARERR;
	}

	switch(ctrl) {
	case CTRL_SYNC:
		DBG("sync(%d)\n", drv);
		if(sync_device(fatfs[drv].device)) {
			return RES_ERROR;
		}
		break;

	case GET_SECTOR_COUNT:
		rt = ioctl_device(fatfs[drv].device, IOCMD_SD_GET_SECTOR_COUNT, 0, 0);
		if(rt == 0) {
			return RES_ERROR;
		}
		*(DWORD *)buff = (DWORD)rt;
		DBG("sector = %08x(%d)\n", rt, rt);
		break;

	case GET_BLOCK_SIZE:
		rt = ioctl_device(fatfs[drv].device, IOCMD_SD_GET_BLOCK_SIZE, 0, 0);
		if(rt == 0) {
			return RES_ERROR;
		}
		*(DWORD *)buff = (DWORD)rt;
		DBG("block = %08x(%d)\n", rt, rt);
		break;
	}

	return RES_OK;
}


/*
  同期処理
*/
#include "task/mutex.h"
#include "task/syscall.h"

static struct st_mutex fs_mutex[FF_VOLUMES];
static const char *mutex_name[FF_VOLUMES] = {
	"disk0",
#if FF_VOLUMES > 1
	"disk1"
#endif
};

/* Create a sync object */
int ff_cre_syncobj(BYTE volume, FF_SYNC_t *mutex)
{
	mutex_register(&fs_mutex[volume], mutex_name[volume]);

	*mutex = &fs_mutex[volume];

	return 1;
}

/* Lock sync object */
int ff_req_grant(FF_SYNC_t mutex)
{
	if(mutex_lock(mutex, 1000) == 0) {
		SYSERR_PRINT("ff lock timeout\n");
	}

	return 1;
}

/* Unlock sync object */
void ff_rel_grant(FF_SYNC_t mutex)
{
	mutex_unlock(mutex);
}

/* Delete a sync object */
int ff_del_syncobj(FF_SYNC_t mutex)
{
	mutex_unregister(mutex);

	return 1;
}
