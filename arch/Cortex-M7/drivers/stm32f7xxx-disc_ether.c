/** @file
    @brief	STM32F7XXX Discovery Ether

    @date	2017.02.05
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "device.h"
#include "interrupt.h"
#include "str.h"
#include "tkprintf.h"
#include "device/ether_ioctl.h"
#include "task/event.h"
#include "task/syscall.h"

#include "stm32f7xx_hal_conf.h"
#include "stm32f7xx_hal_eth.h"
#include "stm32f7xx_hal.h"

//#define DEBUGKBITS 0x10
#include "dkprintf.h"


/* LAN8742A PHY Address*/
#define LAN8742A_PHY_ADDRESS	0x00U

#define RMII_TXD1_Pin		GPIO_PIN_14
#define RMII_TXD1_GPIO_Port	GPIOG
#define RMII_TXD0_Pin		GPIO_PIN_13
#define RMII_TXD0_GPIO_Port	GPIOG
#define RMII_TX_EN_Pin		GPIO_PIN_11
#define RMII_TX_EN_GPIO_Port	GPIOG
#define RMII_RXER_Pin		GPIO_PIN_5
#define RMII_RXER_GPIO_Port	GPIOD
#define RMII_MDC_Pin		GPIO_PIN_1
#define RMII_MDC_GPIO_Port	GPIOC
#define RMII_REF_CLK_Pin	GPIO_PIN_1
#define RMII_REF_CLK_GPIO_Port	GPIOA
#define RMII_RXD0_Pin		GPIO_PIN_4
#define RMII_RXD0_GPIO_Port	GPIOC
#define RMII_MDIO_Pin		GPIO_PIN_2
#define RMII_MDIO_GPIO_Port	GPIOA
#define RMII_RXD1_Pin		GPIO_PIN_5
#define RMII_RXD1_GPIO_Port	GPIOC
#define RMII_CRS_DV_Pin		GPIO_PIN_7
#define RMII_CRS_DV_GPIO_Port	GPIOA

#ifndef GSC_ETHERDEV_DEFAULT_MACADDRESS	// $gsc EtherデバイスデフォルトMACアドレス
static unsigned char macaddress[6] = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x01 };
#else
static unsigned char macaddress[6] = {
	(GSC_ETHERDEV_DEFAULT_MACADDRESS >> 40) & 0xff,
	(GSC_ETHERDEV_DEFAULT_MACADDRESS >> 32) & 0xff,
	(GSC_ETHERDEV_DEFAULT_MACADDRESS >> 24) & 0xff,
	(GSC_ETHERDEV_DEFAULT_MACADDRESS >> 16) & 0xff,
	(GSC_ETHERDEV_DEFAULT_MACADDRESS >>  8) & 0xff,
	(GSC_ETHERDEV_DEFAULT_MACADDRESS >>  0) & 0xff
};
#endif

static unsigned char ether_event[ETH_RXBUFNB + 1];
static struct st_event interrupt_evtque;
static ETH_HandleTypeDef EthHandle;
static void *int_sp;
static int flgs_int = 0;
#define FLG_RX	0x01
#define FLG_TX	0x02
#define FLG_ER	0x04

ETH_DMADescTypeDef  DMARxDscrTab[ETH_RXBUFNB] __attribute__((section(".RxDescripSection")));/* Ethernet Rx MA Descriptor */
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TXBUFNB] __attribute__((section(".TxDescripSection")));/* Ethernet Tx DMA Descriptor */
uint8_t Rx_Buff[ETH_RXBUFNB][ETH_RX_BUF_SIZE] __attribute__((section(".RxBUF")));/* Ethernet Receive Buffer */
uint8_t Tx_Buff[ETH_TXBUFNB][ETH_TX_BUF_SIZE] __attribute__((section(".TxBUF")));/* Ethernet Transmit Buffer */


int eth_rx_count = 0;

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
	DKFPRINTF(0x01, "\n");

	flgs_int |= FLG_RX;

	eth_rx_count ++;
}

void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
	DKFPRINTF(0x10, "\n");

	flgs_int |= FLG_TX;
}

void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
	DKFPRINTF(0xff, "\n");
	SYSERR_PRINT("error?\n");

	flgs_int |= FLG_ER;
}

static void inthdr_ether(unsigned int intnum, void *sp)
{
	DKFPRINTF(0x01, "\n");

	int_sp = sp;
	flgs_int = 0;

	HAL_ETH_IRQHandler(&EthHandle);

	DKFPRINTF(0x01, "flgs_int = %02x\n", flgs_int);

	if(flgs_int & FLG_RX) {
		DKPRINTF(0x04, "*");
		DKFPRINTF(0x01, "wakeup\n");
		event_wakeup_ISR(int_sp, &interrupt_evtque, 0);
	}
}

void HAL_ETH_MspInit(ETH_HandleTypeDef* heth)
{
	DKFPRINTF(0x01, "\n");

	GPIO_InitTypeDef GPIO_InitStruct;
	if(heth->Instance==ETH) {
		__HAL_RCC_GPIOA_CLK_ENABLE();
		__HAL_RCC_GPIOC_CLK_ENABLE();
		__HAL_RCC_GPIOG_CLK_ENABLE();
		__HAL_RCC_ETH_CLK_ENABLE();
		GPIO_InitStruct.Pin = RMII_TXD1_Pin|RMII_TXD0_Pin|RMII_TX_EN_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = RMII_MDC_Pin|RMII_RXD0_Pin|RMII_RXD1_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

		GPIO_InitStruct.Pin = RMII_REF_CLK_Pin|RMII_MDIO_Pin|RMII_CRS_DV_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
		HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	}
}

#define SIZEOFSTACK	(1024*1)
static struct st_tcb tcb;
static unsigned int stack[SIZEOFSTACK/sizeof(unsigned int)] ATTR_STACK;

static int rmii_task(void *arg)
{
	(void)arg;

	for(;;) {
		/* some unicast good packets are received */
		if(EthHandle.Instance->MMCRGUFCR > 0U) {
			/* RMII Init is OK: Delete the Thread */
			task_exit();
		} else if(EthHandle.Instance->MMCRFCECR > 10U) {
			/* ETH received too many packets with CRC errors, resetting RMII */
			SYSCFG->PMC &= ~SYSCFG_PMC_MII_RMII_SEL;
			SYSCFG->PMC |= SYSCFG_PMC_MII_RMII_SEL;

			EthHandle.Instance->MMCCR |= ETH_MMCCR_CR;
		} else {
			/* Delay 200 ms */
			task_sleep(200);
		}
	}

	return 0;
}

static int ether_register(struct st_device *dev, char *param)
{
	eventqueue_register(&interrupt_evtque, "ether_int",
			    ether_event, sizeof(unsigned char), ETH_RXBUFNB + 1);

	register_interrupt(IRQ2VECT(ETH_IRQn), inthdr_ether);

	EthHandle.Instance = ETH;
	EthHandle.Init.MACAddr = macaddress;
	EthHandle.Init.AutoNegotiation = ETH_AUTONEGOTIATION_ENABLE;
	EthHandle.Init.Speed = ETH_SPEED_100M;
	EthHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
	EthHandle.Init.MediaInterface = ETH_MEDIA_INTERFACE_RMII;
	EthHandle.Init.RxMode = ETH_RXINTERRUPT_MODE;
#ifdef GSC_ETHERDEV_HARDWARE_CHECKSUM	// $gsc Etherデバイスのハードウェアチェックサムを有効にする
	EthHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_HARDWARE;
#else
	EthHandle.Init.ChecksumMode = ETH_CHECKSUM_BY_SOFTWARE;
#endif
	EthHandle.Init.PhyAddress = LAN8742A_PHY_ADDRESS;

	/* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
	if(HAL_ETH_Init(&EthHandle) == HAL_OK) {
		/* Set netif link flag */
	}

	/* Initialize Tx Descriptors list: Chain Mode */
	HAL_ETH_DMATxDescListInit(&EthHandle, DMATxDscrTab, &Tx_Buff[0][0], ETH_TXBUFNB);

	/* Initialize Rx Descriptors list: Chain Mode  */
	HAL_ETH_DMARxDescListInit(&EthHandle, DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);

	if(HAL_GetREVID() == 0x1000) {
		tkprintf("RMII configuration Hardware Bug Version(0x1000)\n");
		task_add(rmii_task, "ether_rmii", TASK_PRIORITY_DEVICE_DRIVER, &tcb, stack, SIZEOFSTACK, 0);
	}

	return 0;
}

static int ether_open(struct st_device *dev)
{
	DKFPRINTF(0x01, "\n");

	HAL_NVIC_SetPriority(ETH_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ETH_IRQn);

	HAL_ETH_Start(&EthHandle);

	return 0;
}

static int ether_close(struct st_device *dev)
{
	DKFPRINTF(0x01, "\n");

	HAL_ETH_Stop(&EthHandle);

	return 0;
}

static int ether_read(struct st_device *dev, void *data, unsigned int size)
{
	int i;
	unsigned short len = 0;
	unsigned char *buffer;
	volatile ETH_DMADescTypeDef *dmarxdesc;

	DKFPRINTF(0x81, "size = %d\n", size);

	if(HAL_ETH_GetReceivedFrame_IT(&EthHandle) != HAL_OK) {
		DKPRINTF(0x07, "ETH no Receive data\n");
		goto readend;
	} else {
		DKPRINTF(0x01, "ETH Receive OK\n");
	}

	DKPRINTF(0x01, "SegCount = %d\n", (int)EthHandle.RxFrameInfos.SegCount);
	DKPRINTF(0x01, "RxFrameInfos.buffer = %08X\n", (unsigned int)EthHandle.RxFrameInfos.buffer);

	/* Obtain the size of the packet and put it into the "len" variable. */
	len = EthHandle.RxFrameInfos.length;
	buffer = (uint8_t *)EthHandle.RxFrameInfos.buffer;

	DKPRINTF(0x01, "RxFrameInfos.length = %d\n", len);
	memorycopy(data, buffer, len);
	KXBDUMP(0x02, data, len);

	__DMB();
	/* Release descriptors to DMA */
	/* Point to first descriptor */
	dmarxdesc = EthHandle.RxFrameInfos.FSRxDesc;
	/* Set Own bit in Rx descriptors: gives the buffers back to DMA */
	for(i=0; i< EthHandle.RxFrameInfos.SegCount; i++) {
		__DMB();
		dmarxdesc->Status |= ETH_DMARXDESC_OWN;
		__DMB();
		dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
	}

	/* Clear Segment_Count */
	EthHandle.RxFrameInfos.SegCount = 0;

	__DMB();

	/* When Rx Buffer unavailable flag is set: clear it and resume reception */
	if((EthHandle.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET) {
		/* Clear RBUS ETHERNET DMA flag */
		EthHandle.Instance->DMASR = ETH_DMASR_RBUS;
		/* Resume DMA reception */
		EthHandle.Instance->DMARPDR = 0;
	}

readend:
	return len;
}

static int ether_write(struct st_device *dev, const void *data, unsigned int size)
{
	unsigned char *buffer = (unsigned char *)(EthHandle.TxDesc->Buffer1Addr);
	int rtn = size;
	HAL_StatusTypeDef res;
	__IO ETH_DMADescTypeDef *DmaTxDesc;

	DKFPRINTF(0x10, "size = %d\n", size);

	DmaTxDesc = EthHandle.TxDesc;
	if((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET) {
		SYSERR_PRINT("DmaTxDesc use\n");
		return -1;
	}

	__DMB();
	memorycopy(buffer, data, size);
	KXBDUMP(0x02, buffer, size);

	__DMB();
	/* Prepare transmit descriptors to give to DMA */
	res = HAL_ETH_TransmitFrame(&EthHandle, size);
	if(res != HAL_OK) {
		SYSERR_PRINT("HAL_ETH_TransmitFrame error %d\n", res);
	}

	__DMB();
	/* When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission */
	if((EthHandle.Instance->DMASR & ETH_DMASR_TUS) != (uint32_t)RESET) {
		/* Clear TUS ETHERNET DMA flag */
		EthHandle.Instance->DMASR = ETH_DMASR_TUS;

		/* Resume DMA transmission*/
		EthHandle.Instance->DMATPDR = 0;
	}

	return rtn;
}

static int ether_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	DKFPRINTF(0x01, "com = %ld arg = %ld\n", com, arg);

	switch(com) {
	case IOCMD_ETHER_GET_MACADDR:	// MACアドレス取得
		{
			unsigned char *mac = (unsigned char *)param;
			int i;

			for(i=0; i<6; i++) {
				mac[i] = macaddress[i];
			}

			DKPRINTF(0x01, "MAC Addr %02X:%02X:%02X:%02X:%02X:%02X\n",
				 (int)mac[0], (int)mac[1], (int)mac[2],
				 (int)mac[3], (int)mac[4], (int)mac[5]);
		}
		break;

	case IOCMD_ETHER_SET_MACADDR:	// MACアドレス設定
		return -1;	// [TODO]
		break;

	case IOCMD_ETHER_CLEAR_BUF:
		break;

	case IOCMD_ETHER_LINK_UP:
		HAL_ETH_DMARxDescListInit(&EthHandle, DMARxDscrTab, &Rx_Buff[0][0], ETH_RXBUFNB);
		break;

	case IOCMD_ETHER_LINK_DOWN:
		HAL_ETH_Stop(&EthHandle);
		break;

	case IOCMD_ETHER_GET_LINK_STATUS:
		{
			unsigned long phyreg;
			int stat = 0;

			HAL_ETH_ReadPHYRegister(&EthHandle, PHY_BSR, &phyreg);
			DKPRINTF(0x01, "PHY_BSR = %08X\n", phyreg);

			switch(phyreg & PHY_FULLDUPLEX_100M) {
			case PHY_FULLDUPLEX_100M:
				stat |= IORTN_BIT_ETHER_100M;
				stat |= IORTN_BIT_ETHER_FULLDUPLEX;
				break;

			case PHY_HALFDUPLEX_100M:
				stat |= IORTN_BIT_ETHER_100M;
				break;

			case PHY_FULLDUPLEX_10M:
				stat |= IORTN_BIT_ETHER_FULLDUPLEX;
				break;

			case PHY_HALFDUPLEX_10M:
				break;

			default:
				break;
			}

			if((phyreg & PHY_LINKED_STATUS) != 0) {
				stat |= IORTN_BIT_ETHER_LINK_UP;
			}

			return stat;
		}
		break;

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		break;
	}

	return 0;
}

static int ether_select(struct st_device *dev, unsigned int timeout)
{
	DKFPRINTF(0x08, "timeout = %d\n", timeout);

	int rtn = 0;

	rtn = event_wait(&interrupt_evtque, 0, timeout);

	DKFPRINTF(0x08, "return=%d\n", rtn);

	return rtn;
}


const struct st_device ether_device = {
	.name		= DEF_DEV_NAME_ETHER,
	.explan		= "STM32F7xxx-Discovery Ether",
	.register_dev	= ether_register,
	.open		= ether_open,
	.close		= ether_close,
	.read		= ether_read,
	.write		= ether_write,
	.ioctl		= ether_ioctl,
	.select		= ether_select,
};
