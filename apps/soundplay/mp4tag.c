/** @file
    @brief	MP4TAGデコード

    @date	2017.04.08
    @auther	Takashi SHUDO
*/

#include "str.h"
#include "tprintf.h"
#include "mp4tag.h"
#include "file.h"
#include "soundplay.h"
#include "memory.h"
#include "graphics.h"
#include "charcode.h"
#include "artwork.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


#define MAX_MP4TAG_BUFSIZE	256

static int box_decode(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur);

static unsigned char tag_header_buf[MP4TAG_HEADER_SIZE];
static unsigned char tag_buf[MAX_MP4TAG_BUFSIZE + 1];
static int read_size = 0;

static int calc_box_size(unsigned char *tag)
{
	int box_size = 0;

	box_size =
			(((int)(((*(tag + 0)) & 0xff))) << 24) +
			(((int)(((*(tag + 1)) & 0xff))) << 16) +
			(((int)(((*(tag + 2)) & 0xff))) << 8) +
			(((int)(((*(tag + 3)) & 0xff))) << 0);

	return box_size - MP4TAG_HEADER_SIZE;
}

static int mp4tag_box_header_decode(unsigned char *tag)
{
	int box_size = 0;
	unsigned char box_name[4 + 1] = { 0, 0, 0, 0, 0 };

	strncopy(box_name, (const unsigned char *)&tag[4], 4);
	DTPRINTF(0x01, "box = %s\n", box_name);

	box_size = calc_box_size(&tag[0]);

	return box_size;
}

struct st_mp4tag_decode {
	char box_name[5];
	int (* decode)(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur);
};

static int decode_tag_str(unsigned char *str, unsigned char *tag, int len)
{
	int rt = 0;

	if(len > MAX_MINFO_STR) {
		len = MAX_MINFO_STR;
	}

	// UTF-8
#if 0
	utf82sjis(str, tag, len);
#else
	strncopy(str, tag, len);
#endif

	return rt;
}


static int read_data(unsigned char *dest, int box_size, tag_read_func tag_read)
{
	int size = 0;
	int rt = 0;

	rt = tag_read(dest, box_size);
	if(rt < box_size) {
		tprintf("box Read Error(size: %d)\n", rt);
	}
	size += rt;

	return size;
}

static int read_box_data(int box_size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int size = 0;
	int rt = 0;

	if(box_size > MAX_MP4TAG_BUFSIZE) {
		DTPRINTF(0x01, "BIG box(size: %d)\n", box_size);
		rt = read_data(tag_buf, MAX_MP4TAG_BUFSIZE, tag_read);
		if(rt < MAX_MP4TAG_BUFSIZE) {
			tprintf("box Read Error(size: %d)\n", rt);
		}
		size += rt;
		rt = tag_seekcur(box_size - MAX_MP4TAG_BUFSIZE);
		size += rt;
	} else {
		rt = read_data(tag_buf, box_size, tag_read);
		if(rt < box_size) {
			tprintf("box Read Error(size: %d)\n", rt);
		}
		size += rt;
	}

	return size;
}

static unsigned char *malloc_ptr;

static int malloc_read_box_data(int box_size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int size = 0;
	int rt = 0;

	DTPRINTF(0x01, "malloc box(size: %d)\n", box_size);

	malloc_ptr = (unsigned char *)alloc_memory(box_size);
	if(malloc_ptr == 0) {
		tprintf("box malloc fail\n");
		return size;
	}

	rt = read_data(malloc_ptr, box_size, tag_read);
	if(rt < box_size) {
		tprintf("box Read Error(size: %d)\n", rt);
	}
	size += rt;

	return size;
}

/*
 * 各boxデコード
 */

static int decode_ftyp(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_box_data(size, tag_read, tag_seekcur);

	XDUMP(0x02, tag_buf, rt);

	return rt;
}

static int decode_free(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_box_data(size, tag_read, tag_seekcur);

	XDUMP(0x02, tag_buf, 64);

	return rt;
}

static int decode_moov(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt;

	rt = box_decode(info, size, tag_read, tag_seekcur);

	return rt;
}

static int decode_mvhd(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_box_data(size, tag_read, tag_seekcur);

	info->sampling_rate =
			(((int)tag_buf[12]) << 24) +
			(((int)tag_buf[13]) << 16) +
			(((int)tag_buf[14]) <<  8) +
			(((int)tag_buf[15]) <<  0);

	DTPRINTF(0x01, "Sampling Rate : %d\n", info->sampling_rate);

	return rt;
}

static int decode_stts(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_box_data(size, tag_read, tag_seekcur);

	XDUMP(0x02, tag_buf, rt);

	info->sample_count =
			(((int)tag_buf[ 8]) << 24) +
			(((int)tag_buf[ 9]) << 16) +
			(((int)tag_buf[10]) <<  8) +
			(((int)tag_buf[11]) <<  0);

	DTPRINTF(0x01, "Sample Count : %d\n", info->sample_count);

	info->frame_size =
			(((int)tag_buf[12]) << 24) +
			(((int)tag_buf[13]) << 16) +
			(((int)tag_buf[14]) <<  8) +
			(((int)tag_buf[15]) <<  0);

	DTPRINTF(0x01, "Frame Size : %d\n", info->frame_size);

	info->time_length = (unsigned long long)info->sample_count * info->frame_size * 1000 / info->sampling_rate;

	return rt;
}

static int decode_stsd(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rsize = 0;
	int rt;

	rt = read_box_data(8, tag_read, tag_seekcur);
	rsize += rt;
	rt = box_decode(info, size, tag_read, tag_seekcur);
	rsize += rt;

	return rsize;
}

static int decode_mp4a(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rsize = 0;
	int rt;

	rt = read_box_data(28, tag_read, tag_seekcur);
	rsize += rt;
	rt = box_decode(info, size, tag_read, tag_seekcur);
	rsize += rt;

	return rsize;
}

static int decode_esds(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_box_data(size, tag_read, tag_seekcur);
	int bitrate = 0;

	XDUMP(0x02, tag_buf, rt);

	bitrate =
			(((int)tag_buf[26]) << 24) +
			(((int)tag_buf[27]) << 16) +
			(((int)tag_buf[28]) <<  8) +
			(((int)tag_buf[29]) <<  0);

	info->bit_rate = bitrate/1000;

	DTPRINTF(0x01, "Bitrate : %d\n", info->bit_rate);

	return rt;
}

static int decode_stsz(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_box_data(12, tag_read, tag_seekcur);

	info->sample_count =
			(((int)tag_buf[ 8]) << 24) +
			(((int)tag_buf[ 9]) << 16) +
			(((int)tag_buf[10]) <<  8) +
			(((int)tag_buf[11]) <<  0);

	DTPRINTF(0x01, "Sample Count : %d\n", info->sample_count);

	info->sample_size_data = alloc_memory(info->sample_count * 4);
	if(info->sample_size_data == 0) {
		tprintf("Sample size alloc Error\n");
		return -1;
	}

	rt = read_data(info->sample_size_data, info->sample_count * 4, tag_read);
	if(rt < (info->sample_count * 4)) {
		tprintf("Sample size data read Error\n");
		return -1;
	}

#if 0
	int i;

	for(i=0; i<info->sample_count; i++) {
		unsigned long ssize =
				((long)info->sample_size_data[i*4 + 0] << 24) +
				((long)info->sample_size_data[i*4 + 1] << 16) +
				((long)info->sample_size_data[i*4 + 2] <<  8) +
				((long)info->sample_size_data[i*4 + 3] <<  0);
		DTPRINTF(0x01, "Sample size[%d] : %ld\n", i+1, ssize);
	}
#endif

	return rt;
}

static int decode_meta(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rsize = 0;
	int rt;

	rt = read_box_data(4, tag_read, tag_seekcur);
	rsize += rt;
	rt = box_decode(info, size, tag_read, tag_seekcur);
	rsize += rt;

	return rsize;
}

static int decode_ilst(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt;

	rt = box_decode(info, size, tag_read, tag_seekcur);

	return rt;
}

static int decode_hifn(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_box_data(size, tag_read, tag_seekcur);

	XDUMP(0x02, tag_buf, 64);

	return rt;
}

static int decode_name(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_box_data(size, tag_read, tag_seekcur);

	tag_buf[rt] = 0;
	DTPRINTF(0x01, "Title : %s\n", (char *)&tag_buf[16]);

	decode_tag_str(info->title, &tag_buf[16], size-16);

	return rt;
}

static int decode_art(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_box_data(size, tag_read, tag_seekcur);

	tag_buf[rt] = 0;
	DTPRINTF(0x01, "Artist : %s\n", (char *)&tag_buf[16]);

	decode_tag_str(info->artist, &tag_buf[16], size-16);

	return rt;
}

static int decode_alb(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_box_data(size, tag_read, tag_seekcur);

	tag_buf[rt] = 0;
	DTPRINTF(0x01, "Album : %s\n", (char *)&tag_buf[16]);

	decode_tag_str(info->album, &tag_buf[16], size-16);

	return rt;
}

static int decode_trkn(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_box_data(size, tag_read, tag_seekcur);

	DTPRINTF(0x01, "Track : %d/%d\n", (int)tag_buf[19], (int)tag_buf[21]);

	info->track = tag_buf[19];
	info->last_track = tag_buf[21];

	return rt;
}

static int flg_mp4_decode_artwork = 0;

void set_mp4_decode_artwork(int flg_env)
{
	flg_mp4_decode_artwork = flg_env;

	DTFPRINTF(0x01, "art_decode = %d\n", flg_mp4_decode_artwork);
}

#ifdef GSC_LIB_ENABLE_PICOJPEG
#include "jpegdec.h"
#endif

#ifdef GSC_LIB_ENABLE_LIBPNG
#include "pngdec.h"
#endif

//#define DISABLE_DECODE_MULTTASK

static int decode_covr(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt;

	if(flg_mp4_decode_artwork != 0) {
		unsigned char *cover_ptr;
		rt = malloc_read_box_data(size, tag_read, tag_seekcur);
		cover_ptr = malloc_ptr+16;
		XDUMP(0x01, cover_ptr, 64);

		if((cover_ptr[0] == 0xff) && (cover_ptr[1] == 0xd8)) {
			// JPEG
			DTPRINTF(0x04, "JPEG Decode\n");
#ifdef GSC_LIB_ENABLE_PICOJPEG
#ifdef DISABLE_DECODE_MULTTASK
			decode_jpeg_artwork(info, cover_ptr);
#else
			decode_jpeg_artwork_bg(info, cover_ptr, malloc_ptr);
#endif
#endif
		} else if(strncomp((const uchar *)cover_ptr, (const uchar *)"\211PNG", 4) == 0) {
			// PNG
			DTPRINTF(0x04, "PNG Decode\n");
#ifdef GSC_LIB_ENABLE_LIBPNG
#ifdef DISABLE_DECODE_MULTTASK
			decode_png_artwork(info, cover_ptr);
#else
			decode_png_artwork_bg(info, cover_ptr, malloc_ptr);
#endif
#endif
		} else {
			DTPRINTF(0x04, "Unknow format\n");
		}
#ifdef DISABLE_DECODE_MULTTASK
		free_memory(malloc_ptr);
#endif
	} else {
		rt = read_box_data(size, tag_read, tag_seekcur);
	}

	return rt;
}

static int decode_mdat(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = 0;
	unsigned char buf[MP4TAG_HEADER_SIZE + 4 + 4];

	DTPRINTF(0x01, "Find mdat(%d)\n", size);

	// "mdat"->"wide" 暫定対策 [TODO]
	rt = read_data(buf, MP4TAG_HEADER_SIZE + 4 + 4, tag_read);
	XDUMP(0x01, buf, 16);

	if(strncomp((const uchar *)&buf[4], (const uchar *)"wide", 4) == 0) {
		DTPRINTF(0x01, "mdat box\n");
		rt = 0;
	} else {
		DTPRINTF(0x01, "mdat no box\n");
		tag_seekcur(-(MP4TAG_HEADER_SIZE + 4 + 4));
		rt = 0;
	}

	return rt;	// 以後オーディオデータ
}

static int decode_unknown(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt;
	int dsize = 64;

	DTPRINTF(0x01, "Unknown box %c%c%c%c %d\n",
		 tag_header_buf[4],
		 tag_header_buf[5],
		 tag_header_buf[6],
		 tag_header_buf[7],
		 size);

	rt = read_box_data(size, tag_read, tag_seekcur);

	if(rt < dsize) {
		dsize = rt;
	}

	XDUMP(0x02, tag_buf, dsize);

	return rt;
}


static struct st_mp4tag_decode mp4tag_decode_list[] = {
	{ "ftyp", decode_ftyp },
	{ "moov", decode_moov },
	  { "mvhd", decode_mvhd },
	  { "trak", decode_moov },
	    { "mdia", decode_moov },
	      { "mdhd", decode_mvhd },
	      { "minf", decode_moov },
	        { "stbl", decode_moov },
	          { "stsd", decode_stsd },
	            { "mp4a", decode_mp4a },
	              { "esds", decode_esds },
	          { "stts", decode_stts },
	          { "stsz", decode_stsz },
	  { "udta", decode_moov },
	    { "meta", decode_meta },
	      { "ilst", decode_ilst },
		{ "----", decode_hifn },
		{ "\251nam", decode_name },
		{ "\251ART", decode_art },
		{ "\251alb", decode_alb },
		{ "trkn", decode_trkn },
		{ "covr", decode_covr },
	      { "free", decode_free },
	{ "mdat", decode_mdat },
	{ {0, 0, 0, 0, 0}, 0 }
};

static int box_decode(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = 0;
	int box_size = 0;

	while(1) {
		if(size != 0) {
			if(read_size >= size) {
				DTPRINTF(0x01, "Read All %d\n", read_size);
				break;
			}
		}

		// box Header
		rt = tag_read(tag_header_buf, MP4TAG_HEADER_SIZE);
		if(rt < MP4TAG_HEADER_SIZE) {
			tprintf("box Read Error(size: %d)\n", rt);
			return -1;
		}
		read_size += rt;
		DTPRINTF(0x01, "READ SIZE %d %08x\n", read_size, read_size);

		box_size = mp4tag_box_header_decode(tag_header_buf);
		DTPRINTF(0x01, "box size = %d\n", box_size);

		{
			struct st_mp4tag_decode *p_mp4dec = mp4tag_decode_list;

			if(tag_header_buf[4] == 0) {
				DTPRINTF(0x01, "NULL box %02X%02X%02X%02X\n",
					  tag_buf[0],
					  tag_buf[1],
					  tag_buf[2],
					  tag_buf[3]);
				//return read_size;
				return -1;
			}

			while(p_mp4dec->box_name[0] != 0) {
				if(strncomp(&tag_header_buf[4], (const unsigned char *)p_mp4dec->box_name, 4) == 0) {
					rt = p_mp4dec->decode(info, box_size, tag_read, tag_seekcur);
					if(rt == 0) {
						// mdat検出
						goto end;
					} else
					if(rt < 0) {
						// デコードエラー
						read_size = rt;
						goto end;
					} else {
						read_size += rt;
					}
					break;
				}
				p_mp4dec ++;
			}
			// 解析できないbox
			if(p_mp4dec->box_name[0] == 0) {
				rt = decode_unknown(info, box_size, tag_read, tag_seekcur);
				read_size += rt;
			}
		}
	}

end:
	DTPRINTF(0x01, "box READ SIZE %d %08x\n", read_size, read_size);

	return read_size;
}

int mp4tag_decode(struct st_music_info *info, tag_read_func tag_read, tag_seekcur_func tag_seekcur, tag_tell_func tag_tell)
{
	int rtn;

	read_size = 0;

	init_music_info(info);

	info->format = MUSIC_FMT_AAC;

	rtn = box_decode(info, 0, tag_read, tag_seekcur);

	info->frame_start = tag_tell();

	DTPRINTF(0x01, "info->frame_start %d\n", info->frame_start);

	return rtn;
}

void mp4tag_dispose(struct st_music_info *info)
{
	if(info->sample_size_data != 0) {
		free_memory(info->sample_size_data);
		info->sample_size_data = 0;
	}
}
