/** @file
    @brief	STM32F746G Discovery 簡易キードライバ

    @date	2019.11.16
    @author	Takashi SHUDO

    @note

    PI11	KEY0
*/

#include "interrupt.h"
#include "device.h"
#include "device/input_ioctl.h"
#include "sysevent.h"
#include "key.h"
#include "tkprintf.h"
#include "timer.h"

#include "stm32f7xx_hal.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


//#define USE_GPIO_INTERRUPT

#define SCANINTTIME	20	//!< スキャン間隔(20ms)
#define KEYLINEBITS	1

/* KEY Input Definition */
#define GPIO_KEY_USER1			(GPIOI)
#define RCC_AHBPeriph_GPIO_KEY_USER1	(RCC_AHB1ENR_GPIOIEN)
#define KEY_USER1			(GPIO_PIN_11)

static unsigned short key_stat; // キー状態

static const unsigned char key_map[KEYLINEBITS] = {
	KEY_GB_ESC
};

static void KEY_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO_LED clock */
	RCC->AHB1ENR |= (RCC_AHBPeriph_GPIO_KEY_USER1);

	/* Configure GPIO for LEDs as Output push-pull */
	GPIO_InitStructure.Pin 			= KEY_USER1;
	GPIO_InitStructure.Mode 		= GPIO_MODE_INPUT;
	GPIO_InitStructure.Pull 		= GPIO_NOPULL;
	GPIO_InitStructure.Speed 		= GPIO_SPEED_HIGH;
	GPIO_InitStructure.Alternate 	= 0;
	HAL_GPIO_Init(GPIO_KEY_USER1, &GPIO_InitStructure);
}

static void init_gpio(void)
{
	KEY_Configuration();
}

static unsigned short scan_gpio(void)
{
	unsigned short rt;

	/*
	  ポート状態が1ならばOn、0ならばOff
	  関数は1のビットがOn
	*/
	rt = (HAL_GPIO_ReadPin(GPIO_KEY_USER1, KEY_USER1)<<0);

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
	.explan		= "STM32F746G-Disc GPIO Button",
	.register_dev	= gpio_button_register,
	.ioctl		= input_ioctl,
};
