/** @file
    @brief	MP4TAGデコード

    @date	2017.04.08
    @auther	Takashi SHUDO
*/

#ifndef MP4TAG_H
#define MP4TAG_H

#include "music_info.h"

#define MP4TAG_HEADER_SIZE	(8)

void set_mp4_decode_artwork(int flg_env);
int mp4tag_decode(struct st_music_info *info, tag_read_func tag_read, tag_seekcur_func tag_seekcur, tag_tell_func tag_tell);
void mp4tag_dispose(struct st_music_info *info);

#endif // MP4TAG_H
