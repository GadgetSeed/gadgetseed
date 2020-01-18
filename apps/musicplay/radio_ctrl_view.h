/** @file
    @brief	ラジオ再生表示制御

    @date	2019.01.13
    @auther	Takashi SHUDO
*/

#ifndef RADIO_CTRL_VIEW_H
#define RADIO_CTRL_VIEW_H

#include "sysevent.h"
#include "music_info.h"

void init_radio_ctrl_view(void);
void draw_radio_ctrl_view(void);
void draw_radio_play_button(void);
void do_on_radio(void);
void do_off_radio(void);
void do_on_off_radio(void);
void radio_ctrl_proc(struct st_sysevent *event);

#endif // RADIO_CTRL_VIEW_H
