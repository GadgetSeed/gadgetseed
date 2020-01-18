/** @file
    @brief	Internet radio アプリケーション

    @date	2018.09.02
    @auther	Takashi SHUDO
*/

#define GSLOG_PREFIX	"IRD: "

#include "shell.h"
#include "tprintf.h"
#include "storage.h"
#include "file.h"
#include "ff.h"
#include "sysevent.h"
#include "key.h"
#include "console.h"
#include "str.h"
#include "memory.h"
#include "log.h"
#include "task/syscall.h"
#include "lwip/netdb.h"

#include "soundplay.h"

#include "ir_socket.h"
#include "ir_stream.h"
#include "shoutcast.h"
#include "internetradio.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


#define RECONNECT_TIME	10000	// 10s

#define PAUSE_BUFF_SIZE	(1024*8)
#define PLAY_BUFF_SIZE	(1024*32)

extern int stream_read_size;

static uchar ir_url[MAX_URL_STR_LEN + 1];
static uchar ir_port[MAX_URL_STR_LEN + 1];
static uchar ir_path[MAX_URL_STR_LEN + 1] = {0};

static int iradio_open(int argc, uchar *argv[]);
static int iradio_close(int argc, uchar *argv[]);

/**
   @brief	指定したURLの Internet radio を再生
*/

static const struct st_shell_command com_iradio_open = {
	.name		= "open",
	.command	= iradio_open,
	.usage_str	= "<URL> [file]",
	.manual_str	= "Internet radio open from URL",
};

static const struct st_shell_command com_iradio_close = {
	.name		= "close",
	.command	= iradio_close,
	.manual_str	= "Internet radio close",
};

const struct st_shell_command * const com_iradio_list[] = {
	&com_iradio_open,
	&com_iradio_close,
	0
};

static const struct st_shell_command com_iradio = {
	.name		= "iradio",
	.sublist	= com_iradio_list,
	.usage_str	= "<URL> [file]",
	.manual_str	= "Internet radio play from URL",
};

static uchar pipe_fname[] = STREAM_FILENAME;

typedef enum {
	IRP_EVENT_NOEVENT,
	IRP_EVENT_OPEN,
	IRP_EVENT_CLOSE,
	IRP_EVENT_ABORT
} ir_event;

enum {
	IRP_STAT_STOP,
	IRP_STAT_STARTING,
	IRP_STAT_PLAY,
	IRP_STAT_PAUSE
} irplayer_stat = IRP_STAT_STOP;

#define MAX_IREVENT	2
struct st_fifo irplay_event;
static unsigned char irplay_event_buf[MAX_IREVENT+1];

static int flg_play = 1;

void set_ir_event(unsigned char event)
{
	int rt;
	unsigned char data = event;

	rt = write_fifo(&irplay_event, &data, 1);
	if(rt == 0) {
		GSLOG(0, "Sound event full error\n");
	}
}

static unsigned char get_ir_event(void)
{
	int rt;
	unsigned char event;

	rt = read_fifo(&irplay_event, &event, 1);
	if(rt == 0) {
		return IRP_EVENT_NOEVENT;
	} else {
		return event;
	}
}

static uchar *ir_fname;

int open_internetradio(uchar *url)
{
	GSLOG(0, "Open start(%s)\n", url);

	ir_fname = pipe_fname;
	flg_play = 1;

	perse_internet_radio_url(url, ir_url, ir_port, ir_path);

	set_ir_event(IRP_EVENT_OPEN);

	return 0;
}

static int iradio_open(int argc, uchar *argv[])
{
	GSLOG(0, "Open start\n");

	ir_fname = pipe_fname;

	if(argc < 2) {
		print_command_usage(&com_iradio_open);
		return 0;
	}

	if(argc > 2) {
		flg_play = 0;
		ir_fname = argv[2];
	} else {
		flg_play = 1;
	}

	perse_internet_radio_url((const uchar *)argv[1], ir_url, ir_port, ir_path);

	set_ir_event(IRP_EVENT_OPEN);

	return 0;
}

int close_internetradio(void)
{
	GSLOG(0, "Close start\n");

	set_ir_event(IRP_EVENT_CLOSE);

	return 0;
}

static int iradio_close(int argc, uchar *argv[])
{
	close_internetradio();

	return 0;
}

static struct st_tcb stream_tcb;
#define STREAM_SIZEOFSTACK	(1024*10)
static unsigned int stream_stack[STREAM_SIZEOFSTACK/sizeof(unsigned int)] ATTR_STACK;

//#define PRINT_BUFSIZE

static int stream_task(void *arg)
{
#ifdef PRINT_BUFSIZE
	int ssize;
	unsigned long long lsystime = get_kernel_time();
#endif

	while(1) {
		internet_radio_proc();
#ifdef PRINT_BUFSIZE
		if(flg_play != 0) {
			if(internet_radio_status() == RECEIVING) {
				unsigned long long nsystime = get_kernel_time();
				ssize = internet_radio_stream_size();
				if((nsystime - lsystime) >= 1000) {
					if(ssize >= 0) {
						GSLOG(7, "stream read size = %d\n", stream_read_size);
						GSLOG(7, "stream buf size = %d\n", ssize);
					}
					lsystime = nsystime;
				}
			}
		}
#endif
	}

	return 0;
}

static struct st_tcb ir_tcb;
#define IR_SIZEOFSTACK	(1024*2)
static unsigned int ir_stack[IR_SIZEOFSTACK/sizeof(unsigned int)] ATTR_STACK;

static unsigned long long pause_time;
static int ir_connect_wtime = 0;


static void proc_ir_stop(unsigned char event)
{
	switch(event) {
	case IRP_EVENT_OPEN:
		GSLOG(0, "Open(stop)\n");
		ir_connect_wtime = 0;
		connect_internet_radio(ir_url, ir_port, ir_path, ir_fname);
		irplayer_stat = IRP_STAT_STARTING;
		break;

	case IRP_EVENT_NOEVENT:
	case IRP_EVENT_CLOSE:
	default:
		task_sleep(10);
		break;
	}
}

static void proc_ir_starting(unsigned char event)
{
	switch(event) {
	case IRP_EVENT_OPEN:
		GSLOG(0, "Allready opened(starting)\n");
		break;

	case IRP_EVENT_CLOSE:
		GSLOG(0, "Close\n");
		disconnect_internet_radio();
		//soundplay_stop();
		soundplay_close();
		irplayer_stat = IRP_STAT_STOP;
		break;

	case IRP_EVENT_NOEVENT:
		if(internet_radio_stream_size() > (1024*16)) {
			GSLOG(0, "Starting play\n");
			soundplay_play();
			GSLOG(0, "Start play\n");
			irplayer_stat = IRP_STAT_PLAY;
			break;
		} else if(ir_connect_wtime >= 3000) {
			GSLOG(0, "Start play timeout failure\n");
			disconnect_internet_radio();
			irplayer_stat = IRP_STAT_STOP;
			break;
		}
		task_sleep(10);	// Save buffer time
		ir_connect_wtime += 10;
		break;

	default:
		break;
	}
}

static void proc_ir_play(unsigned char event)
{
	switch(event) {
	case IRP_EVENT_OPEN:
		GSLOG(0, "Allready opened(play)\n");
		break;

	case IRP_EVENT_CLOSE:
		GSLOG(0, "Close\n");
		disconnect_internet_radio();
		soundplay_stop();
		soundplay_close();
		irplayer_stat = IRP_STAT_STOP;
		break;

	case IRP_EVENT_NOEVENT:
		if(internet_radio_status() == RECEIVING) {
			int ssize = internet_radio_stream_size();
			if(ssize < PAUSE_BUFF_SIZE) {
				pause_time = get_kernel_time();
				GSLOG(0, "[PLAY -> PAUSE] buffer size = %d\n", ssize);
				GSLOG(0, "Pause\n");
//				exec_command((uchar *)"sound pause");
				soundplay_pause();
				irplayer_stat = IRP_STAT_PAUSE;
			}
		}
		break;

	default:
		break;
	}
}

static unsigned long long lsystime;
static int flg_reconnect = 0;

static void proc_ir_pause(unsigned char event)
{
	switch(event) {
	case IRP_EVENT_OPEN:
		GSLOG(0, "Allready opened(pause)\n");
		break;

	case IRP_EVENT_CLOSE:
		GSLOG(0, "Close\n");
		disconnect_internet_radio();
		soundplay_stop();
		soundplay_close();
		irplayer_stat = IRP_STAT_STOP;
		break;

	case IRP_EVENT_NOEVENT:
		if(internet_radio_status() == RECEIVING) {
			int ssize = internet_radio_stream_size();
			unsigned long long nsystime = get_kernel_time();

			if((nsystime - lsystime) >= 1000) {
				GSLOG(0, "[PAUSE] buffer size = %d\n", ssize);
				lsystime = nsystime;
			}

			if(ssize > PLAY_BUFF_SIZE) {
				GSLOG(0, "[PAUSE -> PLAY] buffer size = %d\n", ssize);
				if(flg_reconnect == 0) {
					GSLOG(0, "Continue\n");
					soundplay_play();
					GSLOG(0, "Continue done\n");
				} else {
					flg_reconnect = 0;
					GSLOG(0, "Resync\n");
					soundplay_resync();
					GSLOG(0, "Resync done\n");
				}
				irplayer_stat = IRP_STAT_PLAY;
				break;
			}

			if((nsystime - pause_time) > RECONNECT_TIME) {
				GSLOG(0, "Try recoonect\n");
				reconnect_internet_radio();
				lsystime = nsystime;
				GSLOG(0, "Wait recoonect...\n");
				while(1) {
					if(internet_radio_status() == RECEIVING) {
						// 接続した
						pause_time = get_kernel_time();
						flg_reconnect = 1;
						GSLOG(0, "internet radio reconnected\n");
						task_sleep(10);
						break;
					} else {
						// 再接続タイムアウト待ち
						unsigned long long nsystime = get_kernel_time();

						if((nsystime - lsystime) >= 20000) {
							GSLOG(0, "reconnected internet radio time out\n");
							disconnect_internet_radio();
							soundplay_stop();
							soundplay_close();
							//flg_play = 0;
							irplayer_stat = IRP_STAT_STOP;
							break;
						} else {
							task_sleep(10);
						}
					}
				}
			}
		}
		break;

	default:
		break;
	}
}

static int internetradio_task(void *arg)
{
	lsystime = get_kernel_time();

	while(1) {
		if(flg_play != 0) {
			unsigned char event;
			event = get_ir_event();

			switch(irplayer_stat) {
			case IRP_STAT_STOP:
				proc_ir_stop(event);
				break;

			case IRP_STAT_STARTING:
				proc_ir_starting(event);
				break;

			case IRP_STAT_PLAY:
				proc_ir_play(event);
				break;

			case IRP_STAT_PAUSE:
				proc_ir_pause(event);
				break;

			default:
				break;
			}
		}

		task_sleep(10);
	}

	return 0;
}

void startup_internetradio(void)
{
	init_fifo(&irplay_event, irplay_event_buf, MAX_IREVENT);
	init_ir_stream();

	mount_storage(2, "null", "pipe");

	add_shell_command((struct st_shell_command *)&com_iradio);

	task_add(stream_task, "ir_stream", TASK_PRIORITY_APP_HIGH, &stream_tcb,
		 stream_stack, STREAM_SIZEOFSTACK, 0);

	task_exec(internetradio_task, "iradio", TASK_PRIORITY_NETWORK, &ir_tcb,
		  ir_stack, IR_SIZEOFSTACK, 0);
}
