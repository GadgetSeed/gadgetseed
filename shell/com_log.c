/** @file
    @brief	ログコマンド

    @date	2018.11.18
    @author	Takashi SHUDO

    @section log_command logコマンド

    log コマンドには以下のサブコマンドがあります。

    | サブコマンド	| 機能				| 詳細			|
    |:------------------|:------------------------------|:----------------------|
    | info		| @copybrief com_log_info	| @ref com_log_info	|
    | print		| @copybrief com_log_print	| @ref com_log_print	|
    | head		| @copybrief com_log_head	| @ref com_log_head	|
    | tail		| @copybrief com_log_tail	| @ref com_log_tail	|
    | recpri		| @copybrief com_log_recpri	| @ref com_log_recpri	|
    | disppri		| @copybrief com_log_disppri	| @ref com_log_disppri	|
    | time		| @copybrief com_log_time	| @ref com_log_time	|
    | disptime		| @copybrief com_log_disptime	| @ref com_log_disptime	|
    | save		| @copybrief com_log_save	| @ref com_log_save	|
*/

#include <math.h>

#include "sysconfig.h"
#include "shell.h"
#include "lineedit.h"
#include "console.h"
#include "str.h"
#include "tprintf.h"
#include "tkprintf.h"
#include "device.h"
#include "log.h"
#include "device/logbuf_ioctl.h"

#define DEFAULT_PRINT_LINE	20
#define MAX_LOG_DISP_BUF_SIZE	256
static unsigned char log_disp_buf[MAX_LOG_DISP_BUF_SIZE + 1];
static struct st_device *log_dev;
static struct st_loginfo loginfo;


/* info command */

static int log_info(int argc, uchar *argv[]);

/**
   @brief	ログ状態を表示
*/
static const struct st_shell_command com_log_info = {
	.name		= "info",
	.command	= log_info,
	.manual_str	= "Display log information"
};

static int log_info(int argc, uchar *argv[])
{
	ioctl_device(log_dev, IOCMD_LOGBUF_GET_INFO, 0, &loginfo);

	tprintf("Log size     : %u / %u (%u%%)\n", loginfo.logsize, loginfo.maxsize,
		(loginfo.logsize*100)/loginfo.maxsize);
	tprintf("Record count : %u\n", loginfo.record_count);
	tprintf("Start time   : %7u.%06u\n", (unsigned int)(loginfo.start_time/1000000), (unsigned int)(loginfo.start_time%1000000));
	tprintf("Last time    : %7u.%06u\n", (unsigned int)(loginfo.last_time/1000000), (unsigned int)(loginfo.last_time%1000000));

	return 0;
}


/* print command */

static void init_print(void);
static int log_print(int argc, uchar *argv[]);

static void print_log(int count)
{
	int i;

	for(i=0; i<count; i++) {
		int rtn = 0;
		unsigned char rd;
		int len = 0;
		if(cgetcnw(&rd)) {
			if(rd == ASCII_CTRL_C) {
				tprintf("\nAbort.\n");
				return;
			}
		}

		len = ioctl_device(log_dev, IOCMD_LOGBUF_GET_LINESIZE, 0, 0);
		rtn = read_device(log_dev, log_disp_buf, len);
		if(rtn != 0) {
			log_disp_buf[rtn] = 0;
			tprintf("%s", (char *)log_disp_buf);
		} else {
			break;
		}
	}
}

/**
   @brief	ログ内容を表示
*/
static const struct st_shell_command com_log_print = {
	.name		= "print",
	.init		= init_print,
	.command	= log_print,
	.usage_str	= "[size[TODO]]",
	.manual_str	= "Print log"
};

static void init_print(void)
{
	log_dev = open_device(DEF_DEV_NAME_LOGBUF);
	if(log_dev == 0) {
		SYSERR_PRINT("Cannot open device \"%s\"\n", DEF_DEV_NAME_LOGBUF);
	}
}

static int log_print(int argc, uchar *argv[])
{
	int count = DEFAULT_PRINT_LINE;

	if(argc > 1) {
		count = dstoi(argv[1]);
	}

	print_log(count);

	return 0;
}


/* head command */

static int log_head(int argc, uchar *argv[]);

/**
   @brief	ログの先頭から表示
*/
static const struct st_shell_command com_log_head = {
	.name		= "head",
	.command	= log_head,
	.usage_str	= "[line] [count]",
	.manual_str	= "Display log from head"
};

static int log_head(int argc, uchar *argv[])
{
	int line = 0;
	int count = DEFAULT_PRINT_LINE;

	if(argc > 1) {
		line = dstoi(argv[1]);
	}

	if(argc > 2) {
		count = dstoi(argv[2]);
	}

	ioctl_device(log_dev, IOCMD_LOGBUF_SEEK_HEAD, line, 0);

	print_log(count);

	return 0;
}


/* tail command */

static int log_tail(int argc, uchar *argv[]);

/**
   @brief	ログの末尾表示
*/
static const struct st_shell_command com_log_tail = {
	.name		= "tail",
	.command	= log_tail,
	.usage_str	= "[count]",
	.manual_str	= "Display log from tail"
};

static int log_tail(int argc, uchar *argv[])
{
	int count = DEFAULT_PRINT_LINE;

	if(argc > 1) {
		count = dstoi(argv[1]);
	}

	ioctl_device(log_dev, IOCMD_LOGBUF_SEEK_TAIL, count, 0);

	while(1) {
		int rtn = 0;
		int rsize = MAX_LOG_DISP_BUF_SIZE;
		unsigned char rd;
		if(cgetcnw(&rd)) {
			if(rd == ASCII_CTRL_C) {
				tprintf("\nAbort.\n");
				return 1;
			}
		}
		rtn = read_device(log_dev, log_disp_buf, rsize);
		if(rtn != 0) {
			log_disp_buf[rtn] = 0;
			tprintf("%s", (char *)log_disp_buf);
		} else {
			break;
		}
	}

	return 0;
}


/* recpri command */

static int log_recpri(int argc, uchar *argv[]);

/**
   @brief	ログ記録優先度を設定
*/
static const struct st_shell_command com_log_recpri = {
	.name		= "recpri",
	.command	= log_recpri,
	.usage_str	= "[priorty]",
	.manual_str	= "Set record log priorty"
};

static int log_recpri(int argc, uchar *argv[])
{
	if(argc < 2) {
		//print_command_usage(&com_log_disp);
		int pri = get_log_record_priority();
		tprintf("Priorty : %d\n", pri);
	} else {
		set_log_record_priority(dstoi(argv[1]));
	}

	return 0;
}


/* disppri command */

static int log_disppri(int argc, uchar *argv[]);

/**
   @brief	ログ表示優先度を設定
*/
static const struct st_shell_command com_log_disppri = {
	.name		= "disppri",
	.command	= log_disppri,
	.usage_str	= "[priorty]",
	.manual_str	= "Set display log priorty"
};

static int log_disppri(int argc, uchar *argv[])
{
	if(argc < 2) {
		//print_command_usage(&com_log_disp);
		int pri = get_log_display_priority();
		tprintf("Priorty : %d\n", pri);
	} else {
		set_log_display_priority(dstoi(argv[1]));
	}

	return 0;
}


/* time command */

static int log_time(int argc, uchar *argv[]);

/**
   @brief	ログ時間検索
*/
static const struct st_shell_command com_log_time = {
	.name		= "time",
	.command	= log_time,
	.usage_str	= "<time> [count]",
	.manual_str	= "Search log time & print"
};

static int log_time(int argc, uchar *argv[])
{
	unsigned int sec = 0;
	unsigned int usec = 0;
	unsigned long long stime = 0;
	int count = DEFAULT_PRINT_LINE;
	uchar *dotp;

	if(argc < 2) {
		print_command_usage(&com_log_time);
		return 0;
	}

	if(argc > 2) {
		count = dstoi(argv[2]);
	}

	dotp = strchar(argv[1], '.');
	if(dotp != 0) {
		int len = 0;
		*dotp = 0;
		len = strleng(dotp+1);
		if(len != 0) {
			usec = dstou(dotp+1) * pow(10, (6 - len));
		}
	}
	sec = dstou(argv[1]);
	stime = ((unsigned long long)sec * 1000000) + usec;
	//tprintf("%llu : %7u.%06u\n",
	//	stime,
	//	(unsigned int)(stime/1000000),
	//	(unsigned int)(stime%1000000));

	ioctl_device(log_dev, IOCMD_LOGBUF_SEEK_TIME, 0, &stime);

	print_log(count);

	return 0;
}


#ifdef GSC_COMP_ENABLE_FATFS
/* save command */

#include "file.h"

static int log_save(int argc, uchar *argv[]);

/**
   @brief	ログをファイルに保存
*/
static const struct st_shell_command com_log_save = {
	.name		= "save",
	.command	= log_save,
	.usage_str	= "<filename>",
	.manual_str	= "Save log to file"
};

static int log_save(int argc, uchar *argv[])
{
	int fd = 0;

	if(argc < 2) {
		print_command_usage(&com_log_save);
		return 0;
	}

	fd = open_file(argv[1], FA_WRITE | FA_CREATE_ALWAYS);
	if(fd < 0) {
		eprintf("Cannot open file \"%s\"\n", argv[1]);
		return 0;
	}

	ioctl_device(log_dev, IOCMD_LOGBUF_SEEK_HEAD, 0, 0);

	while(1) {
		int rtn = 0;
		int rsize = MAX_LOG_DISP_BUF_SIZE;
		unsigned char rd;
		if(cgetcnw(&rd)) {
			if(rd == ASCII_CTRL_C) {
				tprintf("\nAbort.\n");
				goto end;
			}
		}
		rtn = read_device(log_dev, log_disp_buf, rsize);
		if(rtn != 0) {
			write_file(fd, log_disp_buf, rtn);
			tprintf(".");
		} else {
			tprintf("\r\n");
			break;
		}
	}

end:

	close_file(fd);

	return 0;
}
#endif


/* disppri command */

static int log_disptime(int argc, uchar *argv[]);

/**
   @brief	ログタイムスタンプ表示を設定
*/
static const struct st_shell_command com_log_disptime = {
	.name		= "disptime",
	.command	= log_disptime,
	.usage_str	= "[0/1]",
	.manual_str	= "Set display log timestamp"
};

static int log_disptime(int argc, uchar *argv[])
{
	if(argc < 2) {
		//print_command_usage(&com_log_disp);
		int pri = get_logtimestamp_display();
		tprintf("Enable : %d\n", pri);
	} else {
		set_logtimestamp_display(dstoi(argv[1]));
	}

	return 0;
}


/*
 * log command
 */

static const struct st_shell_command * const com_log_list[] = {
	&com_log_info,
	&com_log_print,
	&com_log_head,
	&com_log_tail,
	&com_log_recpri,
	&com_log_disppri,
	&com_log_time,
#ifdef GSC_COMP_ENABLE_FATFS
	&com_log_save,
#endif
	&com_log_disptime,
	0
};

const struct st_shell_command com_log = {
	.name		= "log",	///< ログ操作コマンド
	.manual_str	= "log operation commands",
	.sublist	= com_log_list
}; ///< log表示
