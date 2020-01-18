/** @file
    @brief	音声ファイル操作

    @date	2017.02.12
    @auther	Takashi SHUDO
*/

#ifndef SOUNDFILE_H
#define SOUNDFILE_H

#include "file.h"

#define MAX_FILEBUF	(1024*10)

extern unsigned char comp_audio_data[MAX_FILEBUF];
extern unsigned char comp_audio_file_name[FF_MAX_LFN + 1];

int soundfile_open(unsigned char *fname);
int soundfile_read(unsigned char *buf, int size);
int soundfile_seekcur(int pos);
int soundfile_seekset(int pos);
int soundfile_tell(void);
int soundfile_size(void);
void soundfile_close(void);

#endif // SOUNDFILE_H
