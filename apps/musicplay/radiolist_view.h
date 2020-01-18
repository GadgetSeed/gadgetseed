/** @file
    @brief	ラジオ選曲表示

    @date	2019.01.02
    @auther	Takashi SHUDO
*/

#ifndef RADIOLIST_VIEW_H
#define RADIOLIST_VIEW_H

#include "sysevent.h"

void init_radiolist_view(void);
void set_radio_num_list_view(int radio_num);
void draw_radiolist_view(void);
void radiolist_view_proc(struct st_sysevent *event);

#endif // RADIOLIST_VIEW_H
