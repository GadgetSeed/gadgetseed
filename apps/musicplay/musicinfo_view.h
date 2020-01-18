/** @file
    @brief	音楽情報表示

    @date	2017.05.02
    @auther	Takashi SHUDO
*/

#ifndef MUSICINFO_VIEW_H
#define MUSICINFO_VIEW_H

#include "sysevent.h"
#include "music_info.h"

extern struct st_music_info *minfo;

void setup_musicinfo(void);
void init_musicinfo_view(void);
void reset_musicinfo(void);
void draw_artwork(void);
void set_music_info(struct st_music_info *info);
void set_playtime(unsigned int time);
void draw_music_info(void);
void set_title_str(uchar *str);
void set_artist_str(uchar *str);
void set_album_str(uchar *str);

void draw_track_view(void);
void draw_musicinfo_view(void);
void musicinfo_proc(struct st_sysevent *event);

#endif // MUSICINFO_VIEW_H
