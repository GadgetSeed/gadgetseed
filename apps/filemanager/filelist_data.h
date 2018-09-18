/** @file
    @brief	ファイルリスト

    @date	2017.11.18
    @author	Takashi SHUDO
*/

#ifndef FILELIST_DATA_H
#define FILELIST_DATA_H

#include "file.h"

struct st_fileinfo {
	FS_FILEINFO file_info;
	struct st_fileinfo *next;
};

int create_filelist_data(struct st_fileinfo *fileinfo, const unsigned char *path, int (* filter)(FS_FILEINFO *finfo));
void dispose_filelist_data(struct st_fileinfo *fileinfo);

#endif // FILELIST_DATA_H
