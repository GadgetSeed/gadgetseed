/** @file
    @brief	GadgetSeed FLASH File System

    @date	2019.11.23
    @author	Takashi SHUDO
*/

#define GSLOG_PREFIX	"GSFFS: "

#include "sysconfig.h"
#include "device.h"
#include "str.h"
#include "fifo.h"
#include "crc.h"
#include "tkprintf.h"
#include "fs.h"
#include "log.h"
#include "task/mutex.h"
#include "task/event.h"
#include "task/syscall.h"
#include "device/qspi_ioctl.h"


//#define DEBUGTBITS 0x01
#include "dtprintf.h"

#define GSFFSLOGLVL	9
#define GSFFSERRLVL	0

#ifndef GSC_GSFFS_MAX_FILE_NUM
#define GSC_GSFFS_MAX_FILE_NUM	2	///< $gsc GSFFSでオープンできる最大ファイル数
#endif

#ifndef GSC_GSFFS_MAX_DIR_NUM
#define GSC_GSFFS_MAX_DIR_NUM	1	///< $gsc GSFFSでオープンできる最大ディレクトリ数
#endif

#ifndef GSC_GSFFS_USE_ERASESECTCOUNT
#define GSC_GSFFS_USE_ERASESECTCOUNT	4	///< $gsc GSFFSで使用する消去セクタ数(最低2以上)
#endif

#define FLASH_SECTSIZE		256
#define MAXFNAMELEN		(8+1+3)		// 最大ファイル名文字数
#define MAXFILESIZE		(FLASH_SECTSIZE - 1 - 1 - 2 - 4 - MAXFNAMELEN - 2)	// 最大ファイルサイズ

#define SECTSTAT_UNUSED		0xff
#define SECTSTAT_CREATE		0xfd
#define SECTSTAT_SAVED		0xed
#define SECTSTAT_DELETED	0x00

#define DATAPOS_SECTSTAT	0
#define DATAPOS_ATTRIBUTE	1
#define DATAPOS_SIZE		2
#define DATAPOS_FILENAME	4
#define DATAPOS_DATETIME	16
#define DATAPOS_DATA		20
#define DATAPOS_CRC		254

static struct st_device *qspi_dev;
static struct st_qspi_info qspi_info;
static unsigned int fs_sect_start = 0;
static unsigned int fs_sect_end = 0;
static unsigned int fs_erase_block_num = 0;

struct st_gsffs {
	unsigned char flg_used;
	unsigned char flg_new;
	unsigned char flg_dirty;
	int flags;
	unsigned int sector;
	unsigned char attribute;
	struct st_systime datetime;
	uchar filename[MAXFNAMELEN+1];
	unsigned short filesize;
	unsigned char buf[FLASH_SECTSIZE];
	unsigned short crc;
	int seekptr;
};

static struct st_gsffs gsffs[GSC_GSFFS_MAX_FILE_NUM];

int mount_gsffs(int drvno, struct st_device *dev)
{
	int rt = 0;
	int i;

	DTFPRINTF(0x01, "drvno = %d, dev = %p\n", drvno, dev);

	qspi_dev = dev;

	rt = ioctl_device(dev, IOCMD_QSPI_GET_DEVICE_INFO, 0, (void *)&qspi_info);
	if(rt == 0) {
		gslog(GSFFSLOGLVL, "Flash size           : %d\n", qspi_info.flash_size);
		gslog(GSFFSLOGLVL, "Erase sector size    : %d\n", qspi_info.erase_sector_size);
		gslog(GSFFSLOGLVL, "Erase sectors number : %d\n", qspi_info.erase_sectors_number);
		gslog(GSFFSLOGLVL, "Program page size    : %d\n", qspi_info.prog_page_size);
		gslog(GSFFSLOGLVL, "Program pages number : %d\n", qspi_info.prog_pages_number);

		fs_sect_start = qspi_info.prog_pages_number - 
				(qspi_info.erase_sector_size / qspi_info.prog_page_size) * GSC_GSFFS_USE_ERASESECTCOUNT;
		fs_sect_end   = qspi_info.prog_pages_number;
		fs_erase_block_num  = qspi_info.erase_sector_size / qspi_info.prog_page_size;

		gslog(GSFFSLOGLVL, "GSFFS use erase sector number %d\n", GSC_GSFFS_USE_ERASESECTCOUNT);
		gslog(GSFFSLOGLVL, "GSFFS use prog  sector No. %d - %d\n", fs_sect_start, fs_sect_end-1);
		gslog(GSFFSLOGLVL, "GSFFS elase block per sector %d\n", fs_erase_block_num);
	} else {
		gslog(GSFFSLOGLVL, "Device \"%s\" ioctl IOCMD_QSPI_GET_DEVICE_INFO error(%d)\n", DEF_DEV_NAME_QSPI, rt);
	}

	for(i=0;i<GSC_GSFFS_MAX_FILE_NUM; i++) {
		gsffs[i].flg_used = 0;
		gsffs[i].flg_new = 0;
		gsffs[i].flg_dirty = 0;
	}

	return rt;
}

int unmount_gsffs(int drvno, struct st_device *dev)
{
	int rt = 0;

	DTFPRINTF(0x01, "drvno = %d, dev = %p\n", drvno, dev);

	qspi_dev = 0;

	return rt;
}

static void get_filename(uchar *file_name, const uchar *path)
{
	int len = strleng(path);

	if(len >= 2) {
		if(path[1] == ':') {
			if(path[2] == '/') {
				strncopy(file_name, &path[3], MAXFNAMELEN);
			} else {
				strncopy(file_name, &path[2], MAXFNAMELEN);
			}
		} else {
			strncopy(file_name, (uchar *)path, MAXFNAMELEN);
		}
	} else {
		strncopy(file_name, (uchar *)path, MAXFNAMELEN);
	}
}

#ifdef GSC_DEV_QSPI_MEMORYMAP
#define QSPI_LOCK_TIMEOUT	500

void lock_qspi(void)
{
	int rt = 0;

	rt = lock_device(qspi_dev, QSPI_LOCK_TIMEOUT);
	if(rt < 0) {
		gslog(0, "QSPI lock timeout(%d)\n", rt);
	}
}

void unlock_qspi(void)
{
	int rt = 0;

	rt = unlock_device(qspi_dev);
	if(rt < 0) {
		GSLOG(GSFFSERRLVL, "QSPI unlock abort\n");
	}
}

static void memoryunmap_qspi(void)
{
	int rt = 0;

	lock_qspi();

	rt = ioctl_device(qspi_dev, IOCMD_QSPI_INDIRECT_MODE, 0, 0);
	if(rt != 0) {
		GSLOG(GSFFSERRLVL, "Device \"%s\" ioctl IOCMD_QSPI_INDIRECT_MODE error(%d)\n", DEF_DEV_NAME_QSPI, rt);
	}
}

static void memorymap_qspi(void)
{
	int rt = 0;

	rt = ioctl_device(qspi_dev, IOCMD_QSPI_MEMORYMAP_MODE, 0, 0);
	if(rt != 0) {
		GSLOG(GSFFSERRLVL, "Device \"%s\" ioctl IOCMD_QSPI_MEMORYMAP_MODE error(%d)\n", DEF_DEV_NAME_QSPI, rt);
	}

	unlock_qspi();
}
#endif

static int read_sector(void *buf, unsigned int sector)
{
	int rt = 0;

	DTFPRINTF(0x01, "sector = %u\n", sector);

#ifdef GSC_DEV_QSPI_MEMORYMAP
	memoryunmap_qspi();
#endif
	rt = block_read_device(qspi_dev, buf, sector, 1);
#ifdef GSC_DEV_QSPI_MEMORYMAP
	memorymap_qspi();
#endif

	if(rt <= 0) {
		GSLOG(GSFFSERRLVL, "Device sector read error %d\n", sector);
		return 0;
	}

	return rt;
}

static int write_sector(const void *buf, unsigned int sector)
{
	int rt = 0;

	DTFPRINTF(0x01, "sector = %u\n", sector);

#ifdef GSC_DEV_QSPI_MEMORYMAP
	memoryunmap_qspi();
#endif
	rt = block_write_device(qspi_dev, buf, sector, 1);
#ifdef GSC_DEV_QSPI_MEMORYMAP
	memorymap_qspi();
#endif

	if(rt <= 0) {
		GSLOG(GSFFSERRLVL, "Device sector write error %d\n", sector);
		return 0;
	}

	return rt;
}

static int erase_block(unsigned int block)
{
	int rt = 0;

#ifdef GSC_DEV_QSPI_MEMORYMAP
	memoryunmap_qspi();
#endif
	rt = ioctl_device(qspi_dev, IOCMD_QSPI_ERASE_BLOCK, block, 0);
#ifdef GSC_DEV_QSPI_MEMORYMAP
	memorymap_qspi();
#endif

	if(rt != 0) {
		GSLOG(GSFFSERRLVL, "Device block %d erase error\n", block);
		return 0;
	}

	return rt;
}

static int search_file_sector(uchar *filename, unsigned char *buf)
{
	int i;
	int rt = 0;

	for(i=fs_sect_start; i<fs_sect_end; i++) {
		rt = read_sector(buf, i);
		if(rt == 0) {
			return -1;
		}
		if(buf[DATAPOS_SECTSTAT] == SECTSTAT_SAVED) {
			if(strncomp(filename, &buf[DATAPOS_FILENAME], MAXFNAMELEN) == 0) {
				GSLOG(GSFFSLOGLVL, "Found file \"%s\" sector %d\n", (char *)filename, i);
				return i;
			}
		} else if(buf[DATAPOS_SECTSTAT] == SECTSTAT_CREATE) {
			if(strncomp(filename, &buf[DATAPOS_FILENAME], MAXFNAMELEN) == 0) {
				GSLOG(GSFFSLOGLVL, "File \"%s\" (sector %d) is other process used\n", (char *)filename, i);
				return -2;
			}
		}

	}

	GSLOG(GSFFSLOGLVL, "File not found \"%s\"\n", (char *)filename);

	return -1;
}

static int search_blank_sector(void)
{
	unsigned char buf[FLASH_SECTSIZE];
	int i;
	int rt = 0;
	int del_cnt = 0;

	for(i=fs_sect_start; i<fs_sect_end; i++) {
		rt = read_sector(buf, i);
		if(rt == 0) {
			return -1;
		}
		if(buf[DATAPOS_SECTSTAT] == SECTSTAT_UNUSED) {
			GSLOG(GSFFSLOGLVL, "Found blank sector %d\n", i);
			return i;
		} else if(buf[DATAPOS_SECTSTAT] == SECTSTAT_DELETED) {
			del_cnt ++;
		}
		//tkprintf("i=%d,del=%d\n", i, del_cnt);

		if(((i+1) % fs_erase_block_num) == 0) {
			if(i != 0) {
				if(del_cnt == fs_erase_block_num) {
					int block = i/fs_erase_block_num;
					int new_sect;
					GSLOG(GSFFSLOGLVL, "Sector %d - %d all deleted\n", i+1-fs_erase_block_num, i);
					GSLOG(GSFFSLOGLVL, "Erase block %d\n", block);
					erase_block(block);
					new_sect = block * fs_erase_block_num;
					GSLOG(GSFFSLOGLVL, "New blank sector %d\n", new_sect);
					return new_sect;
				}
			}
			del_cnt = 0;
		}
	}

	GSLOG(GSFFSLOGLVL, "No blank sector\n");

	return -1;
}

static struct st_gsffs *search_blank_fd(void)
{
	int i;

	for(i=0; i<GSC_GSFFS_MAX_FILE_NUM; i++) {
		if(gsffs[i].flg_used == 0) {
			GSLOG(GSFFSLOGLVL, "Use file discriptor %d\n", i);
			return &gsffs[i];
		}
	}

	return 0;
}

/**
   @brief	ファイルを開く

   @param	path	ファイル名
   @param	flags	属性フラグ

   @return	ファイルディスクリプタ
*/
void * open_gsffs(const uchar *path, int flags)
{
	int sector;
	struct st_gsffs *fd = 0;

	DTFPRINTF(0x11, "path = %s, flags = %08X\n", path, flags);

	fd = search_blank_fd();
	if(fd == 0) {
		GSLOG(GSFFSLOGLVL, "No file discriptor\n");
		return 0;
	}

	memoryset(fd->filename, 0x00, MAXFNAMELEN+1);
	memoryset(fd->buf, 0xff, FLASH_SECTSIZE);
	get_filename(fd->filename, path);

	GSLOG(GSFFSLOGLVL, "filename = %s\n", fd->filename);

	sector = search_file_sector(fd->filename, fd->buf);

	if(sector < 0) {
		// ファイルが見つからない
		if((flags & (FA_CREATE_NEW | FA_CREATE_ALWAYS)) == 0) {
			GSLOG(GSFFSLOGLVL, "File not found\n");
			return 0;
		} else {
			// ファイル新規作成
			int rt = 0;
			GSLOG(GSFFSLOGLVL, "New file create\n");
			sector = search_blank_sector();
			if(sector < 0) {
				// セクタの空きがない
				return 0;
			}

			fd->buf[DATAPOS_SECTSTAT] = SECTSTAT_CREATE;
			fd->flg_new = 1;
			fd->attribute = 0;	// 未使用 [TODO]
			fd->filesize = 0;

			memorycopy(&(fd->buf[DATAPOS_FILENAME]), fd->filename, MAXFNAMELEN);

			XDUMP(0x02, fd->buf, FLASH_SECTSIZE);
			rt = write_sector(fd->buf, sector);
			if(rt <= 0) {
				return 0;
			}
		}
		if(sector < 0) {
			return 0;
		}
	} else {
		// ファイルが見つかった
		unsigned short crc = 0;

		fd->flg_new = 0;
		fd->attribute = fd->buf[DATAPOS_ATTRIBUTE];

		fd->datetime.sec = ((fd->buf[DATAPOS_DATA+3]<<24)
				    + (fd->buf[DATAPOS_DATA+2]<<16)
				    + (fd->buf[DATAPOS_DATA+1]<<8)
				    + (fd->buf[DATAPOS_DATA+0]));
		fd->datetime.usec = 0;

		fd->filesize = ((fd->buf[DATAPOS_SIZE]<<8) + fd->buf[DATAPOS_SIZE+1]);

		fd->crc = ((fd->buf[DATAPOS_CRC]<<8) + fd->buf[DATAPOS_CRC+1]);
		crc = crc16(0, fd->buf, FLASH_SECTSIZE-2);
		if(crc != fd->crc) {
			GSLOG(GSFFSERRLVL, "File \"%s\" CRC error\n", fd->filename);
		}

		if((flags & (FA_CREATE_NEW | FA_CREATE_ALWAYS)) != 0) {
			fd->filesize = 0;
			memoryset(&fd->buf[DATAPOS_DATA], 0xff, MAXFILESIZE);
		}

		GSLOG(GSFFSLOGLVL, "Open file filesize %d\n", fd->filesize);
	}

	fd->flags = flags;
	fd->sector = sector;
	fd->seekptr = 0;

	DTFPRINTF(0x01, "filesize = %d\n", fd->filesize);

	return (void *)fd;
}

/**
   @brief	ファイルを閉じる

   @param[in]	fd	ファイルディスクリプタ

   @return	0:成功
*/
int close_gsffs(void *fd)
{
	struct st_gsffs *fdp = fd;
	int rt = 0;
	unsigned short crc;

	DTFPRINTF(0x11, "fd = %p\n", fd);

	if(fdp->flg_dirty == 0) {
		// 書き換えがなかった
		return 0;
	} else {
		// 書き換えられている
		if(fdp->flg_new == 0) {
			// 更新されたファイルなので現在のセクタは使用済みにする
			GSLOG(GSFFSLOGLVL, "Delete sector %d\n", fdp->sector);
			fdp->buf[DATAPOS_SECTSTAT] = SECTSTAT_DELETED;
			rt = write_sector(fdp->buf, fdp->sector);
			if(rt <= 0) {
				return 0;
			}

			fdp->buf[DATAPOS_SECTSTAT] = SECTSTAT_SAVED;
			// 新しいセクタを探す
			fdp->sector = search_blank_sector();
			if(fdp->sector < 0) {
				return 0;
			}
		}
	}

	fdp->buf[DATAPOS_SECTSTAT] = SECTSTAT_SAVED;
	fdp->buf[DATAPOS_ATTRIBUTE] = fdp->attribute;

	fdp->buf[DATAPOS_DATETIME+0] = (((fdp->datetime.sec)>>24) & 0xff);
	fdp->buf[DATAPOS_DATETIME+1] = (((fdp->datetime.sec)>>16) & 0xff);
	fdp->buf[DATAPOS_DATETIME+2] = (((fdp->datetime.sec)>> 8) & 0xff);
	fdp->buf[DATAPOS_DATETIME+3] = (((fdp->datetime.sec)>> 0) & 0xff);

	fdp->buf[DATAPOS_SIZE] = (fdp->filesize)>>8;
	fdp->buf[DATAPOS_SIZE+1] = (fdp->filesize) & 0xff;

	crc = crc16(0, fdp->buf, FLASH_SECTSIZE-2);
	fdp->buf[DATAPOS_CRC] = crc>>8;
	fdp->buf[DATAPOS_CRC+1] = crc & 0xff;

	XDUMP(0x04, fdp->buf, FLASH_SECTSIZE);

	rt = write_sector(fdp->buf, fdp->sector);
	if(rt <= 0) {
		return -1;
	}

	GSLOG(GSFFSLOGLVL, "Write filesize %d\n", fdp->filesize);

	fdp->flg_used = 0;

	return 0;
}

/**
   @brief	ファイルからデータを読み出す

   @param[in]	int	ファイルディスクリプタ
   @param[out]	buf	読み出しデータポインタ
   @param[in]	count	読み出しバイト数

   @return	読み出しバイト数(<0:エラー)
*/
t_ssize read_gsffs(void *fd, void *buf, t_size count)
{
	struct st_gsffs *fdp = fd;
	unsigned char *dp = (unsigned char *)buf;
	t_size rtn = 0;
	int i;

	DTFPRINTF(0x01, "fd = %p, buf = %p, count = %d\n", fd, buf, (int)count);

	for(i=0; i<count; i++) {
		int nextp = fdp->seekptr + 1;

		if(nextp <= fdp->filesize) {
			*dp = fdp->buf[DATAPOS_DATA + fdp->seekptr];
			fdp->seekptr ++;
			dp ++;
			rtn ++;
		} else {
			goto end;
		}
	}
end:
	GSLOG(GSFFSLOGLVL, "Data read length %d\n", i);
	XDUMP(0x02, &(fdp->buf[DATAPOS_DATA]), i);

	return rtn;
}

/**
   @brief	ファイルにデータを書き込む

   @param[in]	fd	ファイルディスクリプタ
   @param[in]	buf	書き込みデータポインタ
   @param[in]	count	書き込みバイト数

   @return	書き込みバイト数(<0:エラー)
*/
t_ssize write_gsffs(void *fd, const void *buf, t_size count)
{
	struct st_gsffs *fdp = fd;
	unsigned char *dp = (unsigned char *)buf;
	t_size rtn = 0;
	int i;

	DTFPRINTF(0x01, "fd = %p, buf = %p, count = %d\n", fd, buf, (int)count);

	if((fdp->flags & FA_WRITE) == 0) {
		return 0;
	}

	for(i=0; i<count; i++) {
		int nextp = fdp->seekptr + 1;

		if(nextp <= MAXFILESIZE) {
			fdp->flg_dirty = 1;
			fdp->buf[DATAPOS_DATA + fdp->seekptr] = *dp;
			fdp->seekptr ++;
			if(fdp->filesize < fdp->seekptr) {
				fdp->filesize = fdp->seekptr;
			}
			dp ++;
			rtn ++;
		} else {
			goto end;
		}
	}
end:
	fdp->datetime.sec = get_systime_sec();
	fdp->datetime.usec = 0;

	GSLOG(GSFFSLOGLVL, "Data write length %d\n", fdp->filesize);
	DTFPRINTF(0x01, "filesize = %d\n", fdp->filesize);
	XDUMP(0x02, fdp->buf, FLASH_SECTSIZE);

	return rtn;
}


t_ssize seek_gsffs(void *fd, t_ssize offset, int whence)
{
	struct st_gsffs *fdp = fd;

	DTFPRINTF(0x01, "fd = %p, offset = %lld, whence = %d\n", fd, offset, whence);

	switch(whence) {
	case SEEK_SET:
		fdp->seekptr = offset;
		return fdp->seekptr;
		break;

	case SEEK_CUR:
		fdp->seekptr = fdp->seekptr + offset;
		if(fdp->seekptr > fdp->filesize) {
			fdp->seekptr = fdp->filesize;
		}
		return fdp->seekptr;
		break;

	case SEEK_END:
		fdp->seekptr = fdp->filesize + offset;
		if(fdp->seekptr > fdp->filesize) {
			fdp->seekptr = fdp->filesize;
		}
		return fdp->seekptr;
		break;
	}

	return -1;
}

/**
   @brief	ファイルアクセス位置の取得

   @param[in]	fd	ファイルディスクリプタ

   @return	先頭からのオフセット位置バイト数
*/
t_size tell_gsffs(void *fd)
{
	struct st_gsffs *fdp = fd;

	DTFPRINTF(0x01, "fd = %p\n", fd);

	return fdp->seekptr;
}


t_ssize size_gsffs(void *fd)
{
	struct st_gsffs *fdp = fd;
	t_size rtn = 0;

	DTFPRINTF(0x02, "fd = %p\n", fd);

	rtn = fdp->filesize;

	return rtn;
}


static FS_DIR gsffs_dir_desc[GSC_GSFFS_MAX_DIR_NUM];
extern const struct st_filesystem gsffs_fs;

/**
   @brief	ディレクトリを開く

   @param[out]	dir	ディレクトリデータ
   @param[in]	path	ディレクトリ名

   @return	エラーコード
*/
FS_DIR * opendir_gsffs(const uchar *path)
{
	int i;

	DTFPRINTF(0x01, "path = %s\n", path);

	for(i=0; i<GSC_GSFFS_MAX_DIR_NUM; i++) {
		if(gsffs_dir_desc[i].fs == 0) {
			gsffs_dir_desc[i].dir.finfo_sector = fs_sect_start;
			gsffs_dir_desc[i].fs = (struct st_filesystem *)&gsffs_fs;
			return &gsffs_dir_desc[i];
		}
	}

	SYSERR_PRINT("cannot open dir \"%s\" (GSC_GSFFS_MAX_DIR_NUM)\n", path);

	return 0;
}


/**
   @brief	ディレクトリを読み出す

   @param[in]	dir	ディレクトリ
   @param[out]	info	ファイル情報

   @return	エラーコード
*/
int readdir_gsffs(FS_DIR *dir, FS_FILEINFO *finfo)
{
	int rt = 0;
	unsigned char buf[FLASH_SECTSIZE];

	DTFPRINTF(0x01, "dir = %p, info = %p\n", dir, finfo);

	for(; dir->dir.finfo_sector<fs_sect_end; dir->dir.finfo_sector++) {
		rt = read_sector(buf, dir->dir.finfo_sector);
		if(rt == 0) {
			return -1;
		}
		if(buf[DATAPOS_SECTSTAT] == SECTSTAT_SAVED) {
			GSLOG(GSFFSLOGLVL, "Dir file found sector %d\n", dir->dir.finfo_sector);

			finfo->fattrib = buf[DATAPOS_ATTRIBUTE];

			finfo->fdatetime = ((buf[DATAPOS_DATETIME+0]<<24)
					    + (buf[DATAPOS_DATETIME+1]<<16)
					    + (buf[DATAPOS_DATETIME+2]<< 8)
					    + (buf[DATAPOS_DATETIME+3]<< 0)
					    + GSC_DIFF_FROM_LOCAL_TIME_SEC);

			finfo->fsize = (buf[DATAPOS_SIZE]<<8) + (buf[DATAPOS_SIZE+1]<<0);

			tsnprintf((char *)finfo->fname, MAXFNAMELEN + 1, "%s", &buf[DATAPOS_FILENAME]);
			dir->dir.finfo_sector++;
			return 0;
		}
	}

	finfo->fname[0] = 0;

	GSLOG(GSFFSLOGLVL, "Dir all sector search end\n");

	return 0;
}

/**
   @brief	ディレクトリを閉じる

   @param[in]	dir	ディレクトリ

   @return	エラーコード
*/
int closedir_gsffs(FS_DIR *dir)
{
	DTFPRINTF(0x01, "dir = %p\n", dir);

	dir->fs = 0;

	return 0;
}


/**
   @brief	ファイルを消去する

   @param	path	ファイル名

   @return	ファイルディスクリプタ
*/
int unlink_gsffs(const uchar *path)
{
	int sector;
	struct st_gsffs *fd = 0;
	int rt = 0;

	DTFPRINTF(0x01, "path = \"%s\"\n", path);

	fd = search_blank_fd();
	if(fd == 0) {
		GSLOG(GSFFSLOGLVL, "No file discriptor\n");
		return 0;
	}

	memoryset(fd->filename, 0x00, MAXFNAMELEN+1);
	memoryset(fd->buf, 0xff, FLASH_SECTSIZE);
	get_filename(fd->filename, path);

	GSLOG(GSFFSLOGLVL, "filename = %s\n", fd->filename);

	sector = search_file_sector(fd->filename, fd->buf);

	if(sector < 0) {
		GSLOG(GSFFSLOGLVL, "File not found %s\n", fd->filename);
		return FR_NO_FILE;
	} else {
		fd->buf[DATAPOS_SECTSTAT] = SECTSTAT_DELETED;
		rt = write_sector(fd->buf, sector);
		if(rt <= 0) {
			return 0;
		}
		GSLOG(GSFFSLOGLVL, "File deleted %s\n", fd->filename);
	}

	return 0;
}

/**
   @brief	ディスクをフォーマットする

   @param[in]	drive	論理ドライブ番号
   @param[in]	part
   @param[in]	alloc	クラスタサイズ

   @return	エラーコード

   @todo	ディスクフォーマット機能
*/
int mkfs_gsffs(const uchar *path, unsigned char part, unsigned short alloc)
{
	int i;

	DTFPRINTF(0x01, "path = %s, part = %d, alloc = %d\n", path, part, alloc);

	for(i=0; i<GSC_GSFFS_USE_ERASESECTCOUNT; i++) {
		int block = qspi_info.erase_sectors_number - GSC_GSFFS_USE_ERASESECTCOUNT + i;
		GSLOG(GSFFSLOGLVL, "Erase block %d\n", block);
		erase_block(block);
	}

	return 0;
}


const struct st_filesystem gsffs_fs = {
	.name		= FSNAME_GSFFS,
	.mount		= mount_gsffs,
	.unmount	= unmount_gsffs,
	.open		= open_gsffs,
	.read		= read_gsffs,
	.write		= write_gsffs,
	.seek		= seek_gsffs,
	.tell		= tell_gsffs,
	.size		= size_gsffs,
	.close		= close_gsffs,
	.opendir	= opendir_gsffs,
	.readdir	= readdir_gsffs,
	.closedir	= closedir_gsffs,
	.unlink		= unlink_gsffs,
	.mkfs		= mkfs_gsffs,
};
