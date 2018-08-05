/** @file
    @brief	ファイル検索

    @date	2012.11.11
    @auther	Takashi SHUDO
*/

#ifndef FILESEARCH_H
#define FILESEARCH_H

#define MAX_PATHNAME_LEN	256
#define FNAME_EXT_LEN	3

struct file_ext {
	unsigned char ext[FNAME_EXT_LEN + 1];
};

typedef int (* fileadd_func)(unsigned char *path);

int file_kind(unsigned char *fname, struct file_ext *ext);
int search_file(unsigned char *path, struct file_ext *ext, fileadd_func func);

#endif // FILESEARCH_H
