/** @file
    @brief	MP3ファイル再生

    @date	2017.03.26
    @auther	Takashi SHUDO
*/

#ifndef MP3PLAY_H
#define MP3PLAY_H

#include "music_info.h"

int mp3file_analyze(struct st_music_info *info, unsigned char *fname);

extern const struct st_shell_command com_mp3_analyze;
extern const struct st_shell_command com_mp3_play;

#endif // MP3PLAY_H
