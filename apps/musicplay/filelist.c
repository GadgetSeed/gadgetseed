/** @file
    @brief	ファイルリスト

    @date	2012.02.23
    @auther	Takashi SHUDO
*/

#include "filelist.h"
#include "filesearch.h"
#include "memory.h"
#include "str.h"
#include "charcode.h"
#include "file.h"
#include "tkprintf.h"
#include "tprintf.h"
#include "random.h"
#include "../soundplay/music_info.h"
#include "../soundplay/mp4tag.h"
#include "../soundplay/id3tag.h"
#include "play_view.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


static struct st_music_info music_info;
static struct file_item item_root, *last_item;
static struct album_item album_item_root, *album_item_last;
static int music_fd = -1;

struct file_item **item_list;
struct album_item **album_list;

int music_file_count = 0;
int music_album_count = 0;

static int mp_openfile(unsigned char *fname)
{
	if(music_fd < 0) {
		music_fd = open_file(fname, FA_READ);
		if(music_fd < 0) {
			tprintf("File open error %s %d\n", sj2utf8(fname), music_fd);
		}
	}

	return music_fd;
}

static int mp_readfile(unsigned char *buf, int size)
{
	int rsize;

	rsize = read_file(music_fd, buf, size);

	return rsize;
}

static int mp_seekfile(unsigned char *buf, int size)
{
	seek_file(music_fd, size, SEEK_CUR);

	return size;
}

static void mp_closefile(void)
{
	if(music_fd >= 0) {
		close_file(music_fd);
		music_fd = -1;
	}
}

static struct album_item * search_album(unsigned char *name)
{
	struct album_item *album = album_item_root.next_album;
	struct album_item *tmp_album;

	while(album != 0) {
		//tprintf("Compare \"%s\" :  \"%s\"\n", album->name, name);
		if(strcomp(album->name, name) == 0) {
			//tprintf("Fined album %s\n", name);
			return album;
		}
		album = album->next_album;
	}

	// アルバムが見つからなかったので
#ifdef PRINT_MUSICFILE_INFO
	tprintf("New Album %s\n", sj2utf8(name));
#endif
	tmp_album = (struct album_item *)alloc_memory(sizeof(struct album_item));
	if(tmp_album == 0) {
		eprintf("Cannot alloc album_item\n");
		return 0;
	} else {
		tmp_album->max_track_count = INIT_ALBUM_MUSIC_NUM;
		tmp_album->file_num_list = (struct track_filenum *)alloc_memory(
				sizeof(struct track_filenum) * tmp_album->max_track_count);
		strncopy(tmp_album->name, name, MAX_TITLE_LEN);
		tmp_album->track_count = 0;

		album_item_last->next_album = tmp_album;
		album_item_last = tmp_album;
		album_item_last->next_album = 0;

		music_album_count ++;
		draw_search_album_count(music_album_count);
	}

	return album_item_last;
}

static void add_item(struct album_item *album, struct file_item *item)
{
	int i;

	for(i=0; i<album->track_count; i++) {
		if(item->track < album->file_num_list[i].track) {
			int j;
			for(j=album->track_count; j>=i; j--) {
				album->file_num_list[j] = album->file_num_list[j-1];
			}
			album->file_num_list[i].track = item->track;
			album->file_num_list[i].file_num = item->number;
			album->track_count ++;
			return;
		}
	}

	album->file_num_list[album->track_count].track = item->track;
	album->file_num_list[album->track_count].file_num = item->number;
	album->track_count ++;
}

static void add_album(struct file_item *item)
{
	struct album_item *album;

	album = search_album(item->album);

	if(album->track_count < album->max_track_count) {
		add_item(album, item);
	} else {
		struct track_filenum *fn_list = album->file_num_list;
		album->file_num_list = (struct track_filenum *)alloc_memory(
				sizeof(struct track_filenum) * (album->max_track_count + INIT_ALBUM_MUSIC_NUM));
		memorycopy(album->file_num_list, fn_list, sizeof(struct track_filenum) * album->max_track_count);
		free_memory(fn_list);
		album->max_track_count += INIT_ALBUM_MUSIC_NUM;

		add_item(album, item);
	}
}

#define EXT_LEN 12

static int add_file(unsigned char *fname)
{
	//tprintf("%d %s\n", music_file_count, fname);

	unsigned char ext[EXT_LEN + 1] = {0};
	int rtn = 0;

	get_filename_extension(ext, fname, EXT_LEN);
	str2cap(ext);

	rtn = mp_openfile(fname);
	if(rtn >= 0) {
		if(strcomp((unsigned char *)"M4A", ext) == 0) {
			rtn = mp4tag_decode(&music_info, mp_readfile, mp_seekfile);
		} else
			if(strcomp((unsigned char *)"MP3", ext) == 0) {
			rtn = id3tag_decode(&music_info, mp_readfile, mp_seekfile);
		} else {
			tprintf("Unknown file type %s\n", sj2utf8(fname));
			rtn = 0;
		}
		mp_closefile();

		if(rtn > 0) {
			struct file_item *tmp_item;

			//disp_music_info(&music_info);

			tmp_item = (struct file_item *)alloc_memory(sizeof(struct file_item));

			if(tmp_item == 0) {
				eprintf("Cannot alloc file_item\n");
				return -1;
			} else {
				strncopy(tmp_item->fname, fname, MAX_PATHNAME_LEN);
				strncopy(tmp_item->title, music_info.title, MAX_TITLE_LEN);
				strncopy(tmp_item->album, music_info.album, MAX_TITLE_LEN);
				strncopy(tmp_item->artist, music_info.artist, MAX_TITLE_LEN);
				tmp_item->time = music_info.time_length;
				tmp_item->track = music_info.track;
				tmp_item->last_track = music_info.last_track;

				tmp_item->number = music_file_count;
				last_item->next = tmp_item;
				last_item = tmp_item;
				last_item->next = 0;
				music_file_count ++;
				draw_search_count(music_file_count);
				add_album(last_item);
			}
		} else {
			//tprintf("File %s analyze error\n", sj2utf8(fname));
		}
	} else {
		tprintf("File %s open error\n", sj2utf8(fname));
	}

	return 0;
}

static void create_music_list(void)
{
	struct file_item *item = item_root.next;
	int i = 0;
#ifdef DEBUG
	unsigned char str[4][MAX_PATHNAME_LEN + 1];
#endif

#ifdef PRINT_MUSICFILE_INFO
	tprintf("Total file %d\n", music_file_count);
#endif

	item_list = (struct file_item **)alloc_memory(sizeof(struct file_item) * music_file_count);
	if(item_list == 0) {
		eprintf("Cannot alloc item_list");
		return;
	}

	while(item != 0) {
		DTPRINTF(0x01, "%4d %32s %32s %32s %2d/%2d %32s\n",
			item->number,
			(char *)sjis2utf8(str[0], item->fname, MAX_PATHNAME_LEN),
			(char *)sjis2utf8(str[1], item->title, MAX_PATHNAME_LEN),
			(char *)sjis2utf8(str[2], item->album, MAX_PATHNAME_LEN),
			item->track,
			item->last_track,
			(char *)sjis2utf8(str[3], item->artist, MAX_PATHNAME_LEN));
		item_list[i] = item;
		item = item->next;
		i ++;
	}
}

static void create_album_list(void)
{
	struct album_item *item = album_item_root.next_album;
	int i = 0, j;

#ifdef PRINT_MUSICFILE_INFO
	tprintf("Total album %d\n", music_album_count);
#endif

	album_list = (struct album_item **)alloc_memory(sizeof(struct album_item) * music_album_count);
	if(album_list == 0) {
		eprintf("Cannot alloc album_list");
		return;
	}

	while(item != 0) {
		DTPRINTF(0x01, "%4d %64s [%d]\n",
			i + 1,
			(char *)sj2utf8(item->name),
			item->track_count);

		for(j=0; j<item->track_count; j++) {
			DTPRINTF(0x01, " %4d :", j+1);
			DTPRINTF(0x01, "%5d %2d %s\n", item->file_num_list[j].file_num, item->file_num_list[j].track,
				(char *)sj2utf8(item_list[item->file_num_list[j].file_num]->title));
		}
		DTPRINTF(0x01, "\n");

		album_list[i] = item;
		item = item->next_album;
		i ++;
	}
}

int get_album_file_count(int album_num)
{
	if(album_num >= music_album_count) {
		return -1;
	}

	return album_list[album_num]->track_count;
}

int get_music_file_num(int album_num, int music_num)
{
	int file_num;

	if(album_num >= music_album_count) {
		return -1;
	}

	if(music_num >= album_list[album_num]->track_count) {
		return -1;
	}

	file_num = album_list[album_num]->file_num_list[music_num].file_num;

	return file_num;
}

#ifdef PRINT_MUSICFILE_INFO
static void disp_album_music(void)
{
	int i, j;

	for(i=0; i<music_album_count; i++) {
		int max_music = get_album_file_count(i);

		tprintf("%4d %64s [%d]\n",
			i + 1,
			(char *)sj2utf8(album_list[i]->name),
			album_list[i]->track_count);

		for(j=0; j<max_music; j++) {
			tprintf(" %4d :", j+1);
			tprintf("%5d %2d %s\n", album_list[i]->file_num_list[j].file_num, album_list[i]->file_num_list[j].track,
				(char *)sj2utf8(item_list[album_list[i]->file_num_list[j].file_num]->title));
		}
	}
}
#endif

int create_filelist(unsigned char *path, struct file_ext *ext)
{
	last_item = &item_root;
	album_item_last = &album_item_root;
	music_file_count = 0;
	music_album_count = 0;

	search_file(path, ext, add_file);

	create_music_list();
	create_album_list();

#ifdef PRINT_MUSICFILE_INFO
	disp_album_music();
#endif

	return 0;
}
