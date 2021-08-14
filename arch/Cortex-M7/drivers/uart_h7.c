/** @file
    @brief	STM32H7 シリアル(UARTドライバ)

    @date	2019.01.19
    @author	Takashi SHUDO

    STM32H747I-Discovery
      UART1_TX	PA9
      UART1_RX	PA10

    STM32F769I-Discovery
      UART1_TX	PA9
      UART1_RX	PA10

      UART6_TX	PC6
      UART6_RX	PC7

    STM32F746G-Discovery
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

//#define USE_UART2	// [TODO] Cannot USE


#ifdef GSC_TARGET_SYSTEM_STM32H747IDISCOVERY
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_uart.h"
#include "stm32h7xx_hal_uart_ex.h"
#else
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_uart.h"
#include "stm32f7xx_hal_uart_ex.h"
#endif

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
#elif defined(GSC_TARGET_SYSTEM_STM32H747IDISCOVERY)
 #define USART_TxPin	GPIO_PIN_9
 #define USART_RxPin	GPIO_PIN_10
 #define USART_TxPort	GPIOA
 #define USART_RxPort	GPIOA
#endif

static void init_rcc_uart1(void)
{
	__HAL_RCC_GPIOA_CLK_ENABLE();
#if defined(GSC_TARGET_SYSTEM_STM32F746GDISCOVERY)
	__HAL_RCC_GPIOB_CLK_ENABLE();
#endif
}

#ifdef USE_UART2
static void init_rcc_uart6(void)
{
	__HAL_RCC_GPIOC_CLK_ENABLE();
}
#endif

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

#ifdef USE_UART2
static void init_gpio_uart6(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_USART6_CLK_ENABLE();

	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART6;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/* Peripheral interrupt init */
	HAL_NVIC_SetPriority(USART6_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(USART6_IRQn);
}
#endif

#define	MAXBUFSIZE	256

struct st_uart_data {
	unsigned char rdata;
	struct st_fifo rfifo;
	unsigned char rbuf[MAXBUFSIZE];

	unsigned char tbuf[2];

	struct st_event rx_evq;
	struct st_event tx_evq;

	UART_HandleTypeDef huart;

	unsigned char flg_txc;
	unsigned char flg_rxc;
};

const struct st_device uart1_device;
#ifdef USE_UART2
const struct st_device uart6_device;
#endif

#ifdef USE_UART2
static struct st_uart_data uart_data[3];
#else
static struct st_uart_data uart_data[2];
#endif


void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &(uart_data[0].huart)) {
		uart_data[0].flg_txc = 1;
	}
#ifdef USE_UART2
	else if(huart == &(uart_data[2].huart)) {
		uart_data[2].flg_txc = 1;
	}
#endif
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart == &(uart_data[0].huart)) {
		uart_data[0].flg_rxc = 1;
	}
#ifdef USE_UART2
	else if(huart == &(uart_data[2].huart)) {
		uart_data[2].flg_rxc = 1;
	}
#endif
}

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
	struct st_uart_data *uart_dt;
	UART_HandleTypeDef *huart;

	//tkprintf("UART int(%ld)\n", intnum);
	if(intnum == IRQ2VECT(USART1_IRQn)) {
		uart_dt = &uart_data[0];
	}
#ifdef USE_UART2
	else if(intnum == IRQ2VECT(USART6_IRQn)) {
		uart_dt = &uart_data[2];
	}
#endif
	else {
		return;
	}
	huart = &(uart_dt->huart);

	HAL_UART_IRQHandler(huart);

	if(uart_dt->flg_txc != 0) {
		uart_dt->flg_txc = 0;
		if(uart_dt->flg_rxc == 0) {
			event_wakeup_ISR(sp, &(uart_dt->tx_evq), 0);
		} else {
			event_push_ISR(sp, &(uart_dt->tx_evq), 0);
		}
	}

	if(uart_dt->flg_rxc != 0) {
		uart_dt->flg_rxc = 0;
		if(write_fifo(&(uart_dt->rfifo), &(uart_dt->rdata), 1) == 0) {	// FIFOへ格納
			// SYSERR_PRINT("FIFO Over Flow\n");
		}
		HAL_UART_Receive_IT(&(uart_data->huart), &(uart_dt->rdata), 1);
		event_wakeup_ISR(sp, &(uart_dt->rx_evq), 0);
	}

	return;
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

		HAL_UART_Receive_IT(&(uart_data->huart), &(uart_data->rdata), 1);
	}
#ifdef USE_UART2
	else if(dev == &uart6_device) {
		eventqueue_register_ISR(&(uart_data->rx_evq),
					uart_rx_eventqueue_name[1], 0, 0, 0);
		eventqueue_register_ISR(&(uart_data->tx_evq),
					uart_tx_eventqueue_name[1],
					uart_data->tbuf, sizeof(unsigned char), 2);
		register_interrupt(IRQ2VECT(USART6_IRQn), inthdr_uart);

		init_rcc_uart6();
		init_gpio_uart6();
		init_uart(uart_data, USART6, 115200);
	}
#endif
	else {
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
#define UART_TE_TIMEOUT	1000
//#define UART_TE_TIMEOUT	0

static int uart_putc(struct st_device *dev, unsigned char td)
{
	struct st_uart_data *uart_data = (dev->private_data);
	UART_HandleTypeDef *huart;
	HAL_StatusTypeDef res = HAL_BUSY;

	huart = &(uart_data->huart);

	while(res == HAL_BUSY) {
		res = HAL_UART_Transmit_IT(huart, &td, 1);
	}

	if(event_wait(&(((struct st_uart_data *)(dev->private_data))->tx_evq), 0, UART_TE_TIMEOUT) < 0) {
		if(UART_TE_TIMEOUT != 0) {
			SYSERR_PRINT("UART TXE timeout\n");
		}
	}

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
		SYSERR_PRINT("Unknown ioctl(%08X)\n", com);
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
		return event_wait(&(((struct st_uart_data *)(dev->private_data))->rx_evq), 0, timeout);
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
	HAL_StatusTypeDef res = HAL_BUSY;

	res = HAL_UART_Receive(&huart_low, rd, 1, 0);
	if(res == HAL_OK) {
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
	HAL_StatusTypeDef res = HAL_BUSY;
	int count = 0;

	while(res == HAL_BUSY) {
		res = HAL_UART_Transmit(&huart_low, &td, 1, UART_TE_TIMEOUT);
		count ++;
		if(count > UART_TE_TIMEOUT) {
			break;
		}
	}

	return 1;
}

const struct st_device uart1_device = {
	.name	= DEF_DEV_NAME_UART,
	.explan	= "STM32H7 UART1",
	.private_data = (void *)&uart_data[0],
	.register_dev = uart_init,
	.getc	= uart_getc,
	.putc	= uart_putc,
	.ioctl	= uart_ioctl,
	.select	= uart_select
};

const struct st_device uart1_low_device = {
	.name	= DEF_DEV_NAME_DEBUG,
	.explan	= "Debug/Error Console",
	.private_data = (void *)&uart_data[1],
	.register_dev = uart_init_low,
	.getc	= uart_getc_low,
	.putc	= uart_putc_low
};

#ifdef USE_UART2
const struct st_device uart6_device = {
	.name	= DEF_DEV_NAME_UART "1",
	.explan	= "STM32H7 UART6",
	.private_data = (void *)&uart_data[2],
	.register_dev = uart_init,
	.getc	= uart_getc,
	.putc	= uart_putc,
	.ioctl	= uart_ioctl,
	.select	= uart_select
};
#endif
