/** @file
    @brief	MMC ドライバ STM32F4xx Nucleo 依存部分

    SPIドライバ使用

    @date	2015.08.11
    @author	Takashi SHUDO

    @info

    CS		PB6
    WP		N/A
    POWER	N/A
*/

#include "sysconfig.h"
#include "tkprintf.h"
#include "device.h"
#include "device/spi_ioctl.h"


/*
  GPIOレジスタ
*/
#define RCC_AHB1ENR	(*(volatile unsigned long *)0x40023830)
#define RCC_AHB1LPENR	(*(volatile unsigned long *)0x40023850)
#define RCC_AHB1_BIT_GPIOB	(1UL<<1)

#define GPIOB_MODER	(*(volatile unsigned long *)0x40020400)
#define GPIO_MODE_BIT_IN	0x00	// Input Mode
#define GPIO_MODE_BIT_OUT	0x01	// Output Mode
#define GPIO_MODE_BIT_AF	0x02	// Alternate functions Mode
#define GPIO_MODE_BIT_AN	0x03	// Analog Mode
#define GPIO_MODE_BIT_ALL	0x03

#define GPIOB_OTYPE	(*(volatile unsigned long *)0x40020404)
#define GPIO_OTYPE_BIT_PP	0x00	// push-pull
#define GPIO_OTYPE_BIT_OD	0x01	// open-drain

#define GPIOB_OSPEEDR	(*(volatile unsigned long *)0x40020408)
#define GPIO_OSPEED_BIT_ALL	0x03
#define GPIO_OSPEED_BIT_50MHz	0x02
#define GPIO_OSPEED_BIT_100MHz	0x03

#define GPIOB_PUPDR	(*(volatile unsigned long *)0x4002040C)
#define GPIO_PUPDR_BIT_NOPULL	0x00	// No pull-up, pull-down
#define GPIO_PUPDR_BIT_PUP	0x01	// pull-up
#define GPIO_PUPDR_BIT_PDOWN	0x02	// pull-down
#define GPIO_PUPDR_BIT_ALL	0x03

#define GPIOB_IDR	(*(volatile unsigned long *)0x40020410)
#define GPIOB_ODR	(*(volatile unsigned long *)0x40020414)
#define GPIOB_BSRRL	(*(volatile unsigned short *)0x40020418)
#define GPIOB_BSRRH	(*(volatile unsigned short *)0x4002041A)
#define GPIOB_LCKR	(*(volatile unsigned long *)0x4002041C)
#define GPIOB_AFRL	(*(volatile unsigned long *)0x40020420)
#define GPIOB_AFRH	(*(volatile unsigned long *)0x40020424)

static void init_pin(int pos)
{
	GPIOB_MODER &= ~(((unsigned long)GPIO_MODE_BIT_ALL) << (2*pos));
	GPIOB_MODER |= (((unsigned long)GPIO_MODE_BIT_OUT) << (2*pos));
	GPIOB_OSPEEDR &= ~(((unsigned long)GPIO_OSPEED_BIT_ALL) << (2*pos));
	GPIOB_OSPEEDR |= (((unsigned long)GPIO_OSPEED_BIT_100MHz) << (2*pos));
	GPIOB_OTYPE &= ~(((unsigned long)1) << pos);
	GPIOB_OTYPE |= (((unsigned long)GPIO_OTYPE_BIT_PP) << pos);
	GPIOB_PUPDR &= ~(((unsigned long)GPIO_PUPDR_BIT_ALL) << (2*pos));
	GPIOB_PUPDR |= (((unsigned long)GPIO_PUPDR_BIT_NOPULL) << (2*pos));
}

#define CS_PIN	6	// GPIOB 6

static void cs_low(void)
{
	GPIOB_BSRRH = (1UL<<CS_PIN);
}

static void cs_high(void)
{
	GPIOB_BSRRL = (1UL<<CS_PIN);
}


static struct st_device *mmc_dev;

static int mmc_register(struct st_device *dev, char *param)
{
	mmc_dev = open_device(param);
	if(mmc_dev == 0) {
		SYSERR_PRINT("Cannot open device \"%s\"\n", param);
		return -1;
	}

	RCC_AHB1ENR |= RCC_AHB1_BIT_GPIOB;
	init_pin(CS_PIN);
	cs_high();

	return 0;
}

static int mmc_getc(struct st_device *dev, unsigned char *rd)
{
	int rt;

//	cs_low();
	rt = getc_device(mmc_dev, rd);
//	cs_high();

	return rt;
}

static int mmc_read(struct st_device *dev, void *data, unsigned int size)
{
	int rt;

//	cs_low();
	rt = read_device(mmc_dev, data, size);
//	cs_high();

	return rt;
}

static int mmc_putc(struct st_device *dev, unsigned char ch)
{
	int rt;

//	cs_low();
	rt = putc_device(mmc_dev, ch);
//	cs_high();

	return rt;
}

static int mmc_write(struct st_device *dev, const void *data, unsigned int size)
{
	int rt;

//	cs_low();
	rt = write_device(mmc_dev, data, size);
//	cs_high();

	return rt;
}

static int mmc_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	int rt = 0;

	switch(com) {
	case IOCMD_SPI_CS0ASSERT:
		cs_low();
		break;

	case IOCMD_SPI_CS0NEGATE:
		cs_high();
		break;

	default:
		rt = ioctl_device(mmc_dev, com, arg, param);
		break;
	}

	return rt;
}

static int mmc_suspend(struct st_device *dev)
{
	int rt;

//	cs_low();
	rt = suspend_device(mmc_dev);
//	cs_high();

	return rt;
}

static int mmc_resume(struct st_device *dev)
{
	int rt;

//	cs_low();
	rt = resume_device(mmc_dev);
//	cs_high();

	return rt;
}


const struct st_device stm_mmc_device = {
	.name		= "mmc_spi",
	.explan		= "STM32F4xx Nucleo SPI MMC",
	.register_dev	= mmc_register,
	.read		= mmc_read,
	.getc		= mmc_getc,
	.write		= mmc_write,
	.putc		= mmc_putc,
	.ioctl		= mmc_ioctl,
	.suspend	= mmc_suspend,
	.resume		= mmc_resume,
};
