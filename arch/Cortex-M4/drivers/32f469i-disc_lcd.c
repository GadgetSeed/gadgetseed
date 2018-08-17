/** @file
    @brief	STM32F469I Discovery LCD ドライバ

    @date	2018.08.15
    @author	Takashi SHUDO
*/

#include "device.h"
#include "device/video_ioctl.h"
#include "timer.h"
#include "tkprintf.h"
#include "graphics.h"

#include "stm32469i_discovery_lcd.h"

#define LCD_WIDTH	800	// 幅
#define LCD_HEIGHT	480	// 高さ

#define __ATTR_FRAMEBUFFER  __attribute__ ((section(".extram"))) __attribute__ ((aligned (4)))
unsigned char lcd_f_buffer_main[LCD_WIDTH * LCD_HEIGHT * 4] __ATTR_FRAMEBUFFER;
unsigned char lcd_f_buffer_sub [LCD_WIDTH * LCD_HEIGHT * 4] __ATTR_FRAMEBUFFER;

LTDC_HandleTypeDef	hltdc_discovery;
DSI_HandleTypeDef	hdsi_discovery;

static void init_framebuf(void)
{
	unsigned char *p;

	for(p=&lcd_f_buffer_main[0];
	    p<&lcd_f_buffer_main[LCD_WIDTH * LCD_HEIGHT * 4];
	    p++) {
		*p = 0; // Black
	}

	for(p=&lcd_f_buffer_sub[0];
	    p<&lcd_f_buffer_sub[LCD_WIDTH * LCD_HEIGHT * 4];
	    p++) {
		*p = 0; // Black
	}
}

extern LTDC_HandleTypeDef  hltdc_eval;
extern DSI_HandleTypeDef hdsi_eval;
DSI_VidCfgTypeDef hdsivideo_handle;

static uint8_t LCD_Init(void)
{
  DSI_PLLInitTypeDef dsiPllInit;
  DSI_PHY_TimerTypeDef  PhyTimings;
  static RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct;
  uint32_t LcdClock  = 19200; /*!< LcdClk = 19200 kHz */

  uint32_t laneByteClk_kHz = 0;
  uint32_t                   VSA; /*!< Vertical start active time in units of lines */
  uint32_t                   VBP; /*!< Vertical Back Porch time in units of lines */
  uint32_t                   VFP; /*!< Vertical Front Porch time in units of lines */
  uint32_t                   VACT; /*!< Vertical Active time in units of lines = imageSize Y in pixels to display */
  uint32_t                   HSA; /*!< Horizontal start active time in units of lcdClk */
  uint32_t                   HBP; /*!< Horizontal Back Porch time in units of lcdClk */
  uint32_t                   HFP; /*!< Horizontal Front Porch time in units of lcdClk */
  uint32_t                   HACT; /*!< Horizontal Active time in units of lcdClk = imageSize X in pixels to display */
  
  
  /* Toggle Hardware Reset of the DSI LCD using
  * its XRES signal (active low) */
  BSP_LCD_Reset();
  
  /* Call first MSP Initialize only in case of first initialization
  * This will set IP blocks LTDC, DSI and DMA2D
  * - out of reset
  * - clocked
  * - NVIC IRQ related to IP blocks enabled
  */
  BSP_LCD_MspInit();
  
/*************************DSI Initialization***********************************/  
  
  /* Base address of DSI Host/Wrapper registers to be set before calling De-Init */
  hdsi_eval.Instance = DSI;
  
  HAL_DSI_DeInit(&(hdsi_eval));
  
#if !defined(USE_STM32469I_DISCO_REVA)
  dsiPllInit.PLLNDIV  = 125;
  dsiPllInit.PLLIDF   = DSI_PLL_IN_DIV2;
  dsiPllInit.PLLODF   = DSI_PLL_OUT_DIV1;
#else  
  dsiPllInit.PLLNDIV  = 100;
  dsiPllInit.PLLIDF   = DSI_PLL_IN_DIV5;
  dsiPllInit.PLLODF   = DSI_PLL_OUT_DIV1;
#endif
  laneByteClk_kHz = 62500; /* 500 MHz / 8 = 62.5 MHz = 62500 kHz */
  
  /* Set number of Lanes */
  hdsi_eval.Init.NumberOfLanes = DSI_TWO_DATA_LANES;
  
  /* TXEscapeCkdiv = f(LaneByteClk)/15.62 = 4 */
  hdsi_eval.Init.TXEscapeCkdiv = laneByteClk_kHz/15620; 
  
  HAL_DSI_Init(&(hdsi_eval), &(dsiPllInit));
  
  /* The following values are same for portrait and landscape orientations */
  VSA  = OTM8009A_480X800_VSYNC;        /* 10 */
  VBP  = OTM8009A_480X800_VBP;          /* 15 */
  VFP  = OTM8009A_480X800_VFP;          /* 16 */
  HSA  = OTM8009A_480X800_HSYNC;        /* 2 */
  HBP  = OTM8009A_480X800_HBP;          /* 20 */
  HFP  = OTM8009A_480X800_HFP;          /* 20 */ 
  HACT = OTM8009A_800X480_WIDTH;        /* 800 */
  VACT = OTM8009A_800X480_HEIGHT;       /* 480 */   

  hdsivideo_handle.VirtualChannelID = LCD_OTM8009A_ID;
  hdsivideo_handle.ColorCoding = LCD_DSI_PIXEL_DATA_FMT_RBG888;
  hdsivideo_handle.VSPolarity = DSI_VSYNC_ACTIVE_HIGH;
  hdsivideo_handle.HSPolarity = DSI_HSYNC_ACTIVE_HIGH;
  hdsivideo_handle.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;  
  hdsivideo_handle.Mode = DSI_VID_MODE_BURST; /* Mode Video burst ie : one LgP per line */
  hdsivideo_handle.NullPacketSize = 0xFFF;
  hdsivideo_handle.NumberOfChunks = 0;
  hdsivideo_handle.PacketSize                = HACT; /* Value depending on display orientation choice portrait/landscape */ 
  hdsivideo_handle.HorizontalSyncActive      = (HSA * laneByteClk_kHz) / LcdClock;
  hdsivideo_handle.HorizontalBackPorch       = (HBP * laneByteClk_kHz) / LcdClock;
  hdsivideo_handle.HorizontalLine            = ((HACT + HSA + HBP + HFP) * laneByteClk_kHz) / LcdClock; /* Value depending on display orientation choice portrait/landscape */
  hdsivideo_handle.VerticalSyncActive        = VSA;
  hdsivideo_handle.VerticalBackPorch         = VBP;
  hdsivideo_handle.VerticalFrontPorch        = VFP;
  hdsivideo_handle.VerticalActive            = VACT; /* Value depending on display orientation choice portrait/landscape */
  
  /* Enable or disable sending LP command while streaming is active in video mode */
  hdsivideo_handle.LPCommandEnable = DSI_LP_COMMAND_ENABLE; /* Enable sending commands in mode LP (Low Power) */
  
  /* Largest packet size possible to transmit in LP mode in VSA, VBP, VFP regions */
  /* Only useful when sending LP packets is allowed while streaming is active in video mode */
  hdsivideo_handle.LPLargestPacketSize = 16;
  
  /* Largest packet size possible to transmit in LP mode in HFP region during VACT period */
  /* Only useful when sending LP packets is allowed while streaming is active in video mode */
  hdsivideo_handle.LPVACTLargestPacketSize = 0;
  
  
  /* Specify for each region of the video frame, if the transmission of command in LP mode is allowed in this region */
  /* while streaming is active in video mode                                                                         */
  hdsivideo_handle.LPHorizontalFrontPorchEnable = DSI_LP_HFP_ENABLE;   /* Allow sending LP commands during HFP period */
  hdsivideo_handle.LPHorizontalBackPorchEnable  = DSI_LP_HBP_ENABLE;   /* Allow sending LP commands during HBP period */
  hdsivideo_handle.LPVerticalActiveEnable = DSI_LP_VACT_ENABLE;  /* Allow sending LP commands during VACT period */
  hdsivideo_handle.LPVerticalFrontPorchEnable = DSI_LP_VFP_ENABLE;   /* Allow sending LP commands during VFP period */
  hdsivideo_handle.LPVerticalBackPorchEnable = DSI_LP_VBP_ENABLE;   /* Allow sending LP commands during VBP period */
  hdsivideo_handle.LPVerticalSyncActiveEnable = DSI_LP_VSYNC_ENABLE; /* Allow sending LP commands during VSync = VSA period */
  
  /* Configure DSI Video mode timings with settings set above */
  HAL_DSI_ConfigVideoMode(&(hdsi_eval), &(hdsivideo_handle));

  /* Configure DSI PHY HS2LP and LP2HS timings */
  PhyTimings.ClockLaneHS2LPTime = 35;
  PhyTimings.ClockLaneLP2HSTime = 35;
  PhyTimings.DataLaneHS2LPTime = 35;
  PhyTimings.DataLaneLP2HSTime = 35;
  PhyTimings.DataLaneMaxReadTime = 0;
  PhyTimings.StopWaitTime = 10;
  HAL_DSI_ConfigPhyTimer(&hdsi_eval, &PhyTimings);
  

/*************************End DSI Initialization*******************************/ 
  
  
/************************LTDC Initialization***********************************/  
  
  /* Timing Configuration */    
  hltdc_eval.Init.HorizontalSync = (HSA - 1);
  hltdc_eval.Init.AccumulatedHBP = (HSA + HBP - 1);
  hltdc_eval.Init.AccumulatedActiveW = (HACT + HSA + HBP - 1);
  hltdc_eval.Init.TotalWidth = (HACT + HSA + HBP + HFP - 1);
  
  /* Initialize the LCD pixel width and pixel height */
  hltdc_eval.LayerCfg->ImageWidth  = HACT;
  hltdc_eval.LayerCfg->ImageHeight = VACT;   
  
  
  /* LCD clock configuration */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 192 Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 192 MHz / 5 = 38.4 MHz */
  /* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_2 = 38.4 MHz / 2 = 19.2 MHz */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIR = 5;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct); 
  
  /* Background value */
  hltdc_eval.Init.Backcolor.Blue = 0;
  hltdc_eval.Init.Backcolor.Green = 0;
  hltdc_eval.Init.Backcolor.Red = 0;
  hltdc_eval.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc_eval.Instance = LTDC;
  
  /* Get LTDC Configuration from DSI Configuration */
  HAL_LTDC_StructInitFromVideoConfig(&(hltdc_eval), &(hdsivideo_handle));
  
  /* Initialize the LTDC */  
  HAL_LTDC_Init(&hltdc_eval);

  /* Enable the DSI host and wrapper after the LTDC initialization
     To avoid any synchronization issue, the DSI shall be started after enabling the LTDC */

  HAL_DSI_Start(&(hdsi_eval));
  
#if !defined(DATA_IN_ExtSDRAM)
  /* Initialize the SDRAM */
  BSP_SDRAM_Init();
#endif /* DATA_IN_ExtSDRAM */
  
  /* Initialize the font */
  BSP_LCD_SetFont(&LCD_DEFAULT_FONT);
  
/************************End LTDC Initialization*******************************/
  
  
/***********************OTM8009A Initialization********************************/  
  
  /* Initialize the OTM8009A LCD Display IC Driver (KoD LCD IC Driver)
  *  depending on configuration set in 'hdsivideo_handle'.
  */
  OTM8009A_Init(OTM8009A_FORMAT_RGB888, OTM8009A_ORIENTATION_LANDSCAPE);
  
/***********************End OTM8009A Initialization****************************/ 
  
  return LCD_OK; 
}

static void Display_DSIIF_Init(void)
{
	LCD_Init();
  	BSP_LCD_DisplayOff();
  
	BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER_BACKGROUND, (uint32_t)lcd_f_buffer_sub); 
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER_BACKGROUND); 
	BSP_LCD_Clear(LCD_COLOR_BLACK);

	BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER_FOREGROUND, (uint32_t)lcd_f_buffer_main);   
	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER_FOREGROUND);
	BSP_LCD_Clear(LCD_COLOR_BLACK); 

	BSP_LCD_SetColorKeying(1, 0);

	BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER_BACKGROUND);
	BSP_LCD_DisplayOn();
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
			BSP_LCD_SetLayerAddress(1, (uint32_t)lcd_f_buffer_main);
			break;

		case 1:
			BSP_LCD_SetLayerAddress(1, (uint32_t)lcd_f_buffer_sub);
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
	.color_depth	= VCOLORDEP_32,
	.frame_num	= 2,
	.frame_buf_ptr	= { 0, 0 },
	.mem_size	= 0,
};

const struct st_device lcd_device = {
	.name		= DEF_DEV_NAME_VIDEO,
	.explan		= "STM32F469I-Discovery LCD",
	.info		= (void *)&lcd_info,
	.register_dev	= lcd_register,
	.ioctl		= lcd_ioctl,
};
