/** @file
    @brief	仮想 QSPI FLASH ROM ドライバ

    @date	2019.11.23
    @author	Takashi SHUDO
*/

#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

#define DEFQSPIFILENAME "QSPI.DAT"

static int qspi_fd;

#include "device.h"
#include "device/sd_ioctl.h"
#include "device/qspi_ioctl.h"
#include "tkprintf.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


#if 1
#define FLASHSIZE	(16*1024*1024)
#define ERASESECTSIZE	(1024*8)
#define SECTSIZE	256
#else
#define FLASHSIZE	(64*1024*1024)
#define ERASESECTSIZE	(1024*4)
#define SECTSIZE	256
#endif

#define QSPIFNAMELEN	64
static char qspi_file_name[QSPIFNAMELEN+1] = DEFQSPIFILENAME;
static unsigned char qspi_data[SECTSIZE];

static struct st_qspi_info qspi_info = {
	.flash_size		= FLASHSIZE,
	.erase_sector_size	= ERASESECTSIZE,
	.erase_sectors_number	= FLASHSIZE / ERASESECTSIZE,
	.prog_page_size		= SECTSIZE,
	.prog_pages_number	= FLASHSIZE / SECTSIZE,
};

void set_qspi_filename(char *fname)
{
	strncpy(qspi_file_name, fname, QSPIFNAMELEN);
}

static int qspi_register(struct st_device *dev, char *param)
{
	int i;

	qspi_fd = open(qspi_file_name, O_RDWR);
	if(qspi_fd < 0) {
		int rt;

		fprintf(stderr, "Cannot open \"%s\"\r\n", qspi_file_name);
		fprintf(stderr, "Create \"%s\"\r\n", qspi_file_name);
		qspi_fd = open(qspi_file_name, O_RDWR | O_CREAT, S_IRWXU);
		if(qspi_fd < 0) {
			fprintf(stderr, "Cannot create \"%s\"\r\n",
				qspi_file_name);
		}

		memset(qspi_data, 0xff, SECTSIZE);

		for(i=0; i<FLASHSIZE; i+=SECTSIZE) {
			rt = write(qspi_fd, qspi_data, SECTSIZE);
			if(rt != SECTSIZE) {
				fprintf(stderr, "NG\r\n");
				return -1;
			}
		}
	}

	return 0;
}

static int qspi_unregister(struct st_device *dev)
{
	close(qspi_fd);

	return 0;
}

static int qspi_open(struct st_device *dev)
{
	return 0;
}

static int qspi_close(struct st_device *dev)
{
	return 0;
}

static int qspi_block_read(struct st_device *dev, void *data, unsigned int sector, unsigned int count)
{
	off64_t srt = 0;
	ssize_t rt = 0;

	DKFPRINTF(0x01, "dev=%p, data=%p, sector=%u, blkcount=%u\n", dev, data, sector, count);

	srt = lseek64(qspi_fd, sector * SECTSIZE, SEEK_SET);
	if(srt < 0) {
		SYSERR_PRINT("Block read(data=%p, sector=%u, count=%u) lseek64() error %ld\n", data, sector, count, srt);
		return -1;
	}

	rt = read(qspi_fd, data, count * SECTSIZE);
	if(rt > 0) {
		rt = count;
		KXDUMP(0x02, data, count * SECTSIZE);
	} else {
		SYSERR_PRINT("Block read(data=%p, sector=%u, count=%u) read() error %ld\n", data, sector, count, rt);
	}

	return (int)rt;
}

static int qspi_block_write(struct st_device *dev, const void *data, unsigned int sector, unsigned int count)
{
	off64_t srt = 0;
	ssize_t rt = 0;
	unsigned char *buf;
	int rtn = 0;
	int i;
	unsigned char *sp, *dp;

	DKFPRINTF(0x01, "dev=%p, data=%p, sector=%u, blkcount=%u\n", dev, data, sector, count);
	KXDUMP(0x02, (void *)data, count * SECTSIZE);

	buf = malloc(SECTSIZE * count);
	if(buf == NULL) {
		SYSERR_PRINT("malloc error %p\n", buf);
		rtn = -1;
		goto end;
	}

	srt = lseek64(qspi_fd, sector * SECTSIZE, SEEK_SET);
	if(srt < 0) {
		SYSERR_PRINT("Block read(data=%p, sector=%u, size=%u) lseek64() error %ld\n", data, sector, count, srt);
		rtn = -1;
		goto error;
	}

	rt = read(qspi_fd, buf, count * SECTSIZE);
	if(rt < 0) {
		SYSERR_PRINT("Block read(data=%p, sector=%u, count=%u) read() error %ld\n", data, sector, count, rt);
		rtn = -1;
		goto end;
	}

	sp = (unsigned char *)data;
	dp = buf;
	for(i=0; i<(count * SECTSIZE); i++) {
		*dp = (*sp & *dp);
		sp ++;
		dp ++;
	}

	srt = lseek64(qspi_fd, sector * SECTSIZE, SEEK_SET);
	if(srt < 0) {
		SYSERR_PRINT("Block write(data=%p, sector=%u, size=%u) lseek64() error %ld\n", data, sector, count, srt);
		rtn = -1;
		goto error;
	}

	rt = write(qspi_fd, buf, count * SECTSIZE);
	if(rt > 0) {
		rtn = count;
	} else {
		SYSERR_PRINT("Block write(data=%p, sector=%u, sector=%u) write() error %ld\n", data, sector, count, rt);
		rtn = rt;
	}

error:
	free(buf);
end:

	return rtn;
}

static int qspi_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	int rt = 0;

	DKFPRINTF(0x01, "dev=%p, com=%u, arg=%u, param=%p\n", dev, com, arg, param);

	switch(com) {
	case IOCMD_SD_GET_SECTOR_COUNT:
		rt = FLASHSIZE / SECTSIZE;
		break;

	case IOCMD_SD_GET_SECTOR_SIZE:
		rt = SECTSIZE;
		break;

	case IOCMD_SD_GET_BLOCK_SIZE:
		rt = SECTSIZE;
		break;

	case IOCMD_QSPI_GET_DEVICE_INFO:
		{
			struct st_qspi_info *qspi = param;

			qspi->flash_size = qspi_info.flash_size;
			qspi->erase_sector_size = qspi_info.erase_sector_size;
			qspi->erase_sectors_number = qspi_info.erase_sectors_number;
			qspi->prog_page_size = qspi_info.prog_page_size;
			qspi->prog_pages_number = qspi_info.prog_pages_number;

			rt = 0;
		}
		break;

	case IOCMD_QSPI_ERASE_BLOCK:
		{
			static unsigned char edata[ERASESECTSIZE];
			off64_t srt = 0;
			ssize_t rt = 0;

			DKFPRINTF(0x01, "IOCMD_QSPI_ERASE_BLOCK block=%u sector=%u\n", arg, arg * ERASESECTSIZE/SECTSIZE);

			memset(edata, 0xff, ERASESECTSIZE);

			srt = lseek64(qspi_fd, arg * ERASESECTSIZE, SEEK_SET);
			if(srt < 0) {
				SYSERR_PRINT("Block write(data=%p, sector=%u) lseek64() error %ld\n", edata, arg, srt);
				return -1;
			}

			rt = write(qspi_fd, edata, ERASESECTSIZE);
			if(rt < 0) {
				SYSERR_PRINT("Block write(data=%p, sector=%u) write() error %ld\n", edata, arg, rt);
				return rt;
			}
			
			rt = QSPISTAT_OK;
		}
		break;

	case IOCMD_QSPI_ERASE_CHIP:
		{
			rt = QSPISTAT_OK;
		}
		break;

	case IOCMD_QSPI_GET_STATUS:
		{
			rt = QSPISTAT_OK;
		}
		break;

	case IOCMD_QSPI_INDIRECT_MODE:
		{
			rt = QSPISTAT_OK;
		}
		break;

	case IOCMD_QSPI_MEMORYMAP_MODE:
		{
			rt = QSPISTAT_OK;
		}
		break;

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		return -1;
	}

	return rt;
}

static int qspi_sync(struct st_device *dev)
{
	return 0;
}

const struct st_device vqspi_device = {
	.name		= DEF_DEV_NAME_QSPI,
	.explan		= "EMU QSPI FLASH ROM",
	.register_dev	= qspi_register,
	.unregister_dev	= qspi_unregister,
	.open		= qspi_open,
	.close		= qspi_close,
	.block_read	= qspi_block_read,
	.block_write	= qspi_block_write,
	.ioctl		= qspi_ioctl,
	.sync		= qspi_sync,
};
