/** @file
    @brief	Internet radio packet ストリーム

    @date	2018.09.19
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "task/syscall.h"
#include "file.h"
#include "sysevent.h"
#include "shell.h"

#include "ir_stream.h"
#include "ir_socket.h"
#include "shoutcast.h"
#include "tkprintf.h"

#include "mp3play.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"

#define GSLOG_PREFIX	"IST: "
#include "log.h"

#ifdef DEBUGTBITS
#define STR(var) #var
static const char ir_ss_name[STREAM_STAT_MAX][20] = {
	STR(DISCONNECTED),
	STR(CONNECTING),
	STR(RECEIVING)
};
#define PRINT_IR_STAT()	DTFPRINTF(0x02, "ir_stream_stat = %s\n", ir_ss_name[ir_stream_stat])
#else
#define PRINT_IR_STAT()
#endif


extern struct st_music_info music_info;

static uchar ir_url[MAX_URL_STR_LEN + 1];
static uchar ir_port[MAX_URL_STR_LEN + 1];
static uchar ir_path[MAX_URL_STR_LEN + 1];

static uchar stream_fname[MAX_URL_STR_LEN + 1];	// ストリーム転送用ファイル名
static int stream_fd = -1;		// ストリーム転送用ファイルディスクリプタ

static stream_stat ir_stream_stat = DISCONNECTED;

#define MAX_IRS_EVENT	4
static struct st_fifo irs_event_fifo;
static unsigned char irs_event_buf[MAX_IRS_EVENT + 1];

typedef enum {
	IRS_EVENT_NOEVENT,
	IRS_EVENT_CONNECT,
	IRS_EVENT_RECONNECT,
	IRS_EVENT_DISCONNECT
} irs_event;

static void set_irs_event(unsigned char event)
{
	int rt;
	unsigned char data = event;

	rt = write_fifo(&irs_event_fifo, &data, 1);
	if(rt == 0) {
		GSLOG(0, "Stream event full error\n");
	}
}

static unsigned char get_irs_event(void)
{
	int rt;
	unsigned char event;

	rt = read_fifo(&irs_event_fifo, &event, 1);
	if(rt == 0) {
		return IRS_EVENT_NOEVENT;
	} else {
		return event;
	}
}

void init_ir_stream(void)
{
	init_fifo(&irs_event_fifo, irs_event_buf, MAX_IRS_EVENT);
}

static int open_streamfd(void)
{
	GSLOG(0, "Open stream file\n");

	stream_fd = open_file(stream_fname, FA_WRITE | FA_CREATE_ALWAYS);
	if(stream_fd < 0) {
		eprintf("Cannot open file \"%s\"\n", stream_fname);
		stream_fd = -1;
		PRINT_IR_STAT();
		return -1;
	} else {
		tprintf("FILE : %s\n", stream_fname);
	}

	return 0;
}

static void close_streamfd(void)
{
	GSLOG(0, "Close stream file\n");

	close_file(stream_fd);
	stream_fd = -1;
}

static int connect_ir(void)
{
	int rt = 0;

	rt = open_internet_radio(ir_url, ir_port);
	if(rt < 0) {
		GSLOG(0, "open_internet_radio error\n");
		PRINT_IR_STAT();
		return -1;
	}

	rt = open_streamfd();
	if(rt < 0) {
		GSLOG(0, "Stream file open error\n");
		return -1;
	}

	start_shoutcast_stream(ir_path);

	return 0;
}

static void dispose_stream_buf(void)
{
//	int rt;
//	unsigned char tmp;
//	int flg_reopen = 0;

	DTFPRINTF(0x02, "\n");

	unlink_file(stream_fname);

#if 0
	if(stream_fd < 0) {
		stream_fd = open_file(stream_fname, FA_READ);
		flg_reopen = 1;
	}

	rt = size_file(stream_fd);
	DTFPRINTF(0x02, "size_file = %d\n", rt);

	do {
		rt = read_file(stream_fd, &tmp, 1);
		//tprintf(".");
	} while(rt != 0);

	if(flg_reopen != 0) {
		close_file(stream_fd);
		stream_fd = -1;
	}
#endif

	DTFPRINTF(0x02, "done\n");
}

static int receive_shoutcast(void)
{
	int rt = 0;

	rt = decode_shoutcast_message(&music_info);
	if(rt != 0) {
		char ctype_name[4][4] = { "???", "MP3", "AAC", "WAV" };

		GSLOG(0, "Name        : %s\n", music_info.album);
		GSLOG(0, "Genre       : %s\n", music_info.genre);
		GSLOG(0, "Bit rate    : %d\n", music_info.bit_rate);
		GSLOG(0, "Sample rate : %d\n", music_info.sampling_rate);
		GSLOG(0, "URL         : %s\n", music_info.url);
		GSLOG(0, "Metaint     : %d\n", music_info.metaint);
		GSLOG(0, "Codec       : %s\n", ctype_name[music_info.format]);

		mp3stream_open((uchar *)STREAM_FILENAME, music_info.album);

		PRINT_IR_STAT();
	} else {
		PRINT_IR_STAT();
		return -1;
	}

	return 0;
}

/*
  切断状態
*/
static void proc_disconnected(unsigned char event)
{
	DTFPRINTF(0x04, "\n");

	switch(event) {
	case IRS_EVENT_NOEVENT:
		task_sleep(10);
		break;

	case IRS_EVENT_CONNECT:
	case IRS_EVENT_RECONNECT:
		// 接続開始
	{
		int rt;

		dispose_stream_buf();
		rt = connect_ir();
		if(rt < 0) {
			GSLOG(0, "Connect error\n");
			close_streamfd();
			create_event(EVT_IRADIO_ERROR, 0, 0);
			ir_stream_stat = DISCONNECTED;
		} else {
			GSLOG(0, "Connect success\n");
			ir_stream_stat = CONNECTING;
		}
	}
	break;

	case IRS_EVENT_DISCONNECT:
		GSLOG(0, "Already disconnect\n");
		break;

	default:
		task_sleep(10);
		break;
	}
}

/*
  shuutcatに接続
*/
static void proc_connecing(unsigned char event)
{
	DTFPRINTF(0x04, "\n");

	switch(event) {
	case IRS_EVENT_NOEVENT:
	{
		int rt;
		rt = receive_shoutcast();
		if(rt < 0) {
			GSLOG(0, "shoutcast connect error\n");
			close_internet_radio();
			dispose_stream_buf();
			close_streamfd();
			create_event(EVT_IRADIO_ERROR, 0, 0);
			ir_stream_stat = DISCONNECTED;
		} else {
			GSLOG(0, "shoutcast connect success\n");
			create_event(EVT_IRADIO_CONNECTED, 0, 0);
			ir_stream_stat = RECEIVING;
		}
	}
	break;

	case IRS_EVENT_CONNECT:
	case IRS_EVENT_RECONNECT:
		GSLOG(0, "Now connecting\n");
		break;

	case IRS_EVENT_DISCONNECT:
		GSLOG(0, "Disconnect\n");
		close_internet_radio();
		dispose_stream_buf();
		close_streamfd();
		ir_stream_stat = DISCONNECTED;
		create_event(EVT_IRADIO_DISCONNECTED, 0, 0);
		break;

	default:
		task_sleep(10);
		break;
	}
}

/*
  ストリーム受信中
*/
static void proc_receiving(unsigned char event)
{
	DTFPRINTF(0x04, "\n");

	switch(event) {
	case IRS_EVENT_NOEVENT:
	{
		static unsigned char buf[1152];
		int frame_size = 0;

		frame_size = decode_shoutcast_stream(&music_info, buf);
		DTPRINTF(0x02, "stream %d\n", frame_size);
		if(frame_size < 0) {
			PRINT_IR_STAT();
			GSLOG(0, "decode_shoutcast_stream return %d, disconnect start\n", frame_size);
			close_internet_radio();
			dispose_stream_buf();
			close_streamfd();
			ir_stream_stat = DISCONNECTED;
			create_event(EVT_IRADIO_ABORT, 0, 0);
			return;
		}

		if(stream_fd >= 0) {
			write_file(stream_fd, buf, frame_size);
			create_event(EVT_IRADIO_RECEIVE, 0, 0);
		} else {
			GSLOG(0, "stream_fd error(%d)\n", stream_fd);
		}
	}
	break;

	case IRS_EVENT_CONNECT:
		GSLOG(0, "Already connected\n");
		break;

	case IRS_EVENT_RECONNECT:
		// 再接続
		GSLOG(0, "Reconnecting\n");
		{
			int rt;

			close_internet_radio();
			dispose_stream_buf();
			close_streamfd();

			rt = connect_ir();
			if(rt < 0) {
				GSLOG(0, "Connect error\n");
				dispose_stream_buf();
				close_streamfd();
				ir_stream_stat = DISCONNECTED;
				create_event(EVT_IRADIO_ABORT, 0, 0);
			} else {
				GSLOG(0, "Connect success\n");
				ir_stream_stat = CONNECTING;
			}
		}
		break;

	case IRS_EVENT_DISCONNECT:
		GSLOG(0, "Disconnect\n");
		close_internet_radio();
		dispose_stream_buf();
		close_streamfd();
		ir_stream_stat = DISCONNECTED;
		create_event(EVT_IRADIO_DISCONNECTED, 0, 0);
		break;

	default:
		task_sleep(10);
		break;
	}
}

void internet_radio_proc(void)
{
	unsigned char event;
	event = get_irs_event();

	switch(ir_stream_stat) {
	case DISCONNECTED:
		proc_disconnected(event);
		break;

	case CONNECTING:
		proc_connecing(event);
		break;

	case RECEIVING:
		proc_receiving(event);
		break;

	default:
		proc_disconnected(event);
		break;
	}
}

int connect_internet_radio(const uchar *url, const uchar *port, const uchar *path, const uchar *fname)
{
	DTFPRINTF(0x01, "url=%s, port=%s, path=%s, fname=%s\n", url, port, path, fname);
	DTFPRINTF(0x01, "ir_stream_stat = %d\n", ir_stream_stat);

	GSLOG(0, "Connect start\n");

	strncopy(ir_url, url, MAX_URL_STR_LEN);
	strncopy(ir_port, port, MAX_URL_STR_LEN);
	strncopy(ir_path, path, MAX_URL_STR_LEN);
	strncopy(stream_fname, fname, MAX_URL_STR_LEN);

	set_irs_event(IRS_EVENT_CONNECT);

	return 0;
}

int reconnect_internet_radio(void)
{
	DTFPRINTF(0x02, "\n");
	PRINT_IR_STAT();

	GSLOG(0, "Reconnect start\n");

	set_irs_event(IRS_EVENT_RECONNECT);

	return 0;
}

int disconnect_internet_radio(void)
{
	DTFPRINTF(0x02, "\n");
	DTFPRINTF(0x03, "ir_stream_stat = %d\n", ir_stream_stat);

	GSLOG(0, "Disconnect start\n");

	set_irs_event(IRS_EVENT_DISCONNECT);

	return 0;
}

stream_stat internet_radio_status(void)
{
	return ir_stream_stat;
}

int internet_radio_stream_size(void)
{
	int fsize = -1;

	DTFPRINTF(0x01, "\n");

	if(stream_fd >= 0) {
		fsize = size_file(stream_fd);
	}

	DTFPRINTF(0x01, "fsize = %d\n", fsize);

	return fsize;
}
