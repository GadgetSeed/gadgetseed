/** @file
    @brief	音楽再生表示(再生時間スライダー)

    @date	2017.05.02
    @auther	Takashi SHUDO
*/

#ifndef PLAYTIME_SLIDER_H
#define PLAYTIME_SLIDER_H

#include "sysevent.h"

void draw_playtime_slider(void);
void activate_playtime_slider(int active);
void set_playtime_slider(unsigned int frame_num);
void playtime_slider_proc(struct st_sysevent *event);

#endif // PLAYTIME_SLIDER_H
