/** @file
    @brief	M3Uファイルデコード

    @date	2018.12.30
    @auther	Takashi SHUDO
*/

#ifndef M3U_H
#define M3U_H

#include "music_info.h"

int m3u_decode(struct st_music_info *info, int fd);

#endif // M3U_H
