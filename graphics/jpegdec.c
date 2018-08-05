/** @file
    @brief	JPEGデコード

    picojpeg を使用
    https://github.com/richgel999/picojpeg

    @date	2017.11.12
    @author	Takashi SHUDO
*/

#include "picojpeg.h"
#include "tprintf.h"
#include "file.h"
#include "tkprintf.h"
#include "graphics.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


#ifdef DEBUGTBITS
static const char scan_type_str[][6] = {
	"GRAY",
	"YH1V1",
	"YH2V1",
	"YH1V2",
	"YH2V2"
};
#endif

/*
 * JPEGデータAPI
 */
static unsigned char *jpeg_data_ptr;
static int flg_reduce = 0;
static void *image_ptr;
static int flg_draw = 0;

static unsigned char pjpeg_data_need_bytes_callback(unsigned char *pBuf, unsigned char buf_size,
						    unsigned char *pBytes_actually_read, void *pCallback_data)
{
	int rsize = 0;

	DTPRINTF(0x02, "pBuf %p, buf_size %d\n", pBuf, buf_size);

	(void)memorycopy(pBuf, jpeg_data_ptr, buf_size);
	rsize = buf_size;
	jpeg_data_ptr += buf_size;

	XDUMP(0x02, pBuf, rsize);

	*pBytes_actually_read = rsize;

	return 0;
}

static pjpeg_image_info_t *image_info;
static PIXEL_DATA gbuf[16*16];
static short jpx, jpy;

static void draw_pict(short px, short py, void *gbuf)
{
	short gw = 8, gh = 8;

	if((px + 8) > image_info->m_width) {
		gw = (image_info->m_width % 8);
		DTPRINTF(0x02, "X=%d, Y=%d, W=%d\n", px, py, gw);
	}
	if((py + 8) > image_info->m_height) {
		gh = (image_info->m_height % 8);
		DTPRINTF(0x02, "X=%d, Y=%d, H=%d\n", px, py, gh);
	}

	if(flg_draw != 0) {
		if(flg_reduce != 0) {
			set_forecolor(*(PIXEL_DATA *)gbuf);
			draw_point(jpx + px/8, jpy + py/8);
		} else {
			draw_image(jpx + px, jpy + py, gw, gh, gbuf, 8);
		}
	} else {
		PIXEL_DATA *dp, *sp = gbuf;;
		if(flg_reduce != 0) {
			dp = (PIXEL_DATA *)(image_ptr + ((image_info->m_width * py) + px) * sizeof(PIXEL_DATA));
			*dp = *(PIXEL_DATA *)gbuf;
		} else {
			int x, y;
			for(y=0; y<gh; y++) {
				dp = (PIXEL_DATA *)(image_ptr + ((image_info->m_width * (py+y)) + px) * sizeof(PIXEL_DATA));
				for(x=0; x<gw; x++) {
					*dp = *sp;
					dp ++;
					sp ++;
				}
			}
		}
	}
}

static void draw_block(short px, short py)
{
	int i;
	int dlen = image_info->m_MCUWidth * image_info->m_MCUHeight;
	unsigned char *pr = image_info->m_pMCUBufR;
	unsigned char *pg = image_info->m_pMCUBufG;
	unsigned char *pb = image_info->m_pMCUBufB;

	DTPRINTF(0x02, "X = %d, Y = %d mcu = %d\n", px, py, dlen);
	DTPRINTF(0x02, "R = %p, B = %p, G = %p\n", pr, pg, pb);
	DTPRINTF(0x02, "W = %d, H = %d\n", image_info->m_MCUWidth, image_info->m_MCUHeight);

	for(i=0; i<dlen; i++) {
		gbuf[i] = RGB(*pr, *pg, *pb);
		pr ++;
		pg ++;
		pb ++;
	}

	XDUMP(0x04, (unsigned char *)gbuf, dlen * sizeof(gbuf[0]));

	switch(image_info->m_scanType) {
	case PJPG_GRAYSCALE:
		// @todo JPEGグレースケール表示対応
		break;

	case PJPG_YH1V1:
		draw_pict(px,   py,   gbuf);
		break;

	case PJPG_YH1V2:
		draw_pict(px,   py,   gbuf);
		draw_pict(px,   py+8, &gbuf[64]);
		break;

	case PJPG_YH2V1:
		draw_pict(px,   py,   gbuf);
		draw_pict(px+8, py,   &gbuf[64]);
		break;

	case PJPG_YH2V2:
		draw_pict(px,   py,   gbuf);
		draw_pict(px+8, py,   &gbuf[64]);
		draw_pict(px,   py+8, &gbuf[128]);
		draw_pict(px+8, py+8, &gbuf[192]);
		break;

	default:
		break;
	}
}

int get_jpeg_data_info(unsigned char *jpeg_data, pjpeg_image_info_t *jpeginfo, int reduce)
{
	unsigned char status;

	jpeg_data_ptr = jpeg_data;
	image_info = jpeginfo;
	flg_reduce = reduce;

	status = pjpeg_decode_init(image_info, pjpeg_data_need_bytes_callback, 0, reduce);

	DTPRINTF(0x01, "status = %d\n", status);
	if(status != 0) {
		DTPRINTF(0x01, "pjpeg_decode_init() failed with status %u\n", status);
		if(status == PJPG_UNSUPPORTED_MODE) {
			DTPRINTF(0x01, "Progressive JPEG files are not supported.\n");
		}
		return -1;
	}

	DTPRINTF(0x01, "IMAGE WIDTH = %d, HEIGHT = %d, COMPS = %d\n", image_info->m_width, image_info->m_height, image_info->m_comps);
	DTPRINTF(0x01, "MCU   WIDTH = %d, HEIGHT = %d\n", image_info->m_MCUWidth, image_info->m_MCUHeight);
	DTPRINTF(0x01, "SCANTYPE = %s\n", scan_type_str[image_info->m_scanType]);

	return 0;
}

/*
 * JPEGファイルAPI
 */
static int jpeg_fd = 0;

static unsigned char pjpeg_file_need_bytes_callback(unsigned char *pBuf, unsigned char buf_size,
						    unsigned char *pBytes_actually_read, void *pCallback_data)
{
	int rsize = 0;

	DTPRINTF(0x02, "pBuf %p, buf_size %d\n", pBuf, buf_size);

	rsize = read_file(jpeg_fd, pBuf, buf_size);
	XDUMP(0x02, pBuf, rsize);

	*pBytes_actually_read = rsize;

	return 0;
}

int get_jpeg_file_info(int fd, pjpeg_image_info_t *jpeginfo, int reduce)
{
	unsigned char status;

	jpeg_fd = fd;
	image_info = jpeginfo;
	flg_reduce = reduce;

	status = pjpeg_decode_init(image_info, pjpeg_file_need_bytes_callback, 0, reduce);

	DTPRINTF(0x01, "status = %d\n", status);
	if(status != 0) {
		DTPRINTF(0x01, "pjpeg_decode_init() failed with status %u\n", status);
		if(status == PJPG_UNSUPPORTED_MODE) {
			DTPRINTF(0x01, "Progressive JPEG files are not supported.\n");
		}
		return -1;
	}

	DTPRINTF(0x01, "IMAGE WIDTH = %d, HEIGHT = %d, COMPS = %d\n", image_info->m_width, image_info->m_height, image_info->m_comps);
	DTPRINTF(0x01, "MCU   WIDTH = %d, HEIGHT = %d\n", image_info->m_MCUWidth, image_info->m_MCUHeight);
	DTPRINTF(0x01, "SCANTYPE = %s\n", scan_type_str[image_info->m_scanType]);

	return 0;
}


static int decode_proc_jpeg(void)
{
	short gy = 0, gx = 0;

	for(;;) {
		unsigned char status;

		status = pjpeg_decode_mcu();

		if(status != 0) {
			if(status != PJPG_NO_MORE_BLOCKS) {
				DTPRINTF(0x01, "pjpeg_decode_mcu() failed with status %u\n", status);
				return -1;
			}
			break;
		}

		XDUMP(0x04, image_info->m_pMCUBufR, 256);
		XDUMP(0x04, image_info->m_pMCUBufG, 256);
		XDUMP(0x04, image_info->m_pMCUBufB, 256);

		draw_block(gx, gy);

		gx += image_info->m_MCUWidth;
		if(gx >= image_info->m_width) {
			gx = 0;
			gy += image_info->m_MCUHeight;
			if(gy >= image_info->m_height) {
				break;
			}
		}
	}

	return 0;
}

/*
 * 描画API
 */
int draw_jpeg(short px, short py)
{
	jpx = px;
	jpy = py;
	flg_draw = 1;

	return decode_proc_jpeg();
}

/*
 * デコードAPI
 */
int decode_jpeg(void *image)
{
	image_ptr = image;
	flg_draw = 0;

	return decode_proc_jpeg();
}
