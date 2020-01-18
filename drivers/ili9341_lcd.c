/** @file
    @brief	ILI9341 LCD GPIO 8bit接続 ドライバ
		Kuman K60((Kuman 2.8inch TFT LCD Shield))

     http://www.kumantech.com/kuman-uno-r3-28-inch-tft-touch-screen-with-sd-card-socket-for-arduino-nano-mega2560-320x240-28quot-lcd-k60_p0278.html

     @date	2017.10.15
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


//#define LCD_WIDTH	240	// 幅
//#define LCD_HEIGHT	320	// 高さ
#define LCD_WIDTH	320	// 幅
#define LCD_HEIGHT	240	// 高さ
//!!!#define LCD_ROTATION	1	// 0:240x320, 1:320x240
#define LCD_ROTATION	0	// 0:240x320, 1:320x240

static int rotation = LCD_ROTATION;

static struct st_device *gpio_dev;
static unsigned short lcd_width  = LCD_WIDTH;
static unsigned short lcd_height = LCD_HEIGHT;
static unsigned short fore_color = 0;
static unsigned short back_color = 0;
#define FMEM_NOP	0
#define FMEM_WRITE	1
#define FMEM_READ	2
static int fmem_stat = FMEM_NOP;

#define ILI9341_SOFTRESET	0x01
#define ILI9341_DISPLAYOFF	0x28
#define ILI9341_POWERCONTROL1	0xc0
#define ILI9341_POWERCONTROL2	0xc1
#define ILI9341_VCOMCONTROL1	0xc5
#define ILI9341_VCOMCONTROL2	0xc7
#define ILI9341_MEMCONTROL	0x36
#define ILI9341_PIXELFORMAT	0x3a
#define ILI9341_FRAMECONTROL	0xb1
#define ILI9341_ENTRYMODE	0xb7
#define ILI9341_SLEEPOUT	0x11
#define ILI9341_DISPLAYON	0x29
#define ILI9341_COLADDRSET	0x2a
#define ILI9341_PAGEADDRSET	0x2b
#define ILI9341_MEMORYWRITE	0x2c
#define ILI9341_MEMORYREAD	0x2e
#define ILI9341_READID4		0xd3

#define ILI9341_MADCTL_MY	0x80
#define ILI9341_MADCTL_MX	0x40
#define ILI9341_MADCTL_MV	0x20
#define ILI9341_MADCTL_RGB	0x00
#define ILI9341_MADCTL_BGR	0x08

static void set_cs(unsigned int cs)
{
	ioctl_device(gpio_dev, IOCMD_VIO_SET_CS, cs, 0);
}

static void reset_ili9341(void)
{
	ioctl_device(gpio_dev, IOCMD_VIO_SET_RESET, 0, 0);
	wait_time(100);
	ioctl_device(gpio_dev, IOCMD_VIO_SET_RESET, 1, 0);
}

static inline void write_reg8(unsigned char addr, unsigned char data)
{
	ioctl_device(gpio_dev, IOCMD_VIO_WRITE_REG8,
		     (((int)addr) << 16) | (int)(data & 0xff), 0);
}

static inline void write_reg16(unsigned char addr, unsigned short data)
{
	ioctl_device(gpio_dev, IOCMD_VIO_WRITE_REG16,
		     (((int)addr) << 16) | (int)(data & 0xffff), 0);
}

static inline void write_reg32(unsigned char addr, unsigned int data)
{
	ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, addr, 0);
	ioctl_device(gpio_dev, IOCMD_VIO_WRITE_DATA32, data, 0);
}

static inline unsigned int read_reg32(unsigned char addr)
{
	return ioctl_device(gpio_dev, IOCMD_VIO_READ_REG32, addr, 0);
}

static void set_window(int x1, int y1, int x2, int y2)
{
	int x, y, t;

	DKPRINTF(0x01, "WI %3d %3d %3d %3d\n", x1, y1, x2, y2);

	switch(rotation) {
	default:
		x  = x1;
		y  = y1;
		break;
	case 1:
		t  = y1;
		y1 = x1;
		x1 = LCD_WIDTH  - 1 - y2;
		y2 = x2;
		x2 = LCD_WIDTH  - 1 - t;
		x  = x2;
		y  = y1;
		break;
	case 2:
		t  = x1;
		x1 = LCD_WIDTH  - 1 - x2;
		x2 = LCD_WIDTH  - 1 - t;
		t  = y1;
		y1 = LCD_HEIGHT - 1 - y2;
		y2 = LCD_HEIGHT - 1 - t;
		x  = x2;
		y  = y2;
		break;
	case 3:
		t  = x1;
		x1 = y1;
		y1 = LCD_HEIGHT - 1 - x2;
		x2 = y2;
		y2 = LCD_HEIGHT - 1 - t;
		x  = x1;
		y  = y2;
		break;
	}

	DKPRINTF(0x01, "WO %3d %3d %3d %3d\n", x1, y1, x2, y2);

	set_cs(0);

	write_reg32(ILI9341_COLADDRSET, ((x1<<16) | x2));
	write_reg32(ILI9341_PAGEADDRSET, ((y1<<16) | y2));
	(void)x;
	(void)y;

	set_cs(1);
}

static void init_ili9341(void)
{
	reset_ili9341();

	set_cs(0);

	write_reg8(ILI9341_SOFTRESET, 0);
	wait_time(50);
	write_reg8(ILI9341_DISPLAYOFF, 0);

	unsigned int id4 = read_reg32(ILI9341_READID4);
	tkprintf("ID4 : %08X\n", id4);

	write_reg8(ILI9341_POWERCONTROL1, 0x23);
	write_reg8(ILI9341_POWERCONTROL2, 0x10);
	write_reg16(ILI9341_VCOMCONTROL1, 0x2B2B);
	write_reg8(ILI9341_VCOMCONTROL2, 0xC0);
	write_reg8(ILI9341_MEMCONTROL, ILI9341_MADCTL_MV | ILI9341_MADCTL_BGR);
	write_reg8(ILI9341_PIXELFORMAT, 0x55);
	write_reg16(ILI9341_FRAMECONTROL, 0x001B);
    
	write_reg8(ILI9341_ENTRYMODE, 0x07);
	/*write_reg32(ILI9341_DISPLAYFUNC, 0x0A822700);*/

	write_reg8(ILI9341_SLEEPOUT, 0);
	wait_time(150);
	write_reg8(ILI9341_DISPLAYON, 0);
	//wait_time(500);

	set_cs(1);

	set_window(0, 0, lcd_width - 1, lcd_height - 1);
}

static void fill_buffer(unsigned short color)
{
	set_window(0, 0, lcd_width - 1, lcd_height - 1);

	ioctl_device(gpio_dev, IOCMD_VIO_SET_WRITEDATA0, (unsigned int)color, 0);

	set_cs(0);
	fmem_stat = FMEM_WRITE;
	ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, ILI9341_MEMORYWRITE, 0);
	ioctl_device(gpio_dev, IOCMD_VIO_REPEAT_DATA, (unsigned int)LCD_WIDTH * LCD_HEIGHT, 0);
	set_cs(1);
}

static int ili9341_lcd_register(struct st_device *dev, char *param)
{
	gpio_dev = open_device(param);
	if(gpio_dev == 0) {
		SYSERR_PRINT("Cannot open device \"%s\"\n", param);
		return -1;
	}

	init_ili9341();

	fill_buffer(0);

	return 0;
}

static int ili9341_lcd_read(struct st_device *dev, void *data, unsigned int size)
{
	int rt;

	if(fmem_stat != FMEM_READ) {
		set_cs(1);
		set_cs(0);
		fmem_stat = FMEM_READ;
		ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, ILI9341_MEMORYREAD, 0);
		ioctl_device(gpio_dev, IOCMD_VIO_READ_DATA8, 0, 0);	// Dummy read
	} else {
		set_cs(0);
	}

	rt = read_device(gpio_dev, data, size);

	set_cs(1);

	return rt;
}

static int ili9341_lcd_write(struct st_device *dev, const void *data, unsigned int size)
{
	int rt;

	if(fmem_stat != FMEM_WRITE) {
		set_cs(0);
		fmem_stat = FMEM_WRITE;
		ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, ILI9341_MEMORYWRITE, 0);
	} else {
		set_cs(0);
	}

	rt = write_device(gpio_dev, data, size);

	set_cs(1);

	return rt;
}

static int ili9341_lcd_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	DKPRINTF(0x01, "LCD %08X %08X\n", com, arg);

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
			DKPRINTF(0x01, "WRITE_WORD %04X\n", arg);
			if(fmem_stat != FMEM_WRITE) {
				set_cs(0);
				fmem_stat = FMEM_WRITE;
				ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, ILI9341_MEMORYWRITE, 0);
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
				ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, ILI9341_MEMORYWRITE, 0);
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
				ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, ILI9341_MEMORYWRITE, 0);
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

			switch(rotation) {
			default:
				dx  = x;
				dy  = y;
				break;
			case 1:
				dx  = LCD_WIDTH - 1 - y;
				dy  = x;
				break;
			}

			set_window(dx, dy, lcd_width - 1, lcd_height - 1);
			set_cs(0);
			fmem_stat = FMEM_NOP;
			write_reg16(ILI9341_MEMORYWRITE, fore_color);
			set_cs(1);
		}
		break;

	case IOCMD_VIDEO_DRAW_BITS:
		{
			if(fmem_stat != FMEM_WRITE) {
				set_cs(0);
				fmem_stat = FMEM_WRITE;
				ioctl_device(gpio_dev, IOCMD_VIO_WRITE_COMMAND, ILI9341_MEMORYWRITE, 0);
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

static struct st_video_info lcd_info = {
	.type		= VIDEOTYPE_CMDDRAW,
#if (LCD_ROTATION == 1)
	.width		= LCD_HEIGHT,
	.height		= LCD_WIDTH,
#else
	.width		= LCD_WIDTH,
	.height		= LCD_HEIGHT,
#endif
	.color_depth	= VCOLORDEP_16,
};

const struct st_device ili9341_lcd_device = {
	.name		= DEF_DEV_NAME_VIDEO,
	.explan		= "ILI9341(K60) LCD",
	.info		= (void *)&lcd_info,
	.register_dev	= ili9341_lcd_register,
	.read		= ili9341_lcd_read,
	.write		= ili9341_lcd_write,
	.ioctl		= ili9341_lcd_ioctl,
};
