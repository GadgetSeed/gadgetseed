/** @file
    @brief	音声ファイル再生

    @date	2017.02.12
    @auther	Takashi SHUDO
*/

#include "task/syscall.h"
#include "tprintf.h"
#include "shell.h"
#include "file.h"
#include "fifo.h"
#include "str.h"
#include "soundplay.h"
#include "soundio.h"
#include "soundfile.h"
#include "music_info.h"
#include "mp3play.h"
#include "m4aplay.h"
#include "mp4tag.h"
#include "id3tag.h"
#include "charcode.h"
#include "tkprintf.h"
#include "artwork.h"

#define GSLOG_PREFIX	"SPL: "
#include "log.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


/*
 * 各デコーダ共有データ
 */
struct st_music_info music_info;

/*
 * プロセス操作
 */
#define MAX_SOUNDEVENT	4
struct st_fifo soundplay_event;
static unsigned char soundplay_event_buf[MAX_SOUNDEVENT+1];
sound_stat soundplay_status = SOUND_STAT_NOTREADY;

int audio_frame_count;
int next_audio_frame_count;
unsigned int audio_play_time;
static int (* soundplay_proc)(void) = 0;

#define SOUNDPLAY_STAT_WAITTIME	10	// ms
#define SOUNDPLAY_STAT_TIMEOUT	1000	// ms

void soundplay_start_proc(int (* func)(void))
{
	GSLOG(0, "Sound play ready\n");

	soundplay_proc = func;

	soundplay_status = SOUND_STAT_READY;
}

/*
 * API
 */
#define EXT_LEN 12

int soundplay_open(uchar *fname)
{
	int rtn = 0;
	uchar ext[EXT_LEN + 1] = {0};

	(void)get_filename_extension(ext, fname, EXT_LEN);
	(void)str2cap(ext);

	if(strcomp((uchar *)"MP3", ext) == 0) {
		GSLOG(0, "Sound open MP3 %s\n", sj2utf8(fname));
		rtn = mp3file_open(fname);
	} else if(strcomp((uchar *)"M4A", ext) == 0) {
		GSLOG(0, "Sound open M4A %s\n", sj2utf8(fname));
		rtn = m4afile_open(fname);
	} else {
		GSLOG(0, "Sound open error %s\n", sj2utf8(fname));
		rtn = -1;
	}

	return rtn;
}

void soundplay_close(void)
{
	soundfile_close();
}

static void set_sound_event(unsigned char event)
{
	int rt;
	unsigned char data = event;

	rt = write_fifo(&soundplay_event, &data, 1);
	if(rt == 0) {
		GSLOG(0, "Sound event full error\n");
	}
}

void soundplay_play(void)
{
	int i;

	GSLOG(0, "Sound play start\n");

	switch(soundplay_status) {
	case SOUND_STAT_READY:
	case SOUND_STAT_END:
		break;

	case SOUND_STAT_NOTREADY:
	case SOUND_STAT_SYNCING:
	case SOUND_STAT_PLAYING:
	default:
		return;
		break;
	}

	set_sound_event(SOUND_EVENT_PLAY);

	for(i=0; i<SOUNDPLAY_STAT_TIMEOUT; i+=SOUNDPLAY_STAT_WAITTIME) {
		if(soundplay_status == SOUND_STAT_PLAYING) {
			break;
		} else {
			task_sleep(10);
		}
	}

	if(i >= SOUNDPLAY_STAT_TIMEOUT) {
		GSLOG(0, "Sound play timeout\n");
	}
}

void soundplay_stop(void)
{
	int i;

	GSLOG(0, "Sound stop\n");

	switch(soundplay_status) {
	case SOUND_STAT_READY:
	case SOUND_STAT_PLAYING:
	case SOUND_STAT_SYNCING:
		break;

	case SOUND_STAT_NOTREADY:
	case SOUND_STAT_END:
	default:
		return;
		break;
	}

	set_sound_event(SOUND_EVENT_STOP);

	for(i=0; i<SOUNDPLAY_STAT_TIMEOUT; i+=SOUNDPLAY_STAT_WAITTIME) {
		if(soundplay_status == SOUND_STAT_END) {
			break;
		} else {
			task_sleep(SOUNDPLAY_STAT_WAITTIME);
		}
	}

	if(i >= SOUNDPLAY_STAT_TIMEOUT) {
		GSLOG(0, "Sound stop timeout\n");
	}
}

void soundplay_pause(void)
{
	int i;

	GSLOG(0, "Sound pause\n");

	if((soundplay_status != SOUND_STAT_SYNCING) &&
	   (soundplay_status != SOUND_STAT_PLAYING)) {
		return;
	}

	set_sound_event(SOUND_EVENT_PAUSE);

	for(i=0; i<SOUNDPLAY_STAT_TIMEOUT; i+=SOUNDPLAY_STAT_WAITTIME) {
		if(soundplay_status != SOUND_STAT_READY) {
			task_sleep(SOUNDPLAY_STAT_WAITTIME);
		} else {
			break;
		}
	}

	if(i >= SOUNDPLAY_STAT_TIMEOUT) {
		GSLOG(0, "Sound pause timeout\n");
	}
}

void soundplay_resync(void)
{
	GSLOG(0, "Sound resync\n");

	if(soundplay_status != SOUND_STAT_READY) {
		return;
	}

	set_sound_event(SOUND_EVENT_SYNC);

	while(soundplay_status == SOUND_STAT_SYNCING) {
		task_sleep(10);
	}
}

static void seek_audio(int pos)
{
	switch(music_info.format) {
	case MUSIC_FMT_MP3:
		mp3file_seek(pos);
		break;

	case MUSIC_FMT_AAC:
		m4afile_seek(pos);
		break;

	default:
		break;
	}
}

void soundplay_seek(int pos)
{
	GSLOG(0, "Sound seek %d\n", pos);

	if(soundplay_status == SOUND_STAT_PLAYING) {
		next_audio_frame_count = pos;
	} else {
		seek_audio(pos);
		audio_play_time = calc_play_time(&music_info, audio_frame_count);
		disp_play_time(audio_play_time);
	}
}

int soundplay_playbackpos(void)
{
	return audio_frame_count;
}

void soundplay_volume(int volume)
{
	soundio_set_volume(volume);
}

void soundplay_artwork(int flg)
{
	set_mp4_decode_artwork(flg);
	set_id3_decode_artwork(flg);
}

/*
 * タスク
 */
static void no_play_proc(void)
{
	soundio_mixer_proc();
	task_sleep(10);
}

static int soundplay_task(void *arg)
{
	while(1) {
		if(soundplay_proc != 0) {
			soundplay_proc();
			soundplay_proc = 0;

			GSLOG(0, "Sound Play Proc End\n");
		} else {
			no_play_proc();
		}
	}

	return 0;
}

/*
 * コマンド
 */

const struct st_shell_command * const com_open_list[] = {
	&com_mp3_open,
	&com_m4a_open,
	0
};

const struct st_shell_command com_open = {
	.name		= "open",
	.manual_str	= "Open sound file",
	.sublist	= com_open_list
};

static int sound_close(int argc, uchar *argv[])
{
	tprintf("Close sound file\n");

	soundplay_close();

	return 0;
}

const struct st_shell_command com_close = {
	.name		= "close",
	.command	= sound_close,
	.manual_str	= "Close sound file"
};

static int sound_play(int argc, uchar *argv[])
{
	tprintf("Play sound file\n");

	soundplay_play();

	return 0;
}

const struct st_shell_command com_play = {
	.name		= "play",
	.command	= sound_play,
	.manual_str	= "Play sound data",
};

static int sound_stop(int argc, uchar *argv[])
{
	tprintf("Stop sound play\n");

	soundplay_stop();

	return 0;
}

const struct st_shell_command com_stop = {
	.name		= "stop",
	.command	= sound_stop,
	.manual_str	= "Stop play sound data"
};

static int sound_pause(int argc, uchar *argv[])
{
	tprintf("Pause sound play\n");

	soundplay_pause();

	return 0;
}

const struct st_shell_command com_pause = {
	.name		= "pause",
	.command	= sound_pause,
	.manual_str	= "Pause play sound data"
};

static int sound_resync(int argc, uchar *argv[])
{
	tprintf("Re Sync Play\n");

	soundplay_resync();

	return 0;
}

const struct st_shell_command com_resync = {
	.name		= "resync",
	.command	= sound_resync,
	.manual_str	= "Resync play sound data"
};

static int sound_seek(int argc, uchar *argv[])
{
	int pos = 0;

	if(argc < 2) {
		tprintf("Now position : %d/%d\n", audio_frame_count, music_info.sample_count);
	} else {
		pos = dstoi(argv[1]);
		tprintf("Set position : %d/%d\n", pos, music_info.sample_count);
		soundplay_seek(pos);
	}

	return 0;
}

const struct st_shell_command com_seek = {
	.name		= "seek",
	.command	= sound_seek,
	.manual_str	= "Seek sound play position time"
};

static int sound_volume(int argc, uchar *argv[])
{
	int volume;

	if(argc < 2) {
		volume = soundio_get_volume();
		tprintf("Now volume : %d\n", volume);
		return 0;
	} else {
		volume = dstoi(argv[1]);
		soundplay_volume(volume);
		DTPRINTF(0x01, "Set volume : %d\n", next_volume);
	}

	return 0;
}

const struct st_shell_command com_volume = {
	.name		= "volume",
	.command	= sound_volume,
	.manual_str	= "Set volume"
};


static int sound_artwork(int argc, uchar *argv[])
{
	if(argc < 2) {
		tprintf("Usage : <0|1>\n");
		return 0;
	} else {
		int flg;
		flg = dstoi(argv[1]);
		soundplay_artwork(flg);
	}

	return 0;
}

const struct st_shell_command com_artwork = {
	.name		= "artwork",
	.command	= sound_artwork,
	.manual_str	= "Set artwork docode"
};

const struct st_shell_command * const com_sound_list[] = {
	&com_open,
	&com_close,
	&com_play,
	&com_stop,
	&com_pause,
	&com_resync,
	&com_seek,
	&com_volume,
	&com_artwork,
	0
};

const struct st_shell_command com_sound = {
	.name		= "sound",
	.manual_str	= "Sound file play commands",
	.sublist	= com_sound_list
};

#include <stdio.h>

static uchar cmd[FF_MAX_LFN + 1];
static unsigned char fname[FF_MAX_LFN + 1];

static int do_mp3_file(uchar *str, uchar *arg)
{
	int rt = 0;

	escaped_str(fname, str);
	tprintf("Play MP3 file \"%s\"\n", sj2utf8(str));
	tsprintf((char *)cmd, "sound %s mp3 %s", arg, fname);
	rt = exec_command(cmd);

	return rt;
}

static const struct st_file_operation mp3_operation = {
	"MP3", do_mp3_file
};


static int do_m4a_file(uchar *str, uchar *arg)
{
	int rt = 0;

	escaped_str(fname, str);
	tprintf("Play M4A file \"%s\"\n", sj2utf8(str));
	tsprintf((char *)cmd, "sound %s m4a %s", arg, fname);
	rt = exec_command(cmd);

	return rt;
}

static const struct st_file_operation m4a_operation = {
	"M4A", do_m4a_file
};


static const struct st_file_operation * const file_operation[] = {
	&mp3_operation,
	&m4a_operation,
	0
};


//#define SIZEOFAPPTS	(1024*4)
//#define SIZEOFAPPTS	(1024*16)
//#define SIZEOFAPPTS	(1024*48)
#define SIZEOFAPPTS	(1024*50)
static struct st_tcb tcb;
static unsigned int stack[SIZEOFAPPTS/sizeof(unsigned int)] ATTR_STACK;

void startup_soundplay(void)
{
	add_shell_command((struct st_shell_command *)&com_sound);

	set_file_operation(file_operation);

	init_fifo(&soundplay_event, soundplay_event_buf, MAX_SOUNDEVENT);

	init_soundio();

	task_exec(soundplay_task, "soundplay", TASK_PRIORITY_REALTIME, &tcb,
		  stack, SIZEOFAPPTS, 0);
}
