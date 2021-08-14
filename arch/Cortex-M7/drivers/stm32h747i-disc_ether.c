/** @file
    @brief	STM32H747I Discovery Ether

    @date	2020.02.01
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

#include "stm32h7xx_hal_conf.h"
#include "stm32h7xx_hal_eth.h"
#include "stm32h7xx_hal.h"
#include "lan8742.h"

#include "lwip/pbuf.h"
#include "lwip/memp.h"

//#define DEBUGKBITS 0x10
#include "dkprintf.h"


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

#define ETH_RX_BUFFER_SIZE	(1536UL)
#define ETH_RX_BUFFER_COUNT     20	/* This app buffers receive packets of its primary service
					 * protocol for processing later. */

static unsigned char ether_event[ETH_RX_BUFFER_COUNT + 1];
static struct st_event interrupt_evtque;
static ETH_HandleTypeDef EthHandle;
static void *int_sp;
static int flgs_int = 0;
#define FLG_RX	0x01
#define FLG_TX	0x02
#define FLG_ER	0x04

ETH_TxPacketConfig TxConfig;
lan8742_Object_t LAN8742;

int32_t ETH_PHY_IO_Init(void);
int32_t ETH_PHY_IO_DeInit (void);
int32_t ETH_PHY_IO_ReadReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal);
int32_t ETH_PHY_IO_WriteReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal);
int32_t ETH_PHY_IO_GetTick(void);

lan8742_IOCtx_t  LAN8742_IOCtx = {ETH_PHY_IO_Init,
				  ETH_PHY_IO_DeInit,
				  ETH_PHY_IO_WriteReg,
				  ETH_PHY_IO_ReadReg,
				  ETH_PHY_IO_GetTick};

typedef struct
{
  struct pbuf_custom pbuf_custom;
#if ETH_RX_BUFFERS_ARE_CACHED
  uint8_t buff[(ETH_RX_BUFFER_SIZE + 31) & ~31] __ALIGNED(32);
#else
  uint8_t buff[(ETH_RX_BUFFER_SIZE + 3) & ~3] __ALIGNED(4);
#endif
} RxBuff_t;

ETH_DMADescTypeDef  DMARxDscrTab[ETH_RX_DESC_CNT] __attribute__((section(".RxDescripSection")));/* Ethernet Rx DMA Descriptor */
ETH_DMADescTypeDef  DMATxDscrTab[ETH_TX_DESC_CNT] __attribute__((section(".TxDescripSection")));/* Ethernet Tx DMA Descriptor */
//uint8_t Rx_Buff[ETH_RX_DESC_CNT][ETH_RX_BUFFER_SIZE] __attribute__((section(".RxBUF")));/* Ethernet Receive Buffer */
//uint8_t Rx_Buff2[ETH_RX_DESC_CNT][ETH_RX_BUFFER_SIZE] __attribute__((section(".RxBUF")));/* Ethernet Receive Buffer */
RxBuff_t EthIfRxBuff[ETH_RX_BUFFER_COUNT] __attribute__((aligned(32), section(".RxBUF"))); /* Ethernet Rx Buffers */
//uint8_t Tx_Buff[ETH_TX_DESC_CNT][ETH_TX_BUF_SIZE] __attribute__((section(".TxBUF")));/* Ethernet Transmit Buffer */

/* LwIP Memory pool descriptor for EthIfRxBuff.
 * This does the same as LWIP_MEMPOOL_DECLARE, except EthIfRxBuff's defined
 * separately for special section placement. */
#if MEMP_STATS
  static struct stats_mem memp_stats_EthIfRxBuff;
#endif
static struct memp *memp_tab_EthIfRxBuff;
static const struct memp_desc memp_EthIfRxBuff = {
#if defined(LWIP_DEBUG) || MEMP_OVERFLOW_CHECK || LWIP_STATS_DISPLAY
	.desc = "RxBuff Pool",
#endif
#if MEMP_STATS
	.stats = &memp_stats_EthIfRxBuff,
#endif
	.size = LWIP_MEM_ALIGN_SIZE(sizeof(RxBuff_t)),
	.num = ETH_RX_BUFFER_COUNT,
	.base = (uint8_t *)EthIfRxBuff,
	.tab = &memp_tab_EthIfRxBuff
};

/* Flag when RxBuffAlloc exhausts the Rx Buffer Pool, so the next RxPktDiscard will
 * repopulate the buffers of the Rx DMA Descriptors. */
static uint8_t RxBuffEmpty;


#if ETH_TX_QUEUE_ENABLE
/* Queue for packet transmit. */
typedef struct TxQueue_t
{
  struct TxQueue_t *next;
  struct pbuf *p;
} TxQueue_t;

static TxQueue_t TxQueue[ETH_TX_QUEUE_SIZE];
static TxQueue_t *TxQueueFree, *TxQueueHead, *TxQueueTail;
#endif /* ETH_TX_QUEUE_ENABLE */

/* USER CODE BEGIN 2 */
/* Relocate the LwIP heap via the ".TxArraySection" section to SRAM3 in the D2 domain.
 * LWIP_RAM_HEAP_POINTER defined in lwipopts.h points to this memory and its definition
 * there prevents LwIP defining its memory.
 */
// START Copy from LwIP's mem.c
struct mem {
  /** index (-> ram[next]) of the next struct */
  mem_size_t next;
  /** index (-> ram[prev]) of the previous struct */
  mem_size_t prev;
  /** 1: this area is used; 0: this area is unused */
  u8_t used;
};
#define SIZEOF_STRUCT_MEM    LWIP_MEM_ALIGN_SIZE(sizeof(struct mem))
#define MEM_SIZE_ALIGNED     LWIP_MEM_ALIGN_SIZE(MEM_SIZE)
// END Copy from LwIP's mem.c
uint8_t lwip_custom_ram_heap[MEM_SIZE_ALIGNED + (2U*SIZEOF_STRUCT_MEM)] __attribute__((section(".TxArraySection")));

/* USER CODE END 2 */


/*******************************************************************************
		PHI IO Functions
*******************************************************************************/
/**
  * @brief  Initializes the MDIO interface GPIO and clocks.
  * @param  None
  * @retval 0 if OK, -1 if ERROR
  */
int32_t ETH_PHY_IO_Init(void)
{
	/* We assume that MDIO GPIO configuration is already done
	   in the ETH_MspInit() else it should be done here
	*/

	/* Configure the MDIO Clock */
	HAL_ETH_SetMDIOClockRange(&EthHandle);

	return 0;
}

/**
  * @brief  De-Initializes the MDIO interface .
  * @param  None
  * @retval 0 if OK, -1 if ERROR
  */
int32_t ETH_PHY_IO_DeInit (void)
{
	return 0;
}

/**
  * @brief  Read a PHY register through the MDIO interface.
  * @param  DevAddr: PHY port address
  * @param  RegAddr: PHY register address
  * @param  pRegVal: pointer to hold the register value
  * @retval 0 if OK -1 if Error
  */
int32_t ETH_PHY_IO_ReadReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t *pRegVal)
{
	int32_t rt;

	rt =  HAL_ETH_ReadPHYRegister(&EthHandle, DevAddr, RegAddr, pRegVal);

	if(rt != HAL_OK) {
		SYSERR_PRINT("HAL_ETH_ReadPHYRegister error(%ld)\n", rt);
		return -1;
	}

	DKFPRINTF(0x01, "RegAddr = %lX, RegVal = %lX\n", RegAddr, *pRegVal);

	return 0;
}

/**
  * @brief  Write a value to a PHY register through the MDIO interface.
  * @param  DevAddr: PHY port address
  * @param  RegAddr: PHY register address
  * @param  RegVal: Value to be written
  * @retval 0 if OK -1 if Error
  */
int32_t ETH_PHY_IO_WriteReg(uint32_t DevAddr, uint32_t RegAddr, uint32_t RegVal)
{
	int32_t rt;

	DKFPRINTF(0x01, "RegAddr = %lX, RegVal = %lX\n", RegAddr, RegVal);

	rt =  HAL_ETH_WritePHYRegister(&EthHandle, DevAddr, RegAddr, RegVal);
	if(rt != HAL_OK) {
		SYSERR_PRINT("HAL_ETH_WritePHYRegister error(%ld)\n", rt);
		return -1;
	}

	return 0;
}

/**
  * @brief  Get the time in millisecons used for internal PHY driver process.
  * @retval Time value
  */
int32_t ETH_PHY_IO_GetTick(void)
{
	int32_t rt = HAL_GetTick();

	DKPRINTF(0x01, "Tick = %ld\n", rt);

	return rt;
}

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
		DKFPRINTF(0x03, "wakeup\n");
		event_wakeup_ISR(int_sp, &interrupt_evtque, 0);
	}
}

/* Private defines -----------------------------------------------------------*/
#define ETH_TX_EN_Pin GPIO_PIN_11
#define ETH_TX_EN_GPIO_Port GPIOG
#define OSC32_OUT_Pin GPIO_PIN_15
#define OSC32_OUT_GPIO_Port GPIOC
#define OSC32_IN_Pin GPIO_PIN_14
#define OSC32_IN_GPIO_Port GPIOC
#define ETH_TXD1_Pin GPIO_PIN_12
#define ETH_TXD1_GPIO_Port GPIOG
#define ETH_TXD0_Pin GPIO_PIN_13
#define ETH_TXD0_GPIO_Port GPIOG
#define CEC_CK_MCO1_Pin GPIO_PIN_8
#define CEC_CK_MCO1_GPIO_Port GPIOA
#define OSC_OUT_Pin GPIO_PIN_1
#define OSC_OUT_GPIO_Port GPIOH
#define OSC_IN_Pin GPIO_PIN_0
#define OSC_IN_GPIO_Port GPIOH
#define ETH_MDC_SAI4_D1_Pin GPIO_PIN_1
#define ETH_MDC_SAI4_D1_GPIO_Port GPIOC
#define ETH_MDIO_Pin GPIO_PIN_2
#define ETH_MDIO_GPIO_Port GPIOA
#define ETH_REF_CLK_Pin GPIO_PIN_1
#define ETH_REF_CLK_GPIO_Port GPIOA
#define ETH_CRS_DV_Pin GPIO_PIN_7
#define ETH_CRS_DV_GPIO_Port GPIOA
#define ETH_RXD0_Pin GPIO_PIN_4
#define ETH_RXD0_GPIO_Port GPIOC
#define ETH_RXD1_Pin GPIO_PIN_5
#define ETH_RXD1_GPIO_Port GPIOC

void HAL_ETH_MspInit(ETH_HandleTypeDef* heth)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	DKFPRINTF(0x01, "\n");

	/* Peripheral clock enable */
	__HAL_RCC_ETH1MAC_CLK_ENABLE();
	__HAL_RCC_ETH1TX_CLK_ENABLE();
	__HAL_RCC_ETH1RX_CLK_ENABLE();

	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	/**ETH GPIO Configuration
	   PG11     ------> ETH_TX_EN
	   PG12     ------> ETH_TXD1
	   PG13     ------> ETH_TXD0
	   PC1     ------> ETH_MDC
	   PA2     ------> ETH_MDIO
	   PA1     ------> ETH_REF_CLK
	   PA7     ------> ETH_CRS_DV
	   PC4     ------> ETH_RXD0
	   PC5     ------> ETH_RXD1
	*/
	GPIO_InitStruct.Pin = ETH_TX_EN_Pin|ETH_TXD1_Pin|ETH_TXD0_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = ETH_MDC_SAI4_D1_Pin|ETH_RXD0_Pin|ETH_RXD1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = ETH_MDIO_Pin|ETH_REF_CLK_Pin|ETH_CRS_DV_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF11_ETH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

static int ethernet_link_check_state(void)
{
	ETH_MACConfigTypeDef MACConf;
	uint32_t PHYLinkState;
	uint32_t linkchanged = 0, speed = 0, duplex =0;
	int stat = 0;

	PHYLinkState = LAN8742_GetLinkState(&LAN8742);
	DKPRINTF(0x01, "PHYLinkState = %lu\n", PHYLinkState);

	if(PHYLinkState <= LAN8742_STATUS_LINK_DOWN) {
		HAL_ETH_Stop(&EthHandle);
		DKPRINTF(0x01, "PHY Link DOWN\n");
	} else if(PHYLinkState > LAN8742_STATUS_LINK_DOWN) {

		stat |= IORTN_BIT_ETHER_LINK_UP;
		DKPRINTF(0x01, "PHY Link UP\n");

		switch(PHYLinkState) {
		case LAN8742_STATUS_100MBITS_FULLDUPLEX:
			stat |= IORTN_BIT_ETHER_100M;
			stat |= IORTN_BIT_ETHER_FULLDUPLEX;
			duplex = ETH_FULLDUPLEX_MODE;
			speed = ETH_SPEED_100M;
			linkchanged = 1;
			DKPRINTF(0x01, "100M FULL\n");
			break;

		case LAN8742_STATUS_100MBITS_HALFDUPLEX:
			stat |= IORTN_BIT_ETHER_100M;
			duplex = ETH_HALFDUPLEX_MODE;
			speed = ETH_SPEED_100M;
			linkchanged = 1;
			DKPRINTF(0x01, "100M HALF\n");
			break;

		case LAN8742_STATUS_10MBITS_FULLDUPLEX:
			stat |= IORTN_BIT_ETHER_FULLDUPLEX;
			duplex = ETH_FULLDUPLEX_MODE;
			speed = ETH_SPEED_10M;
			linkchanged = 1;
			DKPRINTF(0x01, "10M FULL\n");
			break;

		case LAN8742_STATUS_10MBITS_HALFDUPLEX:
			duplex = ETH_HALFDUPLEX_MODE;
			speed = ETH_SPEED_10M;
			linkchanged = 1;
			DKPRINTF(0x01, "10M HALF\n");
			break;

		default:
			break;
		}

		if(linkchanged) {
			duplex = ETH_FULLDUPLEX_MODE;	//!!! 強制的に100M FULLに設定すると何故か動く
			speed = ETH_SPEED_100M;		//!!!
			/* Get MAC Config MAC */
			HAL_ETH_GetMACConfig(&EthHandle, &MACConf);
			MACConf.DuplexMode = duplex;
			MACConf.Speed = speed;
			HAL_ETH_SetMACConfig(&EthHandle, &MACConf);
		}
	}

	return stat;
}


/**
  * @brief  Custom free for the Rx Buffer Pool.
  * @param  p Pointer to the packet buffer struct.
  * @retval None
  */
static void RxPoolCustomFree(struct pbuf *p)
{
  LWIP_MEMPOOL_FREE(EthIfRxBuff, p);

  /* If the Rx Buffer Pool was exhausted, signal the ethernetif_input task to
   * call HAL_ETH_GetRxDataBuffer to rebuild the Rx descriptors. */
  if (RxBuffEmpty)
  {
//    xTaskNotifyGive(EthIfThread);
    RxBuffEmpty = 0;
  }
}

/**
  * @brief  Callback function given to the ETH driver at init to allocate a
  *         buffer.
  * @retval Pointer to the buffer, or NULL if no buffers are available.
  */
static uint8_t *RxBuffAlloc(void)
{
  struct pbuf_custom *p = LWIP_MEMPOOL_ALLOC(EthIfRxBuff);
  uint8_t *buff;

  if (p)
  {
    /* Get the buff from the struct pbuf address. */
    buff = (uint8_t *)p + offsetof(RxBuff_t, buff);

    /* Initialize the struct pbuf.
     * This must be performed whenever a buffer's allocated because it may be
     * changed by lwIP or the app, e.g., pbuf_free decrements ref. */
    pbuf_alloced_custom(PBUF_RAW, 0, PBUF_REF, p, buff, ETH_RX_BUFFER_SIZE);
  }
  else
  {
    /* Rx Buffer Pool is exhausted. */
    RxBuffEmpty = 1;
    buff = NULL;
  }

  return buff;
}

/**
  * @brief  Callback function given to the ETH driver at init to free a buffer.
  * @param  buff Pointer to the buffer, or NULL if no buffers are available.
  * @retval None
  */
static void RxBuffFree(uint8_t *buff)
{
  struct pbuf *p;

  /* Get the struct pbuf from the buff address. */
  p = (struct pbuf *)(buff - offsetof(RxBuff_t, buff));
  RxPoolCustomFree(p);
}

/**
  * @brief  Callback function given to the ETH driver at init to chain buffers
  *         as a packet.
  * @param  ppPktFirst Double-pointer to the first packet buffer struct of the
  *         packet.
  * @param  ppPktLast Double-pointer to the last packet buffer struct of the
  *         packet.
  * @param  buff Pointer to the buffer.
  * @param  buffLength length of the buffer.
  * @retval None
  */
static void RxPktAssemble(void **ppPktFirst, void **ppPktLast, uint8_t *buff, uint16_t buffLength)
{
  struct pbuf **ppFirst = (struct pbuf **)ppPktFirst;
  struct pbuf **ppLast = (struct pbuf **)ppPktLast;
  struct pbuf *p;

  /* Get the struct pbuf from the buff address. */
  p = (struct pbuf *)(buff - offsetof(RxBuff_t, buff));
  p->next = NULL;
  p->tot_len = 0;
  p->len = buffLength;

  /* Chain the buffer. */
  if (!*ppFirst)
  {
    /* The first buffer of the packet. */
    *ppFirst = p;
  }
  else
  {
    /* Chain the buffer to the end of the packet. */
    (*ppLast)->next = p;
  }
  *ppLast = p;

  /* Update the total length of all the buffers of the chain. Each pbuf in the chain should have its tot_len
   * set to its own length, plus the length of all the following pbufs in the chain. */
  for (p = *ppFirst; p != NULL; p = p->next)
  {
    p->tot_len += buffLength;
  }

  /* Invalidating cache isn't necessary if the rx buffers are in a not-cacheable or write-through MPU region. */
#if ETH_RX_BUFFERS_ARE_CACHED
  /* Invalidate data cache because Rx DMA's writing to physical memory makes it stale. */
  SCB_InvalidateDCache_by_Addr((uint32_t *)buff, buffLength);
#endif
}

/**
  * @brief  Callback function given to the ETH driver at init to discard a
  *         packet.
  * @param  pPkt Pointer to the packet buffer struct.
  * @retval None
  */
static void RxPktDiscard(void *pPkt)
{
  struct pbuf *p = (struct pbuf *)pPkt;
  struct pbuf *nextp;

  while (p)
  {
    nextp = p->next;
    RxPoolCustomFree(p);
    p = nextp;
  }
}

/**
  * @brief  Initialize the Rx Buffer Pool.
  * @retval None
  */
static void RxPoolInit(void)
{
  int k;

  /* Initialize the custom_free_function of each pbuf_custom here to save
   * cycles later. */
  for (k = 0; k < ETH_RX_BUFFER_COUNT; k++)
  {
    EthIfRxBuff[k].pbuf_custom.custom_free_function = RxPoolCustomFree;
  }

  /* Initialize the Rx Buffer Pool. */
  LWIP_MEMPOOL_INIT(EthIfRxBuff);
}


static int ether_register(struct st_device *dev, char *param)
{
//	uint32_t idx = 0;
	int32_t rt;

	eventqueue_register(&interrupt_evtque, "ether_int",
			    ether_event, sizeof(unsigned char), ETH_RX_BUFFER_COUNT + 1);

	register_interrupt(IRQ2VECT(ETH_IRQn), inthdr_ether);

	EthHandle.Instance = ETH;
	EthHandle.Init.MACAddr = macaddress;
	EthHandle.Init.MediaInterface = HAL_ETH_RMII_MODE;
	EthHandle.Init.RxDesc = DMARxDscrTab;
	EthHandle.Init.TxDesc = DMATxDscrTab;
	EthHandle.Init.RxBuffLen = ETH_RX_BUF_SIZE;
	EthHandle.Init.RxBuffLen = ETH_RX_BUFFER_SIZE;

	EthHandle.Init.RxBuffAlloc = RxBuffAlloc;
	EthHandle.Init.RxBuffFree = RxBuffFree;
	EthHandle.Init.RxPktAssemble = RxPktAssemble;
	EthHandle.Init.RxPktDiscard = RxPktDiscard;

	RxPoolInit();

	/* configure ethernet peripheral (GPIOs, clocks, MAC, DMA) */
	rt = HAL_ETH_Init(&EthHandle);
	if(rt != HAL_OK) {
		tkprintf("HAL_ETH_Init Error %ld\n", rt);
	}

#if 0
	for(idx = 0; idx < ETH_RX_DESC_CNT; idx ++) {
		HAL_ETH_DescAssignMemory(&EthHandle, idx, EthIfRxBuff[idx], NULL);
//		HAL_ETH_DescAssignMemory(&EthHandle, idx, Rx_Buff[idx], Rx_Buff2[idx]);
	}
#endif

	memoryset(&TxConfig, 0, sizeof(ETH_TxPacketConfig));
	TxConfig.Attributes = ETH_TX_PACKETS_FEATURES_CSUM | ETH_TX_PACKETS_FEATURES_CRCPAD;
	TxConfig.ChecksumCtrl = ETH_CHECKSUM_IPHDR_PAYLOAD_INSERT_PHDR_CALC;
	TxConfig.CRCPadCtrl = ETH_CRC_PAD_INSERT;

	/* Set PHY IO functions */
	rt = LAN8742_RegisterBusIO(&LAN8742, &LAN8742_IOCtx);
	if(rt != LAN8742_STATUS_OK) {
		tkprintf("LAN8742_RegisterBusIO Error %ld\n", rt);
	}

#if 0	// PYHの初期化はエラーになる。初期化しなくても(なぜか)動く[TODO]
	/* Initialize the LAN8742 ETH PHY */
	rt = LAN8742_Init(&LAN8742);
	if(rt != LAN8742_STATUS_OK) {
		tkprintf("LAN8742_Init Error %ld\n", rt);
	}
#endif

	ethernet_link_check_state();

	return 0;
}

static int ether_open(struct st_device *dev)
{
	DKFPRINTF(0x01, "\n");

	HAL_NVIC_SetPriority(ETH_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(ETH_IRQn);

	HAL_ETH_Start_IT(&EthHandle);

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
//	ETH_BufferTypeDef RxBuff;
	struct pbuf *RxBuff;
	uint32_t framelength = 0;
	uint32_t errorCode;

	DKFPRINTF(0x01, "size = %d\n", size);

#if 0
	if(HAL_ETH_IsRxDataAvailable(&EthHandle) == 0) {
		DKPRINTF(0x07, "ETH no Receive data\n");
		goto readend;
	} else {
		DKPRINTF(0x03, "ETH Receive OK\n");
	}
#endif

	DKPRINTF(0x01, "RxState = %d\n", (int)EthHandle.RxState);

	if(HAL_ETH_GetRxDataBuffer(&EthHandle, (void **)&RxBuff) == HAL_OK)  {
		ETH_RxPacketInfo RxPacketInfo;
		DKPRINTF(0x20, "buff = %p\n", RxBuff);
		DKPRINTF(0x20, "len = %d\n", RxBuff->len);
		KXBDUMP(0x20, (unsigned char *)RxBuff->payload, RxBuff->len);

//		tkprintf("BuildDescCount = %d\n", EthHandle.RxDescList.Pkt.BuildDescCount);

		HAL_ETH_GetRxDataInfo(&EthHandle, &RxPacketInfo);
		if(RxPacketInfo.SegmentCnt > 1) {
			tkprintf("SegmentCnt = %lu\n", RxPacketInfo.SegmentCnt);
		}
		//DKPRINTF(0x80, "ErrorCode = %lu\n", RxPacketInfo.ErrorCode);

		HAL_ETH_GetRxDataErrorCode(&EthHandle, &errorCode);
		if(errorCode) {
			RxPktDiscard(RxBuff);
		} else {
			framelength = RxBuff->len;
			if(framelength > size) {
				tkprintf("size over %ld %u\n", framelength, size);
			}
			//SCB_InvalidateDCache_by_Addr((uint32_t *)RxBuff->payload, framelength);
			memorycopy(data, RxBuff->payload, framelength);
			if(RxBuff->next != 0) {
				tkprintf("next = %p\n", RxBuff->next);
			}
			RxPktDiscard(RxBuff);
		}

//		int dn = EthHandle.RxDescList.AppDescNbr;
//		if(dn > 1) {
//			tkprintf("AppDescNbr = %d\n", dn);
//		}
//		DKPRINTF(0x20, "CurRxDesc = %lu\n", EthHandle.RxDescList.CurRxDesc);
//		DKPRINTF(0x20, "FirstAppDesc = %lu\n", EthHandle.RxDescList.FirstAppDesc);
//		DKPRINTF(0x20, "AppDescNbr = %lu\n", EthHandle.RxDescList.AppDescNbr);
//		DKPRINTF(0x20, "AppContextDesc = %lu\n", EthHandle.RxDescList.AppContextDesc);
//		DKPRINTF(0x20, "ItMode = %lu\n", EthHandle.RxDescList.ItMode);

//		HAL_ETH_GetRxDataLength(&EthHandle, &framelength);
//		DKPRINTF(0x03, "framelength = %lu\n", framelength);

		/* Build Rx descriptor to be ready for next data reception */
//!!!		HAL_ETH_BuildRxDescriptors(&EthHandle);

		/* Invalidate data cache for ETH Rx Buffers */
//		SCB_InvalidateDCache_by_Addr((uint32_t *)RxBuff.buffer, framelength);

//		memorycopy(data, RxBuff.buffer, framelength);
//		KXBDUMP(0x01, RxBuff.buffer, framelength);

//!!!		HAL_ETH_BuildRxDescriptors(&EthHandle);//!!!
	}
//	if(HAL_ETH_GetRxDataBuffer(&EthHandle, (void **)&RxBuff) == HAL_OK)  {
//		DKPRINTF(0x40, "buff = %p\n", RxBuff);
//		DKPRINTF(0x40, "len = %d\n", RxBuff->len);
//		HAL_ETH_GetRxDataErrorCode(&EthHandle, &errorCode);
//	}

//!!!readend:
	return framelength;
}

#define ETH_DMA_TRANSMIT_TIMEOUT                (20U)

static int ether_write(struct st_device *dev, const void *data, unsigned int size)
{
	int rt = 0;
	int rtn = size;
	ETH_BufferTypeDef TxBuff[ETH_TX_DESC_CNT];

//	memoryset(&Tx_Buff[0], 0, ETH_TX_DESC_CNT*sizeof(ETH_BufferTypeDef));

//	memorycopy(&Tx_Buff[0], data, size);
//	SCB_CleanDCache_by_Addr((uint32_t *)&Tx_Buff[0], size);

//	TxBuff[0].buffer = (uint8_t *)&Tx_Buff[0];
	uint8_t *dataStart = (uint8_t *)data;
	uint8_t *lineStart = (uint8_t *)((uint32_t)dataStart & ~31);
	SCB_CleanDCache_by_Addr((uint32_t *)lineStart, size + (dataStart - lineStart));

	TxBuff[0].buffer = (uint8_t *)data;
	TxBuff[0].len = size;
	TxBuff[0].next = 0;

	TxConfig.Length = size;
	TxConfig.TxBuffer = TxBuff;

	DKFPRINTF(0x04, "size = %d\n", size);
	KXBDUMP(0x04, TxBuff[0].buffer, TxBuff[0].len);

//	rt = HAL_ETH_Transmit(&EthHandle, &TxConfig, ETH_DMA_TRANSMIT_TIMEOUT);
	rt = HAL_ETH_Transmit_IT(&EthHandle, &TxConfig);
	if(rt != HAL_OK) {
		SYSERR_PRINT("HAL_ETH_Transmit error(%d)\n", rt);
	}

	return rtn;
}

static int ether_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	DKFPRINTF(0x08, "com = %08X, arg = %u\n", com, arg);

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
		HAL_ETH_Start_IT(&EthHandle);
		break;

	case IOCMD_ETHER_LINK_DOWN:
		HAL_ETH_Stop(&EthHandle);
		break;

	case IOCMD_ETHER_GET_LINK_STATUS:
		{
			int stat = 0;

			stat = ethernet_link_check_state();
#if 1	// リンク状態が正しく取れないようなので強制的に100M FULLとしている[TODO]
			stat |= IORTN_BIT_ETHER_LINK_UP;
			stat |= IORTN_BIT_ETHER_100M;
			stat |= IORTN_BIT_ETHER_FULLDUPLEX;
#endif

			return stat;
		}
		break;

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		break;
	}

	return 0;
}

static int ether_epbuf_get(struct st_device *dev, void **data)
{
	struct pbuf *RxBuff;
	uint32_t errorCode;
	ETH_RxPacketInfo RxPacketInfo;
	HAL_StatusTypeDef rtn;

	DKFPRINTF(0x20, "data = %p\n", data);
	DKPRINTF(0x01, "RxState = %d\n", (int)EthHandle.RxState);

	rtn = HAL_ETH_GetRxDataBuffer(&EthHandle, data);
	if(rtn == HAL_OK)  {
		RxBuff = *data;
		DKFPRINTF(0x20, "RxBuff = %p\n", RxBuff);
		DKPRINTF(0x20, "buff = %p\n", RxBuff);
		DKPRINTF(0x20, "len = %d\n", RxBuff->len);
		KXBDUMP(0x20, (unsigned char *)RxBuff->payload, RxBuff->len);

//		tkprintf("BuildDescCount = %d\n", EthHandle.RxDescList.Pkt.BuildDescCount);

		HAL_ETH_GetRxDataInfo(&EthHandle, &RxPacketInfo);
		if(RxPacketInfo.SegmentCnt > 1) {
			tkprintf("SegmentCnt = %lu\n", RxPacketInfo.SegmentCnt);
		}

		HAL_ETH_GetRxDataErrorCode(&EthHandle, &errorCode);
		if(errorCode) {
			RxPktDiscard(RxBuff);
			RxPacketInfo.SegmentCnt = 0;
		}
	} else {
		tkprintf("HAL_ETH_GetRxDataBuffer error %d\n", rtn);
		RxPacketInfo.SegmentCnt = 0;
	}

	return RxPacketInfo.SegmentCnt;
}

static int ether_epbuf_release(struct st_device *dev, void *data)
{
	struct pbuf *RxBuff = data;

	DKFPRINTF(0x20, "data = %p\n", data);

	RxPktDiscard(RxBuff);

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
	.explan		= "STM32H747I-Discovery Ether",
	.register_dev	= ether_register,
	.open		= ether_open,
	.close		= ether_close,
	.read		= ether_read,
	.write		= ether_write,
	.ioctl		= ether_ioctl,
	.epbuf_get	= ether_epbuf_get,
	.epbuf_release	= ether_epbuf_release,
	.select		= ether_select,
};
