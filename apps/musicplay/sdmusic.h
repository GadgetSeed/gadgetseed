/** @file
    @brief	SD音楽再生

    @date	2019.01.12
    @auther	Takashi SHUDO
*/

#ifndef SDCARD_H
#define SDCARD_H

#include "sysevent.h"

typedef enum {
	MODE_SD_INFO,
	MODE_ALBUM_SEL,
	MODE_MUSIC_SEL
} enum_sd_disp_mode;
extern enum_sd_disp_mode sd_disp_mode;

typedef enum {
	SDMUSICPLAY_STAT_NOTREADY,
	SDMUSICPLAY_STAT_OPENED,
	SDMUSICPLAY_STAT_PLAY,
	SDMUSICPLAY_STAT_STOP
} enum_sdmusicplay_status;
extern enum_sdmusicplay_status sdmusicplay_status;

extern int play_album_num;
extern int play_track_num;
extern int play_file_num;
extern int playbackpos;
extern int flg_shuffle;

#define REPERAT_OFF	0
#define REPERAT_ALL	1
#define REPERAT_1ALBUM	2
#define REPERAT_1MUSIC	3
extern int flg_repeat;

void set_play_file_num(void);
void open_now_sdmusic(void);
void start_sdmusic_play(void);
void ff_sdmusic_play(void);
void fr_sdmusic_play(void);
void stop_sdmusic_play(void);
void pause_sdmusic_play(void);
void continue_sdmusic_play(void);

void init_sdmusic(void);
void draw_sdmusic(void);
int sdmusic_sound_proc(struct st_sysevent *event);
void sdmusic_proc(struct st_sysevent *event);

void suspend_sdmusic(void);
void resume_sdmusic(void);

#endif // SDCARD_H
