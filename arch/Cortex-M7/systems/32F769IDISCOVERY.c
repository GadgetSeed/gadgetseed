/** @file
 * @brief	STM32F769I Discovery Initialize
 *
 * @date	2017.01.03
 * @author	Takashi SHUDO
 */

#include "sysconfig.h"
#include "tkprintf.h"

#ifdef GSC_DEV_ENABLE_I2C
#include "device/i2c_ioctl.h"
#endif

#ifdef GSC_DEV_ENABLE_RTC
#include "device/rtc_ioctl.h"
#endif

#ifdef GSC_COMP_ENABLE_GRAPHICS
#include "graphics.h"
#include "framebuf.h"
#endif

#include "stm32f7xx_hal.h"

#include "stm32f769i_discovery_sdram.h"

/** System Clock Configuration
*/
/*
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 200000000(Due to SDRAM Support upto 200MHz)
  *            HCLK(Hz)                       = 200000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 400
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            PLLSAI_N                       = 192
  *            PLLSAI_P                       = 4
  *            PLLSAI_Q                       = 4
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 6
*/
#define PLL_M		25
#define PLL_N		400
#define PLL_P		RCC_PLLP_DIV2
#define PLL_Q		9
#define PLL_R		7
#define PLLSAI_N	384
#define PLLSAI_P	RCC_PLLSAIP_DIV8 /* 384/8=48MHz */
#define PLLSAI_Q	4

/* Copy ITCM Codes Valiables */
extern unsigned int _etdata;
extern unsigned int _stitcm;
extern unsigned int _sitcm;
extern unsigned int _eitcm;

/* Prototypes ----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
/**************************************************************************/
/*!
	Internal Functions
*/
/**************************************************************************/
#if 0
/**************************************************************************/
/*! 
    @brief	Copy Volatile ITCM-RAM.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
static void datacopy(unsigned int romstart, unsigned int start, unsigned int len)
{
	unsigned int *pulDest = (unsigned int*) start;
	unsigned int *pulSrc  = (unsigned int*) romstart;
	unsigned int loop;
	for (loop = 0; loop < len; loop = loop + 4)
		*pulDest++ = *pulSrc++;
}

static void ITCM_Datainit(void)
{
	unsigned int *LoadAddr, *ExeAddr, *EndAddr, SectionLen;

    /* Copy ITCM Codes into ITCM-RAM */
	LoadAddr = &_stitcm;
	ExeAddr  = &_sitcm;
	EndAddr  = &_eitcm;
	SectionLen = (void*)EndAddr - (void*)ExeAddr;
	datacopy((unsigned int)LoadAddr, (unsigned int)ExeAddr, SectionLen);
}
#endif

/**************************************************************************/
/*! 
    @brief	Configures Main system clocks & power.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_PeriphCLKInitTypeDef RCC_PeriphCLKInitStruct;

	HAL_StatusTypeDef ret = HAL_OK;

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.HSEState	= RCC_HSE_ON;
	RCC_OscInitStruct.LSEState	= RCC_LSE_ON;
	RCC_OscInitStruct.PLL.PLLState	= RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource	= RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = PLL_M;
	RCC_OscInitStruct.PLL.PLLN = PLL_N;
	RCC_OscInitStruct.PLL.PLLP = PLL_P;
	RCC_OscInitStruct.PLL.PLLQ = PLL_Q;
#if defined (STM32F765xx) || defined (STM32F767xx) || defined (STM32F769xx) || defined (STM32F777xx) || defined (STM32F779xx) 
	RCC_OscInitStruct.PLL.PLLR = PLL_R;
#endif

	ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if(ret != HAL_OK)
	{
		while(1) { ; }
	}

	/* Activate the OverDrive to reach the 216 MHz Frequency */
	ret = HAL_PWREx_EnableOverDrive();
	if(ret != HAL_OK)
	{
		while(1) { ; }
	}

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType		= (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource		= RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider		= RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider	= RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider	= RCC_HCLK_DIV2;

	ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6);
	if(ret != HAL_OK)
	{
		while(1) { ; }
	}

	/* Select PLLSAI output as USB-FS/SDMMC/SAI2 clock(CLK48) source */
	RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
	RCC_PeriphCLKInitStruct.Clk48ClockSelection  = RCC_CLK48SOURCE_PLLSAIP;
	RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIN = PLLSAI_N;
	RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIP = PLLSAI_P;
	RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIQ = PLLSAI_Q;
	if(HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) != HAL_OK)
	{
		while(1) { ; }
	}

	/* Change SDMMC Clock Source into PLLCLK48(48MHz) */
	RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC1;
	RCC_PeriphCLKInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
	RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIN = PLLSAI_N;
	RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIP = PLLSAI_P;
	RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIQ = PLLSAI_Q;
	if(HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) != HAL_OK)
	{
		while(1) { ; }
	}

#if defined (STM32F765xx) || defined (STM32F767xx) || defined (STM32F769xx) || defined (STM32F777xx) || defined (STM32F779xx)  
	/* Change SDMMC Clock Source into PLLCLK48(48MHz) */
	RCC_PeriphCLKInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SDMMC2;
	RCC_PeriphCLKInitStruct.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_CLK48;
	RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIN = PLLSAI_N;
	RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIP = PLLSAI_P;
	RCC_PeriphCLKInitStruct.PLLSAI.PLLSAIQ = PLLSAI_Q;
	if(HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphCLKInitStruct) != HAL_OK)
	{
		while(1) { ; }
	}
#endif

	/* Enable I/O Compensation Cell for GPIO over 50MHz! */
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	SYSCFG->CMPCR |= SYSCFG_CMPCR_CMP_PD;
}

/**************************************************************************/
/*! 
    @brief	CPU L1-Cache enable.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
static void CPU_CACHE_Enable(void)
{
	/* Invalidate I-Cache : ICIALLU register */
	SCB_InvalidateICache();

	/* Enable branch prediction */
	SCB->CCR |= (1 <<18);
	__DSB();

	/* Invalidate I-Cache : ICIALLU register */
	SCB_InvalidateICache();

	/* Enable I-Cache */
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_InvalidateDCache();
	SCB_EnableDCache();

}

/**************************************************************************/
/*! 
    @brief	Configure the MPU attributes as Write Through for SRAM1/2.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
static void MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct;

	/* Disable the MPU */
	HAL_MPU_Disable();

#if defined (STM32F765xx) || defined (STM32F767xx) || defined (STM32F769xx) || defined (STM32F777xx) || defined (STM32F779xx) 
	/* Configure the MPU attributes as Write-Through for Internal SRAM */
	MPU_InitStruct.Enable 			= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress 		= 0x20020000;
	MPU_InitStruct.Size 			= MPU_REGION_SIZE_512KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable 	= MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable 		= MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable 		= MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number 			= MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField 	= MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec 		= MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes as Write-Through for External SDRAM */
	MPU_InitStruct.Enable 			= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress 		= 0xC0000000;
	MPU_InitStruct.Size 			= MPU_REGION_SIZE_16MB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable 	= MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable 		= MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable 		= MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number 			= MPU_REGION_NUMBER1;
	MPU_InitStruct.TypeExtField 	= MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec 		= MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes as as Write-Through for QSPI */
	MPU_InitStruct.Enable 			= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress 		= 0x90000000;
	MPU_InitStruct.Size 			= MPU_REGION_SIZE_64MB;
	MPU_InitStruct.AccessPermission = MPU_REGION_PRIV_RO;
	MPU_InitStruct.IsBufferable 	= MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable 		= MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable 		= MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number 			= MPU_REGION_NUMBER3;
	MPU_InitStruct.TypeExtField 	= MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec 		= MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);	
#else

	/* Configure the MPU attributes as Write-Through for Internal SRAM1/2 */
	MPU_InitStruct.Enable 			= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress 		= 0x20010000;
	MPU_InitStruct.Size 			= MPU_REGION_SIZE_256KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable 	= MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable 		= MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable 		= MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number 			= MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField 	= MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec 		= MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes as Write-Through for External SDRAM */
	MPU_InitStruct.Enable 			= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress 		= 0xC0000000;
	MPU_InitStruct.Size 			= MPU_REGION_SIZE_8MB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable 	= MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable 		= MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable 		= MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number 			= MPU_REGION_NUMBER1;
	MPU_InitStruct.TypeExtField 	= MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec 		= MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);
#endif

	/* Enable the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
 * @brief	CPUを初期化する
 */
#define INTERVAL		1000UL
void SysTickInit(__IO uint32_t interval);

void init_cpu(void)
{
	SystemInit();

	MPU_Config();

	CPU_CACHE_Enable();

	if (HAL_Init() != HAL_OK) {
		while(1){};
	}

	/* Initialize ITCM Functions */
//	ITCM_Datainit();

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
#include "timer.h"

#ifdef GSC_COMP_ENABLE_FATFS
#include "storage.h"
#include "file.h"
#endif

extern const struct st_device logout_device;
extern const struct st_device logbuf_device;

extern const struct st_device uart1_low_device;
extern const struct st_device uart1_device;
extern const struct st_device uart6_device;

extern const struct st_device null_device;

extern const struct st_device rtc_device;
extern const struct st_device rx8025_device;

extern const struct st_device i2c1_device;
extern const struct st_device i2c4_device;
extern const struct st_device adt7410_device;

extern const struct st_device sdmmc_device;

extern const struct st_device led_device;

extern const struct st_device lcd_device;
extern const struct st_device framebuf_device;
extern const struct st_device ts_device;

extern const struct st_device buzzer_device;
extern const struct st_device sound_device;

extern const struct st_device adc_device;

extern const struct st_device grconsole_device;

extern const struct st_device gpio_button_device;
extern const struct st_device gpio_keyboard_device;

extern const struct st_device eeprom_device;

extern const struct st_device ether_device;

extern const struct st_device audio_device;

#ifdef GSC_COMP_ENABLE_FATFS
#include "device/sd_ioctl.h"

static const char * const storade_devices[] = {
	DEF_DEV_NAME_SD,
	0
};
#endif

/**
 * @brief	ドライバ初期化以前に行う初期化処理
 */
void init_system(int *argc, char ***argv)
{
	// システム初期化処理

	/*
	 * ハンドラモードのどのレベルからでもスレッドモードに移行可能に
	 */
	SCB->CCR |= SCB_CCR_NONBASETHRDENA_Msk;
	/*
	 * 
	 */
	SCB->CCR |= SCB_CCR_STKALIGN_Msk;
//	SCB->CCR |= SCB_CCR_USERSETMPEND_Msk;
//	SCB->CCR |= SCB_CCR_BFHFNMIGN_Msk;
}

void init_system2(void)
{
	BSP_SDRAM_Init();
}

/**
 * @brief	基本ドライバ初期化後に登録するユーザドライバ登録処理
 */
void init_system_drivers(void)
{
	// シリアルコンソール初期化
	register_device(&uart1_device, 0);
	register_console_out_dev(&uart1_device);
	register_console_in_dev(&uart1_device);
#ifdef GSC_KERNEL_MESSAGEOUT_LOG
	register_error_out_dev(&logout_device);
	register_log_out_dev(&logbuf_device);
#else
	register_error_out_dev(&uart1_low_device);
#endif

	// USART6
	register_device(&uart6_device, 0);

#ifdef GSC_DEV_ENABLE_LED	// $gsc LEDデバイスを有効にする
	// LED
	register_device(&led_device, 0);
#endif

#ifdef GSC_DEV_ENABLE_BUTTON	// $gsc ボタンデバイスを有効にする
	// ボタン
	register_device(&gpio_button_device, 0);
#endif

#ifdef GSC_DEV_ENABLE_KEYBOARD	// $gsc キーボードデバイスを有効にする
	// キー
	register_device(&gpio_keyboard_device, 0);
#endif

#ifdef GSC_DEV_ENABLE_NULL	// $gsc NULLデバイスを有効にする
	// NULLデバイス初期化
	register_device(&null_device, 0);
#endif
}

/**
 * @brief
 *
 * プロセス起動後の初期化処理
 *
 * @note
 *
 * プロセスAPIを必要とする初期化を行う
 */
void init_system_process(void)
{
#ifdef GSC_DEV_ENABLE_I2C
	// I2Cドライバ
	register_device(&i2c1_device, 0);
	register_device(&i2c4_device, 0);
#endif

#ifdef GSC_COMP_ENABLE_GRAPHICS
	// グラフィックスドライバ初期化
	register_device(&lcd_device, 0);
	register_device(&framebuf_device, 0);
	init_graphics("fb");	// グラフィックス初期化
#ifdef GSC_COMP_ENABLE_FONTS
#ifdef GSC_DEV_ENABLE_GRCONSOLE
	register_device(&grconsole_device, 0);
#endif
#endif
#ifdef GSC_DEV_ENABLE_TOUCHSENSOR
	register_device(&ts_device, 0);
#endif
#endif

#ifdef GSC_DEV_ENABLE_STORAGE	// $gsc ストレージ(SDカードなど)デバイスを有効にする
	register_device(&sdmmc_device, 0);
#endif

#ifdef GSC_COMP_ENABLE_FATFS
	// ファイルシステム初期化
	init_storage();
	init_file();
	register_storage_device(storade_devices);
#endif

#ifdef GSC_DEV_ENABLE_I2C	// $gsc I2Cホストインタフェースデバイスを有効にする
# ifdef GSC_DEV_ENABLE_TEMPSENSOR_ADT7410
	register_device(&adt7410_device, DEF_DEV_NAME_I2C);
# endif
# ifdef GSC_DEV_ENABLE_I2CEEPROM	// $gsc I2C EEPROMデバイスを有効にする
	register_device(&eeprom_device, DEF_DEV_NAME_I2C);
# endif
# ifdef GSC_DEV_ENABLE_RTC
	// RTC初期化
	register_device(&rx8025_device, DEF_DEV_NAME_I2C);
	init_time(DEF_DEV_NAME_RTC);
# else
	init_time(0);
# endif
#else
# ifdef GSC_DEV_ENABLE_RTC
	// RTC初期化
	register_device(&rtc_device, 0);
	init_time(DEF_DEV_NAME_RTC);
# else
	init_time(0);
# endif
#endif

	// 乱数初期化
#ifdef GSC_LIB_ENABLE_RANDOM
	init_genrand(get_systime_sec());
#endif

#ifdef GSC_DEV_ENABLE_BUZZER	// $gsc 圧電ブザーデバイスを有効にする
#include "device/buzzer_ioctl.h"
	register_device(&buzzer_device, 0);
	register_device(&sound_device, DEF_DEV_NAME_BUZZER);
#endif

#ifdef GSC_DEV_ENABLE_ADC	// $gsc A/Dコンバータデバイスを有効にする
	register_device(&adc_device, 0);
#endif

#ifdef GSC_DEV_ENABLE_ETHER
	// Etherドライバ
	register_device(&ether_device, 0);
#endif

#ifdef GSC_DEV_ENABLE_AUDIO
	// サウンド、オーディオドライバ登録
	register_device(&audio_device, 0);
#endif
}

#include "stm32f7xx_hal_iwdg.h"

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
