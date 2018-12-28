/** @file
    @brief	STM32L152 シリアル(USARTドライバ)

    @date	2018.08.12
    @author	Takashi SHUDO

    USART2_TX	PA2
    USART2_RX	PA3

    USART4_TX	PC10
    USART4_RX	PC11
*/

#include "device.h"
#include "interrupt.h"
#include "fifo.h"
#include "tkprintf.h"
#include "device/uart_ioctl.h"
#include "task/event.h"
#include "task/syscall.h"

#include "stm32l1xx_hal.h"

static void init_rcc_usart2(void)
{
	__GPIOA_CLK_ENABLE();
}

static void init_rcc_usart4(void)
{
	__GPIOC_CLK_ENABLE();
}

static void init_gpio_usart2(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__USART2_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static void init_gpio_usart4(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__USART4_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

#define	MAXBUFSIZE	256

typedef struct st_usart_data {
	struct st_fifo rfifo;
	unsigned char rbuf[MAXBUFSIZE];

	unsigned char tbuf[2];

	struct st_event rx_evq;
	struct st_event tx_evq;

	UART_HandleTypeDef huart;
} st_usart_data;

const struct st_device usart2_device;
const struct st_device usart4_device;

static st_usart_data usart_data[3];

static void init_usart(st_usart_data *usart_dt, USART_TypeDef *usart, int irq)
{
	UART_HandleTypeDef *huart = &(usart_dt->huart);

	huart->Instance = usart;
	huart->Init.BaudRate = 115200;
	huart->Init.WordLength = UART_WORDLENGTH_8B;
	huart->Init.StopBits = UART_STOPBITS_1;
	huart->Init.Parity = UART_PARITY_NONE;
	huart->Init.Mode = UART_MODE_TX_RX;
	huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart->Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(huart);

	HAL_NVIC_EnableIRQ(irq);
	__HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
}

void inthdr_usart(unsigned int intnum, void *sp)
{
	unsigned char rd;
	st_usart_data *usart_dt;
	UART_HandleTypeDef *huart;
	USART_TypeDef *usart;

	//tkprintf("USART int(%ld)\n", intnum);
	if(intnum == IRQ2VECT(USART2_IRQn)) {
		usart_dt = &usart_data[0];
	} else if(intnum == IRQ2VECT(UART4_IRQn)) {
		usart_dt = &usart_data[1];
	} else {
		return;
	}
	huart = &(usart_dt->huart);
	usart = huart->Instance;

	if(usart->SR & USART_SR_RXNE) {
		usart->SR &= ~(USART_SR_RXNE);
		rd = usart->DR;
		if(write_fifo(&(usart_dt->rfifo), &rd, 1) != 0) {	// FIFOへ格納
			// SYSERR_PRINT("FIFO Over Flow\n");
		}

#if 0	// DEBUG
		if(usart_dt == &usart_data[1]) {
			tkprintf("%c", rd);
		}
#endif
		event_wakeup_ISR(sp, &(usart_dt->rx_evq), 0);
	}

	if(usart->SR & USART_SR_PE) {
		usart->SR &= ~(USART_SR_PE);
		// SYSERR_PRINT("USART Parity Error\n");
	}

	if(usart->SR & USART_SR_FE) {
		usart->SR &= ~(USART_SR_FE);
		// SYSERR_PRINT("USART Framing Error\n");
	}

	if(usart->SR & USART_SR_NE) {
		usart->SR &= ~(USART_SR_NE);
		// SYSERR_PRINT("USART Noise Error\n");
	}

	if(usart->SR & USART_SR_ORE) {
		usart->SR &= ~(USART_SR_ORE);
		// SYSERR_PRINT("USART OverRun Error\n");
	}

	if(usart->SR & USART_SR_TXE) {
		usart->CR1 &= ~(USART_CR1_TXEIE);
		event_wakeup_ISR(sp, &(usart_dt->tx_evq), 0);
	}
}

const static char usart_rx_eventqueue_name[2][10] = {
	"usart2_rx", "usart4_rx"
};

const static char usart_tx_eventqueue_name[2][10] = {
	"usart2_tx", "usart4_tx"
};

/*
  USARTを初期化する

  8bit, stop bit 1, パリティ無し, 調歩同期
*/
static int usart_init(struct st_device *dev, char *param)
{
	st_usart_data *usart_data = (dev->private_data);

	init_fifo(&(usart_data->rfifo), usart_data->rbuf, MAXBUFSIZE);

	if(dev == &usart2_device) {
		eventqueue_register_ISR(&(usart_data->rx_evq),
					usart_rx_eventqueue_name[0], 0, 0, 0);
		eventqueue_register_ISR(&(usart_data->tx_evq),
					usart_tx_eventqueue_name[0],
					usart_data->tbuf, sizeof(unsigned char), 2);
		NVIC_SetPriority(USART2_IRQn, 0);
		register_interrupt(IRQ2VECT(USART2_IRQn), inthdr_usart);

		init_rcc_usart2();
		init_gpio_usart2();
		init_usart(usart_data, USART2, USART2_IRQn);
	} else if(dev == &usart4_device) {
		eventqueue_register_ISR(&(usart_data->rx_evq),
					usart_rx_eventqueue_name[1], 0, 0, 0);
		eventqueue_register_ISR(&(usart_data->tx_evq),
					usart_tx_eventqueue_name[1],
					usart_data->tbuf, sizeof(unsigned char), 2);
		NVIC_SetPriority(UART4_IRQn, 0);
		register_interrupt(IRQ2VECT(UART4_IRQn), inthdr_usart);

		init_rcc_usart4();
		init_gpio_usart4();
		init_usart(usart_data, UART4, UART4_IRQn);
	} else {
		return -1;
	}

	return 0;
}

/*
  USARTからの受信データを取得する
*/
static int usart_getc(struct st_device *dev, unsigned char *rd)
{
	return read_fifo(&(((st_usart_data *)(dev->private_data))->rfifo), rd, 1);
}

#define USART_TC_TIMEOUT	100000
#define USART_TE_TIMEOUT	1000
//#define USART_TE_TIMEOUT	0

/*
  USART出力
*/
static int usart_putc(struct st_device *dev, unsigned char td)
{
	int timeout = USART_TC_TIMEOUT;
	USART_TypeDef *usart = ((st_usart_data *)(dev->private_data))->huart.Instance;

	while(!(usart->SR & USART_SR_TC)) {
		timeout --;
		if(timeout == 0) {
			SYSERR_PRINT("USART TC timeout\n");
			break;
		}
	}
	usart->DR = td;

#if 1	// 送信完了割り込み使用
	usart->CR1 |= USART_CR1_TXEIE;	// 送信データエンプティ割り込み許可

	if(event_wait(&(((st_usart_data *)(dev->private_data))->tx_evq), 0, USART_TE_TIMEOUT) < 0) {
		if(USART_TE_TIMEOUT != 0) {
			SYSERR_PRINT("USART TXE timeout\n");
		}
	}
#endif

	return 1;
}

static int usart_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	UART_HandleTypeDef *huart = &(((st_usart_data *)(dev->private_data))->huart);

	switch(com) {
	case IOCMD_UART_SPEED:
		HAL_UART_DeInit(huart);
		huart->Init.BaudRate = arg;
		HAL_UART_Init(huart);
		break;

	default:
		SYSERR_PRINT("Unknown ioctl(%08X)\n", com);
		return -1;
		break;
	}

	return 0;
}

static int usart_select(struct st_device *dev, unsigned int timeout)
{
	if(fifo_size(&(((st_usart_data *)(dev->private_data))->rfifo)) != 0) {
		return timeout;
	} else {
		return event_wait(&(((st_usart_data *)(dev->private_data))->rx_evq),
				  0, timeout);
	}
}


/*
  usart2_low
*/

static UART_HandleTypeDef huart_low;

/*
  USARTを初期化する
*/
static int usart_init_low(struct st_device *dev, char *param)
{
	init_rcc_usart2();
	init_gpio_usart2();

	huart_low.Instance = USART2;
	huart_low.Init.BaudRate = 115200;
	huart_low.Init.WordLength = UART_WORDLENGTH_8B;
	huart_low.Init.StopBits = UART_STOPBITS_1;
	huart_low.Init.Parity = UART_PARITY_NONE;
	huart_low.Init.Mode = UART_MODE_TX_RX;
	huart_low.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart_low.Init.OverSampling = UART_OVERSAMPLING_16;
	HAL_UART_Init(&huart_low);

	return 0;
}

/*
  USARTからの受信データを取得する(割り込み無し)
*/
static int usart_getc_low(struct st_device *dev, unsigned char *rd)
{
	USART_TypeDef *usart = huart_low.Instance;

	if(usart->SR & USART_SR_RXNE) {
		usart->SR &= ~(USART_SR_RXNE);
		*rd = usart->DR;
		return 1;
	} else {
		return 0;
	}
}

/*
  USART出力(割り込み無し)
*/
static int usart_putc_low(struct st_device *dev, unsigned char td)
{
	int timeout = USART_TC_TIMEOUT;
	USART_TypeDef *usart = huart_low.Instance;

	while(!(usart->SR & USART_SR_TC)) {
		timeout --;
		if(timeout == 0) {
			break;
		}
	}
	usart->DR = td;

	return 1;
}

const struct st_device usart2_device = {
	.name	= DEF_DEV_NAME_UART,
	.explan	= "STM32L1 USART2",
	.private_data = (void *)&usart_data[0],
	.register_dev = usart_init,
	.getc = usart_getc,
	.putc = usart_putc,
	.ioctl = usart_ioctl,
	.select	= usart_select
};

const struct st_device usart4_device = {
	.name	= DEF_DEV_NAME_UART "1",
	.explan	= "STM32L1 USART4",
	.private_data = (void *)&usart_data[1],
	.register_dev = usart_init,
	.getc = usart_getc,
	.putc = usart_putc,
	.ioctl = usart_ioctl,
	.select	= usart_select
};

const struct st_device usart2_low_device = {
	.name	= DEF_DEV_NAME_DEBUG,
	.explan	= "Debug/Error Console",
	.private_data = (void *)&usart_data[2],
	.register_dev = usart_init_low,
	.getc = usart_getc_low,
	.putc = usart_putc_low
};
