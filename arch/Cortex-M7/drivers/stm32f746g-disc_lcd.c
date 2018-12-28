/** @file
    @brief	STM32F746G-Discovery LCD ドライバ

    @date	2017.02.12
    @author	Takashi SHUDO
*/

#include "device.h"
#include "device/video_ioctl.h"
#include "timer.h"
#include "tkprintf.h"
#include "graphics.h"

#include "stm32746g_discovery_lcd.h"

#define LCD_WIDTH	480	// 幅
#define LCD_HEIGHT	272	// 高さ

#define __ATTR_FRAMEBUFFER  __attribute__ ((section(".extram"))) __attribute__ ((aligned (4)))
unsigned char lcd_f_buffer_main[LCD_WIDTH * LCD_HEIGHT * 2] __ATTR_FRAMEBUFFER;
unsigned char lcd_f_buffer_sub [LCD_WIDTH * LCD_HEIGHT * 2] __ATTR_FRAMEBUFFER;

static void init_framebuf(void)
{
	unsigned char *p;

	for(p=&lcd_f_buffer_main[0];
	    p<&lcd_f_buffer_main[LCD_WIDTH * LCD_HEIGHT * 2];
	    p++) {
		*p = 0; // Black
	}

	for(p=&lcd_f_buffer_sub[0];
	    p<&lcd_f_buffer_sub[LCD_WIDTH * LCD_HEIGHT * 2];
	    p++) {
		*p = 0; // Black
	}
}

static int lcd_register(struct st_device *dev, char *param)
{
	((struct st_video_info *)(dev->info))->frame_buf_ptr[0] = lcd_f_buffer_main;
	((struct st_video_info *)(dev->info))->frame_buf_ptr[1] = lcd_f_buffer_sub;
	((struct st_video_info *)(dev->info))->mem_size = sizeof(lcd_f_buffer_main);

	BSP_LCD_Init();
	BSP_LCD_LayerRgb565Init(0, (uint32_t)lcd_f_buffer_main);
	init_framebuf();
	BSP_LCD_DisplayOn();

	return 0;
}

static int lcd_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	switch(com) {
	case IOCMD_VIDEO_SETDISPFRAME:
		switch(arg) {
		case 0:
			BSP_LCD_SetLayerAddress(0, (uint32_t)lcd_f_buffer_main);
			break;

		case 1:
			BSP_LCD_SetLayerAddress(0, (uint32_t)lcd_f_buffer_sub);
			break;

		default:
			return -1;
			break;
		}
		break;

	case IOCMD_VIDEO_CLEAR:
		{
			init_framebuf();
		}
		break;

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		return -1;
	}

	return 0;
}

static struct st_video_info lcd_info = {
	.type		= VIDEOTYPE_FRAMEBUF,
	.width		= LCD_WIDTH,
	.height		= LCD_HEIGHT,
	.color_depth	= VCOLORDEP_16,
	.frame_num	= 2,
	.frame_buf_ptr	= { 0, 0 },
	.mem_size	= 0,
};

const struct st_device lcd_device = {
	.name		= DEF_DEV_NAME_VIDEO,
	.explan		= "STM32F746G-Discovery LCD",
	.info		= (void *)&lcd_info,
	.register_dev	= lcd_register,
	.ioctl		= lcd_ioctl,
};
