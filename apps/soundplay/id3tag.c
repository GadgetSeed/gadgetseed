/** @file
    @brief	ID3TAGデコード

    @date	2017.03.27
    @auther	Takashi SHUDO

    コンパイルオプション
    -Wno-multichar
*/

#include "sysconfig.h"
#include "str.h"
#include "tprintf.h"
#include "memory.h"
#include "id3tag.h"
#include "artwork.h"
#include "charcode.h"

#include "soundfile.h"

//#define DEBUGTBITS 0x04
#include "dtprintf.h"


#define MAX_ID3TAG_BUFSIZE	256

static unsigned char tag_buf[MAX_ID3TAG_BUFSIZE + 1];
static int read_size;

static int read_data(unsigned char *dest, int tag_size, tag_read_func tag_read)
{
	int size = 0;
	int rt = 0;

	rt = tag_read(dest, tag_size);
	if(rt < tag_size) {
		tprintf("tag Read Error(size: %d)\n", rt);
	}
	size += rt;

	return size;
}

static int read_tag_data(int tag_size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int size = 0;
	int rt = 0;

	if(tag_size > MAX_ID3TAG_BUFSIZE) {
		DTPRINTF(0x01, "BIG box(size: %d)\n", tag_size);
		rt = read_data(tag_buf, MAX_ID3TAG_BUFSIZE, tag_read);
		if(rt < MAX_ID3TAG_BUFSIZE) {
			tprintf("tag Read Error(size: %d)\n", rt);
		}
		size += rt;
		rt = tag_seekcur(tag_size - MAX_ID3TAG_BUFSIZE);
		size += rt;
	} else {
		rt = read_data(tag_buf, tag_size, tag_read);
		if(rt < tag_size) {
			tprintf("tag Read Error(size: %d)\n", rt);
		}
		size += rt;
	}

	return size;
}

#if defined(GSC_LIB_ENABLE_PICOJPEG) || defined(GSC_LIB_ENABLE_LIBPNG)
static unsigned char *malloc_ptr;

static int malloc_read_tag_data(int tag_size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int size = 0;
	int rt = 0;

	DTPRINTF(0x01, "malloc box(size: %d)\n", tag_size);

	malloc_ptr = (unsigned char *)alloc_memory(tag_size);
	if(malloc_ptr == 0) {
		tprintf("tag malloc fail\n");
		return size;
	}

	rt = read_data(malloc_ptr, tag_size, tag_read);
	if(rt < tag_size) {
		tprintf("tag Read Error(size: %d)\n", rt);
	}
	size += rt;

	return size;
}
#endif

static int calc_frame_size(unsigned char *tag)
{
	int frame_size = 0;

	frame_size =
			(((int)(((*(tag + 0)) & 0x7f))) << 21) +
			(((int)(((*(tag + 1)) & 0x7f))) << 14) +
			(((int)(((*(tag + 2)) & 0x7f))) << 7) +
			(((int)(((*(tag + 3)) & 0x7f))) << 0);

	return frame_size;
}

static int id3tag_header_decode(unsigned short *ver, unsigned char *tag)
{
	int tag_size = 0;

	// Version
	*ver = (*(tag + 3) << 8) + (*(tag + 4) << 0);
	DTPRINTF(0x01, "TAG Ver : %04X\n", *ver);

	// Flasg
	DTPRINTF(0x01, "Flags : %02X\n", (*(tag + 5)));

	// Size
	if(strncomp((const unsigned char *)tag, (unsigned char *)"ID3", 3) == 0) {
		tag_size = calc_frame_size(&tag[6]);
	} else {
		return -1;
	}

	return tag_size;
}

static int id3tag_frame_header_decode(unsigned int *id, unsigned char *tag)
{
	int frame_size = 0;
	unsigned char frame_id[4 + 1] = { 0, 0, 0, 0, 0 };

	strncopy(frame_id, (const unsigned char *)tag, 4);
	DTPRINTF(0x01, "ID = %s\n", frame_id);

	*id =
			(((unsigned int)tag[0]) << 24) +
			(((unsigned int)tag[1]) << 16) +
			(((unsigned int)tag[2]) <<  8) +
			(((unsigned int)tag[3]) <<  0);

	frame_size = calc_frame_size(&tag[4]);

	return frame_size;
}

struct st_id3tag_decode {
	unsigned char tag_id[5];
	int (* decode)(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur);
};

#if 0
static int decode_tag_str(unsigned char *str, unsigned char *tag, int len)
#else
static int decode_tag_str(uchar *str, unsigned char *tag, int len)
#endif
{
	int rt = 0;

	if(len > MAX_MINFO_STR) {
		len = MAX_MINFO_STR;
	}

	switch(tag[0]) {
	case 0: // SO-8859-1
		strncopy(str, &tag[1], len);
		break;

	case 1: // UTF-16/BOMあり
		/* TODO */
		break;

	case 2: // UTF-16BE/BOMなし
		/* TODO */
		break;

	case 3: // UTF-8
#if 0
		utf82sjis(str, &tag[1], len);
#else
		strncopy(str, &tag[1], len);
#endif
		break;

	default:
		DTPRINTF(0x01, "Unknown codec(%02X)\n", (int)tag[0]);
		break;
	}

	return rt;
}

/*
 * 各TAGデコード
 */

static int decode_TPE1(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_tag_data(size, tag_read, tag_seekcur);

	DTPRINTF(0x01, "%02X\n", (int)tag_buf[0]);
	tag_buf[size] = 0;
	DTPRINTF(0x01, "Artist : %s\n", &tag_buf[1]);

	decode_tag_str(info->artist, tag_buf, size);

	return rt;
}

static int decode_TIT2(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_tag_data(size, tag_read, tag_seekcur);

	DTPRINTF(0x01, "%02X\n", (int)tag_buf[0]);
	tag_buf[size] = 0;
	DTPRINTF(0x01, "Title : %s\n", &tag_buf[1]);

	decode_tag_str(info->title, tag_buf, size);

	return rt;
}

static int decode_TALB(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_tag_data(size, tag_read, tag_seekcur);

	DTPRINTF(0x01, "%02X\n", (int)tag_buf[0]);
	tag_buf[size] = 0;
	DTPRINTF(0x01, "Album : %s\n", &tag_buf[1]);

	decode_tag_str(info->album, tag_buf, size);

	return rt;
}

static int decode_TRCK(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_tag_data(size, tag_read, tag_seekcur);
	int i;

	DTPRINTF(0x01, "%02X\n", (int)tag_buf[0]);
	tag_buf[size] = 0;
	DTPRINTF(0x01, "Track : %s\n", &tag_buf[1]);

	for(i=1; i<size; i++) {
		if(tag_buf[i] == '/') {
			tag_buf[i] = 0;
			info->track = dstoi(&tag_buf[1]);
			info->last_track = dstoi(&tag_buf[i+1]);
			break;
		}
	}

	return rt;
}

static int decode_TLEN(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_tag_data(size, tag_read, tag_seekcur);

	DTPRINTF(0x01, "%02X\n", (int)tag_buf[0]);
	tag_buf[size] = 0;
	DTPRINTF(0x01, "Time Length : %s(ms)\n", &tag_buf[1]);

	info->time_length = dstoi(&tag_buf[1]);
	info->sample_count = (long long)info->time_length * info->sampling_rate / 1152/*info->frame_length*/ / 1000;

	return rt;
}

static int flg_id3_decode_artwork = 0;

void set_id3_decode_artwork(int flg_env)
{
	flg_id3_decode_artwork = flg_env;

	DTFPRINTF(0x01, "art_decode = %d\n", flg_id3_decode_artwork);
}

#ifdef GSC_LIB_ENABLE_PICOJPEG
#include "jpegdec.h"
#endif

#ifdef GSC_LIB_ENABLE_LIBPNG
#include "pngdec.h"
#endif

//#define DISABLE_DECODE_MULTTASK

static int decode_APIC(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
#if defined(GSC_LIB_ENABLE_PICOJPEG) || defined(GSC_LIB_ENABLE_LIBPNG)
	int rt;

	static const char apic_jpeg_str[] = "image/jpeg";
	static const char apic_png_str[] = "image/png";

	if(flg_id3_decode_artwork != 0) {
		unsigned char *cover_ptr;
		rt = malloc_read_tag_data(size, tag_read, tag_seekcur);
		cover_ptr = malloc_ptr+14;
		DTPRINTF(0x01, "%02X\n", (int)malloc_ptr[0]);
		malloc_ptr[size] = 0;

		XDUMP(0x02, malloc_ptr, size);

		if(strncomp(&malloc_ptr[1], (uchar *)apic_jpeg_str, sizeof(apic_jpeg_str)) == 0) {
			DTPRINTF(0x01, "JPEG Artwork\n");
#ifdef GSC_LIB_ENABLE_PICOJPEG
#ifdef DISABLE_DECODE_MULTTASK
			decode_jpeg_artwork(info, cover_ptr);
#else
			decode_jpeg_artwork_bg(info, cover_ptr, malloc_ptr);
#endif
#endif
		} else if(strncomp(&malloc_ptr[1], (uchar *)apic_png_str, sizeof(apic_png_str)) == 0) {
			DTPRINTF(0x01, "PNG Artwork\n");
#ifdef GSC_LIB_ENABLE_LIBPNG
#ifdef DISABLE_DECODE_MULTTASK
			decode_png_artwork(info, cover_ptr);
#else
			decode_png_artwork_bg(info, cover_ptr, malloc_ptr);
#endif
#endif
		} else {
			DTPRINTF(0x01, "Unknow format\n");
		}
#ifdef DISABLE_DECODE_MULTTASK
		free_memory(malloc_ptr);
#endif
	} else {
		rt = read_tag_data(size, tag_read, tag_seekcur);
	}

	return rt;
#else
	return 0;
#endif
}

static int decode_Txxx(struct st_music_info *info, int size, tag_read_func tag_read, tag_seekcur_func tag_seekcur)
{
	int rt = read_tag_data(size, tag_read, tag_seekcur);

	DTPRINTF(0x01, "%02X\n", (int)tag_buf[0]);
	tag_buf[size] = 0;
	DTPRINTF(0x01, "Txxx : %s\n", &tag_buf[1]);

	return rt;
}

static struct st_id3tag_decode id3tag_decode_list[] = {
	{ "TPE1", decode_TPE1 },
	{ "TIT2", decode_TIT2 },
	{ "TALB", decode_TALB },
	{ "TRCK", decode_TRCK },
	{ "TCON", decode_Txxx },
	{ "TDRC", decode_Txxx },
	{ "TSSE", decode_Txxx },
	{ "TCMP", decode_Txxx },
	{ "TPE2", decode_Txxx },
	{ "TLEN", decode_TLEN },
	{ "APIC", decode_APIC },
	{ {0, 0, 0, 0, 0}, 0 }
};

int id3tag_decode(struct st_music_info *info, tag_read_func tag_read, tag_seekcur_func tag_seekcur, tag_seekset_func tag_seekset,
		  tag_size_func tag_size, tag_tell_func tag_tell)
{
	unsigned char tag_header_buf[ID3TAG_HEADER_SIZE];
	int rt = 0;
	int frame_size = 0;
	int header_size = 0;
	unsigned short ver;
	unsigned int id;

	read_size = 0;

	init_music_info(info);

	// ID3TAG Header
	rt = tag_read(tag_header_buf, ID3TAG_HEADER_SIZE);
	if(rt < ID3TAG_HEADER_SIZE) {
		tprintf("Header Read Error(size: %d)\n", rt);
		return -1;
	}
	// read_size += rt;	// サイズにヘッダ分は入れない
	DTPRINTF(0x01, "READ SIZE %d\n", read_size);

	info->format = MUSIC_FMT_MP3;

	frame_size = id3tag_header_decode(&ver, tag_header_buf);
	DTPRINTF(0x01, "ID3TAG size = %d\n", frame_size);
	header_size = frame_size;

	if(ver < 0x0400) {
		// 取り敢えずID3v2.4のみ対応
#ifdef PRINT_MUSICFILE_INFO
		tprintf("Cannot support < ID3v2.4\n");
#endif
		return -1;
	}

	info->frame_size = 1152;	// 取り敢えず固定[TODO]

	while(read_size < header_size) {
		// Frame Header
		rt = tag_read(tag_header_buf, ID3TAG_FRAME_HEADER_SIZE);
		if(rt < ID3TAG_FRAME_HEADER_SIZE) {
			eprintf("Frame Read Error(size: %d)\n", rt);
			return -1;
		}
		read_size += rt;
		DTPRINTF(0x02, "READ SIZE %d\n", read_size);

		frame_size = id3tag_frame_header_decode(&id, tag_header_buf);
		DTPRINTF(0x01, "FRAME size = %d\n", frame_size);

		{
			struct st_id3tag_decode *p_id3dec = id3tag_decode_list;

			if(tag_header_buf[0] == 0) {
				DTPRINTF(0x01, "NULL TAG %02X%02X%02X%02X\n",
					  tag_buf[0],
					  tag_buf[1],
					  tag_buf[2],
					  tag_buf[3]);
				// ヘッダ分ファイルポインタを進めておく
				tag_seekcur(header_size - read_size);
				goto tagend;
			}

			while(p_id3dec->tag_id[0] != 0) {
				if(strncomp(tag_header_buf, p_id3dec->tag_id, 4) == 0) {
					rt = p_id3dec->decode(info, frame_size, tag_read, tag_seekcur);
					read_size += rt;
					break;
				}
				p_id3dec ++;
			}

			// 解析できないtag
			if(p_id3dec->tag_id[0] == 0) {
				rt = decode_Txxx(info, frame_size, tag_read, tag_seekcur);
				read_size += rt;
			}
		}
	}

tagend:
	DTPRINTF(0x01, "FRAME READ SIZE %d\n", read_size);

	if(1) {
		int ssize;
		int scount;
		int rtn = 0;
		unsigned char header[MPEG_FRAME_HEADER_SIZE];

		info->frame_start = tag_tell();
		DTPRINTF(0x04, "stream start = %d\n", info->frame_start);
		ssize = tag_size() - info->frame_start;

		rtn = tag_read(header, MPEG_FRAME_HEADER_SIZE);
		if(rtn != MPEG_FRAME_HEADER_SIZE) {
			eprintf("MPEG Frame read error(%d)\n", rtn);
		}

		tag_seekset(info->frame_start);
		DTPRINTF(0x04, "tag_tell = %d\n", tag_tell());

		rtn = mpeg_frame_header_decode(info, header);
		if(rtn != 0) {
			eprintf("MPEG Frame header decode error(%d)\n", rtn);
		}

		scount = ssize / info->frame_length;
		info->time_length = ssize * 8 / 128;
		info->sample_count = scount;
		DTPRINTF(0x04, "MP3 stream start = %d\n", info->frame_start);
		DTPRINTF(0x04, "MP3 stream size = %d\n", ssize);
		DTPRINTF(0x04, "MP3 time length = %d\n", info->time_length);
		DTPRINTF(0x04, "MP3 sample count = %d\n", scount);
	}

	return header_size;
}

static const unsigned short bitrate_list[16] = {
	0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320, 0
};

static const unsigned short smprate_list[4] = {
	44100, 48000, 32000, 0/* 予約 */
};

#ifdef DEBUGTBITS
static const char channel_str_list[][16] = {
	"STEREO",
	"JOINT STEREO",
	"DUAL CHANNEL",
	"MONORAL"
};
#endif

unsigned short mpeg_frame_header_check(unsigned char *data)
{
	unsigned short tmp;

	tmp = (((unsigned short)data[0]) << 8) + data[1];

	if(tmp != 0xfffb) { // MPEG1 Layer3
		if(strncomp(data, (unsigned char *)"TAG", 3) == 0) {
			return tmp; // おそらくID3v1タグ
		} else {
			DTPRINTF(0x01, "BAD Frame Header(%04X)\n", (int)tmp);
			return tmp;
		}
	}

	return 0;
}

int mpeg_frame_header_decode(struct st_music_info *info, unsigned char *data)
{
	unsigned short tmp;

	tmp = mpeg_frame_header_check(data);
	if(tmp != 0) {
		DTPRINTF(0x01, "BAD Frame Header(%04X)\n", (int)tmp);
		return -1;
	}

	tmp = ((data[2] >> 4) & 0x0f);	// Bit Rate
	info->bit_rate = bitrate_list[tmp];
	DTPRINTF(0x02, "Bit Rate : %d(Kbps)\n", (int)info->bit_rate);

	tmp = ((data[2] >> 2) & 0x03);	// Sampring Rate
	info->sampling_rate = smprate_list[tmp];
	DTPRINTF(0x02, "Sampling Rate : %d(Hz)\n", (int)info->sampling_rate);

	info->mpeg_padding = ((data[2] >> 1) & 0x01); // Padding bit
	DTPRINTF(0x02, "Padding : %d\n", (int)info->mpeg_padding);

	info->channel = ((data[3] >> 6) & 0x03);	// Channel
	DTPRINTF(0x02, "Channel : %s(%02X)\n",
		 channel_str_list[info->channel],
		 (int)info->channel);

	info->frame_length = ((144 * info->bit_rate * 1000)/(info->sampling_rate)) + info->mpeg_padding;
	DTPRINTF(0x02, "Frame Size : %d\n", (int)info->frame_length);

	return 0;
}
