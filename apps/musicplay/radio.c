/** @file
    @brief	ラジオ再生アプリケーション

    @date	2019.01.09
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
#include "tkprintf.h"
#include "log.h"
#include "task/syscall.h"

#include "radio.h"
#include "radio_ctrl_view.h"
#include "radiolist_view.h"
#include "musicplay_view.h"
#include "filelist.h"
#include "musicinfo_view.h"
#include "spectrum_view.h"
#include "playtime_slider.h"
#include "mode_view.h"
#include "radiobuffer_view.h"
#include "internetradio.h"
#include "ir_stream.h"
#include "settings_view.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


enum_radio_disp_mode radio_disp_mode = MODE_RADIO_INFO;
enum_radioplay_status radioplay_status = RADIOPLAY_STAT_STOP;
enum_radioplay_status last_radioplay_status;
int select_radio_num = 0;

void update_radio_list_view(void)
{
	set_radio_num_list_view(select_radio_num);
}

void on_radio_play(void)
{
	if(radio_count == 0) {
		return;
	}

	switch(radioplay_status) {
	case RADIOPLAY_STAT_STOP:
	case RADIOPLAY_STAT_CONNECTING:
	case RADIOPLAY_STAT_CONNECTFAIL:
		break;

	case RADIOPLAY_STAT_PLAYING:
		return;
		break;

	default:
		break;
	}

	tprintf("BROADCASTER : %s\n", radio_list[select_radio_num]->broadcaster_name);
	tprintf("URL : %s\n", radio_list[select_radio_num]->url);

	set_title_str((uchar *)"Connecting...");
	set_artist_str((uchar *)radio_list[select_radio_num]->url);

	open_internetradio((uchar *)radio_list[select_radio_num]->url);
}

static void prepare_radioinfo(void)
{
	reset_musicinfo();
	set_album_str((uchar *)radio_list[select_radio_num]->broadcaster_name);
	update_radio_list_view();

	if(radioplay_status != RADIOPLAY_STAT_STOP) {
		off_radio_play();
		radioplay_status = RADIOPLAY_STAT_CONNECTING;
		task_sleep(10);
		on_radio_play();
	}
}

void next_radio_play(void)
{
	if(radio_count == 0) {
		return;
	}

	select_radio_num ++;
	if(select_radio_num >= radio_count) {
		select_radio_num = 0;
	}

	prepare_radioinfo();
}

void prev_radio_play(void)
{
	if(radio_count == 0) {
		return;
	}

	select_radio_num --;
	if(select_radio_num < 0) {
		select_radio_num = (radio_count - 1);
	}

	prepare_radioinfo();
}

void off_radio_play(void)
{
	if(radio_count == 0) {
		return;
	}

	close_internetradio();

	task_sleep(10);
}

static void radio_list_proc(struct st_sysevent *event)
{
	switch(radio_disp_mode) {
	case MODE_RADIO_INFO:
		break;

	case MODE_RADIO_SEL:
		radiolist_view_proc(event);
		break;

	default:
		break;
	}
}

void init_radio(void)
{
	if(radio_count != 0) {
		if(select_radio_num < radio_count) {
			set_album_str((uchar *)radio_list[select_radio_num]->broadcaster_name);
		} else {
			set_album_str((uchar *)"");
		}
	}

	init_radiolist_view();
	init_radiobuffer_view();
}

void draw_radio(void)
{
	draw_radio_ctrl_view();
	draw_track_view();

	switch(radio_disp_mode) {
	case MODE_RADIO_INFO:
		draw_musicinfo_view();
		draw_spectrum(0);
		break;

	case MODE_RADIO_SEL:
		draw_radiolist_view();
		break;

	default:
		break;
	}

	draw_radiobuffer_view();
}

int radio_sound_proc(struct st_sysevent *event)
{
	switch(event->what) {
	case EVT_SOUND_ANALYZE:
		{
			struct st_audio_spectrum *asp = (struct st_audio_spectrum *)event->private_data;

			if(radio_disp_mode == MODE_RADIO_INFO) {
				//tprintf("F:%ld\n", asp->frame_num);
				draw_spectrum(asp);
			}
		}
		break;

	case EVT_SOUND_PREPARED:
		gslog(1, "Prepare start success\n");
		set_music_info((struct st_music_info *)(event->private_data));
		set_playtime_slider(0);
		break;

	case EVT_SOUND_START:
		gslog(1, "Play start success\n");
		radioplay_status = RADIOPLAY_STAT_PLAYING;
		draw_radio_play_button();
		update_radio_list_view();
		break;

	case EVT_SOUND_END:
		gslog(1, "Play end\n");
		radioplay_status = RADIOPLAY_STAT_STOP;
		draw_radio_play_button();
		update_radio_list_view();
		break;

	case EVT_SOUND_STOP:
		gslog(1, "Play stop success\n");
		if(radio_disp_mode == MODE_RADIO_INFO) {
			draw_spectrum(0);
		}
		draw_radio_play_button();
		update_radio_list_view();
		break;

	case EVT_IRADIO_DISCONNECTED:
		radioplay_status = RADIOPLAY_STAT_STOP;
		gslog(0, "Radio disconnected\n");
		create_event(EVT_SOUND_END, 0, 0);
		break;

	case EVT_IRADIO_CONNECTED:
		gslog(0, "Radio connected\n");
		radioplay_status = RADIOPLAY_STAT_CONNECTING;
		draw_radio_play_button();
		break;

	case EVT_IRADIO_RECEIVE:
	{
		if(flg_setting != 0) {
			return 0;
		}

		set_radiobuffer_size(internet_radio_stream_size());
	}
	break;

	case EVT_IRADIO_ERROR:
		gslog(0, "Radio connect error\n");
		set_title_str((uchar *)"Connect ERROR");
		draw_music_info();
		radioplay_status = RADIOPLAY_STAT_CONNECTFAIL;
		draw_radio_play_button();
		break;

	case EVT_IRADIO_ABORT:
		gslog(0, "Radio abort\n");
		set_title_str((uchar *)"Connect ABORT");
		draw_music_info();
		radioplay_status = RADIOPLAY_STAT_CONNECTFAIL;
		draw_radio_play_button();
		break;

	case EVT_SOUND_PAUSE:
		gslog(1, "Play pause success\n");
		radioplay_status = RADIOPLAY_STAT_CONNECTING;
		draw_radio_play_button();
		update_radio_list_view();
		if(radio_disp_mode == MODE_RADIO_INFO) {
			draw_spectrum(0);
		}
		break;

	case EVT_SOUND_STATUS:
	{
		if(flg_setting != 0) {
			return 0;
		}

		set_playtime(*(unsigned long *)event->private_data);
	}
	break;

	case EVT_KEYDOWN:
	case EVT_KEYDOWN_REPEAT:
		switch(event->arg) {
		case KEY_GB_ESC:
		case KEY_GB_BS:
		case KEY_GB_SPACE:
			gslog(1, "Radio On/Off\n");
			do_on_off_radio();
			break;

		default:
			break;
		}

	default:
		break;
	}

	return 0;
}

void radio_proc(struct st_sysevent *event)
{
	radio_ctrl_proc(event);

	radio_list_proc(event);

	radio_sound_proc(event);
}

void suspend_radio(void)
{
	last_radioplay_status = radioplay_status;
	off_radio_play();
}

void resume_radio(void)
{
	reset_musicinfo();
	init_radio();

	if(last_radioplay_status != RADIOPLAY_STAT_STOP) {
		radioplay_status = RADIOPLAY_STAT_CONNECTING;
		on_radio_play();
	}
}
