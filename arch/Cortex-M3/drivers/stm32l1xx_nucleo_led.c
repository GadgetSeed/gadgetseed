/** @file
    @brief	STM32L152RE-Nucleo LEDドライバ

    @date	2018.08.13
    @author	Takashi SHUDO
*/

#include "device.h"

#define RCC_AHB1ENR	(*(volatile unsigned long *)0x40023830)
#define RCC_AHB1LPENR	(*(volatile unsigned long *)0x40023850)
#define RCC_AHB1_BIT_GPIOD	(1<<3)

/*
  GPIOレジスタ
*/
typedef struct st_reg_gpio {
	unsigned long	MODER;
	unsigned long	OTYPE;
	unsigned long	OSPEEDR;
	unsigned long	PUPDR;
	unsigned long	IDR;
	unsigned long	ODR;
	unsigned long	BSRR;
	unsigned long	LCKR;
	unsigned long	AFRL;
	unsigned long	AFRH;
} st_reg_gpio;

#define GPIOA_BASE	(0x40020000 + 0x000)

#define GPIOA	((st_reg_gpio *)GPIOA_BASE)

#define GPIO_MODE_BIT_IN	0x00	// Input Mode
#define GPIO_MODE_BIT_OUT	0x01	// Output Mode
#define GPIO_MODE_BIT_AF	0x02	// Alternate functions Mode
#define GPIO_MODE_BIT_AN	0x03	// Analog Mode
#define GPIO_MODE_BIT_ALL	0x03

#define GPIO_OTYPE_BIT_PP	0x00	// push-pull
#define GPIO_OTYPE_BIT_OD	0x01	// open-drain

#define GPIO_OSPEED_BIT_ALL	0x03
#define GPIO_OSPEED_BIT_50MHz	0x02
#define GPIO_OSPEED_BIT_100MHz	0x03

#define GPIO_PUPDR_BIT_NOPULL	0x00	// No pull-up, pull-down
#define GPIO_PUPDR_BIT_PUP	0x01	// pull-up
#define GPIO_PUPDR_BIT_PDOWN	0x02	// pull-down
#define GPIO_PUPDR_BIT_ALL	0x03

#define GPIO_AFR_BIT_I2C123	0x04
#define GPIO_AFR_BIT_USART123	0x07
#define GPIO_AFR_BIT_ALL	0x0F

static void init_pin(st_reg_gpio *gpio, int pin)
{
	volatile st_reg_gpio *gpiox = gpio;

	gpiox->MODER &= ~(((unsigned long)GPIO_MODE_BIT_ALL) << (2*pin));
	gpiox->MODER |= (((unsigned long)GPIO_MODE_BIT_OUT) << (2*pin));
	gpiox->OSPEEDR &= ~(((unsigned long)GPIO_OSPEED_BIT_ALL) << (2*pin));
	gpiox->OSPEEDR |= (((unsigned long)GPIO_OSPEED_BIT_50MHz) << (2*pin));
	gpiox->OTYPE &= ~(((unsigned long)1) << pin);
	gpiox->OTYPE |= (((unsigned long)GPIO_OTYPE_BIT_PP) << pin);
	gpiox->PUPDR &= ~(((unsigned long)GPIO_PUPDR_BIT_ALL) << (2*pin));
	gpiox->PUPDR |= (((unsigned long)GPIO_PUPDR_BIT_NOPULL) << (2*pin));
//	gpiox->PUPDR |= (((unsigned long)GPIO_PUPDR_BIT_PUP) << (2*pin));

	if(pin < 8) {
		gpiox->AFRL &= ~(((unsigned long)GPIO_AFR_BIT_ALL) << (4*pin));
		gpiox->AFRL |= (((unsigned long)GPIO_AFR_BIT_I2C123) << (4*pin));
	} else {
		gpiox->AFRH &= ~(((unsigned long)GPIO_AFR_BIT_ALL) << (4*(pin-8)));
		gpiox->AFRH |= (((unsigned long)GPIO_AFR_BIT_I2C123) << (4*(pin-8)));
	}
}

#define POS_LD2	5	// Green
#define BIT_LD2		(1 << POS_LD2)

static unsigned long flg_led_on;

static int led_register(struct st_device *dev, char *param)
{
	flg_led_on = 0;

	RCC_AHB1ENR |= RCC_AHB1_BIT_GPIOD;

	init_pin(GPIOA, POS_LD2);

	return 0;
}

static int led_getc(struct st_device *dev, unsigned char *rd)
{
	*rd = ((GPIOA->ODR & BIT_LD2) >> POS_LD2);

	return 1;
}

static int led_putc(struct st_device *dev, unsigned char ch)
{
	flg_led_on = ((ch << POS_LD2) & BIT_LD2);
	GPIOA->ODR = flg_led_on;

	return 1;
}

static int led_suspend(struct st_device *dev)
{
	GPIOA->ODR &= ~(BIT_LD2);

	return 0;
}

static int led_resume(struct st_device *dev)
{
	GPIOA->ODR = flg_led_on;

	return 0;
}

const struct st_device led_device = {
	.name		= DEF_DEV_NAME_LED,
	.explan		= "STM32L152RE-Nucleo LED",
	.register_dev	= led_register,
	.getc		= led_getc,
	.putc		= led_putc,
	.suspend	= led_suspend,
	.resume		= led_resume,
}; //!< LEDドライバ
