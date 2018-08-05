/** @file
    @brief	STM32F769I-Dicsovery 簡易キードライバ

    @date	2017.02.04
    @author	Takashi SHUDO

    @note

    PJ3	KEY3
    PF7	KEY2
    PC8	KEY1
    PJ0	KEY0
*/

#include "interrupt.h"
#include "device.h"
#include "sysevent.h"
#include "key.h"
#include "tkprintf.h"
#include "timer.h"

#include "stm32f7xx_hal.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


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
	__GPIOJ_CLK_ENABLE();
	__GPIOF_CLK_ENABLE();
	__GPIOC_CLK_ENABLE();

	/*Configure GPIO pins : PJ3 PJ0 */
	GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

	/*Configure GPIO pins : PF7 */
	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	/*Configure GPIO pins : PC8 */
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
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
	rt = ~((HAL_GPIO_ReadPin(GPIOJ, GPIO_PIN_3) << 3) |
	       (HAL_GPIO_ReadPin(GPIOF, GPIO_PIN_7) << 2) |
	       (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8) << 1) |
	       (HAL_GPIO_ReadPin(GPIOJ, GPIO_PIN_0) << 0));

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

static void key_timer(void *sp, unsigned long long stime)
{
	scan_key(sp);
}

static int gpio_keyboard_register(struct st_device *dev, char *param)
{
	key_stat = 0;

	init_gpio();

	register_timer_func(key_timer, SCANINTTIME);

	return 0;
}

const struct st_device gpio_keyboard_device = {
	.name		= "keyboard",
	.explan		= "STM32F7 GPIO Keyboard x4",
	.register_dev	= gpio_keyboard_register,
};
