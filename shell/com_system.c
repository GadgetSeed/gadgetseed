/** @file
    @brief	システムコマンド

    @date	2007.04.22
    @author	Takashi SHUDO

    @section sys_command sysコマンド

    sys コマンドには以下のサブコマンドがあります。

    | サブコマンド	| 機能				| 詳細			|
    |:------------------|:------------------------------|:----------------------|
    | info		| @copybrief com_sys_info	| @ref com_sys_info	|
    | timercheck	| @copybrief com_sys_timercheck	| @ref com_sys_timercheck	|
    | kerneltime	| @copybrief com_sys_kerneltime	| @ref com_sys_kerneltime	|
    | systime		| @copybrief com_sys_systime	| @ref com_sys_systime	|
    | datetime		| @copybrief com_sys_datetime	| @ref com_sys_datetime	|
    | setdate		| @copybrief com_sys_setdate	| @ref com_sys_setdate	|
    | rtcdate		| @copybrief com_sys_rtcdate	| @ref com_sys_rtcdate	|
    | datesync		| @copybrief com_sys_datesync	| @ref com_sys_datesync	|
    | rtcsync		| @copybrief com_sys_rtcsync	| @ref com_sys_rtcsync	|
    | event		| @copybrief com_sys_event	| @ref com_sys_event	|
    | setevent		| @copybrief com_sys_setevent	| @ref com_sys_setevent	|
    | sleep		| @copybrief com_sys_sleep	| @ref com_sys_sleep	|
    | console		| @copybrief com_sys_console	| @ref com_sys_console	|
    | sysmem		| @copybrief com_sys_sysmem	| @ref com_sys_sysmem	|
    | heap		| @copybrief com_sys_heap	| @ref com_sys_heap	|
    | reboot		| @copybrief com_sys_reboot	| @ref com_sys_reboot	|
    | random		| @copybrief com_sys_random	| @ref com_sys_random	|
    | interrupt		| @copybrief com_sys_interrupt	| @ref com_sys_interrupt	|
*/

#include "gadgetseed.h"
#include "shell.h"
#include "lineedit.h"
#include "console.h"
#include "timer.h"
#include "datetime.h"
#include "str.h"
#include "sysevent.h"
#include "key.h"
#include "memory.h"
#include "tprintf.h"
#include "system.h"
#include "task/syscall.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


/*
  システム情報
*/
static int system_info(int argc, uchar *argv[])
{
	extern const char os_version[];
	extern const char cpu_name[];
	extern const char arch_name[];
	extern const char system_name[];
	extern const char build_date[];
	extern const char build_time[];

	tprintf("Version    : %s\n", os_version);
	tprintf("CPU ARCH   : %s\n", arch_name);
	tprintf("CPU NAME   : %s\n", cpu_name);
	tprintf("SYSTEM     : %s\n", system_name);
	tprintf("Build date : %s %s\n", build_time, build_date);

	return 0;
}

/**
   @brief	システム情報を表示

   [使用例]

   : sys info
   Version    : 0.94\n
   CPU ARCH   : Cortex-M7\n
   CPU NAME   : STM32F769NIH6\n
   SYSTEM     : 32F769IDISCOVERY\n
   Build date : 22:50:34 Jun 20 2018\n
*/
static const struct st_shell_command com_sys_info = {
	.name		= "info",
	.command	= system_info,
	.manual_str	= "Print system info"
};

/*
  タイマーチェック
*/
static int timer_check_time(int argc, uchar *argv[])
{
	unsigned long long lktime = get_kernel_time();

	do {
		unsigned long long ktime = get_kernel_time();
		unsigned long long utime = get_system_utime();

		tprintf("Kernel = %16lld ", ktime);
		tprintf("System = %16lld.%03lld ", utime/1000, utime%1000);
		tprintf("diff = %8d %8lld\n", (int)(ktime - lktime), utime/1000 - ktime);

		if(cwait(1000) >= 0) {
			unsigned char rd;
			if(cgetcnw(&rd) != 0) {
				return 0;
			}
		}

		lktime = ktime;
	} while(1);

	return 0;
}

/**
   @brief	カーネルタイマとシステムタイマの差分を表示し続ける

   何らかのキー入力があればコマンドは停止します。
*/
static const struct st_shell_command com_sys_timercheck = {
	.name		= "timercheck",
	.command	= timer_check_time,
	.manual_str	= "Kerne timer & system time check"
};

/*
  カーネル時間
*/
static int com_kernel_time(int argc, uchar *argv[])
{
	unsigned long long ktime = get_kernel_time();

	tprintf("Kernel Time : %14lld (ms)\n", ktime);

	return 0;
}

/**
   @brief	カーネルタイマ値を表示

   [使用例]

   : sys kerneltime\n
   Kernel Time :       88504437 (ms)
*/
static const struct st_shell_command com_sys_kerneltime = {
	.name		= "kerneltime",
	.command	= com_kernel_time,
	.manual_str	= "Print kernel time"
};

static int sys_time(int argc, uchar *argv[])
{
	unsigned long long kutime = get_system_utime();

	tprintf("System Time : %14lld.%03lld (ms)\n", kutime/1000, kutime%1000);

	return 0;
}

/**
   @brief	システムタイマ値を表示

   [使用例]

   : sys timertime\n
   System Time :       89049644.538 (ms)
*/
static const struct st_shell_command com_sys_systime = {
	.name		= "systime",
	.command	= sys_time,
	.manual_str	= "Print System time"
};

/*
  日時表示
*/
static int datetime(int argc, uchar *argv[])
{
	struct st_systime systime;
	struct st_datetime datetime;
	char str[30];

	get_systime(&systime);
	systime_to_datetime(&datetime, &systime);
	datemtime_to_str(str, &datetime);

	tprintf("%s %s\n", str, GSC_TIMEZONE_STR);
	tprintf("UNIX time : %lld.%03d (sec)\n", systime.sec, systime.usec/1000);

	return 0;
}

/**
   @brief	日付時刻を表示

   [使用例]

   : sys datetime\n
   2018/06/24 Sun 11:07:38.567\n
   UNIX time : 1529806058.567 (sec)
*/
static const struct st_shell_command com_sys_datetime = {
	.name		= "datetime",
	.command	= datetime,
	.manual_str	= "Print datetime"
};

/*
  日時設定
*/
static int setdate(int argc, uchar *argv[]);

/**
   @brief	日付時刻を設定
*/
static const struct st_shell_command com_sys_setdate = {
	.name		= "setdate",
	.command	= setdate,
	.usage_str	= "<year> <month> <day> <hour> <min> <sec> [msec]>",
	.manual_str	= "Set datetime"
};

static int setdate(int argc, uchar *argv[])
{
	struct st_datetime time;
	struct st_systime systime;
	char str[30];

	if(argc < 7) {
		print_command_usage(&com_sys_setdate);
		return 0;
	}

	time.year	= dstoi(argv[1]);
	time.month	= dstoi(argv[2]);
	time.day	= dstoi(argv[3]);
	time.hour	= dstoi(argv[4]);
	time.min	= dstoi(argv[5]);
	time.sec	= dstoi(argv[6]);

	if(argc > 6) {
		time.msec = dstoi(argv[7]);
	} else {
		time.msec = 0;
	}

	time.dayofweek = date_to_dayofweek(time.year, time.month, time.day);

	systime.sec = datetime_to_utc(&time);
	systime.usec = time.msec * 1000;
	set_systime(&systime);

	datemtime_to_str(str, &time);

	tprintf("%s\n", str);

	return 0;
}

#ifdef GSC_DEV_ENABLE_RTC
/*
  RTCの時計を表示する
*/
static int rtcdate(int argc, uchar *argv[]);

/**
   @brief	RTCの時刻を表示
*/
static const struct st_shell_command com_sys_rtcdate = {
	.name		= "rtcdate",
	.command	= rtcdate,
	.manual_str	= "Print RTC datetime"
};

static int rtcdate(int argc, uchar *argv[])
{
	struct st_datetime rtctime;
	char str[DATEMTIME_STR_LEN];
	struct st_systime systime;
	struct st_systime rstime;
	long long rtime, stime, dtime, dsec, dmsec;
	int rt = 0;
	int flg_repeat = 0;
	unsigned int rpt_cnt = 0;

	if(argc > 1) {
		flg_repeat = 1;
		rpt_cnt = dstou(argv[1]);
	}

loop:
	rt = read_rtc_time(&rtctime);
	if(rt == 0) {
		datemtime_to_str(str, &rtctime);
		tprintf("RTC(UTC) : %s\n", str);
	} else {
		tprintf("Can not read RTC time(%d)\n", rt);
	}

	datetime_to_systime(&rstime, &rtctime);
	rtime = (((long long)rstime.sec) * 1000) + rstime.usec / 1000;
	DTPRINTF(0x01, "RTC : %lld\n", rtime);

	get_systime(&systime);
	systime_to_datetime(&rtctime, &systime);
	datemtime_to_str(str, &rtctime);

	tprintf("SYS(%s) : %s\nUNIX time : %11lld.%06d (sec)\n", GSC_TIMEZONE_STR, str,
		systime.sec, systime.usec);

	stime = (((long long)systime.sec) * 1000) + (systime.usec / 1000);
	DTPRINTF(0x01, "SYS : %lld\n", stime);

	dtime = stime - rtime;
	tprintf("Error : ");
	if(dtime < 0) {
		dsec = (-dtime)/1000;
		dmsec = (-dtime) % 1000;
		tprintf("(-)");
	} else {
		dsec = dtime/1000;
		dmsec = dtime % 1000;
		tprintf("(+)");
	}
	//tprintf("%12lld.%03lld (sec)\n", dsec, dmsec);
	tprintf("%12d.%03d (sec)\n", (int)dsec, (int)dmsec);

	if(flg_repeat != 0) {
		if(cwait(rpt_cnt) >= 0) {
			unsigned char rd;
			if(cgetcnw(&rd) != 0) {
				return 0;
			}
		}
		goto loop;
	}

	return 0;
}



/*
  時計をRTCから同期をとる
*/ 
static int datesync(int argc, uchar *argv[])
{
	struct st_datetime time;
	char str[30];

	sync_systime_from_rtc();
	read_rtc_time(&time);
	datemtime_to_str(str, &time);
	tprintf("Set systime from RTC(UTC) : %s\n", str);

	return 0;
}

/**
   @brief	日付時刻をRTCから同期する
*/
static const struct st_shell_command com_sys_datesync = {
	.name		= "datesync",
	.command	= datesync,
	.manual_str	= "System datetime sync from RTC"
};


/*
  RTCを時計から同期をとる
*/ 
static int rtcsync(int argc, uchar *argv[])
{
	struct st_systime systime;
	struct st_datetime datetime;
	char str[30];

	get_systime(&systime);
	unixtime_to_datetime(&datetime, &systime);

	sync_rtc_from_systime();

	datemtime_to_str(str, &datetime);
	tprintf("Set RTC from systime : %s\n", str);

	return 0;
}

/**
   @brief	RTCを日付時刻から同期する
*/
static const struct st_shell_command com_sys_rtcsync = {
	.name		= "rtcsync",
	.command	= rtcsync,
	.manual_str	= "RTC sync from system datetime"
};
#endif // GSC_DEV_ENABLE_RTC

/*
  イベント表示
*/
static void disp_key(unsigned short key)
{
	switch(key) {
	case KEY_HOME: tprintf("KEY_HOME");	break;
	case KEY_ESC: tprintf("KEY_ESC");	break;
	case KEY_ENTER: tprintf("KEY_ENTER");	break;

	case KEY_UP: tprintf("KEY_UP");	break;
	case KEY_DOWN: tprintf("KEY_DOWN");	break;
	case KEY_LEFT: tprintf("KEY_LEFT");	break;
	case KEY_RIGHT: tprintf("KEY_RIGHT");	break;

	case KEY_COMMA: tprintf("KEY_COMMA");	break;
	case KEY_PERIOD: tprintf("KEY_PERIOD");	break;
	case KEY_POWER: tprintf("KEY_POWER");	break;
	case KEY_PGUP: tprintf("KEY_PGUP");	break;
	case KEY_PGDN: tprintf("KEY_PGDN");	break;

	case KEY_F1: tprintf("KEY_F1");	break;
	case KEY_F2: tprintf("KEY_F2");	break;
	case KEY_F3: tprintf("KEY_F3");	break;
	case KEY_F4: tprintf("KEY_F4");	break;
	case KEY_F5: tprintf("KEY_F5");	break;
	case KEY_F6: tprintf("KEY_F6");	break;

	case KEY_R: tprintf("KEY_R");	break;
	case KEY_S: tprintf("KEY_S");	break;
	case KEY_P: tprintf("KEY_P");	break;
	case KEY_F: tprintf("KEY_F");	break;

//	case KEY_BLUE: tprintf("KEY_BLUE");	break;
//	case KEY_RED: tprintf("KEY_RED");	break;
//	case KEY_GREEN: tprintf("KEY_GREEN");	break;
//	case KEY_YELLOW: tprintf("KEY_YELLOW");	break;

	case KEY_0: tprintf("KEY_0");	break;
	case KEY_1: tprintf("KEY_1");	break;
	case KEY_2: tprintf("KEY_2");	break;
	case KEY_3: tprintf("KEY_3");	break;
	case KEY_4: tprintf("KEY_4");	break;
	case KEY_5: tprintf("KEY_5");	break;
	case KEY_6: tprintf("KEY_6");	break;
	case KEY_7: tprintf("KEY_7");	break;
	case KEY_8: tprintf("KEY_8");	break;
	case KEY_9: tprintf("KEY_9");	break;
	case KEY_F10: tprintf("KEY_F10");	break;
	case KEY_F11: tprintf("KEY_F11");	break;
	case KEY_F12: tprintf("KEY_F12");	break;
	case KEY_KPSLASH: tprintf("KEY_KPSLASH");	break;
	case KEY_KPASTERISC: tprintf("KEY_KPASTERISC");	break;
	case KEY_KPHYPHEN: tprintf("KEY_KPHYPHEN");	break;
	case KEY_KPPLUS: tprintf("KEY_KPPLUS");	break;
	case KEY_BACKSPACES: tprintf("KEY_BACKSPACES");	break;
	case KEY_SPACE: tprintf("KEY_SPACE");	break;
	case KEY_LSHIFT: tprintf("KEY_LSHIFT");	break;
	case KEY_KPPERIOD: tprintf("KEY_KPPERIOD");	break;
	case KEY_LEFTCTRL: tprintf("KEY_LEFTCTRL");	break;

	case KEY_GB_UP: tprintf("KEY_GB_UP");	break;
	case KEY_GB_DOWN: tprintf("KEY_GB_DOWN");	break;
	case KEY_GB_LEFT: tprintf("KEY_GB_LEFT");	break;
	case KEY_GB_RIGHT: tprintf("KEY_GB_RIGHT");	break;
	case KEY_GB_ESC: tprintf("KEY_GB_ECS");	break;
	case KEY_GB_ENTER: tprintf("KEY_GB_ENTER");	break;
	case KEY_GB_VOLUP: tprintf("KEY_GB_VOLUP");	break;
	case KEY_GB_VOLDOWN: tprintf("KEY_GB_VOLDOWN");	break;

	default:
		tprintf("??? %d", key);
		break;
	}
}

void display_event(struct st_sysevent *event)
{
	tprintf("[%8d] ", (int)event->when);

	switch(event->what) {
	case EVT_KEYDOWN:
		tprintf("EVT_KEYDOWN   %3d : ", event->arg);
		disp_key(event->arg);
		break;

	case EVT_KEYUP:
		tprintf("EVT_KEYUP     %3d : ", event->arg);
		disp_key(event->arg);
		break;

	case EVT_KEYDOWN_REPEAT:
		tprintf("EVT_KEYDOWN_R %3d : ", event->arg);
		disp_key(event->arg);
		break;

	case EVT_TOUCHSTART:
		tprintf("EVT_TOUCHSTART    : X=%4d Y=%4d",
			event->pos_x, event->pos_y);
		break;

	case EVT_TOUCHMOVE:
		tprintf("EVT_TOUCHMOVE     : X=%4d Y=%4d",
			event->pos_x, event->pos_y);
		break;

	case EVT_TOUCHEND:
		tprintf("EVT_TOUCHEND      : X=%4d Y=%4d",
			event->pos_x, event->pos_y);
		break;

	case EVT_POWEROFF:
		tprintf("EVT_POWEROFF  %3d : ", event->arg);
		break;

	default:
		tprintf("Unknown Event(%d)  %3d : ",
			event->what, event->arg);
		break;
	}
	tprintf("\n");
}

static int dispevent(int argc, uchar *argv[])
{
	struct st_sysevent event;
	uchar rd;

	while(1) {
		if(get_event(&event, 100) == 1) {
			display_event(&event);
#if 0
			if(event.arg == KEY_GB_ENTER) {
				break;
			}
#endif
		}

		if(cgetcnw(&rd)) {
			if(rd == 0x0d) {
				break;
			}
		}
	}

	return 0;
}

/**
   @brief	発生したイベントを表示する

   他のタスクで get_event() でイベントを取得していた場合、イベントが表示されない場合がある。
*/
static const struct st_shell_command com_sys_event = {
	.name		= "event",
	.command	= dispevent,
	.manual_str	= "Print sysevent"
};

static int setevent(int argc, uchar *argv[])
{
	struct st_sysevent event;
	uchar rd;

	tprintf("push any key(k:UP, j:DOWN, h:LEFT, l:RIGHT, [SPC]:SPACE, [ENT]:ENTER, [ESC]:exit\n");

	while(1) {
		task_sleep(1);
		if(cgetcnw(&rd)) {
			switch(rd) {
			case 0x1b:
				tprintf("exit\n");
				goto end;

			case 'k':
				event.arg = KEY_GB_UP;
				event.what = EVT_KEYDOWN;
				set_event(&event);
				event.what = EVT_KEYUP;
				set_event(&event);
				break;

			case 'j':
				event.arg = KEY_GB_DOWN;
				event.what = EVT_KEYDOWN;
				set_event(&event);
				event.what = EVT_KEYUP;
				set_event(&event);
				break;

			case 'h':
				event.arg = KEY_GB_LEFT;
				event.what = EVT_KEYDOWN;
				set_event(&event);
				event.what = EVT_KEYUP;
				set_event(&event);
				break;

			case 'l':
				event.arg = KEY_GB_RIGHT;
				event.what = EVT_KEYDOWN;
				set_event(&event);
				event.what = EVT_KEYUP;
				set_event(&event);
				break;

			case 0x0d:
			case 'n':
				event.arg = KEY_GB_ENTER;
				event.what = EVT_KEYDOWN;
				set_event(&event);
				event.what = EVT_KEYUP;
				set_event(&event);
				break;

			case ' ':
			case 'm':
				event.arg = KEY_GB_SPACE;
				event.what = EVT_KEYDOWN;
				set_event(&event);
				event.what = EVT_KEYUP;
				set_event(&event);
				break;

			default:
				break;
			}
		}
	}

end:
	return 0;
}

/**
   @brief	イベントを発生する

   キー入力によりイベントを発生する。\n
   入力キーと発生できるイベントは以下となる。

   k : EVT_KEYDOWN KEY_GB_UP, EVT_KEYUP KEY_GB_UP

   j : EVT_KEYDOWN KEY_GB_DOWN, EVT_KEYUP KEY_GB_DOWN

   h : EVT_KEYDOWN KEY_GB_LEFT, EVT_KEYUP KEY_GB_LEFT

   l : EVT_KEYDOWN KEY_GB_RIGHT, EVT_KEYUP KEY_GB_RIGHT

   SPACE : EVT_KEYDOWN KEY_GB_SPACE, EVT_KEYUP KEY_GB_SPACE

   ENTER : EVT_KEYDOWN KEY_GB_ENTER, EVT_KEYUP KEY_GB_ENTER

   ESC : EVT_KEYDOWN KEY_GB_ESC, EVT_KEYUP KEY_GB_ESC
*/
static const struct st_shell_command com_sys_setevent = {
	.name		= "setevent",
	.command	=  setevent,
	.manual_str	= "Set sysevent from key input"
};

static int cmd_sleep(int argc, uchar *argv[])
{
	unsigned int t;

	if(argc < 2) {
		return 0;
	}

	t = dstou(argv[1]);

	//wait_time(t);
	task_sleep(t);

	return 0;
}

/**
   @brief	task_sleep() でshellタスクを指定時間スリープする

   スリープ時間はミリ秒で指定する。
*/
static const struct st_shell_command com_sys_sleep = {
	.name		= "sleep",
	.command	= cmd_sleep,
	.usage_str	= "<sleep_time(ms)>",
	.manual_str	= "Sleep shell task"
};

extern struct st_tcb *run_task;

static int console(int argc, uchar *argv[])
{
	tprintf("in  : ");
	if(run_task->stdin_dev == 0) {
		tprintf("-\n");
	} else {
		tprintf("%s\n", run_task->stdin_dev->name);
	}

	tprintf("out : ");
	if(run_task->stdout_dev == 0) {
		tprintf("-\n");
	} else {
		tprintf("%s\n", run_task->stdout_dev->name);
	}

	tprintf("err : ");
	if(run_task->error_dev == 0) {
		tprintf("-\n");
	} else {
		tprintf("%s\n", run_task->error_dev->name);
	}

	return 0;
}

static int console_set(int argc, uchar *argv[])
{
	struct st_device *dev;

	if(argc > 1) {
		dev = open_device((char *)argv[1]);
		if(dev == 0) {
			tprintf("Cannot open device \"%s\".\n", argv[1]);
		} else {
			if(strcomp(argv[0], (uchar *)"in") == 0) {
				set_console_in_device(dev);
			} else if(strcomp(argv[0], (uchar *)"out") == 0) {
				set_console_out_device(dev);
			} else if(strcomp(argv[0], (uchar *)"err") == 0) {
				set_error_out_device(dev);
			} else {
				tprintf("Cannot set device %s\n", argv[0]);
			}
		}
	} else {
		if(strcomp(argv[0], (uchar *)"in") == 0) {
			dev = run_task->stdin_dev;
		} else if(strcomp(argv[0], (uchar *)"out") == 0) {
			dev = run_task->stdout_dev;
		} else if(strcomp(argv[0], (uchar *)"err") == 0) {
			dev = run_task->error_dev;
		} else {
			tprintf("Cannot set device %s\n", argv[0]);
			return 0;
		}
		tprintf("%3s : ", argv[0]);
		if(dev == 0) {
			tprintf("-\n");
		} else {
			tprintf("%s\n", dev->name);
		}
	}

	return 0;
}

static const struct st_shell_command com_sys_console_in = {
	.name		= "in",
	.command	= console_set,
	.usage_str	= "[device_name]",
	.manual_str	= "Set console in device"
};

static const struct st_shell_command com_sys_console_out = {
	.name		= "out",
	.command	= console_set,
	.usage_str	= "[device_name]",
	.manual_str	= "Set console out device"
};

static const struct st_shell_command com_sys_console_err = {
	.name		= "err",
	.command	= console_set,
	.usage_str	= "[device_name]",
	.manual_str	= "Set console error device"
};

static const struct st_shell_command * const com_sys_console_sub[] = {
	&com_sys_console_in,
	&com_sys_console_out,
	&com_sys_console_err,
	0
};

/**
   @brief	shellタスクのコンソールデバイスを表示、設定する
*/
static const struct st_shell_command com_sys_console = {
	.name		= "console",
	.command	= console,
	.usage_str	= "[in|out|err] [device_name]",
	.manual_str	= "Set console device",
	.sublist	= com_sys_console_sub
};

#ifndef GSC_TARGET_SYSTEM_EMU
static int sysmem(int argc, uchar *argv[])
{
	extern long *BSS_START;
	extern long *BSS_END;
	extern long *DATAROM_START;
	extern long *DATARAM_START;
	extern long *DATARAM_END;

	tprintf("DATAROM_START : %08lX\n", (unsigned long)DATAROM_START);
	tprintf("DATARAM_START : %08lX\n", (unsigned long)DATARAM_START);
	tprintf("DATARAM_END   : %08lX\n", (unsigned long)DATARAM_END);
	tprintf("BSS_START     : %08lX\n", (unsigned long)BSS_START);
	tprintf("BSS_END       : %08lX\n", (unsigned long)BSS_END);

	return 0;
}

/**
   @brief	システムメモリ状態を表示する
*/
static const struct st_shell_command com_sys_sysmem = {
	.name		= "sysmem",
	.command	= sysmem,
	.manual_str	= "Print system memory status"
};
#endif // GSC_TARGET_SYSTEM_EMU

#ifdef GSC_MEMORY_ENABLE_HEAP_MEMORY
static int disp_heap(int argc, uchar *argv[])
{
	int sk = 0;
	unsigned long total = heap_total_size();
	unsigned long free = heap_free_size();
	unsigned long use = total - free;

	if(argc > 1) {
		switch(argv[1][0]) {
		case 'k':
		case 'K':
			sk = 1;
			break;

		case 'm':
		case 'M':
			sk = 2;
			break;
		}
	}

	switch(sk) {
	case 2:
		tprintf("Total : %4ld M byte\n", total/1024/1024);
		tprintf("Use   : %4ld M byte\n", use/1024/1024);
		tprintf("Free  : %4ld M byte\n", free/1024/1024);
		break;

	case 1:
		tprintf("Total : %7ld K byte\n", total/1024);
		tprintf("Use   : %7ld K byte\n", use/1024);
		tprintf("Free  : %7ld K byte\n", free/1024);
		break;

	case 0:
	default:
		tprintf("Total : %10ld byte\n", total);
		tprintf("Use   : %10ld byte\n", use);
		tprintf("Free  : %10ld byte\n", free);
		break;
	}

	return 0;
}

/**
   @brief	ヒープメモリの状態を表示する
*/
static const struct st_shell_command com_sys_heap = {
	.name		= "heap",
	.command	= disp_heap,
	.usage_str	= "[k|m]",
	.manual_str	= "Print heap memory status"
};

#if 0
static int mem_alloc(int argc, uchar *argv[]);

static const struct st_shell_command com_mem_alloc = {
	.name		= "memalloc",
	.command	= mem_alloc,
	.usage_str	= "<size>",
	.manual_str	= "Memory alloc"
};

static int mem_alloc(int argc, uchar *argv[])
{
	unsigned char *p;
	unsigned int size;

	if(argc < 2) {
		print_command_usage(&com_mem_alloc);
		return 0;
	}

	size = dstou(argv[1]);
	p = alloc_memory(size);
	tprintf("Memory Size    = %d\n", size);
	tprintf("Memory pointer = 0x%p\n", p);

	return 0;
}


static int mem_free(int argc, uchar *argv[]);

static const struct st_shell_command com_mem_free = {
	.name		= "memfree",
	.command	= mem_free,
	.usage_str	= "<address>",
	.manual_str	= "Free memory"
};

static int mem_free(int argc, uchar *argv[])
{
	unsigned char *p;

	if(argc < 2) {
		print_command_usage(&com_mem_free);
		return 0;
	}

	p = (unsigned char *)(long)hstoi(argv[1]);
	free_memory(p);
	tprintf("Free pointer = 0x%08lX\n", (unsigned long)p);

	return 0;
}
#endif

#endif // GSC_MEMORY_ENABLE_HEAP_MEMORY

static int reboot(int argc, uchar *argv[])
{
	reset_system();

	return 0;
}

/**
   @brief	システムを再起動する
*/
static const struct st_shell_command com_sys_reboot = {
	.name		= "reboot",
	.command	= reboot,
	.manual_str	= "Reboot system"
};

#ifdef GSC_LIB_ENABLE_RANDOM
#include "random.h"

static int random(int argc, uchar *argv[])
{
	unsigned int r = (unsigned int)genrand_int32();

	tprintf("%u\n", r);

	return 0;
}

/**
   @brief	乱数を発生する

   この機能を使用するにはマクロ GSC_LIB_ENABLE_RANDOM を定義する必要があります。
*/
static const struct st_shell_command com_sys_random = {
	.name		= "random",
	.command	= random,
	.manual_str	= "Generate random value"
};
#endif

#ifndef GSC_TARGET_SYSTEM_EMU
#ifdef GSC_KERNEL_ENABLE_INTERRUPT_COUNT
#include "interrupt.h"

static int interrupt(int argc, uchar *argv[])
{
	int i;
	int count = 0;

	for(i=0; i<MAXVECT; i++) {
		count = get_interrupt_count(i);
		if(count != -1) {
			tprintf("%-3d : %d\n", i, count);
		}
	}

	return 0;
}

/**
   @brief	割り込みハンドラが登録されている割り込みの、割込発生回数を表示する

   この機能を使用するにはマクロ GSC_KERNEL_ENABLE_INTERRUPT_COUNT を定義する必要があります。
*/
static const struct st_shell_command com_sys_interrupt = {
	.name		= "interrupt",
	.command	= interrupt,
	.manual_str	= "Print interrupt count"
};
#endif
#endif

/*
  ヘルプメッセージ表示
*/
extern const struct st_shell_command *com_list[];

static void print_command_manual(const struct st_shell_command *cp, int indent)
{
	while(indent) {
		tprintf("  ");
		indent --;
	}
	tprintf("%12s : ", cp->name);
	if(cp->manual_str != 0) {
		tprintf("%s\n", cp->manual_str);
	} else {
		tprintf("\n");
	}
}

static void print_command_list(const struct st_shell_command **cl, int indent)
{
	const struct st_shell_command **cp = cl;

	while((*cp) != 0) {
		if((*cp)->name[0] != 0) {
			DTPRINTF(0x02, "Disp %s\n",(*cp)->name);
			print_command_manual(*cp, indent);
		}
		cp ++;
	}
}

static void print_command_list_manual(const struct st_shell_command **cl, int indent)
{
	const struct st_shell_command **cp = cl;

	if((*cp)->name[0] != 0) {
		DTPRINTF(0x02, "Disp %s\n",(*cp)->name);
		print_command_manual(*cp, indent);
	}

	if((*cp)->sublist != 0) {
		indent ++;
		cp = (const struct st_shell_command **)(*cp)->sublist;
		print_command_list(cp, indent);
	}
}

static int help(int argc, uchar *argv[])
{
	const struct st_shell_command **cp = com_list;
	int arg = 1;
	int i;

	if(argc == 1) {
		DTPRINTF(0x01, "help only\n");
		print_command_list(cp, 0);
		return 0;
	} else {
		while((*cp) != 0) {
			DTPRINTF(0x01, "? %s == %s\n", argv[arg], (*cp)->name);
			if(strcomp((unsigned char *)(*cp)->name, (unsigned char *)argv[arg]) == 0) {
				tprintf(" -> %s", (*cp)->name);
				if((*cp)->sublist == 0) {
					tprintf("\n");
					print_command_manual(*cp, arg);
					if((*cp)->usage_str != 0) {
						print_command_usage(*cp);
					}
					return 0;
				} else {
					if(arg < (argc - 1)) {
						arg ++;
						DTPRINTF(0x01, "Next arg %s\n", argv[arg]);
						cp = (const struct st_shell_command **)(*cp)->sublist;
					} else {
						tprintf("\n");
						print_command_list_manual(cp, arg);
						return 0;
					}
				}
			} else {
				cp ++;
			}
		}

		for(i=1; i<argc; i++) {
			tprintf(" %s", argv[i]);
		}
		tprintf(" : command not found\n");
	}

	return 0;
}

const struct st_shell_command com_help = {
	.name		= "help",
	.command	= help,
	.usage_str	= "[command]",
	.manual_str	= "Print command help message"
}; ///< ヘルプメッセージ表示

/**
   @section help_command ヘルプコマンド

   ヘルプメッセージを表示します。

   [実行例]

   : help\n
   help         : Print command help message\n
   sys          : System operation commands\n
   mem          : Memory operation commands\n
   dev          : Device operation commands\n
   task         : Task operation commands\n
   graph        : Graphics operation commands\n
   file         : File strage operation commands
*/

static const struct st_shell_command * const com_sys_list[] = {
	&com_sys_info,
	&com_sys_timercheck,
	&com_sys_kerneltime,
	&com_sys_systime,
	&com_sys_datetime,
	&com_sys_setdate,
#ifdef GSC_DEV_ENABLE_RTC
	&com_sys_rtcdate,
	&com_sys_datesync,
	&com_sys_rtcsync,
#endif
	&com_sys_event,
	&com_sys_setevent,
	&com_sys_sleep,
	&com_sys_console,
#ifndef GSC_TARGET_SYSTEM_EMU
	&com_sys_sysmem,
#endif
#ifdef GSC_MEMORY_ENABLE_HEAP_MEMORY
	&com_sys_heap,
#if 0
	&com_mem_alloc,
	&com_mem_free,
#endif
#endif
	&com_sys_reboot,
#ifdef GSC_LIB_ENABLE_RANDOM
	&com_sys_random,
#endif
#ifndef GSC_TARGET_SYSTEM_EMU
#ifdef GSC_KERNEL_ENABLE_INTERRUPT_COUNT
	&com_sys_interrupt,
#endif
#endif
	0
};

const struct st_shell_command com_sys = {
	.name		= "sys",
	.usage_str	= "<command>",
	.manual_str	= "System operation commands",
	.sublist	= com_sys_list
}; ///< システム状態取得、設定
