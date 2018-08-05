/** @file
    @brief	STM32F7 ADC(GPIO PA6)

    @date	2017.02.04
    @author	Takashi SHUDO
*/

#include "device.h"
#include "tkprintf.h"
#include "system.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_adc.h"

ADC_HandleTypeDef hadc1;

/* ADC1 init function */
static void MX_ADC1_Init(void)
{
	ADC_ChannelConfTypeDef sConfig;

	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if(HAL_ADC_Init(&hadc1) != HAL_OK) {
		SYSERR_PRINT("ADC1 Initialize Error.\n");
	}

	sConfig.Channel = ADC_CHANNEL_6;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
		SYSERR_PRINT("ADC1 Config Error.\n");
	}
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	if(hadc->Instance==ADC1) {
		__ADC1_CLK_ENABLE();
  
		GPIO_InitStruct.Pin = GPIO_PIN_6;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
}

static int adc_register(struct st_device *dev, char *param)
{
	MX_ADC1_Init();

	return 0;
}

static int adc_open(struct st_device *dev)
{
	return 0;
}

static int adc_close(struct st_device *dev)
{
	return 0;
}

static long adc_read(struct st_device *dev, unsigned char *data, long size)
{
	unsigned long val;
	long rtn = 0;

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	val = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	if(size > 0) {
		*data = ((val >> 8) & 0xff);
		data ++;
		rtn ++;
	}

	if(size > 1) {
		*data = (val & 0xff);
		rtn ++;
	}

	return rtn;
}

const device adc_device = {
	.name		= "adc",
	.explan		= "STM32F7 ADC(GPIO PC0)",
	.register_dev	= adc_register,
	.open		= adc_open,
	.close		= adc_close,
	.read		= adc_read,
};
