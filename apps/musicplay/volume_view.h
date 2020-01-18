/** @file
    @brief	ボリューム表示

    @date	2017.06.03
    @auther	Takashi SHUDO
*/

#ifndef VOLUME_VIEW_H
#define VOLUME_VIEW_H

#include "sysevent.h"

#define VOL_MAX		 99
#define VOL_MIN		  0
#define VOL_DEF		 30

extern int volume;
extern int flg_mute;

void set_disp_volume(int vol);
void init_volume_view(void);
void set_volume_view(int vol, int mute);
void draw_volume_view(void);
void set_volume(unsigned short vol);
void volume_proc(struct st_sysevent *event);

#endif // VOLUME_VIEW_H
