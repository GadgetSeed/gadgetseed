/** @file
    @brief	FatFs ファイルシステムAPI

    @date	2018.09.09
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "device.h"
#include "str.h"
#include "tkprintf.h"
#include "fs.h"
#include "ff.h"
#include "fatfs.h"

//#define DEBUGTBITS 0x03
#include "dtprintf.h"


#ifndef GSC_FATFS_MAX_FILE_NUM
#define GSC_FATFS_MAX_FILE_NUM	2	///< $gsc FatFsでオープンできる最大ファイル数
#endif

#ifndef GSC_FATFS_MAX_DIR_NUM
#define GSC_FATFS_MAX_DIR_NUM	4	///< $gsc FatFsでオープンできる最大ディレクトリ数
#endif

#define ACCESS_SIZE	0x4000

struct st_fatfs fatfs[FF_VOLUMES];

struct st_fatfs_file_desc {
	int flg_used; 	///< ファイルディスクリプタ使用中？
	FIL fat;	///< FatFsファイルディスクリプタ
};	///< FatFsファイルディスクリプタ

static struct st_fatfs_file_desc fatfs_file_desc[GSC_FATFS_MAX_FILE_NUM];

#define DEVNAME_LEN	2	// "X:"

int mount_fatfs(int drvno, struct st_device *dev)
{
	FRESULT result;
	char dev_name[DEVNAME_LEN+1];

	DTFPRINTF(0x01, "drvno = %d, dev = %p\n", drvno, dev);

	if(drvno >= FF_VOLUMES) {
		SYSERR_PRINT("Storage number %d too large.\n", drvno);
		return -1;
	}

	if(dev == 0) {
		SYSERR_PRINT("Device is NULL.\n");
		return -1;
	}

	fatfs[drvno].device = dev;

	tsnprintf(dev_name, DEVNAME_LEN, "%1d:", drvno);
	result = f_mount(&(fatfs[drvno].fatfs), dev_name, 1);

	if(result != FR_OK) {
		eprintf("Device (Storage %d:) mount error.\n", drvno);
		return -1;
	} else {
		return 0;
	}
}

int unmount_fatfs(int drvno, struct st_device *dev)
{
	FRESULT result;
	char dev_name[DEVNAME_LEN+1];

	DTFPRINTF(0x01, "drvno = %d, dev = %p\n", drvno, dev);

	if(drvno >= FF_VOLUMES) {
		SYSERR_PRINT("Storage number %d too large.\n", drvno);
		return -1;
	}

	if(dev == 0) {
		SYSERR_PRINT("Device is NULL.\n");
		return -1;
	}

	sync_device(dev);

	close_device(dev);

	tsnprintf(dev_name, DEVNAME_LEN, "%d:", drvno);

	result = f_mount(&(fatfs[drvno].fatfs), dev_name, 0);

	if(result != FR_OK) {
		SYSERR_PRINT("Device (Storage %d:) unmount error.\n", drvno);
		return -1;
	} else {
		return 0;
	}
}

/**
   @brief	ファイルを開く

   @param	path	ファイル名
   @param	flags	属性フラグ

   @return	ファイルディスクリプタ
*/
void * open_fatfs(const uchar *path, int flags)
{
	int i;
	FRESULT res;

	DTFPRINTF(0x01, "path = %s, flgas = %08X\n", path, flags);

	for(i=0; i<GSC_FATFS_MAX_FILE_NUM; i++) {
		if(fatfs_file_desc[i].flg_used == 0) {
			res = f_open(&(fatfs_file_desc[i].fat), (char *)path, flags);
			if(res == FR_OK) {
				fatfs_file_desc[i].flg_used = 1;
				return (void *)&fatfs_file_desc[i];
			} else {
				//SYSERR_PRINT("cannot open file \"%s\" (%d)", path, res);
				return 0;
			}
		}
	}

	SYSERR_PRINT("cannot open file \"%s\" (GSC_FATFS_MAX_FILE_NUM)\n", path);

	return 0;
}

/**
   @brief	ファイルからデータを読み出す

   @param[in]	int	ファイルディスクリプタ
   @param[out]	buf	読み出しデータポインタ
   @param[in]	count	読み出しバイト数

   @return	読み出しバイト数(<0:エラー)
*/
t_ssize read_fatfs(void *fd, void *buf, t_size count)
{
	struct st_fatfs_file_desc *fatfs = fd;
	unsigned int i, rcount = count;
	FRESULT res;
	int rt = 0;
	UINT size;
	unsigned char *rp = buf;

	DTFPRINTF(0x02, "fd = %p, buf = %p, count = %lld\n", fd, buf, count);

	for(i=0; i<count; i+=ACCESS_SIZE) {
		DTFPRINTF(0x01, "read = %d\n", rcount > ACCESS_SIZE ? ACCESS_SIZE : rcount);
		DTFPRINTF(0x01, "p = %p\n", rp);
		res = f_read(&(fatfs->fat), rp, rcount > ACCESS_SIZE ? ACCESS_SIZE : rcount, &size);
		if(res == FR_OK) {
			DTFPRINTF(0x01, "size = %d\n", size);
			rcount -= size;
			rp += size;
			rt += size;
		} else {
			DTPRINTF(0x01, "read_file error(size = %d, result = %d)\n", size, res);
			return 0 - res;
		}
	}

	return rt;
}

/**
   @brief	ファイルにデータを書き込む

   @param[in]	fd	ファイルディスクリプタ
   @param[in]	buf	書き込みデータポインタ
   @param[in]	count	書き込みバイト数

   @return	書き込みバイト数(<0:エラー)
*/
t_ssize write_fatfs(void *fd, const void *buf, t_size count)
{
	struct st_fatfs_file_desc *fatfs = fd;
	unsigned int i, rcount = count;
	int rt = 0;
	UINT size;
	unsigned char *wp = (unsigned char *)buf;

	DTFPRINTF(0x02, "fd = %p, buf = %p, count = %lld\n", fd, buf, count);

	for(i=0; i<count; i+=ACCESS_SIZE) {
		if(f_write(&(fatfs->fat), wp, rcount > ACCESS_SIZE ? ACCESS_SIZE : rcount, &size) == FR_OK) {
			rcount -= size;
			wp += size;
			rt += size;
		} else {
			return -1;
		}
	}

	return rt;
}

/**
   @brief	ファイルアクセス位置の設定

   @param[in]	fd	ファイルディスクリプタ
   @param[in]	offset	移動バイト数
   @param[in]	whence	移動基準位置

   @return	先頭からのオフセット位置バイト数
*/
t_ssize seek_fatfs(void *fd, t_ssize offset, int whence)
{
	struct st_fatfs_file_desc *fatfs = fd;

	DTFPRINTF(0x02, "fd = %p, offset = %lld, whence = %d\n", fd, offset, whence);

	switch(whence) {
	case SEEK_SET:
		if(f_lseek(&(fatfs->fat), offset) == FR_OK) {
			return fatfs->fat.fptr;
		}
		break;

	case SEEK_CUR:
		if(f_lseek(&(fatfs->fat), fatfs->fat.fptr + offset) == FR_OK) {
			return fatfs->fat.fptr;
		}
		break;

	case SEEK_END:
		if(f_lseek(&(fatfs->fat), f_size(&(fatfs->fat)) + offset) == FR_OK) {
			return fatfs->fat.fptr;
		}
		break;
	}

	return -1;
}

/**
   @brief	ファイルアクセス位置の取得

   @param[in]	fd	ファイルディスクリプタ

   @return	先頭からのオフセット位置バイト数
*/
t_size tell_fatfs(void *fd)
{
	struct st_fatfs_file_desc *fatfs = fd;

	DTFPRINTF(0x01, "fd = %p\n", fd);

	return fatfs->fat.fptr;
}


/**
   @brief	ファイルサイズの取得

   @param[in]	fd	ファイルディスクリプタ

   @return	ファイルサイズ
*/
t_ssize size_fatfs(void *fd)
{
	struct st_fatfs_file_desc *fatfs = fd;

	DTFPRINTF(0x01, "fd = %p\n", fd);

	return f_size(&(fatfs->fat));
}


/**
   @brief	ファイルを閉じる

   @param[in]	fd	ファイルディスクリプタ

   @return	0:成功
*/
int close_fatfs(void *fd)
{
	struct st_fatfs_file_desc *fatfs = fd;

	DTFPRINTF(0x01, "fd = %p\n", fd);

	fatfs->flg_used = 0;

	return f_close(&(fatfs->fat));
}


static FS_DIR fatfs_dir_desc[GSC_FATFS_MAX_DIR_NUM];
extern const struct st_filesystem fatfs_fs;

/**
   @brief	ディレクトリを開く

   @param[out]	dir	ディレクトリデータ
   @param[in]	path	ディレクトリ名

   @return	エラーコード
*/
FS_DIR * opendir_fatfs(const uchar *path)
{
	int i;
	FRESULT res;

	DTFPRINTF(0x01, "path = %s\n", path);

	for(i=0; i<GSC_FATFS_MAX_DIR_NUM; i++) {
		if(fatfs_dir_desc[i].fs == 0) {
			res = f_opendir(&(fatfs_dir_desc[i].dir.dir), (char *)path);
			if(res == FR_OK) {
				fatfs_dir_desc[i].fs = (struct st_filesystem *)&fatfs_fs;
				return &fatfs_dir_desc[i];
			} else {
				//SYSERR_PRINT("cannot open file \"%s\" (%d)", path, res);
				return 0;
			}
		}
	}

	SYSERR_PRINT("cannot open dir \"%s\" (GSC_FATFS_MAX_DIR_NUM)\n", path);

	return 0;
}

static void filinfo_to_fileinfo(FS_FILEINFO *fsi, FILINFO *info)
{
	struct st_datetime datetime;

	fsi->fsize = info->fsize;

	datetime.year	= (info->fdate >> 9) + 1980;
	datetime.month	= (info->fdate >> 5) & 15;
	datetime.day	= (info->fdate & 31);
	datetime.hour	= (info->ftime >> 11);
	datetime.min	= (info->ftime >> 5) & 63;
	datetime.sec	= (info->ftime & 63);
	datetime.msec	= 0;

	fsi->fdatetime	= datetime_to_utc(&datetime);
	fsi->fattrib	= info->fattrib;
	strncopy(fsi->fname, (const uchar *)info->fname, FF_LFN_BUF);
}

/**
   @brief	ディレクトリを読み出す

   @param[in]	dir	ディレクトリ
   @param[out]	info	ファイル情報

   @return	エラーコード
*/
int readdir_fatfs(FS_DIR *dir, FS_FILEINFO *finfo)
{
	FILINFO info;
	FRESULT res = 0;

	DTFPRINTF(0x02, "dir = %p, info = %p\n", dir, finfo);

	res = f_readdir(&(dir->dir.dir), &info);

	if(res == FR_OK) {
		filinfo_to_fileinfo(finfo, &info);
	}

	return res;
}

/**
   @brief	ディレクトリを閉じる

   @param[in]	dir	ディレクトリ

   @return	エラーコード
*/
int closedir_fatfs(FS_DIR *dir)
{
	FRESULT res;

	DTFPRINTF(0x01, "dir = %p\n", dir);

	res = f_closedir(&(dir->dir.dir));

	dir->fs = 0;

	return res;
}


/**
   @brief	ファイルステータスを読み出す

   @param[in]	path	ファイル(ディレクトリ)名
   @param[out]	info	ファイル情報構造体ポインタ

   @return	エラーコード
*/
int stat_fatfs(const uchar *path, FS_FILEINFO *finfo)
{
	FILINFO info;
	FRESULT res;

	DTFPRINTF(0x02, "path = %s, info = %p\n", path, finfo);

	res = f_stat((char *)path, &info);

	if(res == FR_OK) {
		filinfo_to_fileinfo(finfo, &info);
	}

	return res;
}

/**
   @brief	論理ドライブの未使用クラスタ数を取得する

   @param[in]	path	ファイル(ドライブ)名
   @param[out]	sect
   @param[out]	fs

   @return	エラーコード
*/
int getfree_fatfs(const uchar *path, unsigned long *sect, void **fs)
{
	FATFS **fspp = (FATFS **)fs;
	int rtn = 0;

	DTFPRINTF(0x02, "path = %s, sect = %p, fs = %p\n", path, sect, fs);

	rtn = f_getfree((char *)path, sect, fspp);

	return rtn;
}

/**
   @brief	キャッシュされたデータをフラッシュする

   @param[in]	fd	ファイルディスクリプタ

   @return	エラーコード
*/
int sync_fatfs(void *fd)
{
	struct st_fatfs_file_desc *fatfs = fd;

	DTFPRINTF(0x02, "fd = %p\n", fd);

	return f_sync(&(fatfs->fat));
}

/**
   @brief	ファイルを消去する

   @param[in]	path	ファイル名

   @return	エラーコード
*/
int unlink_fatfs(const uchar *path)
{
	DTFPRINTF(0x02, "path = %s\n", path);

	return f_unlink((char *)path);
}

/**
   @brief	ディレクトリを作成する

   @param[in]	path	ディレクトリ名

   @return	エラーコード
*/
int mkdir_fatfs(const uchar *path)
{
	DTFPRINTF(0x02, "path = %s\n", path);

	return f_mkdir((char *)path);
}

#if FF_USE_CHMOD != 0
/**
   @brief	ファイルまたはディレクとの属性を変更する

   @param[in]	path	ファイルまたはディレクトリ名
   @param[in]	flag	設定する属性

   @return	エラーコード
*/
int chmod_fatfs(const uchar *path, unsigned char flag)
{
	DTFPRINTF(0x02, "path = %s, flag = %d\n", path, flag);

	return f_chmod((char *)path, flag, 0xff);
}
#endif

/**
   @brief	ファイル/ディレクトリ名を変更する

   @param[in]	oldpath	変更されるファイル名
   @param[in]	newpath	変更後のファイル名

   @return	エラーコード
*/
int rename_fatfs(const uchar *oldpath, const uchar *newpath)
{
	DTFPRINTF(0x02, "oldpath = %s, newpath = %s\n", oldpath, newpath);

	return f_rename((char *)oldpath, (char *)newpath);
}

#if FF_USE_MKFS != 0
/**
   @brief	ディスクをフォーマットする

   @param[in]	drive	論理ドライブ番号
   @param[in]	part
   @param[in]	alloc	クラスタサイズ

   @return	エラーコード

   @todo	ディスクフォーマット機能
*/
int mkfs_fatfs(const uchar *path, unsigned char part, unsigned short alloc)
{
	unsigned char buf[FF_MIN_SS];

	DTFPRINTF(0x02, "path = %s, part = %d, alloc = %d\n", ath, part, alloc);

	return f_mkfs((char *)path, part, alloc, buf, FF_MIN_SS);
}
#endif

const struct st_filesystem fatfs_fs = {
	.name		= FSNAME_VFAT,
	.mount		= mount_fatfs,
	.unmount	= unmount_fatfs,
	.open		= open_fatfs,
	.read		= read_fatfs,
	.write		= write_fatfs,
	.seek		= seek_fatfs,
	.tell		= tell_fatfs,
	.close		= close_fatfs,
	.opendir	= opendir_fatfs,
	.readdir	= readdir_fatfs,
	.closedir	= closedir_fatfs,
	.stat		= stat_fatfs,
	.size		= size_fatfs,
	.getfree	= getfree_fatfs,
	.sync		= sync_fatfs,
	.unlink		= unlink_fatfs,
	.mkdir		= mkdir_fatfs,
#if FF_USE_CHMOD != 0
	.chmod		= chmod_fatfs,
#endif
	.rename		= rename_fatfs,
#if FF_USE_MKFS != 0
	.mkfs		= mkfs_fatfs,
#endif
};
