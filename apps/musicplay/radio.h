/** @file
    @brief	ラジオ再生アプリケーション

    @date	2019.01.09
    @auther	Takashi SHUDO
*/

#ifndef RADIOPLAY_H
#define RADIOPLAY_H

#include "sysevent.h"

typedef enum {
	MODE_RADIO_INFO,
	MODE_RADIO_SEL
} enum_radio_disp_mode;
extern enum_radio_disp_mode radio_disp_mode;

typedef enum {
	RADIOPLAY_STAT_STOP,
	RADIOPLAY_STAT_CONNECTING,
	RADIOPLAY_STAT_PLAYING,
	RADIOPLAY_STAT_CONNECTFAIL
} enum_radioplay_status;
extern enum_radioplay_status radioplay_status;

extern int select_radio_num;

void update_radio_list_view(void);

void on_radio_play(void);
void next_radio_play(void);
void prev_radio_play(void);
void off_radio_play(void);

void init_radio(void);
void draw_radio(void);
int radio_sound_proc(struct st_sysevent *event);
void radio_proc(struct st_sysevent *event);

void suspend_radio(void);
void resume_radio(void);

#endif // RADIOPLAY_H
