/** @file
    @brief	ShoutCast プロトコル

    @date	2018.09.02
    @auther	Takashi SHUDO
*/

#ifndef SHOUTCAST_H
#define SHOUTCAST_H

#include "str.h"
#include "../soundplay/music_info.h"

int start_shoutcast_stream(uchar *path);
int decode_shoutcast_message(struct st_music_info *music_info);
int decode_shoutcast_stream(struct st_music_info *music_info, unsigned char *stream);

#endif // SHOUTCAST_H
