/** @file
    @brief	圧電ブザードライバ

    @date	2015.08.23
    @author	Takashi SHUDO

    @info

    接続

    圧電ブザー	STM32F4(CPU PIN)
    -----------	----------------
    DIN		PB1(27pin)

    ブザー音のパルスはTIM3で作成

    使用方法

    ioctl(0:Off/1:On, 周波数(Hz))
*/

#include "sysconfig.h"
#include "device.h"
#include "device/buzzer_ioctl.h"
#include "timer.h"
#include "tkprintf.h"

#include "stm32f4xx_hal.h"

TIM_HandleTypeDef htim3;

/* TIM3 init function */
void MX_TIM3_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;

	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 0;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 0;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	HAL_TIM_Base_Init(&htim3);

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);

	HAL_TIM_OC_Init(&htim3);

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

	sConfigOC.OCMode = TIM_OCMODE_TIMING;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4);
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if(htim_base->Instance==TIM3) {
		__TIM3_CLK_ENABLE();

		GPIO_InitStruct.Pin = GPIO_PIN_1;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	}
}


void start_tim3(void)
{
	MX_TIM3_Init();
}

void on_buzzer(long cycle)
{
	TIM_OC_InitTypeDef sConfigOC;
	htim3.Init.Period = cycle;
	HAL_TIM_Base_Init(&htim3);

	sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
	sConfigOC.Pulse = cycle/2;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_4);

	HAL_TIM_OC_Start(&htim3, TIM_CHANNEL_4);
}

void off_buzzer(void)
{
	HAL_TIM_OC_Stop(&htim3, TIM_CHANNEL_4);
}

static int buzzer_register(struct st_device *dev, char *param)
{
	start_tim3();

	return 0;
}

static int buzzer_unregister(struct st_device *dev)
{
	off_buzzer();

	return 0;
}

static int buzzer_open(struct st_device *dev)
{
	return 0;
}

static int buzzer_close(struct st_device *dev)
{
	off_buzzer();

	return 0;
}

static int buzzer_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	switch(com) {
	case IOCMD_BUZZER_OFF:	// Stop
		off_buzzer();
		break;

	case IOCMD_BUZZER_ON:	// Play
		{
			long cycle = (GSC_CPU_CLOCK_HZ/4) / arg;
			on_buzzer(cycle);
		}
		break;

	default:
		off_buzzer();
		SYSERR_PRINT("Unknown ioctl(%08lX)\n", com);
		return -1;
	}

	return 0;
}

static int buzzer_suspend(struct st_device *dev)
{
	off_buzzer();

	return 0;
}

static int buzzer_resume(struct st_device *dev)
{
	start_tim3();

	return 0;
}

const struct st_device buzzer_device = {
	.name		= DEF_DEV_NAME_BUZZER,
	.explan		= "STM32F4 TIM3 buzzer",
	.register_dev	= buzzer_register,
	.unregister_dev	= buzzer_unregister,
	.open		= buzzer_open,
	.close		= buzzer_close,
	.ioctl		= buzzer_ioctl,
	.suspend	= buzzer_suspend,
	.resume		= buzzer_resume,
};
