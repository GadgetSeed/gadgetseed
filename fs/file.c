/** @file
    @brief	ファイル関数

    GadgetSeed のファイルシステムは FatFs を使用しています。

    参考 : http://elm-chan.org/fsw/ff/00index_e.html

    @date	2018.09.15
    @date	2008.03.20
    @author	Takashi SHUDO

    @page file_system ファイルシステム

    GadgetSeedのファイルシステムは [FatFs](http://elm-chan.org/fsw/ff/00index_e.html) を使用しています。

    ファイルシステムを使用するには、以下のコンフィグ項目を有効にして下さい。

    * COMP_ENABLE_FATFS

    ---
    @section file_api ファイルAPI

    include ファイル : file.h

    | API名			| 機能				|
    |:--------------------------|:------------------------------|
    | open_file()		| @copybrief open_file		|
    | read_file()		| @copybrief read_file		|
    | write_file()		| @copybrief write_file		|
    | seek_file()		| @copybrief seek_file		|
    | tell_file()		| @copybrief tell_file		|
    | close_file()		| @copybrief close_file		|
    | opendir_file()		| @copybrief opendir_file	|
    | readdir_file()		| @copybrief readdir_file	|
    | closedir_file()		| @copybrief closedir_file	|
    | stat_file()		| @copybrief stat_file		|
    | getfree_file()		| @copybrief getfree_file	|
    | sync_file()		| @copybrief sync_file		|
    | unlink_file()		| @copybrief unlink_file	|
    | mkdir_file()		| @copybrief mkdir_file		|
    | chmod_file()		| @copybrief chmod_file		|
    | rename_file()		| @copybrief rename_file	|
    | mkfs_file()		| @copybrief mkfs_file		|


    ---
    @section file_util_api ファイルユーティリティAPI

    include ファイル : file.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | get_last_filename()	| @copybrief get_last_filename		|
    | get_filename_extension()	| @copybrief get_filename_extension	|
*/

#include "file.h"
#include "device.h"
#include "storage.h"
#include "str.h"
#include "tkprintf.h"
#include "fs.h"
#include "task/syscall.h"

//#define DEBUGTBITS 0x04
#include "dtprintf.h"

// ファイルディスクリプタはスタティックに確保
#ifndef GSC_FS_MAX_FILE_NUM
#define GSC_FS_MAX_FILE_NUM	2	///< $gsc オープンできる最大ファイル数
#endif

extern struct st_filesystem *filesystems[];
extern struct st_storage_info storage[GSC_FS_VOLUME_NUM];	///< ストレージデバイステーブル

static struct st_file_desc {
	int flg_used; 		///< ファイルディスクリプタ使用フラグ
	struct st_filesystem *fs;	///< ファイルシステム
	void *fs_desc;			///< ファイルシステム用ディスクリプタ
} file_desc[GSC_FS_MAX_FILE_NUM];

static struct st_mutex fsapi_mutex;

/**
   @brief	全てのファイルディスクリプタを初期化する
*/
void init_file(void)
{
	int i;

	for(i=0; i<GSC_FS_MAX_FILE_NUM; i++) {
		file_desc[i].flg_used = 0;
	}

	mutex_register_ISR(&fsapi_mutex, "fsapi");
}

/**
   @brief	ファイル名からストレージ番号を調べる

   @param	path	ファイル名

   @return	ストレージ番号
*/
int get_diskno(const uchar *path)
{
	int devno = 0;

	if(path[1] == ':') {
		if(('0' <= path[0]) && (path[0] <= '9')) {
			devno = path[0] - '0';
		}
	}

	return devno;
}

/**
   @brief	ファイル名からファイルシステムを調べる

   @param	path	ファイル名

   @return	ファイルシステム
*/
struct st_filesystem * get_filesystem(const uchar *path)
{
	int devno;
	struct st_filesystem *fs = 0;

	devno = get_diskno(path);

	if(devno >= GSC_FS_VOLUME_NUM) {
		fs = 0;	// デバイス無し
	} else {
		fs = storage[devno].fs;
	}

	return fs;
}

/**
   @brief	ファイルを開く

   @param	path	ファイル名
   @param	flags	属性フラグ

   @return	ファイルディスクリプタ(0:エラー)
*/
int open_file(const uchar *path, int flags)
{
	int i;
	struct st_filesystem *fs;
	void *fdesc;

	DTFPRINTF(0x01, "path = %s, flgas = %08X\n", path, flags);

	fs = get_filesystem(path);
	if(fs == 0) {
		return -1;
	}
	DTFPRINTF(0x04, "fs->name = %s\n", fs->name);

	mutex_lock(&fsapi_mutex, 0);
	for(i=0; i<GSC_FS_MAX_FILE_NUM; i++) {
		if(file_desc[i].flg_used == 0) {
			file_desc[i].fs = fs;
			fdesc = fs->open(path, flags);
			if(fdesc != 0) {
				file_desc[i].fs_desc = fdesc;
				file_desc[i].flg_used = 1;
				DTFPRINTF(0x04, "fdesc = %p, fd_num = %d\n", fdesc, i);
				mutex_unlock(&fsapi_mutex);
				return i;
			} else {
				DTFPRINTF(0x01, "open error \"%s\" fdesc = %p\n", (char *)path, fdesc);
				mutex_unlock(&fsapi_mutex);
				return -1;
			}
		}
	}
	mutex_unlock(&fsapi_mutex);

	SYSERR_PRINT("cannot open file \"%s\" (GSC_FS_MAX_FILE_NUM)\n", path);

	return -1;
}

#define ACCESS_SIZE	0x4000

/**
   @brief	ファイルからデータを読み出す

   @param[in]	int	ファイルディスクリプタ
   @param[out]	buf	読み出しデータポインタ
   @param[in]	count	読み出しバイト数

   @return	読み出しバイト数(<0:エラー)
*/
t_ssize read_file(int fd, void *buf, t_size count)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x02, "fd = %d, buf = %p, count = %lld\n", fd, buf, count);

	fs = file_desc[fd].fs;
	if(fs == 0) {
		return -1;
	}

	if(fs->read != 0) {
		rtn = fs->read(file_desc[fd].fs_desc, buf, count);
	}

	return rtn;
}

/**
   @brief	ファイルにデータを書き込む

   @param[in]	fd	ファイルディスクリプタ
   @param[in]	buf	書き込みデータポインタ
   @param[in]	count	書き込みバイト数

   @return	書き込みバイト数(<0:エラー)
*/
t_ssize write_file(int fd, const void *buf, t_size count)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x02, "fd = %d, buf = %p, count = %lld\n", fd, buf, count);

	fs = file_desc[fd].fs;
	if(fs == 0) {
		return -1;
	}

	if(fs->write != 0) {
		rtn = fs->write(file_desc[fd].fs_desc, buf, count);
	}

	return rtn;
}

/**
   @brief	ファイルアクセス位置の設定

   @param[in]	fd	ファイルディスクリプタ
   @param[in]	offset	移動バイト数
   @param[in]	whence	移動基準位置

   @return	先頭からのオフセット位置バイト数
*/
t_ssize seek_file(int fd, t_ssize offset, int whence)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x02, "fd = %d, offset = %lld, whence = %d\n", fd, offset, whence);

	fs = file_desc[fd].fs;
	if(fs == 0) {
		return -1;
	}

	if(fs->seek != 0) {
		rtn = fs->seek(file_desc[fd].fs_desc, offset, whence);
	}

	return rtn;
}

/**
   @brief	ファイルアクセス位置の取得

   @param[in]	fd	ファイルディスクリプタ

   @return	先頭からのオフセット位置バイト数
*/
t_size tell_file(int fd)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x02, "fd = %d\n", fd);

	fs = file_desc[fd].fs;
	if(fs == 0) {
		return -1;
	}

	if(fs->tell != 0) {
		rtn = fs->tell(file_desc[fd].fs_desc);
	}

	return rtn;
}

/**
   @brief	ファイルサイズの取得

   @param[in]	fd	ファイルディスクリプタ

   @return	ファイルサイズ
*/
t_ssize size_file(int fd)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x02, "fd = %d\n", fd);

	fs = file_desc[fd].fs;
	if(fs == 0) {
		return -1;
	}

	if(fs->size != 0) {
		rtn = fs->size(file_desc[fd].fs_desc);
	}

	return rtn;
}


/**
   @brief	ファイルを閉じる

   @param[in]	fd	ファイルディスクリプタ

   @return	0:成功
*/
int close_file(int fd)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x01, "fd = %d\n", fd);
	DTFPRINTF(0x04, "fd = %d\n", fd);

	fs = file_desc[fd].fs;
	if(fs == 0) {
		return -1;
	}

	if(fs->close != 0) {
		rtn = fs->close(file_desc[fd].fs_desc);
	}

	file_desc[fd].flg_used = 0;

	if(rtn != 0) {
		SYSERR_PRINT("cannot close file (%d)\n", fd);
	}

	return rtn;
}

/**
   @brief	ディレクトリを開く

   @param[in]	name	ディレクトリ名

   @return	ディレクトリ(0:エラー)
*/
FS_DIR * opendir_file(const uchar *name)
{
	struct st_filesystem *fs;
	FS_DIR *dir = 0;

	DTFPRINTF(0x01, "dir = %p, name = %s\n", dir, name);

	fs = get_filesystem(name);
	if(fs == 0) {
		return 0;
	}

	if(fs->opendir != 0) {
		dir = fs->opendir(name);
	}

	return dir;
}

/**
   @brief	ディレクトリを読み出す

   @param[in]	dir	ディレクトリ
   @param[out]	info	ファイル情報

   @return	エラーコード
*/
int readdir_file(FS_DIR *dir, FS_FILEINFO *finfo)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x02, "dir = %p, info = %p\n", dir, finfo);

	fs = dir->fs;
	if(fs == 0) {
		return -1;
	}

	if(fs->readdir != 0) {
		rtn = fs->readdir(dir, finfo);
	}

	return rtn;
}

/**
   @brief	ディレクトリを閉じる

   @param[in]	dir	ディレクトリ

   @return	エラーコード
*/
int closedir_file(FS_DIR *dir)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x01, "dir = %p\n", dir);

	fs = dir->fs;
	if(fs == 0) {
		return -1;
	}

	if(fs->closedir != 0) {
		rtn = fs->closedir(dir);
	}

	return rtn;
}

/**
   @brief	ファイルステータスを読み出す

   @param[in]	path	ファイル(ディレクトリ)名
   @param[out]	info	ファイル情報構造体ポインタ

   @return	エラーコード
*/
int stat_file(const uchar *path, FS_FILEINFO *finfo)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x02, "path = %s, info = %p\n", path, finfo);

	fs = get_filesystem(path);
	if(fs == 0) {
		return -1;
	}

	if(fs->stat != 0) {
		rtn = fs->stat(path, finfo);
	}

	return rtn;
}

/**
   @brief	論理ドライブの未使用クラスタ数を取得する

   @param[in]	path	ファイル(ドライブ)名
   @param[out]	sect
   @param[out]	fs

   @return	エラーコード
*/
int getfree_file(const uchar *path, unsigned long *sect, void **fso)
{
	struct st_filesystem *fs;
	int rtn = -1;

	DTFPRINTF(0x02, "path = %s, sect = %p, fs = %p\n", path, sect, fso);

	fs = get_filesystem(path);
	if(fs == 0) {
		return -1;
	}

	if(fs->getfree != 0) {
		rtn = fs->getfree(path, sect, fso);
	}

	return rtn;
}

/**
   @brief	キャッシュされたデータをフラッシュする

   @param[in]	fd	ファイルディスクリプタ

   @return	エラーコード
*/
int sync_file(int fd)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x02, "fd = %d\n", fd);

	fs = file_desc[fd].fs;
	if(fs == 0) {
		return -1;
	}

	if(fs->sync != 0) {
		rtn = fs->sync(file_desc[fd].fs_desc);
	}

	return rtn;
}

/**
   @brief	ファイルを消去する

   @param[in]	path	ファイル名

   @return	エラーコード
*/
int unlink_file(const uchar *path)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x02, "path = %s\n", path);

	fs = get_filesystem(path);
	if(fs == 0) {
		return -1;
	}

	if(fs->unlink != 0) {
		rtn = fs->unlink(path);
	}

	return rtn;
}

/**
   @brief	ディレクトリを作成する

   @param[in]	path	ディレクトリ名

   @return	エラーコード
*/
int mkdir_file(const uchar *path)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x02, "path = %s\n", path);

	fs = get_filesystem(path);
	if(fs == 0) {
		return -1;
	}

	if(fs->mkdir != 0) {
		rtn = fs->mkdir(path);
	}

	return rtn;
}

/**
   @brief	ファイルまたはディレクとの属性を変更する

   @param[in]	path	ファイルまたはディレクトリ名
   @param[in]	flag	設定する属性

   @return	エラーコード
*/
int chmod_file(const uchar *path, unsigned char flag)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x02, "path = %s, flag = %d\n", path, flag);

	fs = get_filesystem(path);
	if(fs == 0) {
		return -1;
	}

	if(fs->chmod != 0) {
		rtn = fs->chmod(path, flag);
	}

	return rtn;
}

/**
   @brief	ファイル/ディレクトリ名を変更する

   @param[in]	oldpath	変更されるファイル名
   @param[in]	newpath	変更後のファイル名

   @return	エラーコード

   @info	異なるデバイスへの名前変更はできない
*/
int rename_file(const uchar *oldpath, const uchar *newpath)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x02, "oldpath = %s, newpath = %s\n", oldpath, newpath);

	fs = get_filesystem(oldpath);
	if(fs == 0) {
		return -1;
	}

	if(fs->rename != 0) {
		rtn = fs->rename(oldpath, newpath);
	}

	return rtn;
}

/**
   @brief	ディスクをフォーマットする

   @param[in]	drive	論理ドライブ番号
   @param[in]	part
   @param[in]	alloc	クラスタサイズ

   @return	エラーコード

   @todo	ディスクフォーマット機能
*/
int mkfs_file(const uchar *path, unsigned char part, unsigned short alloc)
{
	struct st_filesystem *fs;
	int rtn = 0;

	DTFPRINTF(0x02, "path = %s, part = %d, alloc = %d\n", path, part, alloc);

	fs = get_filesystem(path);
	if(fs == 0) {
		return -1;
	}

	if(fs->mkfs != 0) {
		rtn = fs->mkfs(path, part, alloc);
	}

	return rtn;
}

/**
   @brief	ファイルパスからファイル名のみを取得する

   @param[out]	filename	ファイル名
   @param[in]	fullpath	ファイルパス名
   @param[in]	len		最大ファイル名バイト数

   @return	ファイル名
*/
uchar * get_last_filename(uchar *filename, const uchar *fullpath, unsigned int len)
{
	uchar *p;

	if(filename == 0) {
		return 0;
	}

	if(fullpath == 0) {
		return 0;
	}

	if(*fullpath == 0) {
		return 0;
	}

	p = (uchar *)fullpath + strleng(fullpath) - 1;

	while((*p) != 0) {
		if((*p == '\\') || *p == '/') {
			if(filename != 0) {
				if(filename != 0) {
					(void)strncopy(filename, p+1, len);
				}
			}
			return p+1;
		}
		p--;
	}

	return 0;
}


/**
   @brief	ファイル拡張子を取得する

   @param[out]	拡張子文字列
   @param[in]	ファイル名
   @param[in]	最大拡張子文字列バイト数

   @return	拡張子文字列
*/
uchar * get_filename_extension(uchar *ext, const uchar *filename, unsigned int len)
{
	uchar *p;

	if(filename == 0) {
		return 0;
	}

	if(*filename == 0) {
		return 0;
	}

	p = (uchar *)filename + strleng(filename) - 1;

	while((*p) != 0) {
		if(*p == '.') {
			if(ext != 0) {
				if(ext != 0) {
					(void)strncopy(ext, p+1, len);
				}
			}
			return p+1;
		}
		p--;
	}

	return 0;
}

char * size2str(char *str, t_size size)
{
	t_size size_g = (1024L*1024*1024L);
	t_size size_m = (1024L*1024);
	t_size size_k = 1024L;

	if(size >= size_g) {
		if((size/size_g) < 10) {
			tsnprintf(str, SIZE_STR_LEN, "%1d.%1dG", (int)(size/size_g), (int)((size-((size/size_g))*size_g))/1024/1024/100);
		} else {
			tsnprintf(str, SIZE_STR_LEN, "%3dG", (int)(size/size_g));
		}
	} else if(size >= size_m) {
		if((size/size_m) < 10) {
			tsnprintf(str, SIZE_STR_LEN, "%1d.%1dM", (int)(size/size_m), (int)((size-((size/size_m))*size_m))/1024/100);
		} else {
			tsnprintf(str, SIZE_STR_LEN, "%3dM", (int)(size/size_m));
		}
	} else if(size >= size_k) {
		if((size/size_k) < 10) {
			tsnprintf(str, SIZE_STR_LEN, "%1d.%1dK", (int)(size/size_k), (int)((size-((size/size_k))*size_k))/100);
		} else {
			tsnprintf(str, SIZE_STR_LEN, "%3dK", (int)(size/size_k));
		}
	} else {
		tsnprintf(str, SIZE_STR_LEN, "%3dB", (int)size);
	}

	return &str[0];
}
