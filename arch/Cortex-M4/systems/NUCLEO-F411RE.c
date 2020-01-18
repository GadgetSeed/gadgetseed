/** @file
    @brief	STM32F411RE Nucleo Initialize

    @date	2015.08.02
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

/*
  System Clock Configuration
*/
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;

	__PWR_CLK_ENABLE();

	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE;
	RCC_OscInitStruct.LSEState = RCC_LSE_ON;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = 16;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 16;
	RCC_OscInitStruct.PLL.PLLN = 192;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	HAL_RCC_OscConfig(&RCC_OscInitStruct);

	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
			|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
	HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);
}

/**
   @brief	CPUを初期化する
*/
void init_cpu(void)
{
	SystemInit();

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

extern const struct st_device usart2_device;
extern const struct st_device usart2_low_device;
extern const struct st_device usart6_device;

extern const struct st_device null_device;

extern const struct st_device rtc_device;

extern const struct st_device i2c_device;
extern const struct st_device adt7410_device;
extern const struct st_device bme280_device;

extern const struct st_device spi1_device;
extern const struct st_device spi2_device;
extern const struct st_device stm_mmc_device;
extern const struct st_device mmc_device;

extern const struct st_device led_device;

extern const struct st_device lcd_gpio_device;
extern const struct st_device ts_analog_device;
extern const struct st_device ili9325_lcd_device;
extern const struct st_device ili9341_lcd_device;
extern const struct st_device hx8357d_lcd_device;

extern const struct st_device buzzer_device;
extern const struct st_device sound_device;

extern const struct st_device adc_device;

extern const struct st_device grconsole_device;

extern const struct st_device gpio_button_device;

extern const struct st_device eeprom_device;

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
//	SCB->CCR |= SCB_CCR_USERSETMPEND_Msk;
//	SCB->CCR |= SCB_CCR_BFHFNMIGN_Msk;
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
	register_device(&usart2_device, 0);
	register_console_out_dev(&usart2_device);
	register_console_in_dev(&usart2_device);
	register_error_out_dev(&usart2_low_device);

	// USART6
	register_device(&usart6_device, 0);

#ifdef GSC_DEV_ENABLE_RTC
	// RTC初期化
	register_device(&rtc_device, 0);
	init_time(DEF_DEV_NAME_RTC);
#else
	init_time(0);
#endif

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
	init_random(get_systime_sec());
#endif
}

/**
   @brief	タスク起動後の初期化処理

   @note	タスクAPIを必要とする初期化を行う
*/
void init_system_process(void)
{
	/*
	  I2Cドライバ
	*/
#ifdef GSC_DEV_ENABLE_I2C
	register_device(&i2c_device, 0);
#  ifdef GSC_DEV_ENABLE_TEMPSENSOR_ADT7410	// $gsc ADT7410温度センサデバイスを有効にする
	register_device(&adt7410_device, DEF_DEV_NAME_I2C);
#  endif
#  ifdef GSC_DEV_ENABLE_TEMPSENSOR_BME280	// $gsc BME280温度、湿度、気圧センサデバイスを有効にする
	register_device(&bme280_device, DEF_DEV_NAME_I2C);
#  endif
#  ifdef GSC_DEV_ENABLE_I2CEEPROM
	register_device(&eeprom_device, DEF_DEV_NAME_I2C);
#  endif
#endif

	/*
	  SPIドライバ
	*/
#ifdef GSC_DEV_ENABLE_STORAGE
	register_device(&spi1_device, 0);
	register_device(&stm_mmc_device, DEF_DEV_NAME_SPI);
	register_device(&mmc_device, "mmc_spi");
#else
#  ifdef GSC_DEV_ENABLE_SPI	// $gsc SPIマスタコントローラデバイスを有効にする
	register_device(&spi1_device, 0);
#  endif
#endif

#ifdef GSC_DEV_ENABLE_SPI2	// $gsc SPI(2)マスタコントローラデバイスを有効にする
	register_device(&spi2_device, 0);
#endif

	/*
	  グラフィックスドライバ
	*/
#ifdef GSC_COMP_ENABLE_GRAPHICS
	register_device(&lcd_gpio_device, 0);
#  ifdef GSC_DEV_ENABLE_LCD_ILI9325	// $gsc ILI9325 LCDデバイスを有効にする
	register_device(&ili9325_lcd_device, DEF_DEV_NAME_VIDEOIO);
#  endif
#  ifdef GSC_DEV_ENABLE_LCD_ILI9341	// $gsc ILI9341 LCDデバイスを有効にする
	register_device(&ili9341_lcd_device, DEF_DEV_NAME_VIDEOIO);
#  endif
#  ifdef GSC_DEV_ENABLE_LCD_HX8357D	// $gsc HX8357D LCDデバイスを有効にする
	register_device(&hx8357d_lcd_device, DEF_DEV_NAME_VIDEOIO);
#  endif
	init_graphics(DEF_DEV_NAME_VIDEO);	// グラフィックス初期化
#  ifdef GSC_DEV_ENABLE_TOUCHSENSOR
	register_device(&ts_analog_device, 0);
#  endif
#  ifdef GSC_DEV_ENABLE_GRCONSOLE
	register_device(&grconsole_device, 0);
#  endif
#endif

	/*
	  ファイルシステム
	*/
#ifdef GSC_COMP_ENABLE_FATFS
	init_storage();
	init_file();
	register_storage_device(storade_devices);
#endif

	/*
	  圧電ブザーサウンド
	*/
#ifdef GSC_DEV_ENABLE_BUZZER
#include "device/buzzer_ioctl.h"
	register_device(&buzzer_device, 0);
	register_device(&sound_device, DEF_DEV_NAME_BUZZER);
#endif

	/*
	  ADC
	*/
#ifdef GSC_DEV_ENABLE_ADC
	register_device(&adc_device, 0);
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
