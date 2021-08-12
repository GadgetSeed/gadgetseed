/** @file
 * @brief	STM32H747I Discovery Initialize
 *
 * @date	2019.01.19
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

#include "stm32h7xx_hal.h"

#include "stm32h7xx_ll_fmc.h"
#include "stm32h7xx_hal_mdma.h"
#include "stm32h7xx_hal_qspi.h"
#include "stm32h7xx_hal_sdram.h"
#include "stm32h747i_discovery_sdram.h"
#include "stm32h747i_discovery_qspi.h"

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 400000000 (Cortex-M7 CPU Clock,upto 480MHz)
  *            HCLK(Hz)                       = 200000000 (Cortex-M4 CPU, Bus matrix Clocks,upto 200MHz)
  *            AHB Prescaler                  = 2 (AHB/AXI Bus Matrix Clock upto 200MHz)
  *            D1 APB3 Prescaler              = 2 (APB3 Clock upto 100MHz)
  *            D2 APB1 Prescaler              = 2 (APB1 Clock upto 100MHz)
  *            D2 APB2 Prescaler              = 2 (APB2 Clock upto 100MHz)
  *            D3 APB4 Prescaler              = 2 (APB4 Clock upto 100MHz)
  *            HSE Frequency(Hz)              = 25000000
  *	           VCO Frequency                  = 800000000 (160*5)
  *            PLL_M                          = 5
  *            PLL_N                          = 160
  *            PLL_P                          = 2
  *            PLL_Q                          = 4
  *            PLL_R                          = 2
  *            VDD(V)                         = 3.3
  *            Flash Latency(WS)              = 4
  * @param  None
  * @retval None
  */
#define PLL1_M		5
#define PLL1_N		160
#define PLL1_P		2
#define PLL1_Q		4
#define PLL1_R		2
#define PLL1_FRACN	0

/* Constants -----------------------------------------------------------------*/

#if 0
/* Variables -----------------------------------------------------------------*/
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
/**************************************************************************/
/*! 
    @brief	Copy Time-Critical codes into ITCM-RAM.
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
/* _stitcm,_sitcm,_eitcm are MUST be decrare in likerscript file */
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
static void SystemClock_Config(void)
{
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;
	HAL_StatusTypeDef ret = HAL_OK;

	/*!< Supply configuration update enable */
	HAL_PWREx_ConfigSupply(PWR_DIRECT_SMPS_SUPPLY);

	/* The voltage scaling allows optimizing the power consumption when the device is 
	 clocked below the maximum system frequency, to update the voltage scaling value 
	 regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

	/* Enable D2 domain SRAM3 Clock (0x30040000 AXI)*/
	__HAL_RCC_D2SRAM3_CLK_ENABLE();

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
	RCC_OscInitStruct.CSIState = RCC_CSI_OFF;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

	RCC_OscInitStruct.PLL.PLLM = PLL1_M;
	RCC_OscInitStruct.PLL.PLLN = PLL1_N;
	RCC_OscInitStruct.PLL.PLLFRACN = PLL1_FRACN;
	RCC_OscInitStruct.PLL.PLLP = PLL1_P;
	RCC_OscInitStruct.PLL.PLLQ = PLL1_Q;
	RCC_OscInitStruct.PLL.PLLR = PLL1_R;

	RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
	RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
	ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	if(ret != HAL_OK) {
		for(;;);
	}

	/* Select PLL as system clock source and configure  bus clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_D1PCLK1 | RCC_CLOCKTYPE_PCLK1 | \
								   RCC_CLOCKTYPE_PCLK2  | RCC_CLOCKTYPE_D3PCLK1);

	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;  
	RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; 
	RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; 
	RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; 
	ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4);
	if(ret != HAL_OK) {
		for(;;);
	}

	/*
	Note : The activation of the I/O Compensation Cell is recommended with communication  interfaces
		  (GPIO, SPI, FMC, QSPI ...)  when  operating at  high frequencies(please refer to product datasheet)       
		  The I/O Compensation Cell activation  procedure requires :
		- The activation of the CSI clock
		- The activation of the SYSCFG clock
		- Enabling the I/O Compensation Cell : setting bit[0] of register SYSCFG_CCCSR

		  To do this please uncomment the following code 
	*/
	__HAL_RCC_CSI_ENABLE();
	__HAL_RCC_SYSCFG_CLK_ENABLE();
	HAL_EnableCompensationCell();
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
    @brief	Configure the MPU attributes as Write Back for SRAM.
		To avoid F*CKING erattum,CANNOT set Write-Through mode.
	@param	None.
    @retval	None.
*/
/**************************************************************************/
static void MPU_Config(void)
{
	MPU_Region_InitTypeDef MPU_InitStruct;

	/* Disable the MPU */
	HAL_MPU_Disable();

	/* Configure the MPU attributes as Write-Back/No-Write-Allocate for AXI-SRAM D1 */
	MPU_InitStruct.Enable		= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress	= 0x24000000;
	MPU_InitStruct.Size		= MPU_REGION_SIZE_512KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable	= MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable	= MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable	= MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number		= MPU_REGION_NUMBER0;
	MPU_InitStruct.TypeExtField	= MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec	= MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes as Write-Back/No-Write-Allocate for AHB-SRAM1 D2 */
	MPU_InitStruct.Enable		= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress	= 0x30000000;
	MPU_InitStruct.Size		= MPU_REGION_SIZE_128KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable	= MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable	= MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable	= MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number		= MPU_REGION_NUMBER1;
	MPU_InitStruct.TypeExtField	= MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec	= MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes as Write-Back/No-Write-Allocate for AHB-SRAM2 D2 */
	MPU_InitStruct.Enable		= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress	= 0x30020000;
	MPU_InitStruct.Size		= MPU_REGION_SIZE_128KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable	= MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable	= MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable	= MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number		= MPU_REGION_NUMBER2;
	MPU_InitStruct.TypeExtField	= MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec	= MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes as Device not cacheable
	   for ETH DMA descriptors */
	MPU_InitStruct.Enable		= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress	= 0x30040000;
	MPU_InitStruct.Size		= MPU_REGION_SIZE_16KB;
	MPU_InitStruct.AccessPermission	= MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable	= MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable	= MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable	= MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number		= MPU_REGION_NUMBER3;
	MPU_InitStruct.TypeExtField	= MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable	= 0x00;
	MPU_InitStruct.DisableExec	= MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes as Normal Non Cacheable
	   for LwIP RAM heap which contains the Tx buffers */
	MPU_InitStruct.Enable		= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress	= 0x30044000;
	MPU_InitStruct.Size		= MPU_REGION_SIZE_16KB;
	MPU_InitStruct.AccessPermission	= MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable	= MPU_ACCESS_NOT_BUFFERABLE;
	MPU_InitStruct.IsCacheable	= MPU_ACCESS_NOT_CACHEABLE;
	MPU_InitStruct.IsShareable	= MPU_ACCESS_SHAREABLE;
	MPU_InitStruct.Number		= MPU_REGION_NUMBER4;
	MPU_InitStruct.TypeExtField	= MPU_TEX_LEVEL1;
	MPU_InitStruct.SubRegionDisable	= 0x00;
	MPU_InitStruct.DisableExec	= MPU_INSTRUCTION_ACCESS_ENABLE;

	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Configure the MPU attributes as Write-Back/No-Write-Allocate for AHB-SRAM4 D3 */
	MPU_InitStruct.Enable		= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress	= 0x38000000;
	MPU_InitStruct.Size		= MPU_REGION_SIZE_64KB;
	MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable	= MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable	= MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable	= MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number		= MPU_REGION_NUMBER5;
	MPU_InitStruct.TypeExtField	= MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable	= 0x00;
	MPU_InitStruct.DisableExec	= MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* External-Memories cache setting */
	/* Configure the MPU attributes as Write-Back/No-Write-Allocate for External SDRAM */
	MPU_InitStruct.Enable		= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress	= 0xD0000000;
	MPU_InitStruct.Size		= MPU_REGION_SIZE_32MB;			/* STM32H747I-Disco have 32Mbyte SDRAM */
	MPU_InitStruct.AccessPermission	= MPU_REGION_FULL_ACCESS;
	MPU_InitStruct.IsBufferable	= MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable	= MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable	= MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number		= MPU_REGION_NUMBER6;
	MPU_InitStruct.TypeExtField	= MPU_TEX_LEVEL0;			/* MUST be No-WriteAllocation to avoid collision LTDC-Hostage */ 
	MPU_InitStruct.SubRegionDisable = 0x00;
	MPU_InitStruct.DisableExec	= MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* External-Memories cache setting */
	/* Configure the MPU attributes as Write-Back for QSPI-DirectRemapping(ReadOnly) */
	MPU_InitStruct.Enable		= MPU_REGION_ENABLE;
	MPU_InitStruct.BaseAddress	= 0x90000000;
	MPU_InitStruct.Size		= MPU_REGION_SIZE_128MB;		/* STM32H747I-Disco have 64*2=128MByte QSPI-ROM */
	MPU_InitStruct.AccessPermission	= MPU_REGION_PRIV_RO_URO;
	MPU_InitStruct.IsBufferable	= MPU_ACCESS_BUFFERABLE;
	MPU_InitStruct.IsCacheable	= MPU_ACCESS_CACHEABLE;
	MPU_InitStruct.IsShareable	= MPU_ACCESS_NOT_SHAREABLE;
	MPU_InitStruct.Number		= MPU_REGION_NUMBER7;
	MPU_InitStruct.TypeExtField	= MPU_TEX_LEVEL0;
	MPU_InitStruct.SubRegionDisable	= 0x00;
	MPU_InitStruct.DisableExec	= MPU_INSTRUCTION_ACCESS_ENABLE;
	HAL_MPU_ConfigRegion(&MPU_InitStruct);

	/* Enable the MPU */
	HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}


/**
 * @brief	CPUを初期化する
 */
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

extern const struct st_device qspi_device;
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

extern void init_qspi(void);

void init_system2(void)
{
	BSP_SDRAM_Init(0);

#ifdef GSC_DEV_ENABLE_QSPI
	init_qspi();
#else
	// QSPI-ROM
	if(0)
	{
		uint8_t RetVal = 0;
		BSP_QSPI_Init_t init;
		init.InterfaceMode = MT25TL01G_QPI_MODE;
		init.TransferRate  = MT25TL01G_DTR_TRANSFER ;
		init.DualFlashMode = MT25TL01G_DUALFLASH_ENABLE;

		if((RetVal = BSP_QSPI_Init(0,&init)) != BSP_ERROR_NONE) {
			SYSERR_PRINT("Failed to initialize the QSPI !! (Error %d)\n", RetVal);
		} else {
			if((RetVal = BSP_QSPI_EnableMemoryMappedMode(0)) != BSP_ERROR_NONE) {
				SYSERR_PRINT("Failed to configure the QSPI !! (Error %d)\n", RetVal);
			}
		}
	}
#endif
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
//	register_device(&uart6_device, 0);

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

#ifdef GSC_DEV_ENABLE_QSPI
	register_device((struct st_device *)&qspi_device, 0);
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
	init_random(get_systime_sec());
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

#include "stm32h7xx_hal_iwdg.h"

static IWDG_HandleTypeDef IwdgHandle;

void reset_system(void)
{
	IwdgHandle.Instance = IWDG1;
	IwdgHandle.Init.Prescaler = IWDG_PRESCALER_32;
	IwdgHandle.Init.Reload = 0;

	if(HAL_IWDG_Init(&IwdgHandle) != HAL_OK) {
		SYSERR_PRINT("HAL_IWDG_Init() error\n");
	}
}
