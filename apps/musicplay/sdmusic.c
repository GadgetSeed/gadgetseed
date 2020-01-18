/** @file
    @brief	SD音楽再生

    @date	2019.01.12
    @auther	Takashi SHUDO
*/

#include "key.h"
#include "shell.h"
#include "random.h"
#include "random.h"
#include "charcode.h"
#define GSLOG_PREFIX	"SDM: "
#include "log.h"
#define LOGLVL	0
#include "task/syscall.h"

#include "sdmusic.h"
#include "sdmusic_ctrl_view.h"
#include "list_view.h"
#include "mode_view.h"
#include "spectrum_view.h"
#include "playtime_slider.h"
#include "musicinfo_view.h"
#include "musicplay.h"
#include "filelist.h"
#include "settings_view.h"

#include "soundplay.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


extern int flg_frame_seek;

int play_album_num = 0;
int play_track_num = 0;
int play_file_num = 0;
int playbackpos = 0;

enum_sdmusicplay_status sdmusicplay_status = SDMUSICPLAY_STAT_NOTREADY;
enum_sd_disp_mode sd_disp_mode = MODE_SD_INFO;

enum_sdmusicplay_status last_sdmusicplay_status;

static void disp_track(void)
{
	GSLOG(LOGLVL, "Track %d/%d\n", play_file_num+1, music_file_count);
}

void set_play_file_num(void)
{
	play_file_num = get_music_file_num(play_album_num, play_track_num);

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	if(musicplay_mode == SDCARD) {
		set_play_album_music_num_list_view(play_album_num, play_track_num);
	}
#else
	set_play_album_music_num_list_view(play_album_num, play_track_num);
#endif
}

void open_now_sdmusic(void)
{
	int rt = 0;

	if(music_file_count == 0) {
		return;
	}

	set_play_file_num();

	DTPRINTF(0x01, "Attach file \"%s\"", sj2utf8(sdmusic_list[play_file_num]->fname));

	soundplay_artwork(1);

	rt = soundplay_open(sdmusic_list[play_file_num]->fname);
	(void)rt;

	DTPRINTF(0x01, " Result %d\n", rt);
}

void start_sdmusic_play(void)
{
	if(music_file_count == 0) {
		return;
	}

	disp_track();

	soundplay_play();
}

void ff_sdmusic_play(void)
{
	if(music_file_count == 0) {
		return;
	}

	switch(flg_repeat) {
	case REPERAT_OFF:
	case REPERAT_ALL:
	{
		if(flg_shuffle == 0) {
			play_track_num ++;

			if(play_track_num >= get_album_file_count(play_album_num)) {
				// 最後の曲を終了
				play_track_num = 0;
				play_album_num ++;
				if(play_album_num >= music_album_count) {
					// 最後のアルバムを終了
					play_album_num = 0;
					if(flg_repeat == REPERAT_OFF) {
						// リピートOFFならば演奏終了
						sdmusicplay_status = SDMUSICPLAY_STAT_STOP;
						set_sdmusic_button_stoping();
						set_playtime(0);
						set_playtime_slider(0);
					}
				}
			}
		} else {
			play_album_num = gen_random() % music_album_count;
			DTPRINTF(0x01, "play_album_num = %d\n", play_album_num);
			play_track_num = gen_random() % get_album_file_count(play_album_num);
			DTPRINTF(0x01, "play_track_num = %d\n", play_track_num);
		}
	}
	break;

	case REPERAT_1ALBUM:
	{
		if(flg_shuffle == 0) {
			play_track_num ++;

			if(play_track_num >= get_album_file_count(play_album_num)) {
				// 最後の曲を終了
				play_track_num = 0;
			}
		} else {
			play_track_num = gen_random() % get_album_file_count(play_album_num);
			DTPRINTF(0x01, "play_track_num = %d\n", play_track_num);
		}
	}
	break;

	case REPERAT_1MUSIC:
	{
		// 同じ曲を繰り返し
	}
	break;

	default:
		break;
	}

	set_play_file_num();
}

void fr_sdmusic_play(void)
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

void stop_sdmusic_play(void)
{
	if(music_file_count == 0) {
		return;
	}

	soundplay_stop();
	soundplay_close();

	set_play_album_music_num_list_view(play_album_num, play_track_num);
}

void pause_sdmusic_play(void)
{
	if(music_file_count == 0) {
		return;
	}

	soundplay_pause();

	set_play_album_music_num_list_view(play_album_num, play_track_num);
}

static void sdcard_list_proc(struct st_sysevent *event)
{
	switch(sd_disp_mode) {
	case MODE_SD_INFO:
		break;

	case MODE_ALBUM_SEL:
	case MODE_MUSIC_SEL:
		list_view_proc(event);
		break;

	default:
		break;
	}
}

void init_sdmusic(void)
{
	init_list_view();
	init_sdmusic_ctrl_view();
}

void draw_sdmusic(void)
{
	draw_sdmusic_ctrl_view();

	switch(sd_disp_mode) {
	case MODE_SD_INFO:
		draw_musicinfo_view();
		draw_spectrum(0);
		break;

	case MODE_ALBUM_SEL:
	case MODE_MUSIC_SEL:
		draw_list_view();
		break;

	default:
		break;
	}

	draw_playtime_slider();
}

int sdmusic_sound_proc(struct st_sysevent *event)
{
	switch(event->what) {
	case 200:
		DTPRINTF(0x01, "Artwork decode event\n");
		draw_artwork();
		break;

	case EVT_SOUND_ANALYZE:
		{
			struct st_audio_spectrum *asp = (struct st_audio_spectrum *)event->private_data;

			if(sd_disp_mode == MODE_SD_INFO) {
				//tprintf("F:%ld\n", asp->frame_num);
				draw_spectrum(asp);
			}

			if(flg_frame_seek == 0) {
				//set_playtime_slider(asp->frame_num);
				set_playtime_slider(audio_frame_count);
			} else {
				flg_frame_seek = 0;
			}
		}
		break;

	case EVT_SOUND_PREPARED:
		GSLOG(LOGLVL, "Prepare success\n");
		if(sdmusicplay_status == SDMUSICPLAY_STAT_PLAY) {
			start_sdmusic_play();
		} else {
			sdmusicplay_status = SDMUSICPLAY_STAT_OPENED;
		}
		set_music_info((struct st_music_info *)(event->private_data));
		set_playtime_slider(audio_frame_count);
		save_config();
		break;

	case EVT_SOUND_START:
		GSLOG(LOGLVL, "Play start success\n");
		set_music_info((struct st_music_info *)(event->private_data));
		set_play_album_music_num_list_view(play_album_num, play_track_num);
		set_sdmusic_button_playing();
		break;

	case EVT_SOUND_END:
		GSLOG(LOGLVL, "Play end\n");
		soundplay_close();
		if(sdmusicplay_status == SDMUSICPLAY_STAT_PLAY) {
			ff_sdmusic_play();
			open_now_sdmusic();
		}
		break;

	case EVT_SOUND_STOP:
		GSLOG(LOGLVL, "Play stop success\n");
		if(sd_disp_mode == MODE_SD_INFO) {
			draw_spectrum(0);
		}
		if(sdmusicplay_status == SDMUSICPLAY_STAT_PLAY) {
			start_sdmusic_play();
		}
		break;

	case EVT_SOUND_PAUSE:
		GSLOG(LOGLVL, "Play pause success\n");
		if(sd_disp_mode == MODE_SD_INFO) {
			draw_spectrum(0);
		}
		break;

	case EVT_SOUND_STATUS:
	{
		if(flg_setting != 0) {
			return 0;
		}

		set_playtime(*(unsigned int *)event->private_data);
	}
	break;

	case EVT_KEYDOWN:
	case EVT_KEYDOWN_REPEAT:
		switch(event->arg) {
		case KEY_GB_ESC:
		case KEY_GB_BS:
		case KEY_GB_SPACE:
			GSLOG(LOGLVL, "PLAY/PAUSE\n");
			do_play_pause();
			save_config();
			break;

		default:
			break;
		}

	default:
		break;
	}

	return 0;
}

void sdmusic_proc(struct st_sysevent *event)
{
	sdmusic_ctrl_proc(event);

	sdcard_list_proc(event);

	sdmusic_sound_proc(event);
}

void suspend_sdmusic(void)
{
	last_sdmusicplay_status = sdmusicplay_status;
	playbackpos = soundplay_playbackpos();
	stop_sdmusic_play();
}

void resume_sdmusic(void)
{
	open_now_sdmusic();
	soundplay_seek(playbackpos);
}
