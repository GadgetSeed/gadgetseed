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
#include "str.h"
#include "tprintf.h"
#include "tkprintf.h"

#include "storage.h"
#include "file.h"
#include "charcode.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


static FILINFO	finfo;


static char *fsize2str(unsigned long size)
{
#ifdef SHORT_FSIZE_STR
#define FSIZE_STR_LEN	5
	static char szstr[FSIZE_STR_LEN+1];

	if(size >= 1024L*1024) {
		tsnprintf(szstr, FSIZE_STR_LEN+1, "%4ldM", size/(1024L*1024));
	} else if(size >= 1024) {
		tsnprintf(szstr, FSIZE_STR_LEN+1, "%4ldK", size/1024);
	} else {
		tsnprintf(szstr, FSIZE_STR_LEN+1, "%4ldB", size);
	}

	return szstr;
#else
#define FSIZE_STR_LEN	((unsigned int)sizeof("XXX.XM"))
	static char szstr[FSIZE_STR_LEN+1];

	if(size >= (1024L*1024)) {
		tsnprintf(szstr, FSIZE_STR_LEN, "%3ld.%1ldM", size/(1024L*1024), (size-((size/(1024L*1024)*(1024L*1024))))/100);
	} else if(size >= 1024) {
		tsnprintf(szstr, FSIZE_STR_LEN, "%3ld.%1ldK", size/1024, (size-((size/1024)*1024))/100);
	} else {
		tsnprintf(szstr, FSIZE_STR_LEN, "%5ldB", size);
	}

	return &szstr[0];
#endif
}

static char *fsize2lstr(unsigned long size)
{
#define FSIZE_LSTR_LEN	10
	static char szstr[FSIZE_LSTR_LEN+1];

	tsnprintf(szstr, FSIZE_LSTR_LEN+1, "%10ld", size);

	return &szstr[0];
}

#include "datetime.h"

static unsigned int now_time;

static char *fdate2str(WORD date, WORD time)
{
#define FDATA_STR_LEN	((unsigned int)sizeof("HH:MM:DD"))
	static char dtstr[FDATA_STR_LEN+1];

	if(date == (unsigned short)(now_time >> 16)) {
		tsnprintf(dtstr, FDATA_STR_LEN, "%02d:%02d:%02d",
			 (int)(time >> 11),
			 (int)(time >> 5) & 63,
			 (int)time & 63);
	} else {
		tsnprintf(dtstr, FDATA_STR_LEN, "%02d/%02d/%02d",
			 (int)(date >> 9) + 1980 - 2000,
			 (int)(date >> 5) & 15,
			 (int)date & 31);
	}

	return &dtstr[0];
}

static char *fdate2lstr(WORD date, WORD time)
{
#define FDATA_LSTR_LEN	((unsigned int)sizeof("YYYY/MM/DD HH:MM:DD"))
	static char dtstr[FDATA_LSTR_LEN+1];

	tsnprintf(dtstr, FDATA_LSTR_LEN, "%04d/%02d/%02d %02d:%02d:%02d",
		  (int)(date >> 9) + 1980,
		  (int)(date >> 5) & 15,
		  (int)date & 31,
		  (int)(time >> 11),
		  (int)(time >> 5) & 63,
		  (int)time & 63);

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

	default:
		tprintf("undifuned error\n");
		break;
	}
}

static DWORD	acc_size;
static WORD	acc_files;
static WORD	acc_dirs;

static int scan_files(unsigned char* path, unsigned int len)
{
	DIR *dirs;
	int res = 0;
	BYTE i;

	if((dirs = opendir_file(path))) {
		i = strleng(path);
		while(((res = readdir_file(dirs, &finfo)) == FR_OK) &&
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
	.usage_str	= "[<drive> <device_name>]"
};

static int mount(int argc, uchar *argv[])
{
	char *sp;

	if(argc > 2) {
		int drv = dstoi(argv[1]);
		if(mount_storage(drv, (char *)argv[2]) == 0) {
			tprintf("Drive %d: %s\n", drv, argv[2]);
		}
	} else {
		int i;
		for(i=0; i<FF_VOLUMES; i++) {
			if(get_storage_device_name(i, &sp) == 0) {
				tprintf("%d: %s\n", i, sp);
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
#define FSECTSIZE	512

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
	unsigned long numcl;
	unsigned char *path = (unsigned char *)defdrive;

	if(argc > 1) {
		path = (unsigned char *)argv[1];
	}

	tprintf("Disk %s\n", (char *)path);

	acc_size = acc_files = acc_dirs = 0;

	fr = getfree_file(path, &numcl, &fs);
	if(fr) {
		print_fresult((char *)path, fr);
		return 0;
	}

	if(fs == 0) {
		tprintf("getfree_file() fs = 0\n");
		return 0;
	}

	tprintf("FAT type                : %ld\n"
		"Bytes/Cluster           : %ld\n"
		"Number of FATs          : %ld\n"
		"Root DIR entries        : %ld\n"
		"Sectors/FAT             : %ld\n"
		"FAT start (lba)         : %ld\n"
		"DIR start (lba,clustor) : %ld\n"
		"Data start (lba)        : %ld\n",
		(DWORD)fs->fs_type,
		(DWORD)fs->csize * FSECTSIZE,
		(DWORD)fs->n_fats,
		(DWORD)fs->n_rootdir,
		(DWORD)fs->fsize,
		(DWORD)fs->fatbase,
		(DWORD)fs->dirbase,
		(DWORD)fs->database
		);

	fr = scan_files(path, FF_MAX_LFN);
	if(fr) {
		print_fresult((char *)path, fr);
		return 0;
	}

	tprintf("%d files, %d bytes\n"
		"%d folders\n"
		"%s available\n",
		(int)acc_files, (int)acc_size,
		(int)acc_dirs,
		fsize2str((DWORD)numcl * fs->csize * FSECTSIZE));

	return 0;
}

#if FF_USE_MKFS != 0
static int format(int argc, uchar *argv[]);

/**
   @brief	ドライブをフォマットする
*/
static const struct st_shell_command com_format = {
	.name		= "format",
	.commadn	= format,
	.usage_str	= "<drive>"
};

static int format(int argc, uchar *argv[])
{
	int fr;
	char path[4];

	if(argc < 2) {
		print_fresult(&com_format);
		return 0;
	}

	tprintf("Formatting drive %s...", argv[1]);
	fr = mkfs_file((unsigned char *)argv[1], 0, FSECTSIZE);
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
	DIR *dir;
	DWORD p1;
	WORD s1, s2;
	FATFS *fs;
	unsigned char *path = (unsigned char *)defdrive;
	char *str;
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
		tprintf(" %s %s  %s %s",
			fdate2str(finfo.fdate, finfo.ftime),
			((finfo.fattrib & AM_DIR) != 0) ? "      " : fsize2str(finfo.fsize),
			sj2utf8((uchar *)finfo.altname),
			cstr);
		tprintf("\n");
	}

	tprintf("%4d File  %s\n", (int)s1, fsize2str(p1));
	tprintf("%4d Dir", (int)s2);
	if(getfree_file(path, &p1, &fs) == FR_OK) {
		tprintf("   %s free\n",
			fsize2str((p1 * fs->csize/2)*1024));
	} else {
		tprintf("\n");
	}

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
	DIR *dir;
	DWORD p1;
	WORD s1, s2;
	FATFS *fs;
	unsigned char *path = (unsigned char *)defdrive;
	char *str;
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
		tprintf(" %s %s %s %s  \"%s\"",
			fdate2lstr(finfo.fdate, finfo.ftime),
			((finfo.fattrib & AM_DIR) != 0) ? "      " : fsize2str(finfo.fsize),
			((finfo.fattrib & AM_DIR) != 0) ? "          " : fsize2lstr(finfo.fsize),
			cstr,
			sj2utf8((uchar *)finfo.altname));
		tprintf("\n");
	}
	tprintf("%4d File  %s\n", (int)s1, fsize2str(p1));
	tprintf("%4d Dir", (int)s2);
	if(getfree_file(path, &p1, &fs) == FR_OK) {
		tprintf("   %s free\n",
			fsize2str((p1 * fs->csize/2)*1024));
	} else {
		tprintf("\n");
	}

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

	ed = seek_file(fd, 0, SEEK_END);

	if(argc > 2) {
		st = hstou(argv[2]);
		if(argc > 3) {
			ed = hstou(argv[3]);
			if(ed <= st) {
				goto exit;
			}
		}
	}

	(void)seek_file(fd, st, SEEK_SET);

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
		tprintf("File open error \"%s\"\n", FWNAME);
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
	&com_file_dir,
	&com_file_dirv,
	&com_file_delete,
	&com_file_fdump,
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
