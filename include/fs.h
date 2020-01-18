/** @file
    @brief	ファイルシステムAPI

    @date 	2018.09.09
    @author	Takashi SHUDO
*/

#ifndef FS_H
#define FS_H

#include "str.h"
#include "datetime.h"
#include "device.h"

#define FSNAME_VFAT	"vfat"
#define FSNAME_PIPE	"pipe"
#define FSNAME_GSFFS	"gsffs"

#ifdef GSC_COMP_ENABLE_FATFS
#include "ff.h"
#if FF_USE_LFN
#define MAX_FNAME_LEN	FF_MAX_LFN
#else
#define MAX_FNAME_LEN	12
#endif
#else
#define MAX_FNAME_LEN	63
#endif

#if 1
typedef unsigned long long	t_size;
typedef long long		t_ssize;
#else
typedef unsigned int	t_size;
typedef int		t_ssize;
#endif

typedef struct {
	t_size	fsize;
	t_time	fdatetime;	///< Local time
	unsigned int fattrib;
	uchar	fname[MAX_FNAME_LEN + 1];
} FS_FILEINFO;	///< ファイル情報

typedef struct {
	struct st_filesystem *fs;
	union {
#ifdef GSC_COMP_ENABLE_FATFS
		DIR dir;
#endif
#ifdef GSC_COMP_ENABLE_PIPEFS
		int finfo_cnt;
#endif
#ifdef GSC_COMP_ENABLE_GSFFS
		int finfo_sector;
#endif
	} dir;
} FS_DIR;	///< ディレクトリ情報

struct st_filesystem {
	char *name;
	int (* mount)(int drvno, struct st_device *dev);
	int (* unmount)(int drvno, struct st_device *dev);

	void * (* open)(const uchar *path, int flags);
	t_ssize (* read)(void *fd, void *buf, t_size count);
	t_ssize (* write)(void *fd, const void *buf, t_size count);
	t_ssize (* seek)(void *fd, t_ssize offset, int whence);
	t_size (* tell)(void *fd);
	t_ssize (* size)(void *fd);
	int (* close)(void *fd);
	FS_DIR * (* opendir)(const uchar *name);
	int (* readdir)(FS_DIR *dir, FS_FILEINFO *finfo);
	int (* closedir)(FS_DIR *dir);
	int (* stat)(const uchar *path, FS_FILEINFO *finfo);
	int (* getfree)(const uchar *path, unsigned long *sect, void **fso);
	int (* sync)(void *fd);
	int (* unlink)(const uchar *path);
	int (* mkdir)(const uchar *path);
	int (* chmod)(const uchar *path, unsigned char flag);
	int (* rename)(const uchar *oldpath, const uchar *newpath);
	int (* mkfs)(const uchar *path, unsigned char part, unsigned short alloc);
}; ///< ファイルシステム構造体

extern struct st_filesystem * search_filesystem(const char *name);

#endif // FS_H
