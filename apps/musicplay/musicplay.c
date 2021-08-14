/** @file
    @brief	音楽再生アプリケーション

    @date	2017.05.01
    @auther	Takashi SHUDO
*/

#include "log.h"
#include "task/syscall.h"
#include "device.h"
#include "device/input_ioctl.h"

#include "musicplay_view.h"
#include "musicplay.h"
#include "filelist.h"
#include "sdmusic.h"
#include "sdmusic_ctrl_view.h"
#include "musicinfo_view.h"
#include "playtime_slider.h"
#include "list_view.h"
#include "volume_view.h"
#include "spectrum_view.h"
#include "appsetting.h"
#include "storage.h"
#include "device/qspi_ioctl.h"
#include "config_view.h"
#include "settings_view.h"
#include "clock_view.h"
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO	/// $gsc インターネットラジオアプリを有効にする
#include "radio.h"
#include "mode_view.h"
#include "radio_ctrl_view.h"
#include "radiolist_view.h"
#include "dialogbox/netset/netset.h"
#endif

#include "soundplay.h"

#ifdef GSC_COMP_ENABLE_GSFFS
#define MUSICPLAY_CONFFILE	"1:musiplay.cfg"
#else
#define MUSICPLAY_CONFFILE	"0:musiplay.cfg"
#endif

//#define DEBUGTBITS 0x01
#include "dtprintf.h"

unsigned short backup_crc;

struct st_conf_header musicplay_conf[] = {
	{ "VOLUME", CFGTYPE_INT, &volume },
	{ "MUTE", CFGTYPE_INT, &flg_mute },
	{ "SDDISP", CFGTYPE_BYTE, &sd_disp_mode },
	{ "SDALBUMNUM", CFGTYPE_INT, &play_album_num },
	{ "SDTRACKNUM", CFGTYPE_INT, &play_track_num },
	{ "SDFILENUM", CFGTYPE_INT, &play_file_num },
	{ "SDSHUFFLE", CFGTYPE_INT, &flg_shuffle },
	{ "SDREPEAT", CFGTYPE_INT, &flg_repeat },
	{ "SDPLAYBACKPOS", CFGTYPE_INT, &playbackpos },
//	{ "SDPLAYSTATUS", CFGTYPE_INT, &sdmusicplay_status },
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	{ "MODE", CFGTYPE_BYTE, &musicplay_mode },
	{ "RADIODISP", CFGTYPE_BYTE, &radio_disp_mode },
	{ "RADIONUM", CFGTYPE_INT, &select_radio_num },
//	{ "RADIOSTATUS", CFGTYPE_INT, &radioplay_status },
#endif
	{ "MINFOCRC", CFGTYPE_HWORD, &backup_crc },
	{ 0, 0, 0 }
};

static struct st_tcb tcb;
#define SIZEOFSTACK	(1024*8)
static unsigned int stack[SIZEOFSTACK/sizeof(unsigned int)] ATTR_STACK;

void save_config(void)
{
	DTPRINTF(0x01, "Save config\n");

	playbackpos = soundplay_playbackpos();

	save_appsetting((uchar *)MUSICPLAY_CONFFILE, musicplay_conf);
}

int sound_proc(struct st_sysevent *event)
{
	int rt = 0;

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	switch(musicplay_mode) {
	case SDCARD:
		rt = sdmusic_sound_proc(event);
		break;

	case RADIO:
		rt = radio_sound_proc(event);
		break;

	default:
		break;
	}
#else
	rt = sdmusic_sound_proc(event);
#endif

	return rt;
}

static int musicplay_proc(void)
{
	while(1) {
		struct st_sysevent event;

//		if(get_event(&event, 50))
		get_event(&event, 50);
		{
			config_proc(&event);

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
			mode_proc(&event);

			switch(musicplay_mode) {
			case SDCARD:
				sdmusic_proc(&event);
				break;

			case RADIO:
				radio_proc(&event);
				break;

			default:
				break;
			}
#else
			sdmusic_proc(&event);
#endif

			musicinfo_proc(&event);
			playtime_slider_proc(&event);
			volume_proc(&event);
			clock_proc(&event);
		}
	}

	gslog(1, "Music Play end\n");

	return 0;
}

const struct st_rect screen_rect = { 0, 0, GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT };
extern const unsigned int fore_color;
extern const unsigned int back_color;

void init_musicplay_view(void)
{
	clear_screen();
	set_draw_mode(GRP_DRAWMODE_NORMAL);

	set_forecolor(back_color);
	draw_fill_rect((struct st_rect *)&screen_rect);

	set_forecolor(fore_color);
	set_backcolor(back_color);
}

void init_settings_volume(void)
{
	volume = VOL_DEF;
	flg_mute = 0;
}

void init_settings_playmode(void)
{
	sd_disp_mode = MODE_SD_INFO;
	flg_shuffle = 0;
	flg_repeat = REPERAT_OFF;
}

void init_settings_playmusic(void)
{
	play_album_num = 0;
	play_track_num = 0;
	play_file_num = 0;
	playbackpos = 0;
}

void reset_musicplay(void)
{
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	if(musicplay_mode == SDCARD) {
		stop_sdmusic_play();
		do_sdmusic_pause();
	} else {
		off_radio_play();
	}
#else
	stop_sdmusic_play();
	do_sdmusic_pause();
#endif
}

void draw_musicplay_view(void)
{

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	init_radio();
	init_mode_view();

	draw_mode_view();

	if(musicplay_mode == RADIO) {
		draw_radio();
	} else {
		draw_sdmusic();
	}
#else
	draw_sdmusic();
#endif

	draw_volume_view();
	draw_config_view();
	draw_clock_view();
}

static void prepare_filelist(void)
{
	int rt = 0;

#ifndef GSC_TARGET_SYSTEM_EMU
	struct st_device *dev;

	dev = open_device(DEF_DEV_NAME_INPUT);
	if(dev != 0) {
		int keybits = ioctl_device(dev, IOCMD_INPUT_SCAN_LINE, 0, 0);
		if(keybits != 0) {
			rt = 1;
			gslog(0, "Force create music file data\n");
		} else {
			rt = load_filelist();
		}
	} else {
		gslog(0, "Cannot scan key\n");
		rt = 1;
	}
#else
	rt = load_filelist();
//	create_filelist();
//	rt = 0;
#endif

	if(rt > 0) {
		create_filelist();
	}
}

static int musicplay_task(void *arg)
{
	int rtn;

	init_musicplay_view();

	rtn = load_appsetting((uchar *)MUSICPLAY_CONFFILE, musicplay_conf);
	if(rtn < 0) {
		save_config();
	}
	// [TODO] musicplay_confの値に異常がないかチェック

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	load_network_setting();
#endif

	prepare_filelist();

	if(backup_crc == minfo_crc) {
		// SDカードの情報を合っている
	} else {
		// SDカードの情報を合っていないので再生ファイル状態は初期化
		gslog(1, "Reset play file number(CRC:%04X,%04X)\n", backup_crc, minfo_crc);
		backup_crc = minfo_crc;
		init_settings_playmusic();
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
		select_radio_num = 0;
#endif
		backup_crc = minfo_crc;
		save_config();
	}

	init_sdmusic();

	init_musicinfo_view();
	init_volume_view();
	init_config_view();
	init_settings_view();
	init_clock_view();

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	init_radio_ctrl_view();

	if(musicplay_mode == SDCARD) {
		open_now_sdmusic();
		if(playbackpos != 0) {
			soundplay_seek(playbackpos);
		}
	}
#else
	open_now_sdmusic();
	if(playbackpos != 0) {
		soundplay_seek(playbackpos);
	}
#endif

	draw_musicplay_view();

	musicplay_proc();

	return 0;
}

void startup_musicplay(void)
{
#ifdef GSC_COMP_ENABLE_GSFFS
	mount_storage(1, DEF_DEV_NAME_QSPI, FSNAME_GSFFS);
#endif

	task_exec(musicplay_task, "musicplay", TASK_PRIORITY_APP_MID, &tcb,
		  stack, SIZEOFSTACK, 0);
}
