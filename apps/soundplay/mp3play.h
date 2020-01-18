/** @file
    @brief	MP3ファイル再生

    @date	2017.03.26
    @auther	Takashi SHUDO
*/

#ifndef MP3PLAY_H
#define MP3PLAY_H

#include "shell.h"

extern const struct st_shell_command com_mp3_open;

int mp3file_open(uchar *fname);
int mp3stream_open(uchar *fname, uchar *stream_title);
void mp3file_seek(int pos);

#endif // MP3PLAY_H
