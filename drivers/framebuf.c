/** @file
    @brief	フレームバッファドライバ(16ビットカラー)

    フレームバッファドライバは仮想グラフィックメモリだけを持つデバイスド
    ライバです。

    上位のグラフィックライブラリより使用されます。
    下位ドライバに、LCDドライバ等の実際に表示できるデバイスが必要です。

    register()実行時に下位ビデオデバイス名を指定することが出来ます。

    @date	2017.01.15
    @author	Takashi SHUDO
*/

#include "device.h"
#include "framebuf.h"
#include "device/video_ioctl.h"
#include "graphics.h"
#include "tkprintf.h"
#include "sysconfig.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


static void framebuf_set_forecolor(struct st_framebuf_context *fc, unsigned int color)
{
	fc->fore_color = color;
}

static void framebuf_set_backcolor(struct st_framebuf_context *fc, unsigned int color)
{
	fc->back_color = color;
}

static void framebuf_draw_point(struct st_framebuf_context *fc, int x, int y)
{
	PIXEL_DATA *dp;

	dp = (PIXEL_DATA *)fc->fb_ptr[fc->draw_frame] + (y * fc->width) + x;

	*dp = (PIXEL_DATA)fc->fore_color;
}

static inline void framebuf_set_ptr(struct st_framebuf_context *fc)
{
	fc->draw_ptr = fc->fb_ptr[fc->draw_frame]
			+ (fc->pen_y * fc->width * sizeof(PIXEL_DATA))
			+ (fc->pen_x * sizeof(PIXEL_DATA));
}

static inline unsigned int framebuf_read_point(struct st_framebuf_context *fc)
{
	unsigned int rtn = *(PIXEL_DATA *)(fc->draw_ptr);

	fc->pen_x ++;
	fc->draw_ptr += sizeof(PIXEL_DATA);
	if(fc->pen_x > fc->clip.right) {
		fc->pen_x = fc->clip.left;
		fc->pen_y ++;
		framebuf_set_ptr(fc);
	}

	return rtn;
}

static inline void framebuf_write_point(struct st_framebuf_context *fc, unsigned int color)
{
	*(PIXEL_DATA *)(fc->draw_ptr) = (PIXEL_DATA)color;

	fc->pen_x ++;
	fc->draw_ptr += sizeof(PIXEL_DATA);
	if(fc->pen_x > fc->clip.right) {
		fc->pen_x = fc->clip.left;
		fc->pen_y ++;
		framebuf_set_ptr(fc);
	}
}

static void framebuf_repeat_data(struct st_framebuf_context *fc, int len)
{
	while(len) {
		framebuf_write_point(fc, fc->fore_color);
		len --;
	}
}

static void framebuf_fill_screen(struct st_framebuf_context *fc, unsigned int data)
{
	unsigned int count = (fc->mem_size)/sizeof(PIXEL_DATA);
	PIXEL_DATA *dp = (PIXEL_DATA *)fc->fb_ptr[fc->draw_frame];

	while(count) {
		*dp = (PIXEL_DATA)data;
		dp ++;
		count --;
	}
}

static const char def_v_dev[] = DEF_DEV_NAME_VIDEO;

static struct st_framebuf_context fb_ctx;
static struct st_mutex framebuf_mutex;

static int framebuf_register(struct st_device *dev, char *param)
{
	struct st_device *v_dev;
	struct st_video_info *v_info;
	struct st_framebuf_context *fc;
	char *dev_name = (char *)def_v_dev;

	if(param != 0) {
		dev_name = param;
	}

	v_dev = open_device(dev_name);
	if(v_dev == 0) {
		SYSERR_PRINT("Cannot open device \"%s\"\n", dev_name);
		return -1;
	}

	dev->info = (void *)(v_dev->info);
	dev->private_data = &fb_ctx;

	v_info = dev->info;
	fc = &fb_ctx;

	fc->v_dev	= v_dev;
	fc->width	= v_info->width;
	fc->height	= v_info->height;
	fc->pixcel_byte	= 2; // 16bit color
	fc->disp_frame	= 0;
	fc->draw_frame	= 0;
	fc->fb_ptr[0]	= v_info->frame_buf_ptr[0];
	fc->fb_ptr[1]	= v_info->frame_buf_ptr[1];
	fc->mem_size	= v_info->mem_size;
	fc->fore_color	= RGB(0, 0, 0);
	fc->clip.left	= 0;
	fc->clip.top	= 0;
	fc->clip.right	= fc->width;
	fc->clip.bottom	= fc->height;
	fc->pen_x	= 0;
	fc->pen_y	= 0;
	fc->draw_ptr	= fc->fb_ptr[0];

	return 0;
}

static int framebuf_read(struct st_device *dev, void *data, unsigned int size)
{
	struct st_framebuf_context *fc = (struct st_framebuf_context *)(dev->private_data);
	int i;
	PIXEL_DATA *dp = (PIXEL_DATA *)data;

	DKFPRINTF(0x01, "data = %p, size = %ld\n", data, size);

	for(i=0; i<size/sizeof(PIXEL_DATA); i++) {
		//PIXEL_DATA tmp = (((PIXEL_DATA)(*data)) << 8) + (*(data + 1));
		*dp = framebuf_read_point(fc);
		dp ++;
	}

	return size;
}

static int framebuf_write(struct st_device *dev, const void *data, unsigned int size)
{
	struct st_framebuf_context *fc = (struct st_framebuf_context *)(dev->private_data);
	int i;
	PIXEL_DATA *dp = (PIXEL_DATA *)data;

	DKFPRINTF(0x01, "data = %p, size = %ld\n", data, size);

	for(i=0; i<size/sizeof(PIXEL_DATA); i++) {
		//PIXEL_DATA tmp = (((PIXEL_DATA)(*data)) << 8) + (*(data + 1));
		framebuf_write_point(fc, *dp);
		dp ++;
	}

	return size;
}

static void reset_rect(struct st_framebuf_context *fc)
{
	fc->clip.left	= 0;
	fc->clip.top	= 0;
	fc->clip.right	= fc->width - 1;
	fc->clip.bottom	= fc->height - 1;
	fc->pen_x	= fc->clip.left;
	fc->pen_y	= fc->clip.top;
	framebuf_set_ptr(fc);
}

static int framebuf_seek(struct st_device *dev, int offset, int whence)
{
	struct st_framebuf_context *fc = (struct st_framebuf_context *)(dev->private_data);

	DKFPRINTF(0x01, "offset = %d, whence = %d\n", offset, whence);

	// [TODO] 画面ハードコピー用に暫定的な処理
	reset_rect(fc);

	return 0;
}

static int framebuf_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	struct st_framebuf_context *fc = (struct st_framebuf_context *)(dev->private_data);

	DKFPRINTF(0x01, "com = %08lX, arg = %08lX\n", com, arg);

	switch(IOCTL(com)) {
	case IOCMD_VIDEO_SETDISPFRAME:
		if(arg >= MAX_FRAMEBUF) {
			return -1;
		}
		fc->disp_frame = arg;

		// 表示バッファを変更
		ioctl_device(fc->v_dev, IOCMD_VIDEO_SETDISPFRAME, arg, 0);
		break;

	case IOCMD_VIDEO_GETDISPFRAME:
		return fc->disp_frame;
		break;

	case IOCMD_VIDEO_SETDRAWFRAME:
		if(arg >= MAX_FRAMEBUF) {
			return -1;
		}
		fc->draw_frame = arg;
		fc->draw_ptr	= fc->fb_ptr[arg];
		break;

	case IOCMD_VIDEO_GETDRAWFRAME:
		return fc->draw_frame;
		break;

	case IOCMD_VIDEO_SETRECT:
		{
			struct st_rect *rect = (struct st_rect *)param;
			fc->clip.left	= rect->left;
			fc->clip.top	= rect->top;
			fc->clip.right	= rect->right;
			fc->clip.bottom	= rect->bottom;
			fc->pen_x	= rect->left;
			fc->pen_y	= rect->top;
			framebuf_set_ptr(fc);
		}
		break;

	case IOCMD_VIDEO_RESETRECT:
		reset_rect(fc);
		break;

	case IOCMD_VIDEO_CLEAR:
		framebuf_fill_screen(fc, RGB(0, 0, 0));
		break;

	case IOCMD_VIDEO_FILL:
		framebuf_fill_screen(fc, arg);
		break;

	case IOCMD_VIDEO_BCKLIGHT:
		switch(arg) {
		case 0:
			break;

		case 1:
			break;
		}
		break;

	case IOCMD_VIDEO_WRITE_WORD:
	case IOCMD_VIDEO_NOLOCK_WRITE_WORD:
		framebuf_write_point(fc, arg);
		break;

	case IOCMD_VIDEO_SET_FORECOLOR:
		framebuf_set_forecolor(fc, (unsigned int)arg);
		break;

	case IOCMD_VIDEO_SET_BACKCOLOR:
		framebuf_set_backcolor(fc, (unsigned int)arg);
		break;

	case IOCMD_VIDEO_REPEAT_DATA:
		framebuf_repeat_data(fc, arg);
		break;

	case IOCMD_VIDEO_DRAW_PIXEL:
		{
			int x = ((arg >>  0) & 0xffff);
			int y = ((arg >> 16) & 0xffff);
			int dx = x;
			int dy = y;

			framebuf_draw_point(fc, dx, dy);
		}
		break;

	case IOCMD_VIDEO_DRAW_BITS:
		{
			int sbit = ((com >> 12) & 0x7);
			int count = (com & 0x0fff);
			unsigned char *data = (unsigned char *)param;
			unsigned char bit = (0x80 >> sbit);
			int i;

			for(i=0; i<count; i++) {
				if(*data & bit) {
					framebuf_write_point(fc, fc->fore_color);
				} else {
					framebuf_write_point(fc, fc->back_color);
				}
				if(bit == 0x01) {
					bit = 0x80;
					data ++;
				} else {
					bit >>= 1;
				}
			}
		}
		break;

	case IOCMD_VIDEO_SCROLL:
		{
			int i, j;
			PIXEL_DATA *sp;
			PIXEL_DATA *dp;
			//int x = (short)((arg >>  0) & 0xffff);
			int y = (short)((arg >> 16) & 0xffff);

			sp = (PIXEL_DATA *)fc->fb_ptr[fc->draw_frame]
					+ (-y * fc->width);
			dp = (PIXEL_DATA *)fc->fb_ptr[fc->draw_frame];

			for(j=0; j<(fc->height + y); j++) {
				for(i=0; i<fc->width; i++) {
					*dp = *sp;
					dp ++;
					sp ++;
				}
			}
		}
		break;

	case IOCMD_VIDEO_LOCK_DEVICE:
	case IOCMD_VIDEO_UNLOCK_DEVICE:
		// 何もしない
		break;

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		return -1;
	}

	return 0;
}

struct st_device framebuf_device = {
	.name		= "fb",
	.explan		= "Frame buffer(16 bit color)",
	.mutex		= &framebuf_mutex,
	.register_dev	= framebuf_register,
	.read		= framebuf_read,
	.write		= framebuf_write,
	.seek		= framebuf_seek,
	.ioctl		= framebuf_ioctl,
};
