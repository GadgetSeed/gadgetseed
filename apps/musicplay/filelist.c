/** @file
    @brief	ファイルリスト

    @date	2012.02.23
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"

#include "log.h"
#include "filelist.h"
#include "filesearch.h"
#include "memory.h"
#include "str.h"
#include "charcode.h"
#include "file.h"
#include "tkprintf.h"
#include "tprintf.h"
#include "font.h"
#include "log.h"
#include "crc.h"
#include "music_info.h"
#include "mp4tag.h"
#include "id3tag.h"
#include "musicplay_view.h"
#include "sdmusic.h"
#include "radio.h"
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
#include "m3u.h"
#include "pls.h"
#endif

//#define DEBUGTBITS 0x01
#include "dtprintf.h"

//#define PRINT_MUSICFILE_INFO
#define MUSICSEARCH_PATH	((uchar *)"0:")
#define MUSICINFO_FILENAME	"0:musicinfo.dat"
//#define MUSICFILE_SAVETEST
#define MUSICFILEINFO_SAVE_LOGLEVEL	9 //6
#define MUSICFILEINFO_LOAD_LOGLEVEL	9 //6


static const struct file_ext audio_file_ext[] = {
	{ "MP3" },
	{ "M4A" },
//	{ "AAC" },
//	{ "WAV" },
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	{ "M3U" },
	{ "PLS" },
#endif
	{ "" }
};

static struct st_music_info music_info __attribute__ ((section(".extram"))) __attribute__ ((aligned (4)));

static struct sdmusic_item item_root, *last_item;
static struct album_item album_item_root, *album_item_last;
static int music_fd = -1;
struct sdmusic_item **sdmusic_list;
struct album_item **album_list;
int music_file_count = 0;
int music_album_count = 0;


#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
static struct radio_item radio_item_root, *radio_list_item;
struct radio_item **radio_list;
int radio_count = 0;
#endif

extern const struct st_rect screen_rect;

static void draw_searching(void)
{
	static const unsigned char str_searching[] = "Music File Searching...";
	int strw;

	set_forecolor(back_color);
	draw_fill_rect((struct st_rect *)&screen_rect);

	set_forecolor(fore_color);
	set_font_by_name(MPFONT);
	set_font_drawmode(FONT_FIXEDWIDTH);

	strw = str_width((unsigned char *)str_searching);
	draw_str((GSC_GRAPHICS_DISPLAY_WIDTH - strw)/2, GSC_GRAPHICS_DISPLAY_HEIGHT/2 - font_height(),
		 (unsigned char *)str_searching, sizeof(str_searching));
}

static void draw_search_count(int count)
{
	unsigned char str_cnt[8];
	int strw;

	tsprintf((char *)str_cnt, "%5d", count);
	strw = str_width(str_cnt);
	draw_str((GSC_GRAPHICS_DISPLAY_WIDTH - strw)/2, GSC_GRAPHICS_DISPLAY_HEIGHT/2 + font_height()*2,
		 str_cnt, 8);
}

static void draw_search_album_count(int count)
{
	unsigned char str_cnt[8];
	int strw;

	tsprintf((char *)str_cnt, "%5d", count);
	strw = str_width(str_cnt);
	draw_str((GSC_GRAPHICS_DISPLAY_WIDTH - strw)/2, GSC_GRAPHICS_DISPLAY_HEIGHT/2 + font_height()*4,
		 str_cnt, 8);
}

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

static int mp_seekcurfile(int pos)
{
	seek_file(music_fd, pos, SEEK_CUR);

	return pos;
}

static int mp_seeksetfile(int pos)
{
	seek_file(music_fd, pos, SEEK_SET);

	return pos;
}

static int mp_sizefile(void)
{
	int rsize = size_file(music_fd);

	return rsize;
}

static int mp_tellfile(void)
{
	int pos = tell_file(music_fd);

	return pos;
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
	gslog(MUSICFILEINFO_SAVE_LOGLEVEL, "New Album %s\n", name);
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

static void add_item(struct album_item *album, struct sdmusic_item *item)
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

static void add_album(struct sdmusic_item *item)
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
			rtn = mp4tag_decode(&music_info, mp_readfile, mp_seekcurfile, mp_tellfile);
		} else if(strcomp((unsigned char *)"MP3", ext) == 0) {
			rtn = id3tag_decode(&music_info, mp_readfile, mp_seekcurfile, mp_seeksetfile, mp_sizefile, mp_tellfile);
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
		} else if(strcomp((unsigned char *)"M3U", ext) == 0) {
			rtn = m3u_decode(&music_info, rtn);
			if(rtn > 0) {
				struct radio_item *tmp_item;
				tmp_item = (struct radio_item *)alloc_memory(sizeof(struct radio_item));
				strncopy(tmp_item->fname, fname, MAX_PATHNAME_LEN);
				strncopy((uchar *)tmp_item->broadcaster_name, music_info.album, MAX_TITLE_LEN);
				strncopy((uchar *)tmp_item->url, music_info.url, MAX_TITLE_LEN);
				radio_list_item->next = tmp_item;
				radio_list_item = tmp_item;
				radio_list_item->next = 0;
				radio_count ++;
			}
			rtn = -1;
		} else if(strcomp((unsigned char *)"PLS", ext) == 0) {
			rtn = pls_decode(&music_info, rtn);
			if(rtn > 0) {
				struct radio_item *tmp_item;
				tmp_item = (struct radio_item *)alloc_memory(sizeof(struct radio_item));
				strncopy(tmp_item->fname, fname, MAX_PATHNAME_LEN);
				strncopy((uchar *)tmp_item->broadcaster_name, music_info.album, MAX_TITLE_LEN);
				strncopy((uchar *)tmp_item->url, music_info.url, MAX_TITLE_LEN);
				radio_list_item->next = tmp_item;
				radio_list_item = tmp_item;
				radio_list_item->next = 0;
				radio_count ++;
			}
			rtn = -1;
#endif
		} else {
			tprintf("Unknown file type \"%s\", filename \"%s\"\n", ext, sj2utf8(fname));
			rtn = 0;
		}
		mp_closefile();

		if(rtn > 0) {
			struct sdmusic_item *tmp_item;

			//disp_music_info(&music_info);

			tmp_item = (struct sdmusic_item *)alloc_memory(sizeof(struct sdmusic_item));

			if(tmp_item == 0) {
				eprintf("Cannot alloc sdmusic_item\n");
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
	struct sdmusic_item *item = item_root.next;
	int i = 0;

	gslog(MUSICFILEINFO_SAVE_LOGLEVEL, "Total file %d\n", music_file_count);

	sdmusic_list = (struct sdmusic_item **)alloc_memory(sizeof(struct sdmusic_item *) * music_file_count);
	if(sdmusic_list == 0) {
		eprintf("Cannot alloc sdmusic_list");
		return;
	}

	while(item != 0) {
		DTPRINTF(0x01, "%4d %32s %32s %32s %2d/%2d %32s\n",
			 item->number,
			 sj2utf8(item->fname),
			 item->title,
			 item->album,
			 item->track,
			 item->last_track,
			 item->artist);
		sdmusic_list[i] = item;
		item = item->next;
		i ++;
	}
}

static void create_album_list(void)
{
	struct album_item *item = album_item_root.next_album;
	int i = 0, j;

	gslog(MUSICFILEINFO_SAVE_LOGLEVEL, "Total album %d\n", music_album_count);

	album_list = (struct album_item **)alloc_memory(sizeof(struct album_item) * music_album_count);
	if(album_list == 0) {
		eprintf("Cannot alloc album_list");
		return;
	}

	while(item != 0) {
		DTPRINTF(0x01, "%4d %64s [%d]\n",
			i + 1,
			item->name,
			item->track_count);

		for(j=0; j<item->track_count; j++) {
			DTPRINTF(0x01, " %4d :", j+1);
			DTPRINTF(0x01, "%5d %2d %s\n", item->file_num_list[j].file_num, item->file_num_list[j].track,
				 sdmusic_list[item->file_num_list[j].file_num]->title);
		}
		DTPRINTF(0x01, "\n");

		album_list[i] = item;
		item = item->next_album;
		i ++;
	}
}

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
static void create_radio_list(void)
{
	struct radio_item *item = radio_item_root.next;
	int i = 0;

	gslog(MUSICFILEINFO_SAVE_LOGLEVEL, "Total album %d\n", music_album_count);

	radio_list = (struct radio_item **)alloc_memory(sizeof(struct radio_item) * radio_count);
	if(radio_list == 0) {
		eprintf("Cannot alloc radio_list");
		return;
	}

	while(item != 0) {
		gslog(MUSICFILEINFO_SAVE_LOGLEVEL,
		      "%3d : %s : %s : %s\n",
		      i,
		      sj2utf8(item->fname),
		      item->broadcaster_name,
		      item->url);

		radio_list[i] = item;
		item = item->next;
		i++;
	}
}
#endif

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
			album_list[i]->name,
			album_list[i]->track_count);

		for(j=0; j<max_music; j++) {
			tprintf(" %4d :", j+1);
			tprintf("%5d %2d %s\n", album_list[i]->file_num_list[j].file_num, album_list[i]->file_num_list[j].track,
				sdmusic_list[album_list[i]->file_num_list[j].file_num]->title);
		}
	}
}
#endif

/*
  musicinfo.dat format

  char	header[4]	Internet Radio : "MIDF" | SD Music "MRID"
  int	ver
  int	music_file_count
  int	music_album_count
  int	radio_count	// Internet Radio
  struct sdmusic_item	sdmusic_item[music_file_count]
  for(i=0; i<music_album_count; i++) {
	struct album_item	album_item[i]
	for(j=0; j<album_item[i].track_count; j++) {
		album_list[i]->file_num_list[j]
	}
  }
 */

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
static const char musicinfofile_header[4] = { 'M', 'R', 'I', 'D' };
#else
static const char musicinfofile_header[4] = { 'M', 'I', 'D', 'F' };
#endif

static const unsigned int	musicinfofile_ver = 1;

unsigned short minfo_crc;

static int save_filelist(void)
{
	int fd;
	int rtn = 0;
	int i;
	struct sdmusic_item *item = item_root.next;

	gslog(0, "Save file list start\n");

	minfo_crc = 0;

	fd = open_file((uchar *)MUSICINFO_FILENAME, FA_WRITE | FA_CREATE_ALWAYS);
	if(fd < 0) {
		eprintf("Cannot create music info file \"%s\"\n", MUSICINFO_FILENAME);
		return -1;
	}

	// Header
	rtn = write_file(fd, musicinfofile_header, 4);
	if(rtn != 4) {
		eprintf("Music info file header save error(%d)\n", rtn);
		rtn = -1;
		goto save_end;
	}
	minfo_crc = crc16(minfo_crc, (unsigned char *)musicinfofile_header, 4);

	// Version
	rtn = write_file(fd, &musicinfofile_ver, sizeof(unsigned int));
	if(rtn != sizeof(unsigned int)) {
		eprintf("Music info file version save error(%d)\n", rtn);
		rtn = -1;
		goto save_end;
	}
	minfo_crc = crc16(minfo_crc, (unsigned char *)&musicinfofile_ver, sizeof(unsigned int));

	gslog(0, "Music count : %d\n", music_file_count);
	rtn = write_file(fd, &music_file_count, sizeof(music_file_count));
	if(rtn != sizeof(music_file_count)) {
		eprintf("Music file count save error(%d)\n", rtn);
		rtn = -1;
		goto save_end;
	}
	minfo_crc = crc16(minfo_crc, (unsigned char *)&music_file_count, sizeof(music_file_count));

	gslog(0, "Album count : %d\n", music_album_count);
	rtn = write_file(fd, &music_album_count, sizeof(music_album_count));
	if(rtn != sizeof(music_album_count)) {
		eprintf("Music album count save error(%d)\n", rtn);
		rtn = -1;
		goto save_end;
	}
	minfo_crc = crc16(minfo_crc, (unsigned char *)&music_album_count, sizeof(music_album_count));

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	gslog(0, "Radio count : %d\n", radio_count);
	rtn = write_file(fd, &radio_count, sizeof(radio_count));
	if(rtn != sizeof(radio_count)) {
		eprintf("Radio count save error(%d)\n", rtn);
		rtn = -1;
		goto save_end;
	}
	minfo_crc = crc16(minfo_crc, (unsigned char *)&radio_count, sizeof(radio_count));
#endif

	// SD File
	i = 0;
	while(item != 0) {
		rtn = write_file(fd, item, sizeof(struct sdmusic_item));
		if(rtn != sizeof(struct sdmusic_item)) {
			eprintf("Music music data %d save error(%d)\n", i, rtn);
			rtn = -1;
			goto save_end;
		}
		minfo_crc = crc16(minfo_crc, (unsigned char *)item, sizeof(struct sdmusic_item));
		gslog(MUSICFILEINFO_SAVE_LOGLEVEL, "[S] %4d %32s %32s %32s %2d/%2d %32s\n",
		      item->number,
		      sj2utf8(item->fname),
		      item->title,
		      item->album,
		      item->track,
		      item->last_track,
		      item->artist);
		item = item->next;
		i ++;
	}

	for(i=0; i<music_album_count; i++) {
		int max_music = get_album_file_count(i);
		int j;

		rtn = write_file(fd, album_list[i], sizeof(struct album_item));
		if(rtn != sizeof(struct album_item)) {
			eprintf("Album data %d save error(%d)\n", i, rtn);
			rtn = -1;
			goto save_end;
		}
		minfo_crc = crc16(minfo_crc, (unsigned char *)album_list[i], sizeof(struct album_item));

		gslog(MUSICFILEINFO_SAVE_LOGLEVEL, "%4d %64s [%d]\n",
		      i + 1,
		      album_list[i]->name,
		      album_list[i]->track_count);

		for(j=0; j<max_music; j++) {
			rtn = write_file(fd, &(album_list[i]->file_num_list[j]), sizeof(struct track_filenum));
			if(rtn != sizeof(struct track_filenum)) {
				eprintf("Album track data %d save error(%d)\n", i, rtn);
				rtn = -1;
				goto save_end;
			}
			minfo_crc = crc16(minfo_crc, (unsigned char *)&(album_list[i]->file_num_list[j]), sizeof(struct track_filenum));

			gslog(MUSICFILEINFO_SAVE_LOGLEVEL, " %4d :", j+1);
			gslog(MUSICFILEINFO_SAVE_LOGLEVEL, "%5d %2d %s\n", album_list[i]->file_num_list[j].file_num,
			      album_list[i]->file_num_list[j].track,
			      sdmusic_list[album_list[i]->file_num_list[j].file_num]->title);
		}
	}

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	// Radio
	{
		struct radio_item *radioitem = radio_item_root.next;
		i = 0;
		while(radioitem != 0) {
			rtn = write_file(fd, radioitem, sizeof(struct radio_item));
			if(rtn != sizeof(struct radio_item)) {
				eprintf("Radio data %d save error(%d)\n", i, rtn);
				rtn = -1;
				goto save_end;
			}
			minfo_crc = crc16(minfo_crc, (unsigned char *)radioitem, sizeof(struct radio_item));
			gslog(MUSICFILEINFO_SAVE_LOGLEVEL, "[S] %4d %32s %32s %32s\n",
			      i,
			      sj2utf8(radioitem->fname),
			      radioitem->broadcaster_name,
			      radioitem->url);
			radioitem = radioitem->next;
			i ++;
		}
	}
#endif

	rtn = write_file(fd, &minfo_crc, sizeof(minfo_crc));
	if(rtn != sizeof(minfo_crc)) {
		eprintf("CRC write error(%d)\n", rtn);
		rtn = -1;
	}

save_end:
	close_file(fd);
	gslog(0, "Save file list done(CRC:%04X)\n", minfo_crc);

	return rtn;
}

int load_filelist(void)
{
	int fd;
	int rtn = 0;
	int i;
	char header[4];
	unsigned int ver;
	unsigned short crc = 0;
	unsigned short file_crc = 0;

	gslog(0, "Load file list start\n");

	fd = open_file((uchar *)MUSICINFO_FILENAME, FA_READ);
	if(fd < 0) {
		eprintf("Cannot open music info file \"%s\"\n", MUSICINFO_FILENAME);
		return -1;
	}

	// Header
	rtn = read_file(fd, header, 4);
	if(rtn != 4) {
		eprintf("Music info file header load error(%d)\n", rtn);
		rtn = -1;
		goto load_end;
	}
	if(strncomp((const uchar *)header, (const uchar *)musicinfofile_header, 4) != 0) {
		eprintf("Music info file header unmatch\n");
		rtn = -1;
		goto load_end;
	}
	crc = crc16(crc, (unsigned char *)header, 4);

	// Version
	rtn = read_file(fd, &ver, sizeof(unsigned int));
	if(rtn != sizeof(unsigned int)) {
		eprintf("Music info file version load error(%d)\n", rtn);
		rtn = -1;
		goto load_end;
	}
	if(ver != musicinfofile_ver) {
		eprintf("Music info file version unmatch(%d,%d)\n", ver, musicinfofile_ver);
		rtn = -1;
		goto load_end;
	}
	crc = crc16(crc, (unsigned char *)&ver, sizeof(unsigned int));

	gslog(0, "Music info file \"%s\" found\n", MUSICINFO_FILENAME);
	rtn = read_file(fd, &music_file_count, sizeof(music_file_count));
	if(rtn != sizeof(music_file_count)) {
		eprintf("Music file count load error(%d)\n", rtn);
		rtn = -1;
		goto load_end;
	}
	crc = crc16(crc, (unsigned char *)&music_file_count, sizeof(music_file_count));
	gslog(0, "Music count : %d\n", music_file_count);

	rtn = read_file(fd, &music_album_count, sizeof(music_album_count));
	if(rtn != sizeof(music_album_count)) {
		eprintf("Music album count load error(%d)\n", rtn);
		rtn = -1;
		goto load_end;
	}
	crc = crc16(crc, (unsigned char *)&music_album_count, sizeof(music_album_count));
	gslog(0, "Album count : %d\n", music_album_count);

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	rtn = read_file(fd, &radio_count, sizeof(radio_count));
	if(rtn != sizeof(radio_count)) {
		eprintf("Radio count load error(%d)\n", rtn);
		rtn = -1;
		goto load_end;
	}
	crc = crc16(crc, (unsigned char *)&radio_count, sizeof(radio_count));
	gslog(0, "Radio count : %d\n", radio_count);
#endif

	// File
	last_item = &item_root;
	for(i=0; i<music_file_count; i++) {
		struct sdmusic_item *tmp_item;

		tmp_item = (struct sdmusic_item *)alloc_memory(sizeof(struct sdmusic_item));
		if(tmp_item == 0) {
			eprintf("Cannot alloc sdmusic_item\n");
			// [TODO] いままでallocしていたメモリを開放
			rtn = -1;
			goto load_end;
		} else {
			rtn = read_file(fd, tmp_item, sizeof(struct sdmusic_item));
			if(rtn != sizeof(struct sdmusic_item)) {
				eprintf("Music music data load error(%d)\n", rtn);
				// [TODO] いままでallocしていたメモリを開放
				rtn = -1;
				goto load_end;
			}
			crc = crc16(crc, (unsigned char *)tmp_item, sizeof(struct sdmusic_item));
			gslog(MUSICFILEINFO_LOAD_LOGLEVEL, "[L] %4d %32s %32s %32s %2d/%2d %32s\n",
			      tmp_item->number,
			      sj2utf8(tmp_item->fname),
			      tmp_item->title,
			      tmp_item->album,
			      tmp_item->track,
			      tmp_item->last_track,
			      tmp_item->artist);
			last_item->next = tmp_item;
			last_item = tmp_item;
			last_item->next = 0;
		}
	}
	create_music_list();

	// Album
	album_item_last = &album_item_root;
	album_list = (struct album_item **)alloc_memory(sizeof(struct album_item *) * music_album_count);
	if(album_list == 0) {
		eprintf("Cannot alloc album_list\n");
		rtn = -1;
		goto load_end;
	}
	for(i=0; i<music_album_count; i++) {
		struct album_item *tmp_album_item;
		struct track_filenum *tmp_file_num;
		int j;

		tmp_album_item = (struct album_item *)alloc_memory(sizeof(struct album_item));
		if(tmp_album_item == 0) {
			eprintf("Cannot alloc tmp_album_item\n");
			rtn = -1;
			goto load_end;
		}
		album_list[i] = tmp_album_item;
		rtn = read_file(fd, tmp_album_item, sizeof(struct album_item));
		if(rtn != sizeof(struct album_item)) {
			eprintf("Album data %d load error(%d)\n", i, rtn);
			rtn = -1;
			goto load_end;
		}
		crc = crc16(crc, (unsigned char *)tmp_album_item, sizeof(struct album_item));

		gslog(MUSICFILEINFO_LOAD_LOGLEVEL, "[L] %4d %64s [%d]\n",
		      i + 1,
		      tmp_album_item->name,
		      tmp_album_item->track_count);

		tmp_file_num = (struct track_filenum *)alloc_memory(sizeof(struct track_filenum) * tmp_album_item->track_count);
		if(tmp_file_num == 0) {
			eprintf("Cannot alloc tmp_file_num\n");
			rtn = -1;
			goto load_end;
		}
		tmp_album_item->file_num_list = tmp_file_num;

		for(j=0; j<tmp_album_item->track_count; j++) {

			rtn = read_file(fd, &tmp_file_num[j], sizeof(struct track_filenum));
			if(rtn != sizeof(struct track_filenum)) {
				eprintf("Album track data %d load error(%d)\n", i, rtn);
				rtn = -1;
				goto load_end;
			}
			crc = crc16(crc, (unsigned char *)&tmp_file_num[j], sizeof(struct track_filenum));

			gslog(MUSICFILEINFO_LOAD_LOGLEVEL, " %4d :", j+1);
			gslog(MUSICFILEINFO_LOAD_LOGLEVEL, "%5d %2d %s\n", tmp_file_num[j].file_num, tmp_file_num[j].track,
			      sdmusic_list[tmp_file_num[j].file_num]->title);
		}

		album_item_last->next_album = tmp_album_item;
		album_item_last = tmp_album_item;
		album_item_last->next_album = 0;
	}
	create_album_list();

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	// Radio
	radio_list_item = &radio_item_root;
	for(i=0; i<radio_count; i++) {
		struct radio_item *tmp_radio_item;

		tmp_radio_item = (struct radio_item *)alloc_memory(sizeof(struct radio_item));
		if(tmp_radio_item == 0) {
			eprintf("Cannot alloc radio_item\n");
			rtn = -1;
			goto load_end;
		} else {
			rtn = read_file(fd, tmp_radio_item, sizeof(struct radio_item));
			if(rtn != sizeof(struct radio_item)) {
				eprintf("Radio data load error(%d)\n", rtn);
				rtn = -1;
				goto load_end;
			}
			crc = crc16(crc, (unsigned char *)tmp_radio_item, sizeof(struct radio_item));
			radio_list_item->next = tmp_radio_item;
			radio_list_item = tmp_radio_item;
			radio_list_item->next = 0;
		}
	}
	create_radio_list();
#endif

	rtn = read_file(fd, &file_crc, sizeof(file_crc));
	if(rtn != sizeof(file_crc)) {
		eprintf("CRC read error(%d)\n", rtn);
		rtn = -1;
		goto load_end;
	}

	if(crc == file_crc) {
		gslog(0, "Load file list OK(CRC:%04X,%04X)\n", crc, file_crc);
		minfo_crc = crc;
		rtn = 0;	// All OK
	} else {
		gslog(0, "Load file list NG(CRC:%04X,%04X)\n", crc, file_crc);
		minfo_crc = 0;
		rtn = -1;	// CRC error
	}

load_end:
	close_file(fd);

	// 本来メモリを開放してから
	if(rtn != 0) {
		last_item = &item_root;
		last_item->next = 0;
		album_item_last = &album_item_root;
		album_item_last->next_album = 0;
		album_item_last->track_count = 0;
		album_item_last->max_track_count = 0;
		album_item_last->file_num_list = 0;
		album_item_last->next_album = 0;

		music_file_count = 0;
		music_album_count = 0;

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
		radio_list_item = &radio_item_root;
		radio_list_item->next = 0;
		radio_count = 0;
#endif
	}

	return rtn;
}

int create_filelist(void)
{
	gslog(0, "Audio File searching...\n");

	memoryset(&music_info, 0, sizeof(music_info));

	item_root.next = 0;
	last_item = &item_root;
	album_item_root.next_album = 0;
	album_item_last = &album_item_root;
	music_file_count = 0;
	music_album_count = 0;
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	radio_item_root.next = 0;
	radio_list_item = &radio_item_root;
	select_radio_num = 0;
#endif

	draw_searching();
	search_file(MUSICSEARCH_PATH, (struct file_ext *)audio_file_ext, add_file);

	create_music_list();
	create_album_list();
#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	create_radio_list();
#endif
#ifdef PRINT_MUSICFILE_INFO
	disp_album_music();
#endif
	save_filelist();

	gslog(0, "Audio File search done\n");

	return 0;
}

static void dispose_itemlist(void)
{
	struct sdmusic_item *item = item_root.next;

	if(sdmusic_list != 0) {
		free_memory(sdmusic_list);
	}

	while(item != 0) {
		struct sdmusic_item *next = item->next;
		DTPRINTF(0x08, "free sdmusic_item %p\n", item);
		free_memory(item);
		item = next;
	}
}

static void dispose_albumlist(void)
{
	struct album_item *item = album_item_root.next_album;

	while(item != 0) {
		struct album_item *next = item->next_album;
		DTPRINTF(0x08, "free album_item %p\n", item);
		free_memory(item);
		item = next;
	}
}

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
static void dispose_radiolist(void)
{
	struct radio_item *item = radio_item_root.next;

	if(radio_list != 0) {
		free_memory(radio_list);
	}

	while(item != 0) {
		struct radio_item *next = item->next;
		DTPRINTF(0x08, "free radio_item %p\n", item);
		free_memory(item);
		item = next;
	}
}
#endif

void dispose_filelist(void)
{
	dispose_itemlist();
	dispose_albumlist();

#ifdef GSC_ENABLE_MUSICPLAY_INTERNETRADIO
	dispose_radiolist();
#endif
}
