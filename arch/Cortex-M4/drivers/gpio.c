/** @file
    @brief	STM32F411 Nucreo 汎用GPIOドライバ

    @date	2015.10.17
    @author	Takashi SHUDO

    @note

    PH1	D0
*/

#include "device.h"
#include "device/gpio_ioctl.h"
#include "tkprintf.h"

#include "stm32f4xx_hal.h"

static void MX_GPIO_Init(GPIO_TypeDef *gpio, unsigned int pin, unsigned int mode)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__GPIOH_CLK_ENABLE();

	GPIO_InitStruct.Pin = pin;
	GPIO_InitStruct.Mode = mode;
//	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(gpio, &GPIO_InitStruct);
}

static int gpio_register(struct st_device *dev, char *param)
{
	MX_GPIO_Init(GPIOH, GPIO_PIN_1, GPIO_MODE_INPUT);	// デフォルトは入力

	return 0;
}

static int gpio_getc(struct st_device *dev, unsigned char *rd)
{
	int rtn = 0;

	return rtn;
}

static int gpio_putc(struct st_device *dev, unsigned char data)
{
	if((data & 0x01) != 0) {
		HAL_GPIO_WritePin(GPIOH, GPIO_PIN_1, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(GPIOH, GPIO_PIN_1, GPIO_PIN_RESET);
	}

	return 1;
}

static int gpio_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	switch(com) {
	case IOCMD_GPIO_DIRECTION:
		switch(arg) {
		case IOARG_GPIO_INPUT:
			MX_GPIO_Init(GPIOH, GPIO_PIN_1, GPIO_MODE_INPUT);
			break;

		case IOARG_GPIO_OUTPUT:
			MX_GPIO_Init(GPIOH, GPIO_PIN_1, GPIO_MODE_OUTPUT_PP);
			break;

		default:
			SYSERR_PRINT("Unknow command %08lX arg %08lX\n", com, arg);
			break;
		}
		break;

	case IOCMD_GPIO_SET_BITS:
		HAL_GPIO_WritePin(GPIOH, GPIO_PIN_1, GPIO_PIN_SET);
		break;

	case IOCMD_GPIO_CLEAR_BITS:
		HAL_GPIO_WritePin(GPIOH, GPIO_PIN_1, GPIO_PIN_RESET);
		break;

	default:
		SYSERR_PRINT("Unknow command %08lX arg %08lX\n", com, arg);
		return -1;
	}

	return 0;
}

const struct st_device gpio_device = {
	.name		= DEF_DEV_NAME_GPIO,
	.explan		= "STM32F4 GPIO",
	.register_dev	= gpio_register,
	.getc		= gpio_getc,
	.putc		= gpio_putc,
	.ioctl		= gpio_ioctl,
};
