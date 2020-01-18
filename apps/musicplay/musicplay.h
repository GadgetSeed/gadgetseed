/** @file
    @brief	音楽再生アプリケーション

    @date	2017.05.01
    @auther	Takashi SHUDO
*/

#ifndef MUSICPLAY_H
#define MUSICPLAY_H

#include "sysevent.h"

void save_config(void);
void init_settings_volume(void);
void init_settings_playmode(void);
void init_settings_playmusic(void);
void init_musicplay_view(void);
void reset_musicplay(void);
void draw_musicplay_view(void);
int sound_proc(struct st_sysevent *event);

#endif // MUSICPLAY_H
