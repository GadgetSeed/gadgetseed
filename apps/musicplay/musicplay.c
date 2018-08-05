/** @file
    @brief	音楽再生アプリケーション

    @date	2017.05.01
    @auther	Takashi SHUDO
*/

#include "shell.h"
#include "tprintf.h"
#include "storage.h"
#include "file.h"
#include "sysevent.h"
#include "key.h"
#include "console.h"
#include "str.h"
#include "memory.h"
#include "random.h"
#include "task/syscall.h"

#include "musicplay_view.h"
#include "musicplay.h"
#include "filelist.h"
#include "play_view.h"
#include "list_view.h"
#include "volume_view.h"
#include "spectrum_view.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


static const struct file_ext audio_file_ext[] = {
	{ "MP3" },
	{ "M4A" },
//	{ "AAC" },
//	{ "WAV" },
	{ "" }
};

int disp_mode = MODE_PLAY;

int musicplay_status = MUSICPLAY_STAT_STOP;

int play_album_num = 0;
int play_track_num = 0;
int play_file_num = 0;

static struct st_tcb tcb;
#define SIZEOFSTACK	(1024*8)
static unsigned int stack[SIZEOFSTACK/sizeof(unsigned int)];

void disp_track(void)
{
	tprintf("Track %d/%d\n", play_file_num+1, music_file_count);
}

void set_play_file_num(void)
{
	play_file_num = get_music_file_num(play_album_num, play_track_num);

	//set_album_num_album_view(play_album_num);
	set_play_album_music_num_list_view(play_album_num, play_track_num);
}

void analyze_now_music(void)
{
	int rt = 0;

	if(music_file_count == 0) {
		return;
	}

	set_play_file_num();

	DTPRINTF(0x01, "Analyze file \"%s\"", sj2utf8(item_list[play_file_num]->fname));

	exec_command((uchar *)"sound artwork 1");

	rt = do_file_operation(item_list[play_file_num]->fname, (uchar *)"analyze");
	(void)rt;

	DTPRINTF(0x01, " Result %d\n",rt);
}

void start_music_play(void)
{
	if(music_file_count == 0) {
		return;
	}

	set_play_file_num();

	disp_track();

	do_file_operation(item_list[play_file_num]->fname, (uchar *)"play");
}

void ff_music_play(void)
{
	if(music_file_count == 0) {
		return;
	}

	if(flg_shuffle == 0) {
		play_track_num ++;

		if(play_track_num >= get_album_file_count(play_album_num)) {
			play_track_num = 0;
			play_album_num ++;
			if(play_album_num >= music_album_count) {
				play_album_num = 0;
			}
		}
	} else {
		play_album_num = genrand_int32() % music_album_count;
		DTPRINTF(0x01, "play_album_num = %d\n", play_album_num);
		play_track_num = genrand_int32() % get_album_file_count(play_album_num);
		DTPRINTF(0x01, "play_track_num = %d\n", play_track_num);
	}

	set_play_file_num();
}

void fr_music_play(void)
{
	if(music_file_count == 0) {
		return;
	}

	play_track_num --;

	if(play_track_num < 0) {
		play_album_num --;
		if(play_album_num < 0) {
			play_album_num = (music_album_count - 1);
		}
		play_track_num = (get_album_file_count(play_album_num) - 1);
	}

	set_play_file_num();
}

void stop_music_play(void)
{
	if(music_file_count == 0) {
		return;
	}

	exec_command((uchar *)"sound stop");

	set_play_album_music_num_list_view(play_album_num, play_track_num);
}

void pause_music_play(void)
{
	if(music_file_count == 0) {
		return;
	}

	exec_command((uchar *)"sound pause");

	set_play_album_music_num_list_view(play_album_num, play_track_num);
}

void continue_music_play(void)
{
	if(music_file_count == 0) {
		return;
	}

	exec_command((uchar *)"sound continue");

	set_play_album_music_num_list_view(play_album_num, play_track_num);
}

extern int flg_frame_move;

static void sound_proc(struct st_sysevent *event)
{
	switch(event->what) {
	case EVT_SOUND_ANALYZE:
		{
			struct st_audio_spectrum *asp = (struct st_audio_spectrum *)event->private_data;
			if(disp_mode == MODE_PLAY) {
				//tprintf("F:%ld\n", asp->frame_num);
				draw_spectrum(asp);
			}

			if(flg_frame_move == 0) {
				set_playtime_slider(asp->frame_num);
			} else {
				flg_frame_move = 0;
			}
		}
		break;

	case EVT_SOUND_PREPARE:
		tprintf("Prepare start success\n");
		set_music_info((struct st_music_info *)(event->private_data));
		set_playtime_slider(0);
		break;

	case EVT_SOUND_START:
		tprintf("Play start success\n");
		set_music_info((struct st_music_info *)(event->private_data));
		musicplay_status = MUSICPLAY_STAT_PLAY;
		set_play_album_music_num_list_view(play_album_num, play_track_num);
		set_play_button_playing();
		break;

	case EVT_SOUND_END:
		tprintf("Play end\n");
		if(musicplay_status != MUSICPLAY_STAT_STOP) {
			ff_music_play();
			task_sleep(10);	// soudplayタスクが終了するまで待つ(暫定対策)
			start_music_play();
		}
		break;

	case EVT_SOUND_STOP:
		tprintf("Play stop success\n");
		if(disp_mode == MODE_PLAY) {
			draw_spectrum(0);
		}
		if(musicplay_status != MUSICPLAY_STAT_STOP) {
			task_sleep(10);	// soudplayタスクが終了するまで待つ(暫定対策)
			start_music_play();
		}
		break;

	case EVT_SOUND_PAUSE:
		tprintf("Play pause success\n");
		musicplay_status = MUSICPLAY_STAT_PAUSE;
		if(disp_mode == MODE_PLAY) {
			draw_spectrum(0);
		}
		break;

	case EVT_SOUND_CONTINUE:
		tprintf("Play continue success\n");
		musicplay_status = MUSICPLAY_STAT_PLAY;
		break;

	case EVT_SOUND_STATUS:
		set_playtime(*(unsigned long *)event->private_data);
		break;

	case EVT_KEYDOWN:
	case EVT_KEYDOWN_REPEAT:
		switch(event->arg) {
		case KEY_GB_ESC:
		case KEY_GB_BS:
		case KEY_GB_SPACE:
			tprintf("Play stop\n");
			stop_music_play();
			break;

		default:
			break;
		}

	default:
		break;
	}
}

int musicplay_proc(void)
{
	analyze_now_music();

	set_volume(VOL_DEF);

	while(1) {
		struct st_sysevent event;

		if(get_event(&event, 50)) {
			switch(disp_mode) {
			case MODE_PLAY:
				break;

			case MODE_ALBUM_SEL:
			case MODE_MUSIC_SEL:
				list_view_proc(&event);
				break;

			default:
				break;
			}

			play_proc(&event);
			sound_proc(&event);
			volume_proc(&event);
		}
	}

	tprintf("Music Play end\n");

	return 0;
}

static int musicplay_task(char *arg)
{
	init_musicplay_view();
	draw_searching();

	tprintf("Audio File searching...\n");
	create_filelist((unsigned char *)"0:", (struct file_ext *)audio_file_ext);

	init_list_view();
	init_play_view();
	init_volume_view();

	draw_play_view();
	draw_volume_view();
	draw_spectrum(0);

	musicplay_proc();

	return 0;
}

void startup_musicplay(void)
{
	task_exec(musicplay_task, "musicplay", 2, &tcb,
		  stack, SIZEOFSTACK, 0);
}
