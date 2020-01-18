/** @file
    @brief	M4A(AAC)ファイル再生

    @date	2017.04.08
    @auther	Takashi SHUDO
*/

#ifndef M4APLAY_H
#define M4APLAY_H

#include "shell.h"

extern const struct st_shell_command com_m4a_open;

int m4afile_open(uchar *fname);
void m4afile_seek(int pos);

#endif // M4APLAY_H
