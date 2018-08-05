/** @file
    @brief	STM32F769I Discovery LCD ドライバ

    @date	2017.01.08
    @author	Takashi SHUDO
*/

#include "device.h"
#include "device/video_ioctl.h"
#include "timer.h"
#include "tkprintf.h"
#include "graphics.h"

#include "stm32f769i_discovery_lcd.h"

#define LCD_WIDTH	800	// 幅
#define LCD_HEIGHT	480	// 高さ

#define __ATTR_FRAMEBUFFER  __attribute__ ((section(".extram"))) __attribute__ ((aligned (4)))
unsigned char lcd_f_buffer_main[LCD_WIDTH * LCD_HEIGHT * 2] __ATTR_FRAMEBUFFER;
unsigned char lcd_f_buffer_sub [LCD_WIDTH * LCD_HEIGHT * 2] __ATTR_FRAMEBUFFER;

LTDC_HandleTypeDef	hltdc_discovery;
DSI_HandleTypeDef	hdsi_discovery;

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

static void Display_DSIIF_Init(void)
{
	LCD_LayerCfgTypeDef  Layercfg;

	BSP_LCD_Init();

	/* Layer Init */
	Layercfg.WindowX0 = 0;
	Layercfg.WindowX1 = LCD_WIDTH-1;
	Layercfg.WindowY0 = 0;
	Layercfg.WindowY1 = LCD_HEIGHT-1; 
	Layercfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
	Layercfg.FBStartAdress = (uint32_t)lcd_f_buffer_main;
	Layercfg.Alpha = 255;
	Layercfg.Alpha0 = 0;
	Layercfg.Backcolor.Blue = 0;
	Layercfg.Backcolor.Green = 0;
	Layercfg.Backcolor.Red = 0;
	Layercfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
	Layercfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
	Layercfg.ImageWidth = LCD_WIDTH;
	Layercfg.ImageHeight = LCD_HEIGHT;
	HAL_LTDC_ConfigLayer(&hltdc_discovery, &Layercfg, 0); 
}

static int lcd_register(struct st_device *dev, char *param)
{
	((struct st_video_info *)(dev->info))->frame_buf_ptr[0] = lcd_f_buffer_main;
	((struct st_video_info *)(dev->info))->frame_buf_ptr[1] = lcd_f_buffer_sub;
	((struct st_video_info *)(dev->info))->mem_size = sizeof(lcd_f_buffer_main);

	Display_DSIIF_Init();
	init_framebuf();

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
		SYSERR_PRINT("Unknow command %08lX arg %08lX\n", com, arg);
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
	.explan		= "STM32F769I-Discovery LCD",
	.info		= (void *)&lcd_info,
	.register_dev	= lcd_register,
	.ioctl		= lcd_ioctl,
};
