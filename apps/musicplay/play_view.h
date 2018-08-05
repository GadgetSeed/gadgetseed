/** @file
    @brief	音楽再生表示

    @date	2017.05.02
    @auther	Takashi SHUDO
*/

#ifndef PLAY_VIEW_H
#define PLAY_VIEW_H

#include "sysevent.h"

void init_musicplay_view(void);
void init_play_view(void);

void set_disp_volume(int vol);
void set_music_info(struct st_music_info *info);
void set_playtime(unsigned int time);
void set_audioinfo(struct st_music_info *info);

void draw_play_view(void);
void set_playtime_slider(unsigned int frame_num);
void set_play_button_playing(void);
void play_proc(struct st_sysevent *event);

void draw_searching(void);
void draw_search_count(int count);
void draw_search_album_count(int count);

#endif // PLAY_VIEW_H
