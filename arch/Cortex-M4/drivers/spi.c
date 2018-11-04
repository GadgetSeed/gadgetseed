/** @file
    @brief	STM32F4 SPIドライバ

    @date	2013.04.06
    @author	Takashi SHUDO

    @note

    SPI1
    PA5	SPI1_SCK
    PA6	SPI1_MISO
    PA7	SPI1_MOSI
    RX	DMA2_Stream2
    TX	DMA2_Stream3

    SPI2
    PB13	SPI2_SCK
    PB14	SPI2_MISO
    PB15	SPI2_MOSI
    RX	DMA1_Stream3
    TX	DMA1_Stream4
*/

#include "sysconfig.h"
#include "device.h"
#include "device/spi_ioctl.h"
#include "interrupt.h"
#include "tkprintf.h"
#include "task/event.h"
#include "task/mutex.h"
#include "task/syscall.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


#define USE_MUTEX
#define USE_DMA
#define USE_DMA_INT

#define VECTNUM_SPI1	51	// SPI1割り込み
#define VECTNUM_SPI2	52	// SPI2割り込み
#define VECTNUM_DMA2_RX	74	// DMA2_Stream2
#define VECTNUM_DMA2_TX	75	// DMA2_Stream3
#define VECTNUM_DMA1_RX	30	// DMA1_Stream3
#define VECTNUM_DMA1_TX	31	// DMA1_Stream4

#define FCPU	(GSC_CPU_CLOCK_HZ/2)
#define CLK_DEFAULT	400000UL	// SCLKの周波数 400KHz
#define CLK_FP256	(FCPU/256)	// SCLKの最小周波数
#define CLK_FP128	(FCPU/128)
#define CLK_FP64	(FCPU/64)
#define CLK_FP32	(FCPU/32)
#define CLK_FP16	(FCPU/16)
#define CLK_FP8		(FCPU/8)
#define CLK_FP4		(FCPU/4)
#define CLK_FP2		(FCPU/2)	// SCLKの最大周波数

/*
  GPIOレジスタ
*/
typedef struct st_reg_gpio {
	unsigned int	MODER;
	unsigned int	OTYPE;
	unsigned int	OSPEEDR;
	unsigned int	PUPDR;
	unsigned int	IDR;
	unsigned int	ODR;
	unsigned int	BSRR;
	unsigned int	LCKR;
	unsigned int	AFRL;
	unsigned int	AFRH;
} st_reg_gpio;

#define GPIOA_BASE	(0x40020000 + 0x000)
#define GPIOB_BASE	(0x40020000 + 0x400)
#define GPIOC_BASE	(0x40020000 + 0x800)

#define GPIOA	((st_reg_gpio *)GPIOA_BASE)
#define GPIOB	((st_reg_gpio *)GPIOB_BASE)
#define GPIOC	((st_reg_gpio *)GPIOC_BASE)

#define GPIO_MODE_BIT_IN	0x00UL	// Input Mode
#define GPIO_MODE_BIT_OUT	0x01UL	// Output Mode
#define GPIO_MODE_BIT_AF	0x02UL	// Alternate functions Mode
#define GPIO_MODE_BIT_AN	0x03UL	// Analog Mode
#define GPIO_MODE_BIT_ALL	0x03UL

#define GPIO_OTYPE_BIT_PP	0x00UL	// push-pull
#define GPIO_OTYPE_BIT_OD	0x01UL	// open-drain

#define GPIO_OSPEED_BIT_ALL	0x03UL
#define GPIO_OSPEED_BIT_100MHz	0x03UL

#define GPIO_PUPDR_BIT_NOPULL	0x00UL	// No pull-up, pull-down
#define GPIO_PUPDR_BIT_PUP	0x01UL	// pull-up
#define GPIO_PUPDR_BIT_PDOWN	0x02UL	// pull-down
#define GPIO_PUPDR_BIT_ALL	0x03UL

#define GPIO_AFR_BIT_SPI12	0x05UL
#define GPIO_AFR_BIT_ALL	0x0FUL


/*
  RCCレジスタ
*/
#define RCC_AHB1ENR	(*(volatile unsigned int *)0x40023830)
#define RCC_AHB1LPENR	(*(volatile unsigned int *)0x40023850)
#define RCC_AHB1_BIT_GPIOA	(1UL<<0)
#define RCC_AHB1_BIT_GPIOB	(1UL<<1)
#define RCC_AHB1_BIT_GPIOC	(1UL<<2)
#define RCC_AHB1_BIT_DMA1	(1UL<<21)
#define RCC_AHB1_BIT_DMA2	(1UL<<22)

#define RCC_APB1ENR	(*(volatile unsigned int *)0x40023840)
#define RCC_APB1_BIT_SPI2	(1UL<<14)

#define RCC_APB2ENR	(*(volatile unsigned int *)0x40023844)
#define RCC_APB2LPENR	(*(volatile unsigned int *)0x40023864)
#define RCC_APB2_BIT_SPI1	(1UL<<12)

/*
  SPIレジスタ
*/
typedef struct st_reg_spi {
	unsigned int	CR1;	// + 0x00
	unsigned int	CR2;	// + 0x04
	unsigned int	SR;	// + 0x08
	unsigned int	DR;	// + 0x0C
	unsigned int	CRCPR;	// + 0x10
	unsigned int	RXCRCR;	// + 0x14
	unsigned int	TXCRCR;	// + 0x18
	unsigned int	I2SCFGR;// + 0x1C
	unsigned int	I2SPR;	// + 0x20
} st_reg_spi;

#define SPI1_BASE	0x40013000
#define SPI2_BASE	0x40003800

#define SPI1	((st_reg_spi *)SPI1_BASE)
#define SPI2	((st_reg_spi *)SPI2_BASE)

#define SPI_CR1_BIT_BIDIMODE	(1UL<<15)
#define SPI_CR1_BIT_BIDIOE	(1UL<<14)
#define SPI_CR1_BIT_CRCEN	(1UL<<13)
#define SPI_CR1_BIT_CRCNEXT	(1UL<<12)
#define SPI_CR1_BIT_DFF		(1UL<<11)
#define SPI_CR1_BIT_RXONLY	(1UL<<10)
#define SPI_CR1_BIT_SSM		(1UL<<9)
#define SPI_CR1_BIT_SSI		(1UL<<8)
#define SPI_CR1_BIT_LSBFIRST	(1UL<<7)
#define SPI_CR1_BIT_SPE		(1UL<<6)
#define SPI_CR1_BIT_BR_256	(7UL<<3)
#define SPI_CR1_BIT_MSTR	(1UL<<2)
#define SPI_CR1_BIT_CPOL	(1UL<<1)
#define SPI_CR1_BIT_CPHA	(1UL<<0)

#define SPI_CR2_BIT_TXEIE	(1UL<<7)
#define SPI_CR2_BIT_RXNEIE	(1UL<<6)
#define SPI_CR2_BIT_ERRIE	(1UL<<5)
#define SPI_CR2_BIT_SSOE	(1UL<<2)
#define SPI_CR2_BIT_TXDMAEN	(1UL<<1)
#define SPI_CR2_BIT_RXDMAEN	(1UL<<0)

#define SPI_SR_BIT_BSY		(1UL<<7)
#define SPI_SR_BIT_OVR		(1UL<<6)
#define SPI_SR_BIT_MODF		(1UL<<5)
#define SPI_SR_BIT_CRCERR	(1UL<<4)
#define SPI_SR_BIT_UDR		(1UL<<3)
#define SPI_SR_BIT_CHSIDE	(1UL<<2)
#define SPI_SR_BIT_TXE		(1UL<<1)
#define SPI_SR_BIT_RXNE		(1UL<<0)

#define SPI_I2SCFGR_BIT_I2SMOD	(1UL<<11)
#define SPI_I2SCFGR_BIT_I2SE	(1UL<<10)
#define SPI_I2SCFGR_BIT_I2SCFG	(3UL<<8)
#define SPI_I2SCFGR_BIT_PCMSYNC	(1UL<<7)
#define SPI_I2SCFGR_BIT_I2SSTD	(3UL<<4)
#define SPI_I2SCFGR_BIT_CKPOLD	(1UL<<3)
#define SPI_I2SCFGR_BIT_DATLEN	(3UL<<1)
#define SPI_I2SCFGR_BIT_CHLEN	(1UL<<0)

/*
  DMAレジスタ
*/
typedef struct st_reg_dmacr {
	unsigned int	CR;
	unsigned int	NDTR;
	unsigned int	PAR;
	unsigned int	M0AR;
	unsigned int	M1AR;
	unsigned int	FCR;
} st_reg_dmacr;

typedef struct st_reg_dma {
	unsigned int	LISR;	// + 0x00
	unsigned int	HISR;	// + 0x04
	unsigned int	LIFCR;	// + 0x08
	unsigned int	HIFCR;	// + 0x0C
	st_reg_dmacr	Sx[8];
} st_reg_dma;

#define DMA1_BASE	0x40026000
#define DMA2_BASE	0x40026400

#define DMA1	((st_reg_dma *)DMA1_BASE)
#define DMA2	((st_reg_dma *)DMA2_BASE)

#define DMA_LISR_BIT_FEIF3	(1UL<<22)
#define DMA_LISR_BIT_TCIF3	(1UL<<27)
#define DMA_LISR_BIT_HTIF3	(1UL<<26)
#define DMA_LISR_BIT_TEIF3	(1UL<<25)
#define DMA_LISR_BIT_DMEIF3	(1UL<<24)
#define DMA_LISR_BIT_FEIF3	(1UL<<22)

#define DMA_LISR_BIT_TCIF2	(1UL<<21)
#define DMA_LISR_BIT_HTIF2	(1UL<<20)
#define DMA_LISR_BIT_TEIF2	(1UL<<19)
#define DMA_LISR_BIT_DMEIF2	(1UL<<18)
#define DMA_LISR_BIT_FEIF2	(1UL<<16)

#define DMA_HISR_BIT_TCIF4	(1UL<<5)
#define DMA_HISR_BIT_HTIF4	(1UL<<4)
#define DMA_HISR_BIT_TEIF4	(1UL<<3)
#define DMA_HISR_BIT_DMEIF4	(1UL<<2)
#define DMA_HISR_BIT_FEIF4	(1UL<<0)

#define DMA_SxCR_BIT_EN		(1UL<<0)
#define DMA_SxCR_BIT_DMEIE	(1UL<<1)
#define DMA_SxCR_BIT_TEIE	(1UL<<2)
#define DMA_SxCR_BIT_HTIE	(1UL<<3)
#define DMA_SxCR_BIT_TCIE	(1UL<<4)
#define DMA_SxCR_BIT_PFCTRL	(1UL<<5)
#define DMA_SxCR_BIT_DIR_ALL	(3UL<<6)
#define DMA_SxCR_BIT_DIR_P2M	(0UL<<6)
#define DMA_SxCR_BIT_DIR_M2P	(1UL<<6)
#define DMA_SxCR_BIT_DIR_M2M	(2UL<<6)
#define DMA_SxCR_BIT_CIRC	(1UL<<8)
#define DMA_SxCR_BIT_PINC	(1UL<<9)
#define DMA_SxCR_BIT_MINC	(1UL<<10)
#define DMA_SxCR_BIT_PSIZE_ALL	(3UL<<11)
#define DMA_SxCR_BIT_PSIZE_BYTE	(0UL<<11)
#define DMA_SxCR_BIT_PSIZE_HW	(1UL<<11)
#define DMA_SxCR_BIT_PSIZE_WORD	(2UL<<11)
#define DMA_SxCR_BIT_MSIZE_ALL	(3UL<<13)
#define DMA_SxCR_BIT_MSIZE_BYTE	(0UL<<13)
#define DMA_SxCR_BIT_MSIZE_HW	(1UL<<13)
#define DMA_SxCR_BIT_MSIZE_WORD	(2UL<<13)
#define DMA_SxCR_BIT_MSIZE_PINCOS	(1UL<<15)
#define DMA_SxCR_BIT_PL_ALL	(3UL<<16)
#define DMA_SxCR_BIT_PL_LOW	(0UL<<16)
#define DMA_SxCR_BIT_PL_MEDIUM	(1UL<<16)
#define DMA_SxCR_BIT_PL_HIGH	(2UL<<16)
#define DMA_SxCR_BIT_PL_VHIGH	(3UL<<16)
#define DMA_SxCR_BIT_DBM	(1UL<<18)
#define DMA_SxCR_BIT_CT		(1UL<<19)
#define DMA_SxCR_BIT_PBURST_ALL	(3UL<<21)
#define DMA_SxCR_BIT_PBURST_SINGLE	(0UL<<21)
#define DMA_SxCR_BIT_PBURST_INCR4	(1UL<<21)
#define DMA_SxCR_BIT_PBURST_INCR8	(2UL<<21)
#define DMA_SxCR_BIT_PBURST_INCR16	(3UL<<21)
#define DMA_SxCR_BIT_MBURST_ALL	(3UL<<23)
#define DMA_SxCR_BIT_MBURST_SINGLE	(0UL<<23)
#define DMA_SxCR_BIT_MBURST_INCR4	(1UL<<23)
#define DMA_SxCR_BIT_MBURST_INCR8	(2UL<<23)
#define DMA_SxCR_BIT_MBURST_INCR16	(3UL<<23)
#define DMA_SxCR_BIT_CHSEL_ALL	(7UL<<25)
#define DMA_SxCR_BIT_CHSEL_CH0	(0UL<<25)
#define DMA_SxCR_BIT_CHSEL_CH1	(1UL<<25)
#define DMA_SxCR_BIT_CHSEL_CH2	(2UL<<25)
#define DMA_SxCR_BIT_CHSEL_CH3	(3UL<<25)
#define DMA_SxCR_BIT_CHSEL_CH4	(4UL<<25)
#define DMA_SxCR_BIT_CHSEL_CH5	(5UL<<25)
#define DMA_SxCR_BIT_CHSEL_CH6	(6UL<<25)
#define DMA_SxCR_BIT_CHSEL_CH7	(7UL<<25)
#define DMA_SxCR_BIT_CHSEL_SHIFT	(25)

#define DMA_SxFCR_BIT_FEIE	(1UL<<7)
#define DMA_SxFCR_BIT_FS_ALL	(7UL<<3)
#define DMA_SxFCR_BIT_DMDIS	(1UL<<2)
#define DMA_SxFCR_BIT_FTH_ALL	(3UL<<0)
#define DMA_SxFCR_BIT_FTH_1P4	(0UL<<0)
#define DMA_SxFCR_BIT_FTH_1P2	(1UL<<0)
#define DMA_SxFCR_BIT_FTH_3P4	(2UL<<0)
#define DMA_SxFCR_BIT_FTH_FULL	(3UL<<0)

#define NVIC_ISER0	(*(volatile unsigned int *)0xE000E100)
#define NVIC_ISER1	(*(volatile unsigned int *)0xE000E104)


struct st_spi_data {
#ifdef USE_MUTEX
	struct st_mutex spimutex;
#endif
#ifdef USE_DMA_INT
	struct st_event dma_evq;
#endif
	st_reg_spi *spi;
	st_reg_dma *dma;
	unsigned char channel;
	unsigned char rx_stream;
	unsigned char tx_stream;
};

static struct st_spi_data spi_data[2];

const struct st_device spi1_device;
const struct st_device spi2_device;

#ifdef USE_MUTEX
#define MUTEX_LOCK_TIMEOUT	1000

void lock_spi(struct st_device *dev)
{
	if(mutex_lock(&(((struct st_spi_data *)(dev->private_data))->spimutex),
		      MUTEX_LOCK_TIMEOUT) < 0) {
		SYSERR_PRINT("%s lock timeout\n", dev->name);
	}
}

void unlock_spi(struct st_device *dev)
{
	mutex_unlock(&(((struct st_spi_data *)(dev->private_data))->spimutex));
}
#else
#define lock_spi(x)
#define unlock_spi(x)
#endif


static void inthdr_spi1(unsigned int intnum, void *sp)
{
}

static void inthdr_spi2(unsigned int intnum, void *sp)
{
}

#ifdef USE_DMA_INT
static void inthdr_spi_dma2(unsigned int intnum, void *sp)
{
	DMA2->LIFCR = DMA_LISR_BIT_TCIF2;

	if(intnum == VECTNUM_DMA2_RX) {
		event_set_ISR(sp, &(((struct st_spi_data *)(spi1_device.private_data))->dma_evq));
	} else {
		SYSERR_PRINT("Invalid Interrupt(%d)\n", (int)intnum);
	}
}

static void inthdr_spi_dma1(unsigned int intnum, void *sp)
{
	DMA1->LIFCR = DMA_LISR_BIT_TCIF3;

	if(intnum == VECTNUM_DMA1_RX) {
		event_set_ISR(sp, &(((struct st_spi_data *)(spi2_device.private_data))->dma_evq));
	} else {
		SYSERR_PRINT("Invalid Interrupt(%d)\n", (int)intnum);
	}

}
#endif

static void init_gpio(st_reg_gpio *gpio, int pin, int pup)
{
	volatile st_reg_gpio *gpiox = gpio;

	if(pin < 8) {
		gpiox->AFRL &= ~(GPIO_AFR_BIT_ALL << (4*pin));
		gpiox->AFRL |= (GPIO_AFR_BIT_SPI12 << (4*pin));
	} else {
		gpiox->AFRH &= ~(GPIO_AFR_BIT_ALL << (4*(pin-8)));
		gpiox->AFRH |= (GPIO_AFR_BIT_SPI12 << (4*(pin-8)));
	}

	gpiox->MODER &= ~(GPIO_MODE_BIT_ALL << (2*pin));
	gpiox->MODER |= (GPIO_MODE_BIT_AF << (2*pin));
	gpiox->OSPEEDR &= ~(GPIO_OSPEED_BIT_ALL << (2*pin));
	gpiox->OSPEEDR |= (GPIO_OSPEED_BIT_100MHz << (2*pin));
	gpiox->OTYPE &= ~(1UL << pin);
	gpiox->OTYPE |= (GPIO_OTYPE_BIT_PP << pin);
	gpiox->PUPDR &= ~(GPIO_PUPDR_BIT_ALL << (2*pin));
	if(pup != 0) {
		gpiox->PUPDR |= (GPIO_PUPDR_BIT_PUP << (2*pin));
	} else {
		gpiox->PUPDR |= (GPIO_PUPDR_BIT_NOPULL << (2*pin));
	}
}

static void init_spi(struct st_device *dev)
{
	volatile st_reg_spi *spix = ((struct st_spi_data *)(dev->private_data))->spi;
#ifdef USE_DMA
	volatile st_reg_dma *dmax = ((struct st_spi_data *)(dev->private_data))->dma;
	int rx_stream = ((struct st_spi_data *)(dev->private_data))->rx_stream;
	int tx_stream = ((struct st_spi_data *)(dev->private_data))->tx_stream;
#endif

	if(dev == &spi1_device) {
		RCC_AHB1ENR |= RCC_AHB1_BIT_GPIOA;	// GPIOA Clock enable

		// GPIOA PIN5,6,7 SPI Function
		// MMCの場合は Pull UP
		init_gpio(GPIOA, 5, 1);
		init_gpio(GPIOA, 6, 1);
		init_gpio(GPIOA, 7, 1);

		RCC_APB2ENR |= RCC_APB2_BIT_SPI1;	// SPI1 Clock enable
	} else if(dev == &spi2_device) {
		RCC_AHB1ENR |= RCC_AHB1_BIT_GPIOB;	// GPIOB Clock enable
#if 0
		RCC_AHB1ENR |= RCC_AHB1_BIT_GPIOC;	// GPIOC Clock enable
#endif

		// GPIOB PIN13,14,15 SPI Function
#ifdef SPI2SCK_PB10
		init_gpio(GPIOB, 10, 0);
#else
		init_gpio(GPIOB, 13, 0);
#endif
#if 0
		init_gpio(GPIOC, 2, 0);
		init_gpio(GPIOC, 3, 0);
#else
		init_gpio(GPIOB, 14, 0);
		init_gpio(GPIOB, 15, 0);
#endif

		RCC_APB1ENR |= RCC_APB1_BIT_SPI2;	// SPI2 Clock enable
	} else {
		SYSERR_PRINT("Invalid device(%p)\n", dev);
	}


	spix->CR1 &= 0x3040;
	spix->CR1 |= (SPI_CR1_BIT_MSTR |	// Master Mode
		      SPI_CR1_BIT_SSM |
		      SPI_CR1_BIT_BR_256 |
		      SPI_CR1_BIT_SSI);
//#ifdef USE_ST7735R
#if 0
	if(dev == &spi2_device) {	// ST7735R用に暫定!!!
		spix->CR1 |= (SPI_CR1_BIT_CPOL | SPI_CR1_BIT_CPHA);
	}
#eles
	if(dev == &spi2_device) {	// ENC28J60用
		spix->CR1 &= ~(SPI_CR1_BIT_CPOL | SPI_CR1_BIT_CPHA);
//		spix->CR1 |= SPI_CR1_BIT_CPHA;
	}
#endif
	spix->I2SCFGR &= ~SPI_I2SCFGR_BIT_I2SMOD;
	spix->CRCPR = 7;

	spix->CR1 |= SPI_CR1_BIT_SPE;

	while((spix->SR & SPI_SR_BIT_TXE) == 0) {
		;
	}
	spix->DR;

#ifdef USE_DMA
	if(dev == &spi1_device) {
		RCC_AHB1ENR |= RCC_AHB1_BIT_DMA2;	// DMA2 Clock enable

		dmax->Sx[rx_stream].CR &= ~DMA_SxCR_BIT_EN;
		dmax->Sx[tx_stream].CR &= ~DMA_SxCR_BIT_EN;

		NVIC_ISER1 |= (1 << ((VECTNUM_DMA2_RX-16) % 32));
	} else if(dev == &spi2_device) {
		RCC_AHB1ENR |= RCC_AHB1_BIT_DMA1;	// DMA1 Clock enable

		dmax->Sx[rx_stream].CR &= ~DMA_SxCR_BIT_EN;
		dmax->Sx[tx_stream].CR &= ~DMA_SxCR_BIT_EN;

		NVIC_ISER0 |= (1 << ((VECTNUM_DMA1_RX-16) % 32));
	} else {
		SYSERR_PRINT("Invalid device(%p)\n", dev);
	}

#endif
}

static int init_driver(struct st_device *dev)
{
	void (* spi_ih)(unsigned int intnum, void *sp) = inthdr_spi1;
	unsigned short spi_vn = VECTNUM_SPI1;
#ifdef USE_DMA_INT
	void (* dma_ih)(unsigned int intnum, void *sp) = inthdr_spi_dma2;
	unsigned short dma_vn = VECTNUM_DMA2_RX;
#endif

	if(dev == &spi1_device) {
		spi_vn = VECTNUM_SPI1;
		((struct st_spi_data *)(dev->private_data))->spi = SPI1;
		spi_ih = inthdr_spi1;
#ifdef USE_DMA_INT
		((struct st_spi_data *)(dev->private_data))->dma = DMA2;
		((struct st_spi_data *)(dev->private_data))->channel = 3;
		((struct st_spi_data *)(dev->private_data))->rx_stream = 2;
		((struct st_spi_data *)(dev->private_data))->tx_stream = 3;
		dma_ih = inthdr_spi_dma2;
		dma_vn = VECTNUM_DMA2_RX;
#endif
#ifdef USE_DMA_INT
		register_interrupt(dma_vn, dma_ih);
		eventqueue_register(&(((struct st_spi_data *)(dev->private_data))->dma_evq),
				    "spi1-dma", 0, 0, 0);
#endif
#ifdef USE_MUTEX
		mutex_register(&(((struct st_spi_data *)(dev->private_data))->spimutex),
			       "spi1-mtx");
#endif
	} else if(dev == &spi2_device) {
		spi_vn = VECTNUM_SPI2;
		((struct st_spi_data *)(dev->private_data))->spi = SPI2;
		spi_ih = inthdr_spi2;
#ifdef USE_DMA_INT
		((struct st_spi_data *)(dev->private_data))->dma = DMA1;
		((struct st_spi_data *)(dev->private_data))->channel = 0;
		((struct st_spi_data *)(dev->private_data))->rx_stream = 3;
		((struct st_spi_data *)(dev->private_data))->tx_stream = 4;
		dma_ih = inthdr_spi_dma1;
		dma_vn = VECTNUM_DMA1_RX;
#endif
#ifdef USE_DMA_INT
		register_interrupt(dma_vn, dma_ih);
		eventqueue_register(&(((struct st_spi_data *)(dev->private_data))->dma_evq),
				    "spi2-dma", 0, 0, 0);
#endif
#ifdef USE_MUTEX
		mutex_register(&(((struct st_spi_data *)(dev->private_data))->spimutex),
			       "spi2-mtx");
#endif
	} else {
		SYSERR_PRINT("Unknown SPI device(%p)\n", dev);
		return -1;
	}

	register_interrupt(spi_vn, spi_ih);

	init_spi(dev);

	return 0;
}

#define SPI_RETRY_TIME	1000000

static int spi_readbyte(struct st_device *dev, unsigned char *rd)
{
	volatile st_reg_spi *spix = ((struct st_spi_data *)(dev->private_data))->spi;
	int i = SPI_RETRY_TIME;

	spix->DR = 0xFF;

	while((spix->SR & SPI_SR_BIT_TXE) == 0) {
		i --;
		if(i == 0) {
			SYSERR_PRINT("TXE timeout\n");
			break;
		}
	}

	while((spix->SR & SPI_SR_BIT_RXNE) == 0) {
		i --;
		if(i == 0) {
			SYSERR_PRINT("RXNE timeout\n");
			break;
		}
	}

	while(spix->SR & SPI_SR_BIT_BSY) {
		i --;
		if(i == 0) {
			SYSERR_PRINT("BSY timeout\n");
			break;
		}
	}

	*rd = spix->DR;

	DKPRINTF(0x02, "SPI1 RD = %02X\n", *rd);

	return 1;
}

#define SPI_WAIT_TIME	500
#ifdef USE_DMA

static int spi_transblock(struct st_device *dev, unsigned char *rd, long size,
			  int dir_read)
{
	static const unsigned char dummy_data = 0xFF;

	volatile st_reg_spi *spix = ((struct st_spi_data *)(dev->private_data))->spi;
	volatile st_reg_dma *dmax = ((struct st_spi_data *)(dev->private_data))->dma;
	unsigned int channel = ((struct st_spi_data *)(dev->private_data))->channel;
	int rx_stream = ((struct st_spi_data *)(dev->private_data))->rx_stream;
	int tx_stream = ((struct st_spi_data *)(dev->private_data))->tx_stream;
	unsigned int tmp;
	int rt = 0;
	int err = 0;

	DKPRINTF(0x01, "SPI DMA *data=%p, size=%ld, W(0)/R(1)=%d\n", rd, size, dir_read);

	while(dmax->Sx[tx_stream].CR & DMA_SxCR_BIT_EN) {
		;
	}

	spix->CR1 &= ~SPI_CR1_BIT_DFF;	// 8 bit data frame

	tmp = dmax->Sx[rx_stream].CR;
	tmp &= ~(DMA_SxCR_BIT_CHSEL_ALL | DMA_SxCR_BIT_MBURST_ALL |
		 DMA_SxCR_BIT_PBURST_ALL | DMA_SxCR_BIT_PL_ALL |
		 DMA_SxCR_BIT_MSIZE_ALL | DMA_SxCR_BIT_PSIZE_ALL |
		 DMA_SxCR_BIT_MINC | DMA_SxCR_BIT_PINC |
		 DMA_SxCR_BIT_CIRC | DMA_SxCR_BIT_DIR_ALL);
	tmp |= ((channel << DMA_SxCR_BIT_CHSEL_SHIFT) |
		DMA_SxCR_BIT_DIR_P2M |
		DMA_SxCR_BIT_PSIZE_BYTE |
		DMA_SxCR_BIT_MSIZE_BYTE |
		DMA_SxCR_BIT_PL_HIGH |
		DMA_SxCR_BIT_PBURST_SINGLE |
		DMA_SxCR_BIT_MBURST_SINGLE);
	if(dir_read) {
		tmp |= DMA_SxCR_BIT_MINC;
	}
	dmax->Sx[rx_stream].CR = tmp;

	tmp = dmax->Sx[rx_stream].FCR;
	tmp &= ~(DMA_SxFCR_BIT_DMDIS | DMA_SxFCR_BIT_FTH_ALL);
	tmp |= DMA_SxFCR_BIT_FTH_FULL;
	tmp = dmax->Sx[rx_stream].FCR = tmp;

	dmax->Sx[rx_stream].NDTR = size;
	dmax->Sx[rx_stream].PAR = (unsigned int)&(spix->DR);
	if(dir_read) {
		dmax->Sx[rx_stream].M0AR = (unsigned int)rd;
	} else {
		dmax->Sx[rx_stream].M0AR = (unsigned int)(&dummy_data);
	}

	tmp = dmax->Sx[tx_stream].CR;
	tmp &= ~(DMA_SxCR_BIT_CHSEL_ALL | DMA_SxCR_BIT_MBURST_ALL |
		 DMA_SxCR_BIT_PBURST_ALL | DMA_SxCR_BIT_PL_ALL |
		 DMA_SxCR_BIT_MSIZE_ALL | DMA_SxCR_BIT_PSIZE_ALL |
		 DMA_SxCR_BIT_MINC | DMA_SxCR_BIT_PINC |
		 DMA_SxCR_BIT_CIRC | DMA_SxCR_BIT_DIR_ALL);
	tmp |= ((channel << DMA_SxCR_BIT_CHSEL_SHIFT) |
		DMA_SxCR_BIT_DIR_M2P |
		DMA_SxCR_BIT_PSIZE_BYTE |
		DMA_SxCR_BIT_MSIZE_BYTE |
		DMA_SxCR_BIT_PL_HIGH |
		DMA_SxCR_BIT_PBURST_SINGLE |
		DMA_SxCR_BIT_MBURST_SINGLE);
	if(dir_read == 0) {
		tmp |= DMA_SxCR_BIT_MINC;
	}
	dmax->Sx[tx_stream].CR = tmp;

	tmp = dmax->Sx[tx_stream].FCR;
	tmp &= ~(DMA_SxFCR_BIT_DMDIS | DMA_SxFCR_BIT_FTH_ALL);
	tmp |= DMA_SxFCR_BIT_FTH_FULL;
	dmax->Sx[tx_stream].FCR = tmp;

	dmax->Sx[tx_stream].NDTR = size;
	dmax->Sx[tx_stream].PAR = (unsigned int)&(spix->DR);
	if(dir_read) {
		dmax->Sx[tx_stream].M0AR = (unsigned int)(&dummy_data);
	} else {
		dmax->Sx[tx_stream].M0AR = (unsigned int)rd;
	}

	spix->CR2 |= (SPI_CR2_BIT_RXDMAEN | SPI_CR2_BIT_TXDMAEN);

#ifdef USE_DMA_INT
	event_clear(&(((struct st_spi_data *)(dev->private_data))->dma_evq));
#endif
	dmax->Sx[rx_stream].CR |= (DMA_SxCR_BIT_TCIE | DMA_SxCR_BIT_TEIE);
	dmax->Sx[rx_stream].CR |= DMA_SxCR_BIT_EN;
	dmax->Sx[tx_stream].CR |= DMA_SxCR_BIT_EN;

#ifdef USE_DMA_INT
	rt = event_wait(&(((struct st_spi_data *)(dev->private_data))->dma_evq),
			0, SPI_WAIT_TIME);
	if(rt == 0) {
		err = 1;
		SYSERR_PRINT("SPI DMA timeout(%ld)\n", rt);
	}
#else
	if(dev == &spi1_device) {
		while((dmax->LISR & DMA_LISR_BIT_TCIF3) == 0) {
			;
		}
	} else if(dev == &spi2_device) {
		while((dmax->HISR & DMA_HISR_BIT_TCIF4) == 0) {
			;
		}
	} else {
		SYSERR_PRINT("Invalid device(%p)\n", dev);
	}

#endif

	dmax->Sx[rx_stream].CR &= ~DMA_SxCR_BIT_EN;
	dmax->Sx[tx_stream].CR &= ~DMA_SxCR_BIT_EN;
	spix->CR2 &= ~(SPI_CR2_BIT_RXDMAEN | SPI_CR2_BIT_TXDMAEN);

#ifndef USE_DMA_INT
	if(dev == &spi1_device) {
		dmax->LIFCR = DMA_LISR_BIT_TCIF2;
	} else if(dev == &spi2_device) {
		dmax->LIFCR = DMA_LISR_BIT_TCIF3;
	} else {
		SYSERR_PRINT("Invalid device(%p)\n", dev);
	}

#endif
	if(dev == &spi1_device) {
		dmax->LIFCR = DMA_LISR_BIT_TCIF3;
	} else if(dev == &spi2_device) {
		dmax->HIFCR = DMA_HISR_BIT_TCIF4;
	} else {
		SYSERR_PRINT("Invalid device(%p)\n", dev);
	}


#ifdef DEBUGKBITS
	{
		int i;
		for(i=0; i<size; i++) {
			DKPRINTF(0x02, "%02X ", rd[i]);
		}
	}
#endif

	if(err != 0) {
		size = -1;
	}

	return size;
}

static int spi_write_cont_word(struct st_device *dev, unsigned short data,
			       long size)
{
	static const unsigned char dummy_data = 0xFF;
	volatile unsigned int wdata = data;

	volatile st_reg_spi *spix = ((struct st_spi_data *)(dev->private_data))->spi;
	volatile st_reg_dma *dmax = ((struct st_spi_data *)(dev->private_data))->dma;
	unsigned int channel = ((struct st_spi_data *)(dev->private_data))->channel;
	int rx_stream = ((struct st_spi_data *)(dev->private_data))->rx_stream;
	int tx_stream = ((struct st_spi_data *)(dev->private_data))->tx_stream;
	unsigned int tmp;
	int rt = 0;

	while(dmax->Sx[tx_stream].CR & DMA_SxCR_BIT_EN) {
		;
	}

	spix->CR1 |= SPI_CR1_BIT_DFF;	// 16 bit data frame

	tmp = dmax->Sx[rx_stream].CR;
	tmp &= ~(DMA_SxCR_BIT_CHSEL_ALL | DMA_SxCR_BIT_MBURST_ALL |
		 DMA_SxCR_BIT_PBURST_ALL | DMA_SxCR_BIT_PL_ALL |
		 DMA_SxCR_BIT_MSIZE_ALL | DMA_SxCR_BIT_PSIZE_ALL |
		 DMA_SxCR_BIT_MINC | DMA_SxCR_BIT_PINC |
		 DMA_SxCR_BIT_CIRC | DMA_SxCR_BIT_DIR_ALL);
	tmp |= ((channel << DMA_SxCR_BIT_CHSEL_SHIFT) |
		DMA_SxCR_BIT_DIR_P2M |
		DMA_SxCR_BIT_PSIZE_WORD |
		DMA_SxCR_BIT_MSIZE_WORD |
		DMA_SxCR_BIT_PL_HIGH |
		DMA_SxCR_BIT_PBURST_SINGLE |
		DMA_SxCR_BIT_MBURST_SINGLE);
	dmax->Sx[rx_stream].CR = tmp;

	tmp = dmax->Sx[rx_stream].FCR;
	tmp &= ~(DMA_SxFCR_BIT_DMDIS | DMA_SxFCR_BIT_FTH_ALL);
	tmp |= DMA_SxFCR_BIT_FTH_FULL;
	tmp = dmax->Sx[rx_stream].FCR = tmp;

	dmax->Sx[rx_stream].NDTR = size * 2;	// WORD = 2 bytes
	dmax->Sx[rx_stream].PAR = (unsigned int)&(spix->DR);
	dmax->Sx[rx_stream].M0AR = (unsigned int)(&dummy_data);

	tmp = dmax->Sx[tx_stream].CR;
	tmp &= ~(DMA_SxCR_BIT_CHSEL_ALL | DMA_SxCR_BIT_MBURST_ALL |
		 DMA_SxCR_BIT_PBURST_ALL | DMA_SxCR_BIT_PL_ALL |
		 DMA_SxCR_BIT_MSIZE_ALL | DMA_SxCR_BIT_PSIZE_ALL |
		 DMA_SxCR_BIT_MINC | DMA_SxCR_BIT_PINC |
		 DMA_SxCR_BIT_CIRC | DMA_SxCR_BIT_DIR_ALL);
	tmp |= ((channel << DMA_SxCR_BIT_CHSEL_SHIFT) |
		DMA_SxCR_BIT_DIR_M2P |
		DMA_SxCR_BIT_PSIZE_WORD |
		DMA_SxCR_BIT_MSIZE_WORD |
		DMA_SxCR_BIT_PL_HIGH |
		DMA_SxCR_BIT_PBURST_SINGLE |
		DMA_SxCR_BIT_MBURST_SINGLE);
	dmax->Sx[tx_stream].CR = tmp;

	tmp = dmax->Sx[tx_stream].FCR;
	tmp &= ~(DMA_SxFCR_BIT_DMDIS | DMA_SxFCR_BIT_FTH_ALL);
	tmp |= DMA_SxFCR_BIT_FTH_FULL;
	dmax->Sx[tx_stream].FCR = tmp;

	dmax->Sx[tx_stream].NDTR = size * 2;	// WORD = 2 bytes
	dmax->Sx[tx_stream].PAR = (unsigned int)&(spix->DR);
	dmax->Sx[tx_stream].M0AR = (unsigned int)&wdata;

	spix->CR2 |= (SPI_CR2_BIT_RXDMAEN | SPI_CR2_BIT_TXDMAEN);

#ifdef USE_DMA_INT
	event_clear(&(((struct st_spi_data *)(dev->private_data))->dma_evq));
#endif
	dmax->Sx[rx_stream].CR |= (DMA_SxCR_BIT_TCIE | DMA_SxCR_BIT_TEIE);
	dmax->Sx[rx_stream].CR |= DMA_SxCR_BIT_EN;
	dmax->Sx[tx_stream].CR |= DMA_SxCR_BIT_EN;

#ifdef USE_DMA_INT
	rt = event_wait(&(((struct st_spi_data *)(dev->private_data))->dma_evq),
			0, SPI_WAIT_TIME);
	if(rt == 0) {
		SYSERR_PRINT("SPI DMA timeout(%d)\n", rt);
	}
#else
	if(dev == &spi1_device) {
		while((dmax->LISR & DMA_LISR_BIT_TCIF3) == 0) {
			;
		}
	} else if(dev == &spi2_device) {
		while((dmax->HISR & DMA_HISR_BIT_TCIF4) == 0) {
			;
		}
	} else {
		SYSERR_PRINT("Invalid device(%p)\n", dev);
	}

#endif

	dmax->Sx[rx_stream].CR &= ~DMA_SxCR_BIT_EN;
	dmax->Sx[tx_stream].CR &= ~DMA_SxCR_BIT_EN;
	spix->CR2 &= ~(SPI_CR2_BIT_RXDMAEN | SPI_CR2_BIT_TXDMAEN);

#ifndef USE_DMA_INT
	if(dev == &spi1_device) {
		dmax->LIFCR = DMA_LISR_BIT_TCIF2;
	} else if(dev == &spi2_device) {
		dmax->LIFCR = DMA_LISR_BIT_TCIF3;
	}
#endif
	if(dev == &spi1_device) {
		dmax->LIFCR = DMA_LISR_BIT_TCIF3;
	} else if(dev == &spi2_device) {
		dmax->HIFCR = DMA_HISR_BIT_TCIF4;
	} else {
		SYSERR_PRINT("Invalid device(%p)\n", dev);
	}

	return size;
}
#endif

static int spi_writebyte(struct st_device *dev, unsigned char data)
{
	volatile st_reg_spi *spix = ((struct st_spi_data *)(dev->private_data))->spi;
	int i = SPI_WAIT_TIME;
	volatile unsigned short tmp;

	spix->CR1 &= ~SPI_CR1_BIT_DFF;	// 8 bit data frame

	spix->DR = data;

	while((spix->SR & SPI_SR_BIT_TXE) == 0) {
		i --;
		if(i == 0) {
			SYSERR_PRINT("TXE timeout\n");
			break;
		}
	}

	while((spix->SR & SPI_SR_BIT_RXNE) == 0) {
		i --;
		if(i == 0) {
			SYSERR_PRINT("RXNE timeout\n");
			break;
		}
	}

	while(spix->SR & SPI_SR_BIT_BSY) {
		i --;
		if(i == 0) {
			SYSERR_PRINT("BSY timeout\n");
			break;
		}
	}

	tmp = spix->DR;
	(void)tmp;

	return 1;
}

static int spi_writeword(struct st_device *dev, unsigned short data)
{
	volatile st_reg_spi *spix = ((struct st_spi_data *)(dev->private_data))->spi;
	int i = SPI_WAIT_TIME;
	volatile unsigned short tmp;

	spix->CR1 |= SPI_CR1_BIT_DFF;	// 16 bit data frame

	spix->DR = data;

	while((spix->SR & SPI_SR_BIT_TXE) == 0) {
		i --;
		if(i == 0) {
			SYSERR_PRINT("TXE timeout\n");
			break;
		}
	}

	while((spix->SR & SPI_SR_BIT_RXNE) == 0) {
		i --;
		if(i == 0) {
			SYSERR_PRINT("RXNE timeout\n");
			break;
		}
	}

	while(spix->SR & SPI_SR_BIT_BSY) {
		i --;
		if(i == 0) {
			SYSERR_PRINT("BSY timeout\n");
			break;
		}
	}

	tmp = spix->DR;
	(void)tmp;

	return 1;
}

/*
 *
 */

static int spi_register(struct st_device *dev, char *param)
{
	if(init_driver(dev)) {
		return -1;
	}

	return 0;
}

static int spi_getc(struct st_device *dev, unsigned char *rd)
{
	int rtn;

	lock_spi(dev);

	rtn = spi_readbyte(dev, rd);

	unlock_spi(dev);

	return rtn;
}

static int spi_read(struct st_device *dev, void *data, unsigned int size)
{
	int len = 0;
	int rtn = 0;

	lock_spi(dev);

#ifdef USE_DMA
	rtn = spi_transblock(dev, data, size, 1);
	len = size;
#else
	while(size) {
		long rtn = spi_readbyte(dev, data);
		if(rtn == 0) {
			break;
		}
		DKPRINTF(0x02, "%02X ", *data);
		data ++;
		size --;
		len ++;
	}
#endif

	unlock_spi(dev);

	if(rtn < 0) {
		len = -1;
	}

	return len;
}

static int spi_putc(struct st_device *dev, unsigned char ch)
{
	int rtn;

	lock_spi(dev);

	rtn = spi_writebyte(dev, ch);

	unlock_spi(dev);

	return rtn;
}

static int spi_write(struct st_device *dev, const void *data, unsigned int size)
{
	int len = 0;
	int rtn = 0;

	lock_spi(dev);

#ifdef USE_DMA
	rtn = spi_transblock(dev, (unsigned char *)data, size, 0);
	len = size;
#else
	while(size) {
		long rtn = spi_writebyte(dev, *data);
		if(rtn == 0) {
			break;
		}
		data ++;
		size --;
		len ++;
	}
#endif

	unlock_spi(dev);

	if(rtn < 0) {
		len = -1;
	}

	return len;
}

static int spi_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	volatile st_reg_spi *spix = ((struct st_spi_data *)(dev->private_data))->spi;
	int rtn = 0;
	int br = 7;

	lock_spi(dev);

	switch(com) {
	case IOCMD_SPI_SPEED:	// 転送速度を設定する
		DKFPRINTF(0x01, "arg = %ld\n", arg);
		if(CLK_FP2 <= arg) {
			br = 0;
			DKPRINTF(0x01, "BR = %d(SPEED = %d bps)\n", br, (int)CLK_FP2);
		} else
		if((CLK_FP4 <= arg) && (arg < CLK_FP2)) {
			br = 1;
			DKPRINTF(0x01, "BR = %d(SPEED = %d bps)\n", br, (int)CLK_FP4);
		} else
		if((CLK_FP8 <= arg) && (arg < CLK_FP4)) {
			br = 2;
			DKPRINTF(0x01, "BR = %d(SPEED = %d bps)\n", br, (int)CLK_FP8);
		} else
		if((CLK_FP16 <= arg) && (arg < CLK_FP8)) {
			br = 3;
			DKPRINTF(0x01, "BR = %d(SPEED = %d bps)\n", br, (int)CLK_FP16);
		} else
		if((CLK_FP32 <= arg) && (arg < CLK_FP16)) {
			br = 4;
			DKPRINTF(0x01, "BR = %d(SPEED = %d bps)\n", br, (int)CLK_FP32);
		} else
		if((CLK_FP64 <= arg) && (arg < CLK_FP32)) {
			br = 5;
			DKPRINTF(0x01, "BR = %d(SPEED = %d bps)\n", br, (int)CLK_FP64);
		} else
		if((CLK_FP128 <= arg) && (arg < CLK_FP64)) {
			br = 6;
			DKPRINTF(0x01, "BR = %d(SPEED = %d bps)\n", br, (int)CLK_FP128);
		} else
		if(arg < CLK_FP128) {
			br = 7;
			DKPRINTF(0x01, "BR = %d(SPEED = %d bps)\n", br, (int)CLK_FP256);
		}

		spix->CR1 &= ~SPI_CR1_BIT_SPE;
		spix->CR1 = ((spix->CR1 & ~SPI_CR1_BIT_BR_256) |
			    (br<<3)); // fcpu/br
		spix->CR1 |= SPI_CR1_BIT_SPE;
		DKPRINTF(0x01, "SPIx_CR1 = %08X\n", (int)spix->CR1);
		break;

	case IOCMD_SPI_FORCE_UNLOCK:	// 強制的にアンロック
		unlock_spi(dev);
		break;

	case IOCMD_SPI_WRITE_WORD:
		rtn = spi_writeword(dev, arg & 0xffff);
		break;

	case IOCMD_SPI_WRITE_CONT_WORD:
		rtn = spi_write_cont_word(dev, arg & 0xffff,
					  (arg >> 16) & 0xffff);
		break;

	default:
		SYSERR_PRINT("Unknown ioctl(%08lX)\n", com);
		break;
	}

	unlock_spi(dev);

	return rtn;
}

static int spi_suspend(struct st_device *dev)
{
	return 0;
}

static int spi_resume(struct st_device *dev)
{
	init_spi(dev);

	return 0;
}

const struct st_device spi1_device = {
	.name		= DEF_DEV_NAME_SPI,
	.explan		= "STM32F4 SPI1",
	.private_data	= (void *)&spi_data[0],
	.register_dev	= spi_register,
	.read		= spi_read,
	.getc		= spi_getc,
	.write		= spi_write,
	.putc		= spi_putc,
	.ioctl		= spi_ioctl,
	.suspend	= spi_suspend,
	.resume		= spi_resume,
};

const struct st_device spi2_device = {
	.name		= DEF_DEV_NAME_SPI "1",
	.explan		= "STM32F4 SPI2",
	.private_data	= (void *)&spi_data[1],
	.register_dev	= spi_register,
	.read		= spi_read,
	.getc		= spi_getc,
	.write		= spi_write,
	.putc		= spi_putc,
	.ioctl		= spi_ioctl,
	.suspend	= spi_suspend,
	.resume		= spi_resume,
};
