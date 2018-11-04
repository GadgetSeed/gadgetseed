/** @file
    @brief	STM32F7 シリアル(UARTドライバ)

    @date	2017.01.03
    @author	Takashi SHUDO

    STM32F769I-Discovery
      UART1_TX	PA9
      UART1_RX	PA10

      UART6_TX	PC6
      UART6_RX	PC7

    STM32F756G-Discovery
      UART1_TX	PA9
      UART1_RX	PB7

      UART6_TX	PC6
      UART6_RX	PC7
*/

#include "sysconfig.h"
#include "device.h"
#include "interrupt.h"
#include "fifo.h"
#include "tkprintf.h"
#include "device/uart_ioctl.h"
#include "task/event.h"
#include "task/syscall.h"

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_uart.h"
#include "stm32f7xx_hal_uart_ex.h"

#if defined(GSC_TARGET_SYSTEM_STM32F769IDISCOVERY)
 #define USART_TxPin	GPIO_PIN_9
 #define USART_RxPin	GPIO_PIN_10
 #define USART_TxPort	GPIOA
 #define USART_RxPort	GPIOA
#elif defined(GSC_TARGET_SYSTEM_STM32F746GDISCOVERY)
 #define USART_TxPin	GPIO_PIN_9
 #define USART_RxPin	GPIO_PIN_7
 #define USART_TxPort	GPIOA
 #define USART_RxPort	GPIOB
#endif

static void init_rcc_uart1(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
#if defined(GSC_TARGET_SYSTEM_STM32F746GDISCOVERY)
	__HAL_RCC_GPIOB_CLK_ENABLE();
#endif
}

static void init_rcc_uart6(void)
{
	__HAL_RCC_GPIOC_CLK_ENABLE();
}

static void init_gpio_uart1(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_USART1_CLK_ENABLE();

	GPIO_InitStruct.Pin = USART_TxPin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
	HAL_GPIO_Init(USART_TxPort, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = USART_RxPin;
	HAL_GPIO_Init(USART_RxPort, &GPIO_InitStruct);

	/* Peripheral interrupt init */
	HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART1_IRQn);
}

static void init_gpio_uart6(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_USART6_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* Peripheral interrupt init */
	HAL_NVIC_SetPriority(USART6_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART6_IRQn);
}

#define	MAXBUFSIZE	256

struct st_uart_data {
	struct st_fifo rfifo;
	unsigned char rbuf[MAXBUFSIZE];

	unsigned char tbuf[2];

	struct st_event rx_evq;
	struct st_event tx_evq;

	UART_HandleTypeDef huart;
};

const struct st_device uart1_device;
const struct st_device uart6_device;

static struct st_uart_data uart_data[3];

static void init_uart(struct st_uart_data *uart_dt, USART_TypeDef *uart, int speed)
{
	UART_HandleTypeDef *huart = &(uart_dt->huart);

	huart->Instance = uart;
	huart->Init.BaudRate = speed;
	huart->Init.WordLength = UART_WORDLENGTH_8B;
	huart->Init.StopBits = UART_STOPBITS_1;
	huart->Init.Parity = UART_PARITY_NONE;
	huart->Init.Mode = UART_MODE_TX_RX;
	huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart->Init.OverSampling = UART_OVERSAMPLING_16;
	huart->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if(HAL_UART_Init(huart) != HAL_OK) {
		SYSERR_PRINT("HAL UART Init error.\n");
	}

	__HAL_UART_ENABLE_IT(huart, UART_IT_RXNE);
}

void inthdr_uart(unsigned int intnum, void *sp)
{
	unsigned char rd;
	struct st_uart_data *uart_dt;
	UART_HandleTypeDef *huart;
	USART_TypeDef *uart;

	//tkprintf("UART int(%ld)\n", intnum);
	if(intnum == IRQ2VECT(USART1_IRQn)) {
		uart_dt = &uart_data[0];
	} else if(intnum == IRQ2VECT(USART6_IRQn)) {
		uart_dt = &uart_data[1];
	} else {
		return;
	}
	huart = &(uart_dt->huart);
	uart = huart->Instance;

	if(uart->ISR & USART_ISR_RXNE) {
		uart->ICR = USART_ICR_ORECF | USART_ICR_NCF | USART_ICR_FECF | USART_ICR_PECF;
		rd = uart->RDR;
		if(write_fifo(&(uart_dt->rfifo), &rd, 1) == 0) {	// FIFOへ格納
			// SYSERR_PRINT("FIFO Over Flow\n");
		}

#if 0	// DEBUG
		if(uart_dt == &uart_data[1]) {
			tkprintf("%c", rd);
		}
#endif
		event_wakeup_ISR(sp, &(uart_dt->rx_evq), 0);
	}

	if(uart->ISR & USART_ISR_PE) {
		uart->ISR &= ~(USART_ISR_PE);
		// SYSERR_PRINT("USART Parity Error\n");
	}

	if(uart->ISR & USART_ISR_FE) {
		uart->ISR &= ~(USART_ISR_FE);
		// SYSERR_PRINT("USART Framing Error\n");
	}

	if(uart->ISR & USART_ISR_NE) {
		uart->ISR &= ~(USART_ISR_NE);
		// SYSERR_PRINT("USART Noise Error\n");
	}

	if(uart->ISR & USART_ISR_ORE) {
		uart->ISR &= ~(USART_ISR_ORE);
		// SYSERR_PRINT("USART OverRun Error\n");
	}

	if(uart->ISR & USART_ISR_TXE) {
		uart->CR1 &= ~(USART_CR1_TXEIE);
		event_wakeup_ISR(sp, &(uart_dt->tx_evq), 0);
	}
}

/*
  @brief	USARTを初期化する
  8bit, stop bit 1, パリティ無し, 調歩同期
*/
const static char uart_rx_eventqueue_name[2][10] = {
	"uart1_rx", "uart6_rx"
};

const static char uart_tx_eventqueue_name[2][10] = {
	"uart1_tx", "uart6_tx"
};

static int uart_init(struct st_device *dev, char *param)
{
	struct st_uart_data *uart_data = (dev->private_data);

	init_fifo(&(uart_data->rfifo), uart_data->rbuf, MAXBUFSIZE);

	if(dev == &uart1_device) {
		eventqueue_register_ISR(&(uart_data->rx_evq),
					uart_rx_eventqueue_name[0], 0, 0, 0);
		eventqueue_register_ISR(&(uart_data->tx_evq),
					uart_tx_eventqueue_name[0],
					uart_data->tbuf, sizeof(unsigned char), 2);
		register_interrupt(IRQ2VECT(USART1_IRQn), inthdr_uart);

		init_rcc_uart1();
		init_gpio_uart1();
		init_uart(uart_data, USART1, 115200);
	} else if(dev == &uart6_device) {
		eventqueue_register_ISR(&(uart_data->rx_evq),
					uart_rx_eventqueue_name[1], 0, 0, 0);
		eventqueue_register_ISR(&(uart_data->tx_evq),
					uart_tx_eventqueue_name[1],
					uart_data->tbuf, sizeof(unsigned char), 2);
		register_interrupt(IRQ2VECT(USART6_IRQn), inthdr_uart);

		init_rcc_uart6();
		init_gpio_uart6();
		init_uart(uart_data, USART6, 115200);
	} else {
		return -1;
	}

	return 0;
}

/*
  @brief	UARTからの受信データを取得する
  @param[out]	rd	受信データポインタ
  @return	!=0:データあり 0:データ無し
*/
static int uart_getc(struct st_device *dev, unsigned char *rd)
{
	return read_fifo(&(((struct st_uart_data *)(dev->private_data))->rfifo), rd, 1);
}

/*
  @brief	UART出力
  @param[in]	td	送信データ
*/
#define UART_TC_TIMEOUT	100000
#define UART_TE_TIMEOUT	1000
//#define UART_TE_TIMEOUT	0

static int uart_putc(struct st_device *dev, unsigned char td)
{
	int timeout = UART_TC_TIMEOUT;
	USART_TypeDef *uart = ((struct st_uart_data *)(dev->private_data))->huart.Instance;

	while(!(uart->ISR & USART_ISR_TC)) {
		timeout --;
		if(timeout == 0) {
			SYSERR_PRINT("UART TC timeout\n");
			break;
		}
	}
	uart->TDR = td;

#if 1	// 送信完了割り込み使用
	uart->CR1 |= USART_CR1_TXEIE;	// 送信データエンプティ割り込み許可

	if(event_wait(&(((struct st_uart_data *)(dev->private_data))->tx_evq), 0, UART_TE_TIMEOUT) < 0) {
		if(UART_TE_TIMEOUT != 0) {
			SYSERR_PRINT("UART TXE timeout\n");
		}
	}
#endif

	return 1;
}

static int uart_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	struct st_uart_data *uart_data = (dev->private_data);

	switch(com) {
	case IOCMD_UART_SPEED:
		{
			UART_HandleTypeDef *huart = &(uart_data->huart);
			init_uart(uart_data, huart->Instance, arg);
		}
		break;

	default:
		SYSERR_PRINT("Unknown ioctl(%08lX)\n", com);
		return -1;
		break;
	}

	return 0;
}

static int uart_select(struct st_device *dev, unsigned int timeout)
{
	if(fifo_size(&(((struct st_uart_data *)(dev->private_data))->rfifo)) != 0) {
		return timeout;
	} else {
		return event_wait(&(((struct st_uart_data *)(dev->private_data))->rx_evq),
				  0, timeout);
	}
}


/*
  uart1_low
*/

static UART_HandleTypeDef huart_low;

/*
  @brief	UARTを初期化する
*/
static int uart_init_low(struct st_device *dev, char *param)
{
	init_rcc_uart1();
	init_gpio_uart1();

	huart_low.Instance = USART1;
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
  @brief	UARTからの受信データを取得する(割り込み無し)
  @param[out]	rd	受信データポインタ
  @return	!=0:データあり 0:データ無し
*/
static int uart_getc_low(struct st_device *dev, unsigned char *rd)
{
	USART_TypeDef *uart = huart_low.Instance;

	if(uart->ISR & USART_ISR_RXNE) {
		uart->ISR &= ~(USART_ISR_RXNE);
		*rd = uart->RDR;
		return 1;
	} else {
		return 0;
	}
}

/*
  @brief	UART出力(割り込み無し)
  @param[in]	td	送信データ
*/
static int uart_putc_low(struct st_device *dev, unsigned char td)
{
	int timeout = UART_TC_TIMEOUT;
	USART_TypeDef *uart = huart_low.Instance;

	while(!(uart->ISR & USART_ISR_TC)) {
		timeout --;
		if(timeout == 0) {
			break;
		}
	}
	uart->TDR = td;

#if 1 // 送信完了待ち
	while(!(uart->ISR & USART_ISR_TC)) {
		timeout --;
		if(timeout == 0) {
			break;
		}
	}
#endif
	return 1;
}

const struct st_device uart1_device = {
	.name	= DEF_DEV_NAME_UART,
	.explan	= "STM32F7 UART1",
	.private_data = (void *)&uart_data[0],
	.register_dev = uart_init,
	.getc	= uart_getc,
	.putc	= uart_putc,
	.ioctl	= uart_ioctl,
	.select	= uart_select
};

const struct st_device uart6_device = {
	.name	= DEF_DEV_NAME_UART "1",
	.explan	= "STM32F7 UART6",
	.private_data = (void *)&uart_data[1],
	.register_dev = uart_init,
	.getc	= uart_getc,
	.putc	= uart_putc,
	.ioctl	= uart_ioctl,
	.select	= uart_select
};

const struct st_device uart1_low_device = {
	.name	= DEF_DEV_NAME_DEBUG,
	.explan	= "Debug/Error Console",
	.private_data = (void *)&uart_data[2],
	.register_dev = uart_init_low,
	.getc	= uart_getc_low,
	.putc	= uart_putc_low
};
