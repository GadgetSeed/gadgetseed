/** @file
    @brief	ファイルマネージャボリューム表示

    @date	2017.12.10
    @author	Takashi SHUDO
*/

#ifndef VOLUME_VIEW_H
#define VOLUME_VIEW_H

#include "sysevent.h"

void init_volume_view(void);
void draw_volume_view(void);
void volume_proc(struct st_sysevent *event);

#endif // VOLUME_VIEW_H
