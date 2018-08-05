/** @file
    @brief	圧電ブザードライバ

    @date	2017.02.04
    @author	Takashi SHUDO

    @info

    接続

    圧電ブザー	STM32F7(CPU PIN)
    -----------	----------------
    DIN		PH6

    ブザー音のパルスはTIM12で作成

    使用方法

    ioctl(0:Off/1:On, 周波数(Hz))
*/

#include "sysconfig.h"
#include "device.h"
#include "device/buzzer_ioctl.h"
#include "timer.h"
#include "tkprintf.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_tim.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


#define ARDUINO_PWM_D6_Pin GPIO_PIN_6
#define ARDUINO_PWM_D6_GPIO_Port GPIOH

TIM_HandleTypeDef htim12;

/* TIM12 init function */
static void MX_TIM12_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig;
	TIM_MasterConfigTypeDef sMasterConfig;
	TIM_OC_InitTypeDef sConfigOC;

	htim12.Instance = TIM12;
	htim12.Init.Prescaler = 0;
	htim12.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim12.Init.Period = 0;
	htim12.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	//htim12.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if(HAL_TIM_Base_Init(&htim12) != HAL_OK) {
		SYSERR_PRINT("TIM12 Initialize error.\n");
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if(HAL_TIM_ConfigClockSource(&htim12, &sClockSourceConfig) != HAL_OK) {
		SYSERR_PRINT("TIM12 Clock Initialize error.\n");
	}

	if(HAL_TIM_OC_Init(&htim12) != HAL_OK) {
		SYSERR_PRINT("TIM12 IC Initialize error.\n");
	}

	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if(HAL_TIMEx_MasterConfigSynchronization(&htim12, &sMasterConfig) != HAL_OK) {
		SYSERR_PRINT("TIM12 MC Initialize error.\n");
	}

	sConfigOC.OCMode = TIM_OCMODE_TIMING;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if(HAL_TIM_OC_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
		SYSERR_PRINT("TIM12 OC Config error.\n");
	}
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	if(htim_base->Instance==TIM12) {
		__HAL_RCC_TIM12_CLK_ENABLE();

		GPIO_InitStruct.Pin = ARDUINO_PWM_D6_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		GPIO_InitStruct.Alternate = GPIO_AF9_TIM12;
		HAL_GPIO_Init(ARDUINO_PWM_D6_GPIO_Port, &GPIO_InitStruct);
	}
}


void start_tim12(void)
{
	MX_TIM12_Init();
}

void on_buzzer(long cycle)
{
	TIM_OC_InitTypeDef sConfigOC;

	if(cycle > 0x3FFFF) {
		DKPRINTF(0x01, "Invalid CYCLE = %d\n", (int)cycle);
		return;
	} else if(cycle > 0x1FFFF) {
		htim12.Init.Prescaler = 2;
		htim12.Init.Period = cycle/4;
	} else if(cycle > 0xFFFF) {
		htim12.Init.Prescaler = 1;
		htim12.Init.Period = cycle/2;
	} else {
		htim12.Init.Prescaler = 0;
		htim12.Init.Period = cycle;
	}

	DKPRINTF(0x01, "Clock Prescaler = %d, Period = %d\n",
		  (int)htim12.Init.Prescaler, (int)htim12.Init.Period);

	if(HAL_TIM_Base_Init(&htim12) != HAL_OK) {
		SYSERR_PRINT("TIM12 Initialize error.\n");
	}

	sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
	sConfigOC.Pulse = htim12.Init.Period/2;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_OC_ConfigChannel(&htim12, &sConfigOC, TIM_CHANNEL_1);

	HAL_TIM_OC_Start(&htim12, TIM_CHANNEL_1);
}

void off_buzzer(void)
{
	HAL_TIM_OC_Stop(&htim12, TIM_CHANNEL_1);
}

static int buzzer_register(struct st_device *dev, char *param)
{
	start_tim12();

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

static int buzzer_ioctl(struct st_device *dev, long com, long arg)
{
	switch(com) {
	case IOCMD_BUZZER_OFF: // Stop
		off_buzzer();
		break;

	case IOCMD_BUZZER_ON:	// Play
		{
			long cycle = (CPU_CLOCK_HZ/4) / arg;
			DKPRINTF(0x01, "CYCLE = %d\n", (int)cycle);
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
	start_tim12();

	return 0;
}

const device buzzer_device = {
	.name		= DEF_DEV_NAME_BUZZER,
	.explan		= "STM32F7 TIM12 buzzer",
	.register_dev	= buzzer_register,
	.unregister_dev	= buzzer_unregister,
	.open		= buzzer_open,
	.close		= buzzer_close,
	.ioctl		= buzzer_ioctl,
	.suspend	= buzzer_suspend,
	.resume		= buzzer_resume,
};
