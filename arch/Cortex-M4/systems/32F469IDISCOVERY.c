/** @file
    @brief	32F469IDISCOVERY Initialize

    @date	2018.08.15
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "tkprintf.h"
#include "device/i2c_ioctl.h"
#include "device/spi_ioctl.h"
#include "device/sd_ioctl.h"
#include "device/rtc_ioctl.h"
#include "device/video_ioctl.h"
#include "device/vio_ioctl.h"

#ifdef GSC_COMP_ENABLE_GRAPHICS
#include "graphics.h"
#endif

#include "stm32f4xx_hal.h"

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 180000000
  *            HCLK(Hz)                       = 180000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 8000000
  *            PLL_M                          = 8
  *            PLL_N                          = 360
  *            PLL_P                          = 2
  *            PLL_Q                          = 7
  *            PLL_R                          = 6
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 5
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	HAL_StatusTypeDef ret = HAL_OK;

	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* The voltage scaling allows optimizing the power consumption when the device is
	   clocked below the maximum system frequency, to update the voltage scaling value
	   regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
#if defined(USE_STM32469I_DISCO_REVA)
	RCC_OscInitStruct.PLL.PLLM = 25;
#else
	RCC_OscInitStruct.PLL.PLLM = 8;
#endif /* USE_STM32469I_DISCO_REVA */
	RCC_OscInitStruct.PLL.PLLN = 360;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 7;
	RCC_OscInitStruct.PLL.PLLR = 6;

	ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if(ret != HAL_OK) {
		// Error
	}

	/* Activate the OverDrive to reach the 180 MHz Frequency */
	ret = HAL_PWREx_EnableOverDrive();
	if(ret != HAL_OK) {
		// Error
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
	if(ret != HAL_OK) {
		// Error
	}
}

/**
   @brief	CPUを初期化する
*/
void init_cpu(void)
{
	SystemInit();

	HAL_Init();

	SystemClock_Config();
}


#include "system.h"
#include "sysconfig.h"
#include "interrupt.h"
#include "device.h"
#include "random.h"
#include "datetime.h"
#include "console.h"
#include "tprintf.h"

#ifdef GSC_COMP_ENABLE_FATFS
#include "storage.h"
#include "file.h"
#endif

extern const struct st_device usart3_device;
extern const struct st_device usart3_low_device;
extern const struct st_device usart6_device;

extern const struct st_device null_device;

extern const struct st_device rtc_device;

extern const struct st_device led_device;

extern const struct st_device lcd_device;
extern const struct st_device framebuf_device;
extern const struct st_device ts_device;

extern const struct st_device grconsole_device;

extern const struct st_device sd_device;

extern const struct st_device irq_device;
extern const struct st_device gpio_device;

#ifdef GSC_COMP_ENABLE_FATFS
static const char * const storade_devices[] = {
	DEF_DEV_NAME_SD,
	0
};
#endif

/**
   @brief	ドライバ初期化以前に行う初期化処理
*/
void init_system(int *argc, char ***argv)
{
	/*
	  ハンドラモードのどのレベルからでもスレッドモードに移行可能に
	*/
	SCB->CCR |= SCB_CCR_NONBASETHRDENA_Msk;
	SCB->CCR |= SCB_CCR_STKALIGN_Msk;
}

void init_system2(void)
{
}

/**
   @brief	基本ドライバ初期化後に登録するユーザドライバ登録処理
*/
void init_system_drivers(void)
{
	// シリアルコンソール初期化
	register_device(&usart3_device, 0);
	register_console_out_dev(&usart3_device);
	register_console_in_dev(&usart3_device);
	register_error_out_dev(&usart3_low_device);

	// USART6
	register_device(&usart6_device, 0);

#ifdef GSC_DEV_ENABLE_LED
	// LED
	register_device(&led_device, 0);
#endif

#ifdef GSC_DEV_ENABLE_BUTTON
	// ボタン
	register_device(&gpio_button_device, 0);
#endif

#ifdef GSC_LIB_ENABLE_RANDOM
	// 乱数初期化
	init_genrand(get_systime_sec());
#endif
}

/**
   @brief	タスク起動後の初期化処理

   @note	タスクAPIを必要とする初期化を行う
*/
void init_system_process(void)
{
	/*
	  グラフィックスドライバ
	*/
#ifdef GSC_COMP_ENABLE_GRAPHICS
	register_device(&lcd_device, 0);
	register_device(&framebuf_device, 0);
	init_graphics("fb");	// グラフィックス初期化
#  ifdef GSC_DEV_ENABLE_TOUCHSENSOR
	register_device(&ts_device, 0);
#  endif
#  ifdef GSC_COMP_ENABLE_FONTS
#    ifdef GSC_DEV_ENABLE_GRCONSOLE
	register_device(&grconsole_device, 0);
#    endif
#  endif
#endif

	/*
	  ファイルシステム
	*/
#ifdef GSC_DEV_ENABLE_STORAGE
	register_device(&sd_device, 0);
#endif

#ifdef GSC_COMP_ENABLE_FATFS
	init_storage();
	init_file();
	register_storage_device(storade_devices);
#endif

#ifdef GSC_DEV_ENABLE_RTC
	// RTC初期化
	register_device(&rtc_device, 0);
	init_time(DEF_DEV_NAME_RTC);
#else
	init_time(0);
#endif

#ifdef GSC_DEV_ENABLE_NULL
	// NULLデバイス初期化
	register_device(&null_device, 0);
#endif
}

#include "stm32f4xx_hal_iwdg.h"

static IWDG_HandleTypeDef IwdgHandle;

void reset_system(void)
{
	IwdgHandle.Instance = IWDG;
	IwdgHandle.Init.Prescaler = IWDG_PRESCALER_32;
	IwdgHandle.Init.Reload = 0;

	if(HAL_IWDG_Init(&IwdgHandle) != HAL_OK) {
		SYSERR_PRINT("HAL_IWDG_Init() error\n");
	}
}
