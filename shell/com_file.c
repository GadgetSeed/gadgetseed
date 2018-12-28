/** @file
    @brief	ファイル操作コマンド

    @date	2007.07.14
    @author	Takashi SHUDO

    @section file_command fileコマンド

    file コマンドには以下のサブコマンドがあります。

    | サブコマンド	| 機能				| 詳細			|
    |:------------------|:------------------------------|:----------------------|
    | mount		| @copybrief com_file_mount	| @ref com_file_mount	|
    | umount		| @copybrief com_file_umount	| @ref com_file_umount	|
    | free		| @copybrief com_file_diskfree	| @ref com_file_diskfree |
    | dir		| @copybrief com_file_dir	| @ref com_file_dir	|
    | delete		| @copybrief com_file_delete	| @ref com_file_delete	|
    | fdump		| @copybrief com_file_fdump	| @ref com_file_fdump	|
    | operation		| @copybrief com_file_operation	| @ref com_file_operation |
    | batch		| @copybrief com_file_batch	| @ref com_file_batch	|
*/

#include "sysconfig.h"
#include "shell.h"
#include "device.h"
#include "fs.h"
#include "str.h"
#include "tprintf.h"
#include "tkprintf.h"

#include "storage.h"
#include "file.h"
#include "charcode.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


static char *fsize2str(t_size size)
{
	static char szstr[SIZE_STR_LEN+1];

	return size2str(szstr, size);
}

static char *fsize2lstr(unsigned int size)
{
#define FSIZE_LSTR_LEN	10
	static char szstr[FSIZE_LSTR_LEN+1];

	tsnprintf(szstr, FSIZE_LSTR_LEN+1, "%10d", size);

	return &szstr[0];
}

#include "datetime.h"

static unsigned int now_time;

static char *fdate2str(t_time datetime)
{
#define FDATA_STR_LEN	((unsigned int)sizeof("HH:MM:DD"))
	static char dtstr[FDATA_STR_LEN+1];
	static t_time now_time;
	struct st_systime unixtime;
	struct st_datetime dt;

	unixtime.sec = datetime;
	unixtime.usec = 0;
	unixtime_to_datetime(&dt, &unixtime);

	if((datetime/(24*60*60)) == (now_time/(24*60*60))) {
		tsprintf((char *)dtstr, "%02d:%02d:%02d",
			 dt.hour,
			 dt.min,
			 dt.sec);
	} else {
		tsprintf((char *)dtstr, "%02d/%02d/%02d",
			 dt.year - 2000,
			 dt.month,
			 dt.day);
	}

	return &dtstr[0];
}

static char *fdate2lstr(t_time datetime)
{
#define FDATA_LSTR_LEN	((unsigned int)sizeof("YYYY/MM/DD HH:MM:DD"))
	static char dtstr[FDATA_LSTR_LEN+1];
	struct st_systime unixtime;
	struct st_datetime dt;

	unixtime.sec = datetime;
	unixtime.usec = 0;
	unixtime_to_datetime(&dt, &unixtime);

	tsnprintf(dtstr, FDATA_LSTR_LEN, "%04d/%02d/%02d %02d:%02d:%02d",
		  dt.year,
		  dt.month,
		  dt.day,
		  dt.hour,
		  dt.min,
		  dt.sec);

	return &dtstr[0];
}

static void print_fresult(char *path, FRESULT fr)
{
	tprintf("%s ", path);

	switch(fr) {
	case FR_OK:		/* 0 */
		tprintf("OK\n");
		break;

	case FR_DISK_ERR:	/* 1 */
		tprintf("disk error\n");
		break;

	case FR_INT_ERR:	/* 2 */
		tprintf("int error\n");
		break;

	case FR_NOT_READY:	/* 3 */
		tprintf("not ready\n");
		break;

	case FR_NO_FILE:	/* 4 */
		tprintf("no file\n");
		break;

	case FR_NO_PATH:	/* 5 */
		tprintf("no path\n");
		break;

	case FR_INVALID_NAME:	/* 6 */
		tprintf("invalid name\n");
		break;

	case FR_DENIED:		/* 7 */
		tprintf("denied\n");
		break;

	case FR_EXIST:		/* 8 */
		tprintf("exist\n");
		break;

	case FR_INVALID_OBJECT:	/* 9 */
		tprintf("invalid object\n");
		break;

	case FR_WRITE_PROTECTED:/* 10 */
		tprintf("Write protected\n");
		break;

	case FR_INVALID_DRIVE:	/* 11 */
		tprintf("invalid drive\n");
		break;

	case FR_NOT_ENABLED:	/* 12 */
		tprintf("not enabled\n");
		break;

	case FR_NO_FILESYSTEM:	/* 13 */
		tprintf("no filesystem\n");
		break;

	case FR_MKFS_ABORTED:	/* 14 */
		tprintf("mkfs aborted\n");
		break;

	case FR_TIMEOUT:	/* 15 */
		tprintf("timeout\n");
		break;

	case FR_LOCKED:		/* 16 */
		tprintf("locked\n");
		break;

	case FR_NOT_ENOUGH_CORE:/* 17 */
		tprintf("not enough core\n");
		break;

	case FR_TOO_MANY_OPEN_FILES:/* 18 */
		tprintf("too many open files\n");
		break;

	case FR_INVALID_PARAMETER:/* 19 */
		tprintf("invalid parameter\n");
		break;

	default:
		tprintf("undifined error(%d)\n", fr);
		break;
	}
}

static unsigned int	acc_size;
static unsigned short	acc_files;
static unsigned short	acc_dirs;

static int scan_files(unsigned char* path, unsigned int len)
{
	FS_DIR *dir;
	FS_FILEINFO finfo;
	int res = 0;
	int i;

	dir = opendir_file(path);
	if(dir != 0) {
		i = strleng(path);
		while(((res = readdir_file(dir, &finfo)) == FR_OK) &&
		      finfo.fname[0]) {
			if((finfo.fattrib & AM_DIR) != 0) {
				acc_dirs++;
				*(path+i) = '/';
				(void)strncopy(path+i+1, (unsigned char *)&finfo.fname[0], len);
				res = scan_files(path, len - i -1);
				*(path+i) = '\0';
				if (res != FR_OK) break;
			} else {
				acc_files++;
				acc_size += finfo.fsize;
			}
		}

		closedir_file(dir);
	}

	return res;
}

#include "str.h"

static int mount(int argc, uchar *argv[]);

/**
   @brief	デバイスをマウントする
*/
static const struct st_shell_command com_file_mount = {
	.name		= "mount",
	.command	= mount,
	.usage_str	= "[<drive> <device_name> [fsname]]"
};

static int mount(int argc, uchar *argv[])
{
	char *devname;
	char *fsname;

	if(argc > 3) {
		int drv = dstoi(argv[1]);
		if(mount_storage(drv, (char *)argv[2], (char *)argv[3]) == 0) {
			tprintf("Drive %d: %s %s\n", drv, argv[2], argv[3]);
		}
	} else if(argc > 2) {
		int drv = dstoi(argv[1]);
		if(mount_storage(drv, (char *)argv[2], FSNAME_VFAT) == 0) {
			tprintf("Drive %d: %s\n", drv, argv[2]);
		}
	} else {
		int i;
		for(i=0; i<GSC_FS_VOLUME_NUM; i++) {
			if(get_storage_device_name(i, &devname, &fsname) == 0) {
				tprintf("%d: %8s %s\n", i, devname, fsname);
			}
		}
	}

	return 0;
}


static int umount(int argc, uchar *argv[]);

/**
   @brief	デバイスをアンマウントする
*/
static const struct st_shell_command com_file_umount = {
	.name		= "umount",
	.command	= umount,
	.usage_str	= "<drive>"
};

static int umount(int argc, uchar *argv[])
{
	int rt = 0;
	int drv;

	if(argc > 1) {
		drv = dstoi(argv[1]);
		rt = unmount_storage(drv);
	} else {
		print_command_usage(&com_file_umount);
	}

	return rt;
}



static char defdrive[FF_MAX_LFN+1] = "0:/";
#define FSECTSIZE	FF_MIN_SS

static int diskfree(int argc, uchar *argv[]);

/**
   @brief	ドライブの空き容量を表示する
*/
static const struct st_shell_command com_file_diskfree = {
	.name		= "free",
	.command	= diskfree,
	.usage_str	= "[drive]"
};

static int diskfree(int argc, uchar *argv[])
{
	FATFS *fs;
	int fr;
	long unsigned int numcl;
	unsigned char *path = (unsigned char *)defdrive;

	if(argc > 1) {
		path = (unsigned char *)argv[1];
	}

	tprintf("Disk %s\n", (char *)path);

	acc_size = acc_files = acc_dirs = 0;

	fr = getfree_file(path, &numcl, (void **)&fs);
	if(fr == -1) {
		tprintf("Not supported\n");
		return 0;
	}
	if(fr) {
		print_fresult((char *)path, fr);
		return 0;
	}

	if(fs == 0) {
		tprintf("getfree_file() fs = 0\n");
		return 0;
	}

	tprintf("FAT type                : %d\n"
		"Bytes/Cluster           : %d\n"
		"Number of FATs          : %d\n"
		"Root DIR entries        : %d\n"
		"Sectors/FAT             : %d\n"
		"FAT start (lba)         : %d\n"
		"DIR start (lba,clustor) : %d\n"
		"Data start (lba)        : %d\n",
		(unsigned int)fs->fs_type,
		(unsigned int)fs->csize * FSECTSIZE,
		(unsigned int)fs->n_fats,
		(unsigned int)fs->n_rootdir,
		(unsigned int)fs->fsize,
		(unsigned int)fs->fatbase,
		(unsigned int)fs->dirbase,
		(unsigned int)fs->database
		);

	fr = scan_files(path, FF_MAX_LFN);
	if(fr) {
		print_fresult((char *)path, fr);
		return 0;
	}

	tprintf("%d files, %u bytes (%s)\n"
		"%d folders\n"
		"%s available\n",
		(int)acc_files,
		acc_size,
		fsize2str(acc_size),
		(int)acc_dirs,
		fsize2str((unsigned int)numcl * fs->csize * FSECTSIZE));

	return 0;
}

#if FF_USE_MKFS != 0
static int format(int argc, uchar *argv[]);

/**
   @brief	ドライブをフォマットする
*/
static const struct st_shell_command com_format = {
	.name		= "format",
	.command	= format,
	.usage_str	= "<drive>"
};

static int format(int argc, uchar *argv[])
{
	int fr;
	char path[4];

	if(argc < 2) {
		print_command_usage(&com_format);
		return 0;
	}

	tprintf("Formatting drive %s...", argv[1]);
	fr = mkfs_file((unsigned char *)argv[1], FM_FAT32, FSECTSIZE);
	if(fr) {
		tprintf("\n");
		tsnprintf(path, 3, "%s:", argv[1]);
		print_fresult(path, fr);
		return 0;
	} else {
		tprintf("\nDone\n");
	}

	return 0;
}
#endif

#if FF_USE_CHMOD != 0
static int chmod(int argc, uchar *argv[]);

/**
   @brief	ファイル属性を設定する
*/
static const struct st_shell_command com_chmod = {
	.name		= "chmod",
	.command	= chmod,
	.usage_str	= "<file> <mode(0x01=RDO,0x02=HID,0x04=SYS,0x10=DIR,0x20=ARC)>"
};

static int chmod(int argc, uchar *argv[])
{
	int fr;
	unsigned char mode = 0;

	if(argc < 2) {
		print_command_usage(&com_chmod);
		return 0;
	}

	mode = hstoi(argv[2]);
	fr = chmod_file(argv[1], mode);
	if(fr) {
		print_fresult((char *)argv[1], fr);
	}

	return 0;
}
#endif

static int dir(int argc, uchar *argv[]);

/**
   @brief	ドライブのファイルリストを表示する
*/
static const struct st_shell_command com_file_dir = {
	.name		= "dir",
	.command	= dir,
	.usage_str	= "[path]"
};

static int dir(int argc, uchar *argv[])
{
	int fr;
	FS_DIR *dir;
	FS_FILEINFO finfo;
	unsigned long p1;
	unsigned short s1, s2;
	FATFS *fs;
	unsigned char *path = (unsigned char *)defdrive;
	uchar *str;
	unsigned char cstr[FF_MAX_LFN + 1];

	str = finfo.fname;
	finfo.fsize = FF_MAX_LFN;

	now_time = fattime();

	if(argc > 1) {
		path = (unsigned char *)argv[1];
	}

	dir = opendir_file(path);
	if(dir == 0) {
		tprintf("Cannot open \"%s\"\n", path);
		return 0;
	}

	p1 = 0;
	s1 = 0;
	s2 = 0;

	for(;;) {
		fr = readdir_file(dir, &finfo);
		if((fr != FR_OK) || !finfo.fname[0]) {
			print_fresult((char *)path, fr);
			break;
		}
		if((finfo.fattrib & AM_DIR) != 0) {
			s2++;
		} else {
			s1++;
			p1 += finfo.fsize;
		}
		(void)sjisstr_to_utf8str(cstr, (uchar *)str, FF_MAX_LFN);
		XDUMP(0x01, (unsigned char *)str, strleng((uchar *)str));
		XDUMP(0x01, (unsigned char *)cstr, strleng(cstr));
		tprintf("%c", ((finfo.fattrib & AM_DIR) != 0) ? 'D' : '-');
		tprintf(" %s %s  %s",
			fdate2str(finfo.fdatetime),
			((finfo.fattrib & AM_DIR) != 0) ? "    " : fsize2str(finfo.fsize),
			cstr);
		tprintf("\n");
	}

	tprintf("%4d File  %s\n", (int)s1, fsize2str(p1));
	tprintf("%4d Dir", (int)s2);
	if(getfree_file(path, &p1, (void **)&fs) == FR_OK) {
		tprintf("   %s free\n",
			fsize2str((p1 * fs->csize/2)*1024));
	} else {
		tprintf("\n");
	}

	closedir_file(dir);

	return 0;
}


static int dirv(int argc, uchar *argv[]);

/**
   @brief	ドライブのファイルリスト詳細を表示する
*/
static const struct st_shell_command com_file_dirv = {
	.name		= "dirv",
	.command	= dirv,
	.usage_str	= "[path]"
};

static int dirv(int argc, uchar *argv[])
{
	int fr;
	FS_DIR *dir;
	FS_FILEINFO finfo;
	unsigned long p1;
	unsigned short s1, s2;
	FATFS *fs;
	unsigned char *path = (unsigned char *)defdrive;
	uchar *str;
	unsigned char cstr[FF_MAX_LFN + 1];

	str = finfo.fname;
	finfo.fsize = FF_MAX_LFN;

	now_time = fattime();

	if(argc > 1) {
		path = (unsigned char *)argv[1];
	}

	dir = opendir_file(path);
	if(dir == 0) {
		tprintf("Cannot open \"%s\"\n", path);
		return 0;
	}

	p1 = 0;
	s1 = 0;
	s2 = 0;

	for(;;) {
		fr = readdir_file(dir, &finfo);
		if((fr != FR_OK) || !finfo.fname[0]) {
			print_fresult((char *)path, fr);
			break;
		}
		if((finfo.fattrib & AM_DIR) != 0) {
			s2++;
		} else {
			s1++;
			p1 += finfo.fsize;
		}
		(void)sjisstr_to_utf8str(cstr, (uchar *)str, FF_MAX_LFN);
		XDUMP(0x01, (unsigned char *)str, strleng((uchar *)str));
		XDUMP(0x01, (unsigned char *)cstr, strleng(cstr));
		tprintf("%c%c%c%c%c",
			((finfo.fattrib & AM_DIR) != 0) ? 'D' : '-',
			((finfo.fattrib & AM_RDO) != 0) ? 'R' : '-',
			((finfo.fattrib & AM_HID) != 0) ? 'H' : '-',
			((finfo.fattrib & AM_SYS) != 0) ? 'S' : '-',
			((finfo.fattrib & AM_ARC) != 0) ? 'A' : '-');
		tprintf(" %s %s %s %s",
			fdate2lstr(finfo.fdatetime),
			((finfo.fattrib & AM_DIR) != 0) ? "    " : fsize2str(finfo.fsize),
			((finfo.fattrib & AM_DIR) != 0) ? "          " : fsize2lstr(finfo.fsize),
			cstr);
		tprintf("\n");
	}
	tprintf("%4d File  %s\n", (int)s1, fsize2str(p1));
	tprintf("%4d Dir", (int)s2);
	if(getfree_file(path, &p1, (void **)&fs) == FR_OK) {
		tprintf("   %s free\n",
			fsize2str((p1 * fs->csize/2)*1024));
	} else {
		tprintf("\n");
	}

	closedir_file(dir);

	return 0;
}


static int delete(int argc, uchar *argv[]);

/**
   @brief	ファイルを削除する
*/
static const struct st_shell_command com_file_delete = {
	.name		= "delete",
	.command	= delete,
	.usage_str	= "<file_name>"
};

static int delete(int argc, uchar *argv[])
{
	int fr;

	if(argc > 1) {
		fr = unlink_file((unsigned char *)argv[1]);

		if(fr) {
			print_fresult((char *)argv[1], fr);
		}
	} else {
		print_command_usage(&com_file_delete);
	}

	return 0;
}



#define FDSIZE	FSECTSIZE

static int fdump(int argc, uchar *argv[]);

/**
   @brief	ファイルの内容をダンプ表示する
*/
static const struct st_shell_command com_file_fdump = {
	.name		= "fdump",
	.command	= fdump,
	.usage_str	= "<file_name> [start [end]]"
};

static int fdump(int argc, uchar *argv[])
{
	int fd;
	unsigned char fdbuf[16];
	unsigned int st = 0;
	unsigned int ed = st + FDSIZE - 1;
	unsigned int dp;

	if(argc < 2) {
		print_command_usage(&com_file_fdump);
		return 0;
	}

	if(argc > 1) {
		fd = open_file((unsigned char *)argv[1], FA_READ);
		if(fd < 0) {
			tprintf("Cannot open \"%s\"\n", argv[1]);
			goto exit;
		}
	}

	//ed = seek_file(fd, 0, SEEK_END);

	if(argc > 2) {
		st = hstou(argv[2]);
		if(argc > 3) {
			ed = hstou(argv[3]);
			if(ed <= st) {
				goto exit;
			}
		}
	}

	if(st != 0) {
		(void)seek_file(fd, st, SEEK_SET);
	}

	for(dp=st; dp<=ed; dp+=16) {
		unsigned char *p;
		int i;
		int fr;
		uchar rd;
		int rlen = 16;

		if(((ed+1) - dp) < 16) {
			rlen = ((ed+1) - dp);
		}

		fr = read_file(fd, fdbuf, rlen);
		if(fr < 0) {
			tprintf("Cannot read \"%s\"\n", argv[1]);
			goto close;
		} else if(fr == 0) {
			goto close;
		}

		p=fdbuf;
		tprintf("%08X : ", dp);

		for(i=0; i<8; i++) {
			if(i < fr) {
				tprintf("%02X ", (int)*p);
			} else {
				tprintf("   ");
			}
			p++;
		}
		tprintf(" ");
		for(i=0; i<8; i++) {
			if(i < (fr-8)) {
				tprintf("%02X ", (int)*p);
			} else {
				tprintf("   ");
			}
			p++;
		}

		p=fdbuf;
		tprintf("  \"");

		for(i=0; i<16; i++) {
			if(i < fr) {
				if(((' ' <= *p) && (*p <= 'Z'))
				   || (('a' <= *p) && (*p <= 'z'))) {
					cputc(*p);
				} else {
					cputc('.');
				}
			} else {
				break;
			}
			p++;
		}
		tprintf("\"\n");

		if(cgetcnw(&rd) != 0) {
			if(rd == ASCII_CTRL_C) {
				tprintf("Abort.\n");
				goto close;
			}
		}
	}

close:
	close_file(fd);
exit:
	return 0;
}


static int fsize(int argc, uchar *argv[]);

/**
   @brief	ファイルのサイズを表示する
*/
static const struct st_shell_command com_file_fsize = {
	.name		= "fsize",
	.command	= fsize,
	.usage_str	= "<file_name>"
};

static int fsize(int argc, uchar *argv[])
{
	int fd;
	int filesize;

	if(argc < 2) {
		print_command_usage(&com_file_fdump);
		return 0;
	}

	fd = open_file((unsigned char *)argv[1], FA_READ);
	if(fd < 0) {
		tprintf("Cannot open \"%s\"\n", argv[1]);
		goto exit;
	}

	filesize = size_file(fd);
	tprintf("%d\n", filesize);

	close_file(fd);
exit:
	return 0;
}


static int operation(int argc, uchar *argv[]);

/**
   @brief	ファイルをアプリケーションで実行する
*/
static const struct st_shell_command com_file_operation = {
	.name		= "operation",
	.command	= operation,
	.usage_str	= "<file_name>"
};

static int operation(int argc, uchar *argv[])
{
	int rt = 0;

	if(argc > 1) {
		if(argc > 2) {
			rt = do_file_operation((unsigned char *)argv[1], argv[2]);
		} else {
			rt = do_file_operation((unsigned char *)argv[1], 0);
		}
	} else {
		print_command_usage(&com_file_operation);
	}

	return rt;
}


#include "batch.h"

static int batch(int argc, uchar *argv[]);

/**
   @brief	バッチファイルを実行する
*/
static const struct st_shell_command com_file_batch = {
	.name		= "batch",
	.command	= batch,
	.usage_str	= "<file_name>"
};

static int batch(int argc, uchar *argv[])
{
	int rt = 0;

	if(argc > 1) {
		exec_batch(argv[1]);
	} else {
		print_command_usage(&com_file_batch);
	}

	return rt;
}

//#define GSC_SHELL_USE_FWTEST
#ifdef GSC_SHELL_USE_FWTEST	// $gsc ファイル書き込みテストコマンド(fwtest)を有効にする
static int fwtest(int argc, uchar *argv[]);

/**
   @brief	ファイル作成書き込みテスト
*/
static const struct st_shell_command com_file_fwtest = {
	.name		= "fwtest",
	.command	= fwtest,
};

#define FWSIZE	1024
#define FWNAME	"fwtest.dat"

static int fwtest(int argc, uchar *argv[])
{
	int fd = 0;
	int rt = 0;
	int i;
	static unsigned char wbuf[FWSIZE];
	static const uchar *dfname = (uchar *)FWNAME;
	uchar *fname = (uchar *)dfname;

	for(i=0; i<FWSIZE; i++) {
		wbuf[i] = (i & 0xff);
	}
	//xdump(wbuf, FWSIZE);

	if(argc > 1) {
		fname = argv[1];
	}

	fd = open_file(fname, FA_WRITE | FA_CREATE_ALWAYS);
	if(fd < 0) {
		tprintf("File open error \"%s\"\n", fname);
		return -1;
	}
	rt = write_file(fd, wbuf, FWSIZE);
	if(rt != FWSIZE) {
		tprintf("File write error(%d)\n", rt);
		close_file(fd);
		return -1;
	}
	close_file(fd);

	return 0;
}
#endif


static const struct st_shell_command * const com_file_list[] = {
	&com_file_mount,
	&com_file_umount,
	&com_file_diskfree,
#if FF_USE_MKFS != 0
	&com_format,
#endif
#if FF_USE_CHMOD != 0
	&com_chmod,
#endif
	&com_file_dir,
	&com_file_dirv,
	&com_file_delete,
	&com_file_fdump,
	&com_file_fsize,
	&com_file_operation,
	&com_file_batch,
#ifdef GSC_SHELL_USE_FWTEST
	&com_file_fwtest,
#endif
	0
};

const struct st_shell_command com_file = {
	.name		= "file",
	.manual_str	= "File strage operation commands",
	.sublist	= com_file_list
}; ///< ファイル操作
