/** @file
    @brief	音楽リスト表示

    @date	2017.05.14
    @auther	Takashi SHUDO
*/

#ifndef LIST_VIEW_H
#define LIST_VIEW_H

#include "sysevent.h"

void prepare_album_view(void);
void prepare_music_view(void);
void draw_list_view(void);
void set_play_album_music_num_list_view(int album_num, int music_num);
void set_album_num_album_view(int album_num);
void list_view_proc(struct st_sysevent *event);
void init_list_view(void);

#endif // LIST_VIEW_H
