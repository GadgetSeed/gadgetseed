/** @file
    @brief	HX8357D LCD GPIO 8bit接続 ドライバ
		Kuman MAR3520(Kuman 3.5inch TFT LCD Shield)

    @date	2017.11.02
    @author	Takashi SHUDO
*/

#include "device.h"
#include "device/video_ioctl.h"
#include "device/vio_ioctl.h"
#include "timer.h"
#include "tkprintf.h"
#include "graphics.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


#define LCD_WIDTH	480	// 幅
#define LCD_HEIGHT	320	// 高さ

static struct st_device *gpio_dev;
static unsigned short lcd_width  = LCD_WIDTH;
static unsigned short lcd_height = LCD_HEIGHT;
static unsigned short fore_color = 0;
static unsigned short back_color = 0;

#define FMEM_NOP	0
#define FMEM_WRITE	1
#define FMEM_READ	2
static int fmem_stat = FMEM_NOP;

#define HX8357_NOP	0x00
#define HX8357_SWRESET	0x01
#define HX8357_RDDID	0x04
#define HX8357_RDDST	0x09

#define HX8357B_RDPOWMODE	0x0A
#define HX8357B_RDMADCTL	0x0B
#define HX8357B_RDCOLMOD	0x0C
#define HX8357B_RDDIM	0x0D
#define HX8357B_RDDSDR	0x0F

#define HX8357_SLPIN	0x10
#define HX8357_SLPOUT	0x11
#define HX8357B_PTLON	0x12
#define HX8357B_NORON   0x13

#define HX8357_INVOFF	0x20
#define HX8357_INVON	0x21
#define HX8357_DISPOFF	0x28
#define HX8357_DISPON	0x29

#define HX8357_CASET	0x2A
#define HX8357_PASET	0x2B
#define HX8357_RAMWR	0x2C
#define HX8357_RAMRD	0x2E

#define HX8357B_PTLAR	0x30
#define HX8357_TEON	0x35
#define HX8357_TEARLINE	0x44
#define HX8357_MADCTL	0x36
#define HX8357_COLMOD	0x3A

#define HX8357_SETOSC	0xB0
#define HX8357_SETPWR1	0xB1
#define HX8357B_SETDISPLAY	0xB2
#define HX8357_SETRGB	0xB3
#define HX8357D_SETCOM	0xB6

#define HX8357B_SETDISPMODE	0xB4
#define HX8357D_SETCYC	0xB4
#define HX8357B_SETOTP	0xB7
#define HX8357D_SETC	0xB9

#define HX8357B_SET_PANEL_DRIVING	0xC0
#define HX8357D_SETSTBA	0xC0
#define HX8357B_SETDGC	0xC1
#define HX8357B_SETID	0xC3
#define HX8357B_SETDDB	0xC4
#define HX8357B_SETDISPLAYFRAME	0xC5
#define HX8357B_GAMMASET	0xC8
#define HX8357B_SETCABC	0xC9
#define HX8357_SETPANEL	0xCC


#define HX8357B_SETPOWER	0xD0
#define HX8357B_SETVCOM	0xD1
#define HX8357B_SETPWRNORMAL	0xD2

#define HX8357B_RDID1	0xDA
#define HX8357B_RDID2	0xDB
#define HX8357B_RDID3	0xDC
#define HX8357B_RDID4	0xDD

#define HX8357D_SETGAMMA	0xE0

#define HX8357B_SETGAMMA	0xC8
#define HX8357B_SETPANELRELATED	0xE9

#define HX8357B_MADCTL_MY	0x80
#define HX8357B_MADCTL_MX	0x40
#define HX8357B_MADCTL_MV	0x20
#define HX8357B_MADCTL_ML	0x10
#define HX8357B_MADCTL_RGB	0x00
#define HX8357B_MADCTL_BGR	0x08
#define HX8357B_MADCTL_MH	0x04

static void set_cs(unsigned int cs)
{
	ioctl_device(gpio_dev, IOCMD_VIO_SET_CS, cs, 0);
}

static void reset_hx8357d(void)
{
	ioctl_device(gpio_dev, IOCMD_VIO_SET_RESET, 0, 0);
	wait_time(100);
	ioctl_device(gpio_dev, IOCMD_VIO_SET_RESET, 1, 0);
}

static inline void write_reg8(unsigned char addr, unsigned char data)
{
	ioctl_device(gpio_dev, IOCMD_VIO_WRITE_REG8,
		     (((long)addr) << 16) | (long)(data & 0xff), 0);
}

static inline void write_reg16(unsigned char addr, unsigned short data)
{
	ioctl_device(gpio_dev, IOCMD_VIO_WRITE_REG16,
		     (((long)addr) << 16) | (long)(data & 0xffff), 0);
}

static inline void write_reg(unsigned char addr, int len, unsigned char *data)
{
	ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, addr, 0);

	while(len) {
		ioctl_device(gpio_dev, IOCMD_VIO_WRITE_DATA8, *data, 0);
		data ++;
		len --;
	}
}

static inline void write_reg32(unsigned char addr, unsigned long data)
{
	ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, addr, 0);
	ioctl_device(gpio_dev, IOCMD_VIO_WRITE_DATA32, data, 0);
}

static inline unsigned long read_reg32(unsigned char addr)
{
	return ioctl_device(gpio_dev, IOCMD_VIO_READ_REG32, addr, 0);
}

static void set_window(int x1, int y1, int x2, int y2)
{
	DKPRINTF(0x01, "WO %3d %3d %3d %3d\n", x1, y1, x2, y2);

	set_cs(0);

	write_reg32(HX8357_CASET, ((x1<<16) | x2));
	write_reg32(HX8357_PASET, ((y1<<16) | y2));

	set_cs(1);
}

static void init_hx8357d(void)
{
	reset_hx8357d();

	set_cs(0);

	ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, HX8357_SWRESET, 0);
	static unsigned char setc[3] = { 0xFF, 0x83, 0x57 };
	write_reg(HX8357D_SETC, 3, setc);
	wait_time(250);
	static unsigned char setrgb[4] = { 0x00, 0x00, 0x06, 0x06 };
	write_reg(HX8357_SETRGB, 4, setrgb);
	write_reg8(HX8357D_SETCOM, 0x25);	// -1.52V
	write_reg8(HX8357_SETOSC, 0x68);	// Normal mode 70Hz, Idle mode 55 Hz
	write_reg8(HX8357_SETPANEL, 0x05);	// BGR, Gate direction swapped
	static unsigned char setpwr1[6] = { 0x00, 0x15, 0x1C, 0x1C, 0x83, 0xAA };
	write_reg(HX8357_SETPWR1, 6, setpwr1);
	static unsigned char setstba[6] = { 0x50, 0x50, 0x01, 0x3C, 0x1E, 0x08 };
	write_reg(HX8357D_SETSTBA, 6, setstba);
	// MEME GAMMA HERE
	static unsigned char setcyc[7] = { 0x02, 0x40, 0x00, 0x2A, 0x2A, 0x0D, 0x78 };
	write_reg(HX8357D_SETCYC, 7, setcyc);
	write_reg8(HX8357_COLMOD, 0x55);
//	write_reg8(HX8357_MADCTL, 0xC0);	// 320 x 480
	write_reg8(HX8357_MADCTL, 0x60);	// 480 x 320
	write_reg8(HX8357_TEON, 0x00);
	write_reg16(HX8357_TEARLINE, 0x0002);
	ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, HX8357_SLPOUT, 0);
	wait_time(150);
	ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, HX8357_DISPON, 0);
	wait_time(50);
	
	set_cs(1);

	set_window(0, 0, lcd_width - 1, lcd_height - 1);
}

static void fill_buffer(unsigned short color)
{
	set_window(0, 0, lcd_width - 1, lcd_height - 1);

	ioctl_device(gpio_dev, IOCMD_VIO_SET_WRITEDATA0, (unsigned int)color, 0);

	set_cs(0);
	fmem_stat = FMEM_WRITE;
	ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, HX8357_RAMWR, 0);
	ioctl_device(gpio_dev, IOCMD_VIO_REPEAT_DATA, (unsigned int)LCD_WIDTH * LCD_HEIGHT, 0);
	set_cs(1);
}

static int hx8357d_lcd_register(struct st_device *dev, char *param)
{
	gpio_dev = open_device(param);
	if(gpio_dev == 0) {
		SYSERR_PRINT("Cannot open device \"%s\"\n", param);
		return -1;
	}

	init_hx8357d();

	fill_buffer(0);

	return 0;
}

static int hx8357d_lcd_read(struct st_device *dev, void *data, unsigned int size)
{
	int rt;

	if(fmem_stat != FMEM_READ) {
		set_cs(1);
		set_cs(0);
		fmem_stat = FMEM_READ;
		ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, HX8357_RAMRD, 0);
		ioctl_device(gpio_dev, IOCMD_VIO_READ_DATA8, 0, 0);	// Dummy read
	} else {
		set_cs(0);
	}

	rt = read_device(gpio_dev, data, size);

	set_cs(1);

	return rt;
}

static int hx8357d_lcd_write(struct st_device *dev, const void *data, unsigned int size)
{
	int rt;

	if(fmem_stat != FMEM_WRITE) {
		set_cs(0);
		fmem_stat = FMEM_WRITE;
		ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, HX8357_RAMWR, 0);
	} else {
		set_cs(0);
	}

	rt = write_device(gpio_dev, data, size);

	set_cs(1);

	return rt;
}

static int hx8357d_lcd_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	DKFPRINTF(0x01, "com = %08X, arg = %08X\n", com, arg);

	switch(IOCTL(com)) {
	case IOCMD_VIDEO_LOCK_DEVICE:
		ioctl_device(gpio_dev, IOCMD_VIO_LOCK_BUS, 0, 0);
		break;

	case IOCMD_VIDEO_UNLOCK_DEVICE:
		ioctl_device(gpio_dev, IOCMD_VIO_UNLOCK_BUS, 0, 0);
		break;

	case IOCMD_VIDEO_SETRECT:
		{
			struct st_rect *rect = (struct st_rect *)param;
			DKPRINTF(0x01, "RECT %3d %3d %3d %3d\n", rect->left, rect->top, rect->right, rect->bottom);
			set_window(rect->left, rect->top, rect->right, rect->bottom);
			fmem_stat = FMEM_NOP;
			return 0;
		}
		break;

	case IOCMD_VIDEO_RESETRECT:
		{
			set_window(0, 0, lcd_width - 1, lcd_height - 1);
			fmem_stat = FMEM_NOP;
			return 0;
		}
		break;

	case IOCMD_VIDEO_CLEAR:
		fill_buffer(0);
		return 0;
		break;

	case IOCMD_VIDEO_FILL:
		fill_buffer(arg);
		return 0;
		break;

	case IOCMD_VIDEO_BCKLIGHT:
		break;

	case IOCMD_VIDEO_WRITE_WORD:
		{
			DKPRINTF(0x01, "WORD %04X\n", arg);
			if(fmem_stat != FMEM_WRITE) {
				set_cs(0);
				fmem_stat = FMEM_WRITE;
				ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, HX8357_RAMWR, 0);
			}
			ioctl_device(gpio_dev, IOCMD_VIO_WRITE_DATA16, arg & 0xffff, 0);
		}
		break;

	case IOCMD_VIDEO_NOLOCK_WRITE_WORD:
		{
			DKPRINTF(0x01, "WORD %04X\n", arg);
			if(fmem_stat != FMEM_WRITE) {
				set_cs(0);
				fmem_stat = FMEM_WRITE;
				ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, HX8357_RAMWR, 0);
			}
			ioctl_device(gpio_dev, IOCMD_VIO_NOLOCK_WRITE_DATA16, arg & 0xffff, 0);
		}
		break;

	case IOCMD_VIDEO_SET_FORECOLOR:
		{
			fore_color = arg;
			ioctl_device(gpio_dev, IOCMD_VIO_SET_WRITEDATA0, arg, 0);
		}
		break;

	case IOCMD_VIDEO_SET_BACKCOLOR:
		{
			back_color = arg;
			ioctl_device(gpio_dev, IOCMD_VIO_SET_WRITEDATA1, arg, 0);
		}
		break;

	case IOCMD_VIDEO_REPEAT_DATA:
		{
			if(fmem_stat != FMEM_WRITE) {
				set_cs(0);
				fmem_stat = FMEM_WRITE;
				ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, HX8357_RAMWR, 0);
			}
			ioctl_device(gpio_dev, IOCMD_VIO_REPEAT_DATA, arg, 0);
			set_cs(1);
		}
		break;

	case IOCMD_VIDEO_DRAW_PIXEL:
		{
			int x = ((arg >>  0) & 0xffff);
			int y = ((arg >> 16) & 0xffff);
			int dx;
			int dy;

			DKPRINTF(0x01, "P %3d %3d\n", x, y);

			dx  = x;
			dy  = y;

			set_window(dx, dy, lcd_width - 1, lcd_height - 1);
			set_cs(0);
			fmem_stat = FMEM_NOP;
			write_reg16(HX8357_RAMWR, fore_color);
			set_cs(1);
		}
		break;

	case IOCMD_VIDEO_DRAW_BITS:
		{
			if(fmem_stat != FMEM_WRITE) {
				set_cs(0);
				fmem_stat = FMEM_WRITE;
				ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, HX8357_RAMWR, 0);
			}
			ioctl_device(gpio_dev, IOCMD_VIO_REPEAT_BITS | (com & 0xffff), arg, param);
		}
		break;

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		return -1;
	}

	return 0;
}

static int hx8357d_lcd_suspend(struct st_device *dev)
{
	return 0;
}

static int hx8357d_lcd_resume(struct st_device *dev)
{
	return 0;
}

static struct st_video_info lcd_info = {
	.type		= VIDEOTYPE_CMDDRAW,
	.width		= LCD_WIDTH,
	.height		= LCD_HEIGHT,
	.color_depth	= VCOLORDEP_16,
};

const struct st_device hx8357d_lcd_device = {
	.name		= DEF_DEV_NAME_VIDEO,
	.explan		= "HX8357D(MAR3520) LCD",
	.info		= (void *)&lcd_info,
	.register_dev	= hx8357d_lcd_register,
	.read		= hx8357d_lcd_read,
	.write		= hx8357d_lcd_write,
	.ioctl		= hx8357d_lcd_ioctl,
	.suspend	= hx8357d_lcd_suspend,
	.resume		= hx8357d_lcd_resume,
};
