/** @file
    @brief	音声ファイル再生

    @date	2017.02.12
    @auther	Takashi SHUDO
*/

#include "task/syscall.h"
#include "tprintf.h"
#include "shell.h"
#include "file.h"
#include "str.h"
#include "device.h"
#include "device/audio_ioctl.h"
#include "ff.h"
#include "soundplay.h"
#include "music_info.h"
#include "sysevent.h"
#include "mp3play.h"
#include "m4aplay.h"
#include "spectrum_analyzer.h"
#include "fft.h"
#include "mp4tag.h"
#include "id3tag.h"
#include "charcode.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


#define SPIO_TIMEOUT	500

/*
 * 各デコーダ共有データ
 */
struct st_music_info music_info;

/*
 * AUDIOデバイス操作
 */
static const char audio_devname[] = "audio";
static struct st_device *audio_dev;
static int smp_rate = 48000;
static int now_volume = 20;
static int next_volume = 20;
int audio_buf_size = MAX_FILEBUF;

void soundplay_mixer_proc(void)
{
	if(now_volume != next_volume) {
		now_volume = next_volume;
		if(lock_device(audio_dev, SPIO_TIMEOUT) == 0) {
			tprintf("audio ctrl lock timeout\n");
		}
		ioctl_device(audio_dev, IOCMD_AUDIO_SET_VOLUME, now_volume, 0);
		unlock_device(audio_dev);
		create_event(EVT_SOUND_VOLUME, now_volume, 0);
	}
}

void soundplay_prepare_sound(void)
{
	DTPRINTF(0x01, "[soundplay] PREPARE\n");

	create_event(EVT_SOUND_PREPARE, 0, (void *)&music_info);
}

void soundplay_start_sound(void)
{
	DTPRINTF(0x01, "[soundplay] START\n");

	ioctl_device(audio_dev, IOCMD_AUDIO_PLAY_START, 0, 0);

	create_event(EVT_SOUND_START, 0, (void *)&music_info);
}

void soundplay_continue_sound(void)
{
	DTPRINTF(0x01, "[soundplay] CONTINUE\n");

	ioctl_device(audio_dev, IOCMD_AUDIO_PLAY_START, 0, 0);

	create_event(EVT_SOUND_CONTINUE, 0, (void *)&music_info);
}

void soundplay_end_sound(void)
{
	DTPRINTF(0x01, "[soundplay] END\n");

	ioctl_device(audio_dev, IOCMD_AUDIO_PLAY_STOP, 0, 0);

	soundplay_closefile();

	create_event(EVT_SOUND_END, 0, 0);
}

void soundplay_stop_sound(void)
{
	DTPRINTF(0x01, "[soundplay] STOP\n");

	ioctl_device(audio_dev, IOCMD_AUDIO_PLAY_STOP, 0, 0);

	soundplay_closefile();

	create_event(EVT_SOUND_STOP, 0, 0);
}

void soundplay_pause_sound(void)
{
	DTPRINTF(0x01, "[soundplay] PAUSE\n");

	ioctl_device(audio_dev, IOCMD_AUDIO_PLAY_STOP, 0, 0);

	create_event(EVT_SOUND_PAUSE, 0, (void *)&music_info);
}

int soundplay_set_smprate(int rate)
{
	int rtn = 0;

	smp_rate = rate;

	rtn =  ioctl_device(audio_dev, IOCMD_AUDIO_SET_SMPRATE, smp_rate, 0);

	return rtn;
}

int soundplay_set_audiobuf_size(int size)
{
	return ioctl_device(audio_dev, IOCMD_AUDIO_SET_BUFSIZE, size, 0);
}

static struct st_audio_spectrum asp;

int soundplay_write_audiobuf(unsigned char *buf, int size)
{
	int rtn;

	rtn = select_device(audio_dev, SPIO_TIMEOUT);
	if(rtn == 0) {
		tprintf("Audio Decode & Play time too int(%d).\n", rtn);
	}

	if(lock_device(audio_dev, SPIO_TIMEOUT) == 0) {
		tprintf("audio buf write lock timeout\n");
	}

	rtn = write_device(audio_dev, buf, size);
	unlock_device(audio_dev);

	proc_spectrum_analyse(&(asp.spectrum[0]), (short *)buf);
	proc_spectrum_analyse(&(asp.spectrum[SPA_ANA_SMP]), (short *)(buf+2));
	asp.frame_num = audio_frame_count;

	create_event(EVT_SOUND_ANALYZE, 0, (void *)&asp);

	return rtn;
}

void soundplay_wait_audiobuf(unsigned char **p_buf)
{
	unsigned char *p_abuf;

	ioctl_device(audio_dev, IOCMD_AUDIO_WAIT_BUFFER, 0, (void *)&p_abuf);

	*p_buf = p_abuf;
}

/*
 * ファイル操作
 */
unsigned char file_buf[MAX_FILEBUF];	// ファイルデータ読み出しバッファ
unsigned char cname[FF_MAX_LFN + 1];	// UTF-8ファイル名
static int audio_fd = -1;

int soundplay_openfile(unsigned char *fname)
{
	DTFPRINTF(0x02, "%s\n", fname);

	if(audio_fd < 0) {
		audio_fd = open_file(fname, FA_READ);
		if(audio_fd < 0) {
			tprintf("File open error %s %d\n", fname, audio_fd);
		}
	}

	DTFPRINTF(0x02, "fd = %d\n", audio_fd);

	return audio_fd;
}

int soundplay_readfile(unsigned char *buf, int size)
{
	int rsize;

	DTFPRINTF(0x02, "fd = %d, buf = %p, size = %ld\n", audio_fd, buf, size);

	rsize = read_file(audio_fd, buf, size);

	return rsize;
}

int soundplay_seekfile(unsigned char *buf, int size)
{
	DTFPRINTF(0x02, "fd = %d, buf = %p, size = %ld\n", audio_fd, buf, size);

	seek_file(audio_fd, size, SEEK_CUR);

	return size;
}

int soundplay_seeksetfile(int pos)
{
	DTFPRINTF(0x02, "fd = %d, pos = %ld\n", audio_fd, pos);

	seek_file(audio_fd, pos, SEEK_SET);

	return pos;
}

int soundplay_tellfile(void)
{
	int offset;

	DTFPRINTF(0x02,"audio_fd = %d\n", audio_fd);

	offset = tell_file(audio_fd);

	return offset;
}

void soundplay_closefile(void)
{
	DTFPRINTF(0x02,"audio_fd = %d\n", audio_fd);

	if(audio_fd >= 0) {
		close_file(audio_fd);
		audio_fd = -1;
	}
}

/*
 * プロセス操作
 */
int soundplay_status = SOUND_STOP;	// 0:STOP 1:PLAY 2:PAUSE
int next_soundplay_status = -1;

int audio_frame_count;
int next_audio_frame_count;
unsigned int audio_play_time;
static int (* soundplay_proc)(void) = 0;

void soundplay_start_proc(int (* func)(void))
{
	soundplay_proc = func;

	soundplay_status = SOUND_PLAY;
}

void soundplay_stop_play(void)
{
	tprintf("Sound stop\n");

	if(soundplay_status == SOUND_STOP) {
		return;
	}

	next_soundplay_status = SOUND_STOP;

	while(soundplay_status != SOUND_STOP) {
		task_sleep(1);
	}
}

void soundplay_pause_play(void)
{
	tprintf("Sound pause\n");

	if(soundplay_status == SOUND_STOP) {
		return;
	}

	next_soundplay_status = SOUND_PAUSE;

	while(soundplay_status != SOUND_PAUSE) {
		task_sleep(1);
	}

	soundplay_pause_sound();
}

void soundplay_continue_play(void)
{
	tprintf("Sound continue\n");

	if(soundplay_status != SOUND_PAUSE) {
		return;
	}

	next_soundplay_status = SOUND_PLAY;

	while(soundplay_status != SOUND_PLAY) {
		task_sleep(1);
	}

	soundplay_continue_sound();
}

void soundplay_move_play(int pos)
{
	tprintf("Sound move\n");

	next_audio_frame_count = pos;
}

static void no_play_proc(void)
{
	soundplay_mixer_proc();
	task_sleep(10);
}

static int soundplay_task(char *arg)
{
	while(1) {
		if(soundplay_proc != 0) {
			soundplay_proc();
			soundplay_proc = 0;

			tprintf("Sound Play Proc End\n");
		} else {
			no_play_proc();
		}
	}

	return 0;
}

void soundplay_init_time(void)
{
	audio_frame_count = 0;
	next_audio_frame_count = -1; // 0以上はそこに移動
	audio_play_time = 0;
}

/*
 * コマンド
 */

extern const struct st_shell_command com_wav_analyze;
extern const struct st_shell_command com_wav_play;

const struct st_shell_command * const com_analyze_list[] = {
	&com_wav_analyze,
	&com_mp3_analyze,
	&com_m4a_analyze,
	0
};

const struct st_shell_command com_analyze = {
	.name		= "analyze",
	.manual_str	= "Analyze Sound Data",
	.sublist	= com_analyze_list
};

const struct st_shell_command * const com_play_list[] = {
	&com_wav_play,
	&com_mp3_play,
	&com_m4a_play,
	0
};

const struct st_shell_command com_play = {
	.name		= "play",
	.manual_str	= "Play Sound Data",
	.sublist	= com_play_list
};

static int stop(int argc, uchar *argv[])
{
	tprintf("Stop Play\n");

	soundplay_stop_play();

	return 0;
}

const struct st_shell_command com_stop = {
	.name		= "stop",
	.command	= stop,
	.manual_str	= "STOP play Sound Data"
};

static int sound_pause(int argc, uchar *argv[])
{
	tprintf("Pause Play\n");

	soundplay_pause_play();

	return 0;
}

const struct st_shell_command com_pause = {
	.name		= "pause",
	.command	= sound_pause,
	.manual_str	= "PAUSE play Sound Data"
};

static int sound_continue(int argc, uchar *argv[])
{
	tprintf("Continue Play\n");

	soundplay_continue_play();

	return 0;
}

const struct st_shell_command com_continue = {
	.name		= "continue",
	.command	= sound_continue,
	.manual_str	= "CONTINUE play Sound Data"
};

static int cmd_move(int argc, uchar *argv[])
{
	int pos = 0;

	if(argc < 2) {
		tprintf("Now Position : %d/%d\n", audio_frame_count, music_info.sample_count);
	} else {
		pos = dstoi(argv[1]);
		tprintf("Set Position : %d/%d\n", pos, music_info.sample_count);
		soundplay_move_play(pos);
	}

	return 0;
}

const struct st_shell_command com_move = {
	.name		= "move",
	.command	= cmd_move,
	.manual_str	= "Move play position time"
};

static int cmd_volume(int argc, uchar *argv[])
{
	int volume;

	if(argc < 2) {
		ioctl_device(audio_dev, IOCMD_AUDIO_GET_VOLUME, 0, (void *)&volume);
		tprintf("Now Volume : %d\n", volume);
		return 0;
	} else {
		next_volume = dstoi(argv[1]);
		DTPRINTF(0x01, "Set Volume : %d\n", next_volume);
	}

	return 0;
}

const struct st_shell_command com_volume = {
	.name		= "volume",
	.command	= cmd_volume,
	.manual_str	= "Set Volume"
};

static int cmd_artwork(int argc, uchar *argv[])
{
	if(argc < 2) {
		tprintf("Usage : <0|1>\n");
		return 0;
	} else {
		int flg;
		flg = dstoi(argv[1]);
		set_mp4_decode_artwork(flg);
		set_id3_decode_artwork(flg);
	}

	return 0;
}

const struct st_shell_command com_artwork = {
	.name		= "artwork",
	.command	= cmd_artwork,
	.manual_str	= "Set Artwork Docode"
};

const struct st_shell_command * const com_sound_list[] = {
	&com_analyze,
	&com_play,
	&com_stop,
	&com_pause,
	&com_continue,
	&com_move,
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

static int do_wav_file(uchar *str, uchar *arg)
{
	int rt = 0;

	escaped_str(fname, str);
	//tprintf("%s\n", fname);
	sjisstr_to_utf8str(cname, str, FF_MAX_LFN);
	tprintf("Play Sound File \"%s\"\n", cname);
	tsprintf((char *)cmd, "sound %s wav %s", arg, fname);
	rt = exec_command(cmd);

	return rt;
}

static const struct st_file_operation wav_operation = {
	"WAV", do_wav_file
};


static int do_mp3_file(uchar *str, uchar *arg)
{
	int rt = 0;

	escaped_str(fname, str);
	sjisstr_to_utf8str(cname, str, FF_MAX_LFN);
	tprintf("Play MP3 File \"%s\"\n", cname);
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
	sjisstr_to_utf8str(cname, (unsigned char *)str, FF_MAX_LFN);
	tprintf("Play M4A File \"%s\"\n", cname);
	tsprintf((char *)cmd, "sound %s m4a %s", arg, fname);
	rt = exec_command(cmd);

	return rt;
}

static const struct st_file_operation m4a_operation = {
	"M4A", do_m4a_file
};


static const struct st_file_operation * const file_operation[] = {
	&wav_operation,
	&mp3_operation,
	&m4a_operation,
	0
};


//#define SIZEOFAPPTS	(1024*4)
//#define SIZEOFAPPTS	(1024*16)
#define SIZEOFAPPTS	(1024*48)
static struct st_tcb tcb;
static unsigned int stack[SIZEOFAPPTS/sizeof(unsigned int)];

void startup_soundplay(void)
{
	init_window_table();

	add_shell_command((struct st_shell_command *)&com_sound);

	set_file_operation(file_operation);

	audio_dev = open_device((char *)audio_devname);
	if(audio_dev == 0) {
		tprintf("Cannot open \"%s\"\n", audio_devname);
		return;
	}

	ioctl_device(audio_dev, IOCMD_AUDIO_GET_BUFSIZE, 0, (void *)&audio_buf_size);
	if(audio_buf_size > MAX_FILEBUF) {
		tprintf("Error AUDIO Buffer size = %d\n", audio_buf_size);
		return;
	}
	tprintf("AUDIO Buffer Size : %d\n", audio_buf_size);

	task_exec(soundplay_task, "soundplay", 1, &tcb,
		  stack, SIZEOFAPPTS, 0);
}
