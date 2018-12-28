/** @file
    @brief	仮想 MMC ドライバ

    @date	2009.10.30
    @author	Takashi SHUDO
*/

/*
  MMCデータの作成方法

  8MBサイズ(count=8)
  $ dd if=/dev/zero of=MMC.DAT bs=1M count=8
  $ /sbin/mkfs.vfat MMC.DAT

  MMCデータのマウント

  $ mkdir mnt
  $ sudo mount -w -o loop,uid=[ユーザー名],iocharset=utf8 -t vfat MMC.DAT mnt
*/

#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define MMCFILE "MMC.DAT"

static int mmc_fd;

#include "device.h"
#include "device/sd_ioctl.h"
#include "tkprintf.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


//#define MMCSIZE	((long)1024*1024*8)	// 8MBytes
#define MMCSIZE	((long)1024*1024*512)	// 512MBytes
#define SECTSIZE 512

static unsigned char mmc_data[SECTSIZE];

static int mmc_register(struct st_device *dev, char *param)
{
	int i;

	mmc_fd = open(MMCFILE, O_RDWR);
	if(mmc_fd < 0) {
		int rt;

		fprintf(stderr, "Cannot open \"%s\".\r\n", MMCFILE);
		fprintf(stderr, "Create \"%s\".\r\n", MMCFILE);
		mmc_fd = open(MMCFILE, O_RDWR | O_CREAT, S_IRWXU);
		if(mmc_fd < 0) {
			fprintf(stderr, "Cannot create \"%s\".\r\n",
				MMCFILE);
		}

		memset(mmc_data, 0, SECTSIZE);

		for(i=0; i<MMCSIZE; i+=SECTSIZE) {
			rt = write(mmc_fd, mmc_data, SECTSIZE);
			if(rt != SECTSIZE) {
				fprintf(stderr, "NG.\r\n");
				return -1;
			}
		}
	}

	return 0;
}

static int mmc_unregister(struct st_device *dev)
{
	close(mmc_fd);

	return 0;
}

static int mmc_open(struct st_device *dev)
{
	return 0;
}

static int mmc_close(struct st_device *dev)
{
	return 0;
}

static int mmc_block_read(struct st_device *dev, void *data, unsigned int sector, unsigned int count)
{
	off64_t srt = 0;
	ssize_t rt = 0;

	DKFPRINTF(0x01, "dev=%p, data=%p, sector=%u, blkcount=%u\n", dev, data, sector, count);

	srt = lseek64(mmc_fd, sector * SECTSIZE, SEEK_SET);
	if(srt < 0) {
		SYSERR_PRINT("Block read(data=%p, sector=%u, count=%u) lseek64() error %ld\n", data, sector, count, srt);
		return -1;
	}

	rt = read(mmc_fd, data, count * SECTSIZE);
	if(rt > 0) {
		rt = count;
		KXDUMP(0x02, data, count * SECTSIZE);
	} else {
		SYSERR_PRINT("Block read(data=%p, sector=%u, count=%u) read() error %ld\n", data, sector, count, rt);
	}

	return (int)rt;
}

static int mmc_block_write(struct st_device *dev, const void *data, unsigned int sector, unsigned int count)
{
	off64_t srt = 0;
	ssize_t rt = 0;

	DKFPRINTF(0x01, "dev=%p, data=%p, sector=%u, blkcount=%u\n", dev, data, sector, count);
	KXDUMP(0x02, (void *)data, count * SECTSIZE);

	srt = lseek64(mmc_fd, sector * SECTSIZE, SEEK_SET);
	if(srt < 0) {
		SYSERR_PRINT("Block read(data=%p, sector=%u, size=%u) lseek64() error %ld\n", data, sector, count, srt);
		return -1;
	}

	rt = write(mmc_fd, data, count * SECTSIZE);
	if(rt > 0) {
		return count;
	} else {
		SYSERR_PRINT("Block write(data=%p, sector=%u, sector=%u) write() error %ld\n", data, sector, count, rt);
		return rt;
	}
}

static int mmc_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	int rt = 0;

	switch(com) {
	case IOCMD_SD_GET_SECTOR_COUNT:
		rt = MMCSIZE / SECTSIZE;
		break;

	case IOCMD_SD_GET_SECTOR_SIZE:
		rt = SECTSIZE;
		break;

	case IOCMD_SD_GET_BLOCK_SIZE:
		rt = SECTSIZE;
		break;

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		return 0;
	}

	return rt;
}

static int mmc_sync(struct st_device *dev)
{
	return 0;
}

const struct st_device vmmc_device = {
	.name		= DEF_DEV_NAME_SD,
	.explan		= "EMU MMC Storage",
	.register_dev	= mmc_register,
	.unregister_dev	= mmc_unregister,
	.open		= mmc_open,
	.close		= mmc_close,
	.block_read	= mmc_block_read,
	.block_write	= mmc_block_write,
	.ioctl		= mmc_ioctl,
	.sync		= mmc_sync,
};
