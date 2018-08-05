/** @file
    @brief	STM32 ADC(GPIO PC0)

    @date	2015.08.31
    @author	Takashi SHUDO
*/

#include "device.h"
#include "system.h"

#include "stm32f4xx_hal.h"

ADC_HandleTypeDef hadc1;

/* ADC1 init function */
void MX_ADC1_Init(void)
{
	ADC_ChannelConfTypeDef sConfig;

	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV2;
	hadc1.Init.Resolution = ADC_RESOLUTION12b;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = EOC_SINGLE_CONV;
	HAL_ADC_Init(&hadc1);

	sConfig.Channel = ADC_CHANNEL_10;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	HAL_ADC_ConfigChannel(&hadc1, &sConfig);
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if(hadc->Instance==ADC1) {
		__ADC1_CLK_ENABLE();
  
		GPIO_InitStruct.Pin = GPIO_PIN_0;
		GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
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

static int adc_read(struct st_device *dev, void *data, unsigned int size)
{
	unsigned int val;
	unsigned char *dp = data;
	long rtn = 0;

	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	val = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);

	if(size > 0) {
		*dp = ((val >> 8) & 0xff);
		dp ++;
		rtn ++;
	}

	if(size > 1) {
		*dp = (val & 0xff);
		rtn ++;
	}

	return rtn;
}

const struct st_device adc_device = {
	.name		= "adc",
	.explan		= "STM32F4 ADC(GPIO PC0)",
	.register_dev	= adc_register,
	.open		= adc_open,
	.close		= adc_close,
	.read		= adc_read,
};
