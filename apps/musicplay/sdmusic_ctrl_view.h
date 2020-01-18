/** @file
    @brief	SD音楽再生表示制御

    @date	2017.05.02
    @auther	Takashi SHUDO
*/

#ifndef SDMUSIC_CTRL_VIEW_H
#define SDMUSIC_CTRL_VIEW_H

#include "sysevent.h"
#include "music_info.h"

void init_sdmusic_ctrl_view(void);
void draw_sdmusic_ctrl_view(void);
void set_sdmusic_button_playing(void);
void set_sdmusic_button_stoping(void);
void do_sdmusic_play(void);
void do_sdmusic_pause(void);
void do_play_pause(void);
void sdmusic_ctrl_proc(struct st_sysevent *event);

#endif // SDMUSIC_CTRL_VIEW_H
