/** @file
    @brief	仮想 16bits color LCD ドライバ

    @date	2016.12.28
    @author	Takashi SHUDO
*/

#include "device.h"
#include "device/video_ioctl.h"
#include "graphics.h"
#include "tkprintf.h"
#include "sysconfig.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


static void off_backlight(void)
{
}

static void on_backlight(void)
{
}

#define LCD_WIDTH	GSC_GRAPHICS_DISPLAY_WIDTH		// $gsc グラフィックデバイスの表示幅
#define LCD_HEIGHT	GSC_GRAPHICS_DISPLAY_HEIGHT	// $gsc グラフィックデバイスの表示高さ
#define LCD_MAX_ADDR	(LCD_WIDTH*LCD_HEIGHT)

void vlcd_set_disp_frame(int num);
void vlcd_set_draw_frame(int num);
void vlcd_set_forecolor(unsigned long color);
void vlcd_set_backcolor(unsigned long color);
void vlcd_set_rect(struct st_rect *rect);
void vlcd_reset_rect(void);
void vlcd_fill(unsigned long data);
void vlcd_read_data(unsigned int *data, long len);
void vlcd_write_data(unsigned int *data, long len);
void vlcd_draw_point(int x, int y);
void vlcd_write_point(unsigned long color);
void vlcd_repeat_data(int len);

static void lcd_fill_screen(unsigned long data)
{
	vlcd_fill(data);
}

static unsigned char power;	//!< 電源状態

static int vlcd_register(struct st_device *dev, char *param)
{
	power = 1;

	return 0;
}

static int vlcd_read(struct st_device *dev, void *data, unsigned int size)
{
	DKFPRINTF(0x01, "data = %p, size = %d\n", data, size);

	if(!power) {
		return 0;
	}

	vlcd_read_data((unsigned int *)data, size/sizeof(unsigned int));

	return size;
}

static int vlcd_write(struct st_device *dev, const void *data, unsigned int size)
{
	DKFPRINTF(0x01, "data = %p, size = %d\n", data, size);

	if(!power) {
		return 0;
	}

	vlcd_write_data((unsigned int *)data, size/sizeof(unsigned int));

	return size;
}

static int vlcd_seek(struct st_device *dev, int offset, int whence)
{
	DKFPRINTF(0x01, "offset = %d, whence = %d\n", offset, whence);

	if(!power) {
		return 0;
	}

	// [TODO] 画面ハードコピー用に暫定的な処理
	vlcd_reset_rect();

	return 0;
}

#define MAX_FRAMEBUF	2
static int disp_frame = 0;
static int draw_frame = 0;

extern unsigned int vlcd_fore_color;
extern unsigned int vlcd_back_color;

static int vlcd_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	if(!power) {
		return 0;
	}

	DKFPRINTF(0x01, "com = %08lX, arg = %08lX\n", com, arg);

	switch(IOCTL(com)) {
	case IOCMD_VIDEO_LOCK_DEVICE:
		// Do nothing
		break;

	case IOCMD_VIDEO_UNLOCK_DEVICE:
		// Do nothing
		break;

	case IOCMD_VIDEO_SETDISPFRAME:
		if(arg >= MAX_FRAMEBUF) {
			return -1;
		}
		disp_frame = arg;
		vlcd_set_disp_frame(arg);
		break;

	case IOCMD_VIDEO_GETDISPFRAME:
		return disp_frame;
		break;

	case IOCMD_VIDEO_SETDRAWFRAME:
		if(arg >= MAX_FRAMEBUF) {
			return -1;
		}
		draw_frame = arg;
		vlcd_set_draw_frame(arg);
		break;

	case IOCMD_VIDEO_GETDRAWFRAME:
		return draw_frame;
		break;

	case IOCMD_VIDEO_SETRECT:
		{
			struct st_rect *rect = (struct st_rect *)param;
			vlcd_set_rect(rect);
			return 0;
		}
		break;

	case IOCMD_VIDEO_RESETRECT:
		vlcd_reset_rect();
		return 0;
		break;

	case IOCMD_VIDEO_CLEAR:
		lcd_fill_screen(RGB(0, 0, 0));
		return 0;
		break;

	case IOCMD_VIDEO_FILL:
		lcd_fill_screen(arg);
		return 0;
		break;

	case IOCMD_VIDEO_BCKLIGHT:
		switch(arg) {
		case 0:
			off_backlight();
			break;

		case 1:
			on_backlight();
			break;
		}
		return 0;
		break;

	case IOCMD_VIDEO_WRITE_WORD:
	case IOCMD_VIDEO_NOLOCK_WRITE_WORD:
		vlcd_write_point(arg);
		return 0;
		break;

	case IOCMD_VIDEO_SET_FORECOLOR:
		vlcd_set_forecolor((unsigned long)arg);
		return 0;
		break;

	case IOCMD_VIDEO_SET_BACKCOLOR:
		vlcd_set_backcolor((unsigned long)arg);
		return 0;
		break;

	case IOCMD_VIDEO_REPEAT_DATA:
//		SYSERR_PRINT("IOCMD_VIDEO_REPEAT_DATA %08X %d\n", (int)com, (int)arg);
		vlcd_repeat_data(arg);
		return 0;
		break;

	case IOCMD_VIDEO_DRAW_PIXEL:
		{
			int x = ((arg >>  0) & 0xffff);
			int y = ((arg >> 16) & 0xffff);
			int dx = x;
			int dy = y;

			vlcd_draw_point(dx, dy);
			return 0;
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
					vlcd_write_point(vlcd_fore_color);
				} else {
					vlcd_write_point(vlcd_back_color);
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

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		return -1;
	}

	return 0;
}

static int vlcd_suspend(struct st_device *dev)
{
	power = 0;

	off_backlight();

	// 電源Off

	return 0;
}

static int vlcd_resume(struct st_device *dev)
{
	lcd_fill_screen(RGB(0, 0, 0));

	power = 1;

	return 0;
}

static struct st_video_info vlcd_info = {
	.type		= VIDEOTYPE_FRAMEBUF,
	.width		= LCD_WIDTH,
	.height		= LCD_HEIGHT,
	.color_depth	= VCOLORDEP_32,
	.frame_num	= MAX_FRAMEBUF,
	.frame_buf_ptr	= { 0, 0 },
	.mem_size	= 0,
};

const struct st_device vlcd_device = {
	.name		= DEF_DEV_NAME_VIDEO,
	.explan		= "EMU Display Window",
	.info		= (void *)&vlcd_info,
	.register_dev	= vlcd_register,
	.read		= vlcd_read,
	.write		= vlcd_write,
	.seek		= vlcd_seek,
	.ioctl		= vlcd_ioctl,
	.suspend	= vlcd_suspend,
	.resume		= vlcd_resume,
};
