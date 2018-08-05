/** @file
    @brief	STM32F411 Nucreo 簡易キードライバ

    @date	2015.09.22
    @author	Takashi SHUDO

    @note

    PC4	KEY3
    PC5	KEY2
    PB2	KEY1
    PB12	KEY0
*/

#include "interrupt.h"
#include "device.h"
#include "sysevent.h"
#include "key.h"
#include "tkprintf.h"
#include "timer.h"

#include "stm32f4xx_hal.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


//#define USE_GPIO_INTERRUPT

#define SCANINTTIME	20	//!< スキャン間隔(20ms)

#define KEYLINEBITS	4
static unsigned short key_stat; // キー状態

static const unsigned char key_map[KEYLINEBITS] = {
	KEY_GB_UP,
	KEY_GB_ENTER,
	KEY_GB_DOWN,
	KEY_GB_ESC,
};

static void MX_GPIO_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStruct;

	/* GPIO Ports Clock Enable */
	__GPIOC_CLK_ENABLE();
	__GPIOB_CLK_ENABLE();

	/*Configure GPIO pins : PC4 PC5 */
	GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5;
#ifdef USE_GPIO_INTERRUPT
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
#else
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
#endif
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : PB2 PB12 */
	GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_12;
#ifdef USE_GPIO_INTERRUPT
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
#else
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
#endif
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

//  HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
#ifdef USE_GPIO_INTERRUPT
	HAL_NVIC_EnableIRQ(EXTI2_IRQn);
	HAL_NVIC_EnableIRQ(EXTI4_IRQn);
	HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
#endif
}

static void init_gpio(void)
{
	MX_GPIO_Init();
}

static unsigned short scan_gpio(void)
{
	unsigned short rt;

	/*
	  ポート状態が0ならばOn、1ならばOff
	  関数は1のビットがOn
	*/
	rt = ~((HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2 )<<3) |
	       (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)<<2) |
	       (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_4 )<<1) |
	       (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_5 )<<0));

	DKPRINTF(0x02, "PORT=%04X\n", (int)rt);

	return rt;
}

static void scan_key(void *sp)
{
	int i;
	unsigned short mask;
	unsigned short bit;
	struct st_sysevent event;
	int flg_evt = 0;

	mask = 0x0001;
	bit = scan_gpio();

	for(i=0; i<KEYLINEBITS; i++) {
		if(bit & mask) {
			// 押されている
			if((key_stat & mask) == 0) {
				// 前は押されていなかった
				event.what = EVT_KEYDOWN;
				event.arg = key_map[i];
				DKPRINTF(0x01, "EVT_KEYDOWN(%d)\n",
					  event.arg);
				key_stat |= mask;
				push_event_interrupt(sp, &event);
				flg_evt = 1;
			}
		} else {
			// いまは押されていなくて
			if(key_stat & mask) {
				// 前は押されていた
				event.what = EVT_KEYUP;
				event.arg = key_map[i];
				DKPRINTF(0x01, "EVT_KEYUP(%d)\n",
					  event.arg);
				key_stat &= ~mask;
				push_event_interrupt(sp, &event);
				flg_evt = 1;
			}
		}
		mask <<= 1;
	}

	// push したイベント分、カウンタを進める必要がある
	if(flg_evt) {
		set_event_interrupt(sp);
	}
}

#ifndef USE_GPIO_INTERRUPT
static void key_timer(void *sp, unsigned long long stime)
{
	scan_key(sp);
}

#else
static void inthdr_exti2(unsigned long intnum, void *sp)
{
	__HAL_GPIO_EXTI_CLEAR_IT(EXTI_IMR_MR2);
	DKPRINTF(0x02, "INT %d(%d)\n", (int)intnum, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2));

	scan_key(sp);
}

static void inthdr_exti4(unsigned long intnum, void *sp)
{
	__HAL_GPIO_EXTI_CLEAR_IT(EXTI_IMR_MR4);
	DKPRINTF(0x02, "INT %d(%d)\n", (int)intnum, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4));

	scan_key(sp);
}

static void inthdr_exti9_5(unsigned long intnum, void *sp)
{
	__HAL_GPIO_EXTI_CLEAR_IT(EXTI_IMR_MR5);
	DKPRINTF(0x02, "INT %d(%d)\n", (int)intnum, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5));

	scan_key(sp);
}

static void inthdr_exti15_10(unsigned long intnum, void *sp)
{
	__HAL_GPIO_EXTI_CLEAR_IT(EXTI_IMR_MR12);
	DKPRINTF(0x02, "INT %d(%d)\n", (int)intnum, HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12));

	scan_key(sp);
}
#endif

static int gpio_button_register(struct st_device *dev, char *param)
{
	key_stat = 0;

	init_gpio();

#ifdef USE_GPIO_INTERRUPT
	register_interrupt(IRQ2VECT(EXTI2_IRQn), inthdr_exti2);
	register_interrupt(IRQ2VECT(EXTI4_IRQn), inthdr_exti4);
	register_interrupt(IRQ2VECT(EXTI9_5_IRQn), inthdr_exti9_5);
	register_interrupt(IRQ2VECT(EXTI15_10_IRQn), inthdr_exti15_10);
#else
	register_timer_func(key_timer, SCANINTTIME);
#endif

	return 0;
}

const struct st_device gpio_button_device = {
	.name		= DEF_DEV_NAME_INPUT,
	.explan		= "STM32F4 GPIO Button",
	.register_dev	= gpio_button_register,
};
