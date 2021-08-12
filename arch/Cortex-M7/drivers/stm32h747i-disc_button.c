/** @file
    @brief	STM32H747I Discovery 簡易キードライバ

    @date	2020.01.28
    @author	Takashi SHUDO

    @note

    PA0	KEY0
*/

#include "interrupt.h"
#include "device.h"
#include "device/input_ioctl.h"
#include "sysevent.h"
#include "key.h"
#include "tkprintf.h"
#include "timer.h"

#include "stm32h7xx_hal.h"
#include "stm32h747i_discovery.h"

//#define DEBUGKBITS 0x08
#include "dkprintf.h"


#define SCANINTTIME	20	//!< スキャン間隔(20ms)
#define KEYLINEBITS	6

static unsigned short key_stat; // キー状態

static const unsigned char key_map[KEYLINEBITS] = {
	KEY_GB_ESC,
	KEY_GB_ENTER,
	KEY_GB_DOWN,
	KEY_GB_LEFT,
	KEY_GB_RIGHT,
	KEY_GB_UP
};

static void KEY_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	BUTTON_WAKEUP_GPIO_CLK_ENABLE();

	GPIO_InitStructure.Pin		= BUTTON_WAKEUP_PIN;
	GPIO_InitStructure.Mode		= GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull		= GPIO_NOPULL;
	GPIO_InitStructure.Speed	= GPIO_SPEED_HIGH;
	GPIO_InitStructure.Alternate	= 0;
	HAL_GPIO_Init(BUTTON_WAKEUP_GPIO_PORT, &GPIO_InitStructure);
}

static void init_gpio(void)
{
	KEY_Configuration();

	BSP_JOY_Init(JOY1, JOY_MODE_GPIO, JOY_ALL);
}

static unsigned short scan_gpio(void)
{
	unsigned short rt;
	int jrt;

	/*
	  ポート状態が1ならばOn、0ならばOff
	  関数は1のビットがOn
	*/
	rt = (HAL_GPIO_ReadPin(BUTTON_WAKEUP_GPIO_PORT, BUTTON_WAKEUP_PIN)<<0);

//	jrt = BSP_JOY_GetState(JOY1, JOY_ALL);
	{
		int i;
		for(i=0; i<5; i++) {
			int bit = (0x04<<i);
			jrt = HAL_GPIO_ReadPin(GPIOK, bit);
			DKPRINTF(0x08, "bit = %02x, %d\n", bit, jrt);
			if(jrt == 0) {
				rt |= (1 << (i+1));
			}
		}
	}
	DKPRINTF(0x08, "rt = %02x\n", rt);

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

static int gpio_button_register(struct st_device *dev, char *param)
{
	key_stat = 0;

	init_gpio();

	register_timer_func(key_timer, SCANINTTIME);

	return 0;
}

static int input_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	int rt = 0;

	switch(com) {
	case IOCMD_INPUT_SCAN_LINE:
		rt = scan_gpio();
		break;

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		break;
	}

	return rt;
}


const struct st_device gpio_button_device = {
	.name		= DEF_DEV_NAME_INPUT,
	.explan		= "STM32H747I-Disc GPIO Button",
	.register_dev	= gpio_button_register,
	.ioctl		= input_ioctl,
};
