/** @file
    @brief	ファイルリスト

    @date	2012.02.23
    @auther	Takashi SHUDO
*/

#ifndef FILELIST_H
#define FILELIST_H

#include "filesearch.h"

#define MAX_TITLE_LEN	63
#define INIT_ALBUM_MUSIC_NUM	16

struct file_item {
	unsigned char fname[MAX_PATHNAME_LEN+1];
	unsigned char artist[MAX_TITLE_LEN+1];
	unsigned char album[MAX_TITLE_LEN+1];
	unsigned char title[MAX_TITLE_LEN+1];
	int number;
	int time;
	unsigned char track;
	unsigned char last_track;
	struct file_item *next;
};

struct track_filenum {
	unsigned short track;
	unsigned short file_num;
};

struct album_item {
	unsigned char name[MAX_TITLE_LEN+1];
	int track_count;
	int max_track_count;
	struct track_filenum *file_num_list;
	struct album_item *next_album;
};

extern struct file_item **item_list;
extern struct album_item **album_list;
extern int music_file_count;
extern int music_album_count;

int get_album_file_count(int album_num);
int get_music_file_num(int album_num, int music_num);
int create_filelist(unsigned char *path, struct file_ext *ext);

#endif // FILELIST_H
