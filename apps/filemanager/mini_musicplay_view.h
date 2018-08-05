/** @file
    @brief	ファイルマネージャミニミュージックプレーヤ

    @date	2017.12.10
    @author	Takashi SHUDO
*/

#ifndef MINI_MUSICPLAY_VIEW_H
#define MINI_MUSICPLAY_VIEW_H

#include "sysevent.h"

void init_mini_musicplay_view(void);
void draw_mini_musicplay_view(void);
void mini_musicplay_proc(struct st_sysevent *event);

#endif // MINI_MUSICPLAY_VIEW_H
