/** @file
    @brief	ファイル関数

    GadgetSeed のファイルシステムは FatFs を使用しています。

    参考 : http://elm-chan.org/fsw/ff/00index_e.html

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
#include "str.h"
#include "tkprintf.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"

// ファイルディスクリプタはスタティックに確保
#ifndef GSC_FATFS_MAX_FILE_NUM
#define GSC_FATFS_MAX_FILE_NUM	2	///< $gsc FATFS最大ファイル数
#endif

static unsigned char flg_file[GSC_FATFS_MAX_FILE_NUM]; ///< ファイルディスクリプタフラグ
static FIL file[GSC_FATFS_MAX_FILE_NUM];	///< ファイルディスクリプタ

/**
   @brief	全てのファイルディスクリプタを初期化する
*/
void init_file(void)
{
	int i;

	for(i=0; i<GSC_FATFS_MAX_FILE_NUM; i++) {
		flg_file[i] = 0;
	}
}

/**
   @brief	ファイルを開く

   @param	path	ファイル名
   @param	flags	属性フラグ

   @return	ファイルディスクリプタ
*/
int open_file(const uchar *path, int flags)
{
	int i;
	FRESULT res;

	DTFPRINTF(0x01, "path = %s, flgas = %08X\n", path, flags);

	for(i=0; i<GSC_FATFS_MAX_FILE_NUM; i++) {
		if(flg_file[i] == 0) {
			res = f_open(&file[i], (char *)path, flags);
			if(res == FR_OK) {
				flg_file[i] = 1;
				return i;
			} else {
				//SYSERR_PRINT("cannot open file \"%s\" (%d)", path, res);
				return 0 - res;
			}
		}
	}

	SYSERR_PRINT("cannot open file \"%s\" (GSC_FATFS_MAX_FILE_NUM)\n", path);

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
int read_file(int fd, void *buf, unsigned int count)
{
	unsigned int i, rcount = count;
	FRESULT res;
	int rt = 0;
	UINT size;
	unsigned char *rp = buf;

	DTFPRINTF(0x02, "fd = %d, buf = %p, count = %ld\n", fd, buf, count);

	for(i=0; i<count; i+=ACCESS_SIZE) {
		//eprintf("read = %d\n", rcount > ACCESS_SIZE ? ACCESS_SIZE : rcount);
		//eprintf("p = %p\n", rp);
		res = f_read(&file[fd], rp, rcount > ACCESS_SIZE ? ACCESS_SIZE : rcount, &size);
		if(res == FR_OK) {
			//eprintf("size = %d\n", size);
			rcount -= size;
			rp += size;
			rt += size;
		} else {
			//eprintf("read_file error(size = %d, result = %d)\n",
			//	  size, res);
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
int write_file(int fd, const void *buf, unsigned int count)
{
	unsigned int i, rcount = count;
	int rt = 0;
	UINT size;
	unsigned char *wp = (unsigned char *)buf;

	DTFPRINTF(0x02, "fd = %d, buf = %p, count = %ld\n", fd, buf, count);

	for(i=0; i<count; i+=ACCESS_SIZE) {
		if(f_write(&file[fd], wp,
			   rcount > ACCESS_SIZE ? ACCESS_SIZE : rcount,
			   &size) == FR_OK) {
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
int seek_file(int fd, unsigned int offset, int whence)
{
	DTFPRINTF(0x02, "fd = %d, offset = %ld, whence = %d\n", fd, offset, whence);

	switch(whence) {
	case SEEK_SET:
		if(f_lseek(&file[fd], offset) == FR_OK) {
			return file[fd].fptr;
		}
		break;

	case SEEK_CUR:
		if(f_lseek(&file[fd], file[fd].fptr + offset) == FR_OK) {
			return file[fd].fptr;
		}
		break;

	case SEEK_END:
		if(f_lseek(&file[fd], f_size(&file[fd]) + offset) == FR_OK) {
			return file[fd].fptr;
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
int tell_file(int fd)
{
	DTFPRINTF(0x01, "fd = %d\n", fd);

	return file[fd].fptr;
}

/**
   @brief	ファイルを閉じる

   @param[in]	fd	ファイルディスクリプタ

   @return	0:成功
*/
int close_file(int fd)
{
	DTFPRINTF(0x01, "fd = %d\n", fd);

	if((0 <= fd) && (fd < GSC_FATFS_MAX_FILE_NUM)) {
		if(flg_file[fd] != 0) {
			flg_file[fd] = 0;
			return f_close(&file[fd]);
		}
	}

	SYSERR_PRINT("cannot close file (%d)\n", fd);

	return -1;
}

/**
   @brief	ディレクトリを開く

   @param[in]	name	ディレクトリ名

   @return	ディレクトリ
*/
DIR * opendir_file(const uchar *name)
{
	FRESULT res;
	static DIR dir;

	DTFPRINTF(0x01, "name = %s\n", name);

	/**
	 * 取り敢えず dir は固定で確保
	 * 複数の opendir_file() はコールできない
	 */

	res = f_opendir(&dir, (char *)name);
	if(res != FR_OK) {
		//SYSERR_PRINT("res = %d\n", res);
		return 0;
	}

	return &dir;
}

/**
   @brief	ディレクトリを読み出す

   @param[in]	dir	ディレクトリ
   @param[out]	info	ファイル情報

   @return	エラーコード
*/
int readdir_file(DIR *dir, FILINFO *info)
{
	DTFPRINTF(0x02, "dir = %p, info = %p\n", dir, info);

	return f_readdir(dir, info);
}

/**
   @brief	ファイルステータスを読み出す

   @param[in]	path	ファイル(ディレクトリ)名
   @param[out]	info	ファイル情報構造体ポインタ

   @return	エラーコード
*/
int stat_file(const uchar *path, FILINFO *info)
{
	DTFPRINTF(0x02, "path = %s, info = %p\n", path, info);

	return f_stat((char *)path, info);
}

/**
   @brief	論理ドライブの未使用クラスタ数を取得する

   @param[in]	path	ファイル(ドライブ)名
   @param[out]	sect
   @param[out]	fs

   @return	エラーコード
*/
int getfree_file(const uchar *path, unsigned long *sect, FATFS **fs)
{
	DTFPRINTF(0x02, "path = %s, sect = %p, fs = %p\n", path, sect, fs);

	return f_getfree((char *)path, sect, fs);
}

/**
   @brief	キャッシュされたデータをフラッシュする

   @param[in]	fd	ファイルディスクリプタ

   @return	エラーコード
*/
int sync_file(int fd)
{
	DTFPRINTF(0x02, "fd = %d\n", fd);

	return f_sync(&file[fd]);
}

/**
   @brief	ファイルを消去する

   @param[in]	path	ファイル名

   @return	エラーコード
*/
int unlink_file(const uchar *path)
{
	DTFPRINTF(0x02, "path = %s\n", path);

	return f_unlink((char *)path);
}

/**
   @brief	ディレクトリを作成する

   @param[in]	path	ディレクトリ名

   @return	エラーコード
*/
int mkdir_file(const uchar *path)
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
int chmod_file(const uchar *path, unsigned char flag)
{
	DTFPRINTF(0x02, "path = %s, flag = %d\n", path, flag);

	return f_chmod((char *)path, flag, flag);
}
#endif

/**
   @brief	ファイル/ディレクトリ名を変更する

   @param[in]	oldpath	変更されるファイル名
   @param[in]	newpath	変更後のファイル名

   @return	エラーコード
*/
int rename_file(const uchar *oldpath, const uchar *newpath)
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
int mkfs_file(const uchar *path, unsigned char part, unsigned short alloc)
{
	DTFPRINTF(0x02, "path = %s, part = %d, alloc = %d\n", ath, part, alloc);

	return f_mkfs((char *)path, part, alloc);
}
#endif

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
