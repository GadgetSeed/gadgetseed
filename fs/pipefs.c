/** @file
    @brief	パイプファイルシステムAPI

    @date	2018.09.17
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "device.h"
#include "str.h"
#include "fifo.h"
#include "tkprintf.h"
#include "fs.h"
#include "task/mutex.h"
#include "task/event.h"
#include "task/syscall.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"

#define	PIPE_TIMEOUT	5000	///< pipe read/write タイムアウト

#ifndef GSC_PIPEFS_MAX_PIPE_NUM
#define GSC_PIPEFS_MAX_PIPE_NUM	1	///< $gsc 最大パイプ数
#endif

#ifndef GSC_PIPEFS_MAX_BUF_COUNT
#define GSC_PIPEFS_MAX_BUF_COUNT	(1024*2)	///< $gsc パイプバッファサイズ
#endif

#define MAX_PIPE_OPEN_COUNT	3	// 1パイプでオープンできる最大数

#ifndef GSC_PIPEFS_MAX_DIR_NUM
#define GSC_PIPEFS_MAX_DIR_NUM	2	///< $gsc オープンできる最大ディレクトリ数
#endif

#define PIPE_NAME_LEN	8

static int flg_pipefs_mount = 0;
static int pipefs_drvno;

#define __ATTR_PIPEBUFFER  __attribute__ ((section(".extram"))) __attribute__ ((aligned (4)))
struct st_pipefs {
	char fname[8];
	struct st_mutex mutex;
	char we_name[8];
	struct st_event w_event;
	char re_name[8];
	struct st_event r_event;
	struct st_fifo fifo;
	unsigned char buf[GSC_PIPEFS_MAX_BUF_COUNT+1];
	int writer_num;
	int reader_num;
};

struct st_pipefs_file_desc {
	struct st_pipefs *pipefs;
	int flg_used;
	int flags;
};

static struct st_pipefs pipefs[GSC_PIPEFS_MAX_PIPE_NUM] __ATTR_PIPEBUFFER;
static struct st_pipefs_file_desc pipefs_file_desc[GSC_PIPEFS_MAX_PIPE_NUM][MAX_PIPE_OPEN_COUNT];

int mount_pipefs(int drvno, struct st_device *dev)
{
	int i, j;

	DTFPRINTF(0x01, "drvno = %d, dev = %p\n", drvno, dev);

	if(flg_pipefs_mount != 0) {
		SYSERR_PRINT("pipefs already mounted\n");
		return -1;
	}

	pipefs_drvno = drvno;

	for(j=0; j<GSC_PIPEFS_MAX_PIPE_NUM; j++) {
		struct st_pipefs *fs = &pipefs[j];

		tsnprintf(fs->fname, PIPE_NAME_LEN, "pipe%d", j);
		init_fifo(&(fs->fifo), fs->buf, GSC_PIPEFS_MAX_BUF_COUNT + 1);
		mutex_register_ISR(&(fs->mutex), fs->fname);
		tsnprintf(fs->we_name, PIPE_NAME_LEN, "w-pipe%d", j);
		eventqueue_register_ISR(&(fs->w_event), fs->we_name, 0, 0, 0);
		tsnprintf(fs->re_name, PIPE_NAME_LEN, "r-pipe%d", j);
		eventqueue_register_ISR(&(fs->r_event), fs->re_name, 0, 0, 0);
		fs->writer_num = 0;
		fs->reader_num = 0;
		for(i=0; i<MAX_PIPE_OPEN_COUNT; i++) {
			struct st_pipefs_file_desc *fd = &pipefs_file_desc[j][i];
			fd->pipefs = fs;
			fd->flg_used = 0;
			fd->flags = 0;
		}
	}

	return 0;
}

int unmount_pipefs(int drvno, struct st_device *dev)
{
	int i;

	DTFPRINTF(0x01, "drvno = %d, dev = %p\n", drvno, dev);

	if(flg_pipefs_mount == 0) {
		SYSERR_PRINT("pipefs not mounted\n");
		return -1;
	}

	for(i=0; i<GSC_PIPEFS_MAX_PIPE_NUM; i++) {
		struct st_pipefs *fs = &pipefs[i];

		eventqueue_unregister_ISR(&(fs->w_event));
		mutex_unregister_ISR(&(fs->mutex));
	}

	flg_pipefs_mount = 0;

	return 0;
}

/**
   @brief	ファイルを開く

   @param	path	ファイル名
   @param	flags	属性フラグ

   @return	ファイルディスクリプタ
*/
void * open_pipefs(const uchar *path, int flags)
{
	int i, j;
	char fname[10];

	DTFPRINTF(0x11, "path = %s, flags = %08X\n", path, flags);

	for(j=0; j<GSC_PIPEFS_MAX_PIPE_NUM; j++) {
		tsprintf(fname, "%d:%s", pipefs_drvno, pipefs[j].fname);
		if(strcomp((uchar *)fname, path) == 0) {
			for(i=0; i<MAX_PIPE_OPEN_COUNT; i++) {
				if(pipefs_file_desc[j][i].flg_used == 0) {
					struct st_pipefs_file_desc *pfd = &pipefs_file_desc[j][i];
					pfd->flg_used = 1;
					pfd->flags = flags;
					if(pfd->pipefs->writer_num == 0) {
						clear_fifo(&(pfd->pipefs->fifo));
					}
					if(flags & FA_WRITE) {
						pfd->pipefs->writer_num ++;
					}
					if(flags & FA_READ) {
						pfd->pipefs->reader_num ++;
					}
					DTFPRINTF(0x11, "fd->fs->writer_num = %d\n", pfd->pipefs->writer_num);
					DTFPRINTF(0x11, "fd->fs->reader_num = %d\n", pfd->pipefs->reader_num);
					return (void *)pfd;
				}
			}
		}
	}

	return 0;
}

/**
   @brief	ファイルを閉じる

   @param[in]	fd	ファイルディスクリプタ

   @return	0:成功
*/
int close_pipefs(void *fd)
{
	struct st_pipefs_file_desc *pfd;

	DTFPRINTF(0x11, "fd = %p\n", fd);

	pfd = fd;
	DTFPRINTF(0x11, "flags = %08X\n", pfd->flags);

	if(pfd->flags & FA_WRITE) {
		pfd->pipefs->writer_num --;
		if(pfd->pipefs->writer_num == 0) {
			DTFPRINTF(0x11, "event_wakeup w_event\n");
			event_wakeup(&(pfd->pipefs->w_event), 0);
			event_clear(&(pfd->pipefs->w_event));
		}
	}
	DTFPRINTF(0x11, "fd->fs->writer_num = %d\n", pfd->pipefs->writer_num);

	if(pfd->flags & FA_READ) {
		pfd->pipefs->reader_num --;
		if(pfd->pipefs->reader_num == 0) {
			DTFPRINTF(0x11, "event_wakeup w_event\n");
			event_wakeup(&(pfd->pipefs->r_event), 0);
			event_clear(&(pfd->pipefs->r_event));
		}
	}
	DTFPRINTF(0x11, "fd->fs->reader_num = %d\n", pfd->pipefs->reader_num);

	pfd->flg_used = 0;

	return 0;
}

/**
   @brief	ファイルからデータを読み出す

   @param[in]	int	ファイルディスクリプタ
   @param[out]	buf	読み出しデータポインタ
   @param[in]	count	読み出しバイト数

   @return	読み出しバイト数(<0:エラー)
*/
t_ssize read_pipefs(void *fd, void *buf, t_size count)
{
	struct st_pipefs_file_desc *pfd = fd;
	struct st_pipefs *fs = pfd->pipefs;
	t_size rtn = 0;
	int rt;
	int fsize;
	int timer = PIPE_TIMEOUT;

	DTFPRINTF(0x01, "fd = %p, buf = %p, count = %d\n", fd, buf, (int)count);

	while(1) {
		fsize = fifo_size(&(pfd->pipefs->fifo));
		DTFPRINTF(0x03, "fsize = %d\n", fsize);
		if(fsize < count) {
			DTFPRINTF(0x01, "event_wait\n");
			rt = event_wait(&(pfd->pipefs->w_event), 0, 10);
			DTFPRINTF(0x01, "event_wait wakeup %d\n", rt);
			fsize = fifo_size(&(pfd->pipefs->fifo));
			if(fsize == 0) {
				if(pfd->pipefs->writer_num == 0) {
					DTFPRINTF(0x01, "read_pipefs fifo_size()=0\n");
					return 0;	// EOF
				} else {
					task_sleep(10);
				}
			}
			if(rt < 0) {
				timer -= 10;
				if(timer <= 0) {
					DTFPRINTF(0x01, "read_pipefs timeout\n");
					return -1;
				}
			}
		} else {
			DTFPRINTF(0x01, "break\n");
			break;
		}
	}

	mutex_lock(&(fs->mutex), PIPE_TIMEOUT);

	rtn = read_fifo(&(pfd->pipefs->fifo), (unsigned char *)buf, count);

	mutex_unlock(&(fs->mutex));

	DTFPRINTF(0x01, "fd = %p, buf = %p, rtn = %d\n", fd, buf, (int)rtn);

	event_wakeup(&(pfd->pipefs->r_event), 0);

	return rtn;
}

/**
   @brief	ファイルにデータを書き込む

   @param[in]	fd	ファイルディスクリプタ
   @param[in]	buf	書き込みデータポインタ
   @param[in]	count	書き込みバイト数

   @return	書き込みバイト数(<0:エラー)
*/
t_ssize write_pipefs(void *fd, const void *buf, t_size count)
{
	struct st_pipefs_file_desc *pfd = fd;
	struct st_pipefs *fs = pfd->pipefs;
	t_size rtn = 0;
	int rt;
	int fsize;
	int timer = PIPE_TIMEOUT;

	DTFPRINTF(0x01, "fd = %p, buf = %p, count = %d\n", fd, buf, (int)count);

	while(1) {
		fsize = fifo_free_size(&(pfd->pipefs->fifo));
		DTFPRINTF(0x03, "free = %d\n", fsize);
		if(fsize < count) {
			DTFPRINTF(0x01, "event_wait\n");
			rt = event_wait(&(pfd->pipefs->r_event), 0, 10);
			DTFPRINTF(0x01, "event_wait wakeup %d\n", rt);
			fsize = fifo_free_size(&(pfd->pipefs->fifo));
			if(fsize < count) {
				if(pfd->pipefs->reader_num == 0) {
					DTFPRINTF(0x01, "read_pipefs fifo_free_size()=%d\n", fsize);
					return 0;
				} else {
					task_sleep(10);
				}
			}
			if(rt < 0) {
				timer -= 10;
				if(timer <= 0) {
					DTFPRINTF(0x01, "write_pipefs timeout\n");
					return 0;
				}
			}
		} else {
			DTFPRINTF(0x01, "break\n");
			break;
		}
	}

	mutex_lock(&(fs->mutex), PIPE_TIMEOUT);

	DTFPRINTF(0x01, "fifo.wp %p\n", pfd->pipefs->fifo.wp);
	rtn = write_fifo(&(pfd->pipefs->fifo), (unsigned char *)buf, count);

	mutex_unlock(&(fs->mutex));

	DTFPRINTF(0x01, "fd = %p, buf = %p, rtn = %d\n", fd, buf, (int)rtn);

	event_wakeup(&(pfd->pipefs->w_event), 0);

	return rtn;
}


t_ssize size_pipefs(void *fd)
{
	struct st_pipefs_file_desc *fs = fd;
	t_size rtn = 0;

	DTFPRINTF(0x02, "fd = %p\n", fd);

	rtn = fifo_size(&(fs->pipefs->fifo));

	return rtn;
}


static FS_DIR pipefs_dir_desc[GSC_PIPEFS_MAX_DIR_NUM];
extern const struct st_filesystem pipefs_fs;

/**
   @brief	ディレクトリを開く

   @param[out]	dir	ディレクトリデータ
   @param[in]	path	ディレクトリ名

   @return	エラーコード
*/
FS_DIR * opendir_pipefs(const uchar *path)
{
	int i;

	DTFPRINTF(0x01, "path = %s\n", path);

	for(i=0; i<GSC_PIPEFS_MAX_DIR_NUM; i++) {
		if(pipefs_dir_desc[i].fs == 0) {
			pipefs_dir_desc[i].dir.finfo_cnt = 0;
			pipefs_dir_desc[i].fs = (struct st_filesystem *)&pipefs_fs;
			return &pipefs_dir_desc[i];
		}
	}

	SYSERR_PRINT("cannot open dir \"%s\" (GSC_PIPEFS_MAX_DIR_NUM)\n", path);

	return 0;
}


/**
   @brief	ディレクトリを読み出す

   @param[in]	dir	ディレクトリ
   @param[out]	info	ファイル情報

   @return	エラーコード
*/
int readdir_pipefs(FS_DIR *dir, FS_FILEINFO *finfo)
{
	DTFPRINTF(0x01, "dir = %p, info = %p\n", dir, finfo);

	if(dir->dir.finfo_cnt < GSC_PIPEFS_MAX_PIPE_NUM) {
		finfo->fsize = fifo_size(&(pipefs_file_desc[dir->dir.finfo_cnt]->pipefs->fifo));
		finfo->fdatetime = 0;
		finfo->fattrib = 0;
		tsnprintf((char *)finfo->fname, MAX_FNAME_LEN, "%s", pipefs_file_desc[dir->dir.finfo_cnt]->pipefs->fname);
		dir->dir.finfo_cnt ++;
	} else {
		finfo->fname[0] = 0;
	}

	return 0;
}

/**
   @brief	ディレクトリを閉じる

   @param[in]	dir	ディレクトリ

   @return	エラーコード
*/
int closedir_pipefs(FS_DIR *dir)
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
int unlink_pipefs(const uchar *path)
{
	int j;
	char fname[10];

	DTFPRINTF(0x11, "path = %s\n", path);

	for(j=0; j<GSC_PIPEFS_MAX_PIPE_NUM; j++) {
		tsprintf(fname, "%d:%s", pipefs_drvno, pipefs[j].fname);
		if(strcomp((uchar *)fname, path) == 0) {
			struct st_pipefs_file_desc *pfd = &pipefs_file_desc[j][0];
			clear_fifo(&(pfd->pipefs->fifo));
		}
	}

	return 0;
}


const struct st_filesystem pipefs_fs = {
	.name		= FSNAME_PIPE,
	.mount		= mount_pipefs,
	.unmount	= unmount_pipefs,
	.open		= open_pipefs,
	.read		= read_pipefs,
	.write		= write_pipefs,
	.size		= size_pipefs,
	.close		= close_pipefs,
	.opendir	= opendir_pipefs,
	.readdir	= readdir_pipefs,
	.closedir	= closedir_pipefs,
	.unlink		= unlink_pipefs,
};
