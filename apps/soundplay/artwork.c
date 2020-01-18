/** @file
    @brief	ID3,MP4アートワーク画像処理

    @date	2019.12.16
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "task/syscall.h"
#include "sysevent.h"
#include "memory.h"
#include "jpegdec.h"
#include "pngdec.h"
#define GSLOG_PREFIX	"ART: "
#include "log.h"
#include "artwork.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


#define MAXARTWORKMEMORY	(1024*1024*2)

struct st_artwork_data {
	int kind;
	struct st_music_info *info;
	void *data;
	void *freemem;
};

void decode_jpeg_artwork(struct st_music_info *info, void *jpegdata)
{
	pjpeg_image_info_t jpeginfo;
	void *image;
	unsigned int imagesize;
	int rt = 0;

	get_jpeg_data_info(jpegdata, &jpeginfo, 0);
	GSLOG(8, "JPEG Width = %d, Height = %d\n", jpeginfo.m_width, jpeginfo.m_height);

	//draw_jpeg(0, 0);

	imagesize = ((((jpeginfo.m_width+(15))/16)*16)) * (((jpeginfo.m_height+(15))/16)*16) * sizeof(PIXEL_DATA);
	if(imagesize > MAXARTWORKMEMORY) {
		info->flg_have_artwork = MINFO_ARTWORK_NOIMAGE;
		return;
	}

	image = alloc_memory(imagesize);

	if(image == 0) {
		GSLOG(0, "JPEG decode cannnot alloc memory %d(%dx%d)\n", imagesize, jpeginfo.m_width, jpeginfo.m_height);
		info->flg_have_artwork = MINFO_ARTWORK_NOIMAGE;
	}

	rt = decode_jpeg(image);
	if(rt != 0) {
		info->flg_have_artwork = MINFO_ARTWORK_NOIMAGE;
	} else {
		//draw_image(0, 0, jpeginfo.m_width, jpeginfo.m_height, image, jpeginfo.m_width);
		resize_image(info->artwork, ART_WIDTH, ART_HEIGHT, image, jpeginfo.m_width, jpeginfo.m_height);
		//draw_image(0, 0, ART_WIDTH, ART_HEIGHT, info->artwork, ART_WIDTH);
		info->flg_have_artwork = MINFO_ARTWORK_JPEG;
	}

	free_memory(image);
}

void decode_png_artwork(struct st_music_info *info, void *pngdata)
{
	short width, height;
	void *image;
	unsigned int imagesize;
	int rt = 0;

	get_png_data_info(pngdata, &width, &height);
	GSLOG(8, "PING Width = %d, Height = %d\n", width, height);
	//draw_png(0, 0);

	imagesize = width * height * 4;
	if(imagesize > MAXARTWORKMEMORY) {
		info->flg_have_artwork = MINFO_ARTWORK_NOIMAGE;
		return;
	}

	image = alloc_memory(imagesize);

	if(image == 0) {
		GSLOG(0, "PNG decode cannnot alloc memory %d(%dx%d)\n", imagesize, width, height);
		dispose_png_info();
		info->flg_have_artwork = MINFO_ARTWORK_NOIMAGE;
		return;
	}

	rt = decode_png(image);
	if(rt != 0) {
		info->flg_have_artwork = MINFO_ARTWORK_NOIMAGE;
	} else {
		resize_image(info->artwork, ART_WIDTH, ART_HEIGHT, image, width, height);
		//draw_image(0, 0, ART_WIDTH, ART_HEIGHT, info->artwork, ART_WIDTH);
		info->flg_have_artwork = MINFO_ARTWORK_PNG;
	}

	free_memory(image);

	dispose_png_info();
}

static int artwork_task(void *arg)
{
	struct st_artwork_data artwork;

	DTFPRINTF(0x01, "Artwork decode task start\n");

	memorycopy((void *)&artwork, arg, sizeof(struct st_artwork_data));
	DTFPRINTF(0x01, "info = %p, data = %p, free = %p\n", artwork.info, artwork.data, artwork.freemem);

	switch(artwork.kind) {
	case 0:
		DTFPRINTF(0x01, "JPEG decode start\n");
		decode_jpeg_artwork(artwork.info, artwork.data);
		DTFPRINTF(0x01, "JPEG decode end\n");
		break;

	case 1:
		DTFPRINTF(0x01, "PNG decode start\n");
		decode_png_artwork(artwork.info, artwork.data);
		DTFPRINTF(0x01, "PNG decode end\n");
		break;

	default:
		DTFPRINTF(0x01, "Kind error\n");
		return 0;
		break;
	}

	DTFPRINTF(0x01, "info = %p, data = %p, free = %p\n", artwork.info, artwork.data, artwork.freemem);

	free_memory(artwork.freemem);

	create_event(200, 0, 0);

	DTFPRINTF(0x01, "Artwork decode task end\n");

	return 0;
}


#define SIZEOFARTWORKTS	(1024*4)
static struct st_tcb artwork_tcb;
static unsigned int artwork_stack[SIZEOFARTWORKTS/sizeof(unsigned int)] ATTR_STACK;

static void startup_artwork_task(int kind, struct st_music_info *info, void *data, void *freemem)
{
	static struct st_artwork_data artwork_data;

	if(artwork_tcb.status != PSTAT_DORMANT) {
		DTFPRINTF(0x01, "Artwork task abort\n");
		task_kill(artwork_tcb.id);
	}

	artwork_data.kind = kind;
	artwork_data.info = info;
	artwork_data.data = data;
	artwork_data.freemem = freemem;

	task_add(artwork_task, "artdec", TASK_PRIORITY_APP_LOW, &artwork_tcb, artwork_stack, SIZEOFARTWORKTS, (void *)&artwork_data);
}

void decode_jpeg_artwork_bg(struct st_music_info *info, void *data, void *freemem)
{
	DTFPRINTF(0x01, "info = %p, data = %p, free = %p\n", info, data, freemem);

	startup_artwork_task(0, info, data, freemem);
}

void decode_png_artwork_bg(struct st_music_info *info, void *data, void *freemem)
{
	DTFPRINTF(0x01, "info = %p, data = %p, free = %p\n", info, data, freemem);

	startup_artwork_task(1, info, data, freemem);
}
