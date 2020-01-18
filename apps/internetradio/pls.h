/** @file
    @brief	PLSファイルデコード

    @date	2019.01.03
    @auther	Takashi SHUDO
*/

#ifndef PLS_H
#define PLS_H

#include "music_info.h"

int pls_decode(struct st_music_info *info, int fd);

#endif // PLS_H
