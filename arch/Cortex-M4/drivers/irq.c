/** @file
    @brief	STM32F411 外部割り込み(EXTI)(IRQ) ドライバ

    @date	2015.10.13
    @author	Takashi SHUDO

    @note

    EXTI0
    PA0 - PI0

    PH0	IRQ0
*/

#include "interrupt.h"
#include "device.h"
#include "device/irq_ioctl.h"
#include "tkprintf.h"

#include "stm32f4xx_hal.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


static unsigned char flg_have_int = 0;
static void (* inth_func)(unsigned int intnum, void *sp);

static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__GPIOC_CLK_ENABLE();

	/*Configure GPIO pins : PH0 */
	GPIO_InitStruct.Pin = GPIO_PIN_13;
//	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;	// 下りエッジ割り込み
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;	// 両エッジ割り込み
//	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

//	HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
//	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static void init_gpio(void)
{
	MX_GPIO_Init();
}

static void inthdr_exti15_10(unsigned int intnum, void *sp)
{
	__HAL_GPIO_EXTI_CLEAR_IT(EXTI_IMR_MR13);
	DKPRINTF(0x01, "INT %d(%d)\n", intnum, HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13));

	if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == 0) {
		flg_have_int = 1;
		if(inth_func) {
			inth_func(intnum, sp);
		}
	} else {
		flg_have_int = 0;
	}
}

static int irq_register(struct st_device *dev, char *param)
{
	inth_func = 0;

	init_gpio();

	register_interrupt(IRQ2VECT(EXTI15_10_IRQn), inthdr_exti15_10);

	return 0;
}

static int irq_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	switch(com) {
	case 0: // DEBUG
		return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

	case IOCMD_IRQ_REGISTER:
		inth_func = (void (*)(unsigned int, void *))param;
		break;

	case IOCMD_IRQ_UNREGISTER:
		inth_func = 0;
		break;

	case IOCMD_IRQ_ENABLE:
		HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
		break;

	case IOCMD_IRQ_DISABLE:
		HAL_NVIC_DisableIRQ(EXTI15_10_IRQn);
		break;

	case IOCMD_IRQ_SET_EDGE:
		// [TODO]
		SYSERR_PRINT("IOCMD_IRQ_SET_EDGE not support\n");
		break;

	case IOCMD_IRQ_GET_LEVEL:
		return HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);

	case IOCMD_IRQ_GET_INT:
		return flg_have_int;

	default:
		SYSERR_PRINT("Unknow command %08lX arg %08lX\n", com, arg);
		return -1;
	}

	return 0;
}

const struct st_device irq_device = {
	.name		= DEF_DEV_NAME_IRQ,
	.explan		= "STM32F4 EXIT0-15",
	.register_dev	= irq_register,
	.ioctl		= irq_ioctl,
};
