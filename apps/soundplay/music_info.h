/** @file
    @brief	音楽情報データ

    @date	2017.03.30
    @auther	Takashi SHUDO
*/

#ifndef MUSIC_INFO_H
#define MUSIC_INFO_H

#include "sysconfig.h"
#include "graphics.h"
#include "str.h"

#define MAX_MINFO_STR	64

#define MUSIC_FMT_UNKNOWN	0
#define MUSIC_FMT_MP3		1
#define MUSIC_FMT_AAC		2
#define MUSIC_FMT_WAV		3

#if GSC_GRAPHICS_DISPLAY_WIDTH >= 800
#define ART_WIDTH	128
#define ART_HEIGHT	128
#else
#define ART_WIDTH	96
#define ART_HEIGHT	96
#endif

//#define MUSICINFOSTR ushort
#define MUSICINFOSTR uchar

struct st_music_info {
	MUSICINFOSTR title[MAX_MINFO_STR];
	MUSICINFOSTR artist[MAX_MINFO_STR];
	MUSICINFOSTR album[MAX_MINFO_STR];
	MUSICINFOSTR genre[MAX_MINFO_STR];
	MUSICINFOSTR url[MAX_MINFO_STR];
	unsigned char format;	// MP3/AAC(M4A)/WAV
	unsigned char vbr;
	unsigned int time_length;
	unsigned char track;
	unsigned char last_track;
	unsigned short bit_rate;
	unsigned short sampling_rate;
	unsigned short frame_size;

	unsigned char mpeg_padding;	// for MP3
	unsigned char channel;
	unsigned short frame_length;	// for MP3
	unsigned int frame_start;	// for MP3/M4A

	unsigned int sample_count;	// for MP3/M4A
	unsigned char *sample_size_data;	// for M4A
	int metaint;
	#define MINFO_ARTWORK_NOIMAGE	0
	#define MINFO_ARTWORK_JPEG	1
	#define MINFO_ARTWORK_PNG	2
	int flg_have_artwork;	// 0:No image, 1:JPEG, 2:PNG
	PIXEL_DATA artwork[ART_WIDTH * ART_HEIGHT];
};

typedef int (* tag_read_func)(unsigned char *data, int size);
typedef int (* tag_seekcur_func)(int pos);
typedef int (* tag_seekset_func)(int pos);
typedef int (* tag_size_func)(void);
typedef int (* tag_tell_func)(void);

void init_music_info(struct st_music_info *info);
void disp_music_info(struct st_music_info *info);
unsigned int calc_play_time(struct st_music_info *info, unsigned int frame_count);
void disp_play_time(unsigned int play_time);

#endif // MUSIC_INFO_H
