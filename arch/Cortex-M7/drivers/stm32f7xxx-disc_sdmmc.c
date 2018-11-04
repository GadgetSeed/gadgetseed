/** @file
    @brief	SD MMC ドライバ STM32F[769I|746G]-Discovery

    @date	2016.01.09
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "tkprintf.h"
#include "timer.h"
#include "device.h"
#include "str.h"
#include "interrupt.h"
#include "device/sd_ioctl.h"
#include "task/syscall.h"

#ifdef GSC_TARGET_SYSTEM_STM32F769IDISCOVERY
#include "stm32f769i_discovery_sd.h"
#define SDMMC_IRQn	SDMMC2_IRQn
#elif defined(GSC_TARGET_SYSTEM_STM32F746GDISCOVERY)
#include "stm32746g_discovery_sd.h"
#define SDMMC_IRQn	SDMMC1_IRQn
#endif

#define ENABLE_SDMMC_DMA	/// DMA転送を使用する
#define SD_TIMEOUT (3*1000)

//#define DEBUGKBITS 0x13
#include "dkprintf.h"


#ifdef ENABLE_SDMMC_DMA
extern SD_HandleTypeDef uSdHandle;

static struct st_event dma_rx_evt;
static struct st_event dma_tx_evt;

void HAL_SD_ErrorCallback(SD_HandleTypeDef *hsd)
{
	SYSERR_PRINT("Error\n");
}

void HAL_SD_XferErrorCallback(SD_HandleTypeDef *hsd)
{
	SYSERR_PRINT("Error\n");
}

void BSP_SD_AbortCallback(void)
{
	SYSERR_PRINT("Abort\n");
}

static int flg_dma_rx_cmp = 0;
static int flg_dma_tx_cmp = 0;
static int flg_sd_cmp = 0;

void BSP_SD_ReadCpltCallback(void)
{
	DKFPRINTF(0x02, "\n");

	flg_dma_rx_cmp = 1;
}

void BSP_SD_WriteCpltCallback(void)
{
	DKFPRINTF(0x02, "\n");

	flg_dma_tx_cmp = 1;
}

/* DMA2 Stream0 */
static void inthdr_sdmmc_dma_rx(unsigned int intnum, void *sp)
{
	DKFPRINTF(0x02, "intnum=%d\n", intnum);

	HAL_DMA_IRQHandler(uSdHandle.hdmarx);

	DKFPRINTF(0x02, "end\n");

	if(flg_sd_cmp != 0) {
		DKFPRINTF(0x02, "event wakeup dma_rx_evt\n");
		flg_sd_cmp = 0;
		event_wakeup_ISR(sp, &dma_rx_evt, 0);
	}
}

/* DMA2 Stream5 */
static void inthdr_sdmmc_dma_tx(unsigned int intnum, void *sp)
{
	DKFPRINTF(0x02, "intnum=%d\n", intnum);

	HAL_DMA_IRQHandler(uSdHandle.hdmatx);

	DKFPRINTF(0x02, "end\n");

	if(flg_sd_cmp != 0) {
		DKFPRINTF(0x02, "event wakeup dma_tx_evt\n");
		flg_sd_cmp = 0;
		event_wakeup_ISR(sp, &dma_tx_evt, 0);
	}
}

static void inthdr_sd(unsigned int intnum, void *sp)
{
	DKFPRINTF(0x02, "intnum=%d\n", intnum);

	HAL_SD_IRQHandler(&uSdHandle);

	DKFPRINTF(0x02, "end\n");

	if(flg_dma_rx_cmp != 0) {
		flg_dma_rx_cmp = 0;
		DKFPRINTF(0x02, "event wakeup dma_rx_evt\n");
		event_wakeup_ISR(sp, &dma_rx_evt, 0);
	} else {
		flg_sd_cmp = 1;
	}

	if(flg_dma_tx_cmp != 0) {
		flg_dma_tx_cmp = 0;
		DKFPRINTF(0x02, "event wakeup dma_tx_evt\n");
		event_wakeup_ISR(sp, &dma_tx_evt, 0);
	} else {
		flg_sd_cmp = 1;
	}
}
#endif //ENABLE_SDMMC_DMA

static int sdmmc_register(struct st_device *dev, char *param)
{
	unsigned char stat;

	stat = BSP_SD_Init();

	if(stat != HAL_OK) {
		SYSERR_PRINT("SD/MMC Initialize error(%d)\n", (int)stat);
		return -1;
	}

#ifdef ENABLE_SDMMC_DMA
	//BSP_SD_ITConfig();

	eventqueue_register_ISR(&dma_rx_evt, "sddmarx", 0, 0, 0);
	HAL_NVIC_SetPriority(SD_DMAx_Rx_IRQn, 0, 0);	// 割り込みプライオリティは最低(0)
	register_interrupt(IRQ2VECT(SD_DMAx_Rx_IRQn), inthdr_sdmmc_dma_rx);

	eventqueue_register_ISR(&dma_tx_evt, "sddmatx", 0, 0, 0);
	HAL_NVIC_SetPriority(SD_DMAx_Tx_IRQn, 0, 0);	// 割り込みプライオリティは最低(0)
	register_interrupt(IRQ2VECT(SD_DMAx_Tx_IRQn), inthdr_sdmmc_dma_tx);

	HAL_NVIC_SetPriority(SDMMC_IRQn, 0, 0);	// 割り込みプライオリティは最低(0)
	register_interrupt(IRQ2VECT(SDMMC_IRQn), inthdr_sd);
#endif //ENABLE_SDMMC_DMA

	return 0;
}

static int sdmmc_unregister(struct st_device *dev)
{
#ifdef ENABLE_SDMMC_DMA
	unregister_interrupt(IRQ2VECT(SD_DMAx_Rx_IRQn));
	unregister_interrupt(IRQ2VECT(SD_DMAx_Tx_IRQn));
	unregister_interrupt(IRQ2VECT(SDMMC_IRQn));
#endif //ENABLE_SDMMC_DMA

	return 0;
}

static int sdmmc_open(struct st_device *dev)
{
	DKFPRINTF(0x01, "dev=%p\n", dev);

	return 0;
}

static int sdmmc_close(struct st_device *dev)
{
	DKFPRINTF(0x01, "dev=%p\n", dev);

	return 0;
}

static uint32_t sdbuf[BLOCKSIZE];

static int sdmmc_block_read(struct st_device *dev, void *data, unsigned int sector, unsigned int blkcount)
{
	unsigned char res;
#ifdef ENABLE_SDMMC_DMA
	uint32_t alignedAddr;
	int tout;
#endif

	DKFPRINTF(0x01, "dev=%p, data=%p, sector=%u, blkcount=%u\n", dev, data, sector, blkcount);

#ifdef ENABLE_SDMMC_DMA
	if((unsigned int)data & 0x3) {
		DKFPRINTF(0x08, "data=%p\n", data);
		/*
		  4のN倍アドレス以外のメモリアドレスにDMAで書き込みを行うとデータ化けが起きる
		  暫定対策として、DMAではSRAMに転送してからSDRAMにコピーする
		 */
		int i;
		for(i=0; i<blkcount; i++) {
			flg_dma_rx_cmp = 0;
			flg_sd_cmp = 0;
			res = BSP_SD_ReadBlocks_DMA((uint32_t *)sdbuf, sector+i, 1);
			if(res != MSD_OK) {
				SYSERR_PRINT("BSP_SD_ReadBlocks_DMA(%d)\n", res);
				goto rd_error;
			}

			tout = event_wait(&dma_rx_evt, 0, SD_TIMEOUT);
			if(tout < 0) {
				SYSERR_PRINT("event_wait dma_rx_evt timeout(res = %d)\n", res);
			}

			res = BSP_SD_GetCardState();
			if(res == SD_TRANSFER_OK) {
				alignedAddr = (uint32_t)sdbuf & ~0x1F;
				SCB_InvalidateDCache_by_Addr((uint32_t *)alignedAddr, BLOCKSIZE + ((uint32_t)sdbuf - alignedAddr));
				memorycopy(data, (const void *)sdbuf, BLOCKSIZE);
				data += BLOCKSIZE;
				res = MSD_OK;
			} else {
				SYSERR_PRINT("BSP_SD_GetCardState error(%d)\n", res);
				res = MSD_ERROR;
				break;
			}
		}
	} else {
		flg_dma_rx_cmp = 0;
		flg_sd_cmp = 0;

		res = BSP_SD_ReadBlocks_DMA((uint32_t *)data, sector, blkcount);
		if(res != MSD_OK) {
			SYSERR_PRINT("BSP_SD_ReadBlocks_DMA(%d)\n", res);
			goto rd_error;
		}

		tout = event_wait(&dma_rx_evt, 0, SD_TIMEOUT);
		if(tout < 0) {
			SYSERR_PRINT("event_wait dma_rx_evt timeout(res = %d)\n", res);
		}

		res = BSP_SD_GetCardState();
		if(res == SD_TRANSFER_OK) {
			alignedAddr = (uint32_t)data & ~0x1F;
			SCB_InvalidateDCache_by_Addr((uint32_t *)alignedAddr, blkcount * BLOCKSIZE + ((uint32_t)data - alignedAddr));
		} else {
			SYSERR_PRINT("BSP_SD_GetCardState error(%d)\n", res);
			res = MSD_ERROR;
		}
	}
rd_error:
#else
	disable_interrupt();	/// @todo 割込を禁止しないと SDMMC_FLAG_RXOVERR エラーとなる。解析する。
	res = BSP_SD_ReadBlocks((uint32_t *)data, sector, blkcount, SD_TIMEOUT);
	enable_interrupt();
#endif

	if(res == MSD_OK) {
		KXDUMP(0x04, data, blkcount * BLOCKSIZE);
		return blkcount;
	} else {
		SYSERR_PRINT("Block read(data=%p, sector=%u, size=%d) error %u\n", data, sector, blkcount, res);
		return 0;
	}
}

static int sdmmc_block_write(struct st_device *dev, const void *data, unsigned int sector, unsigned int blkcount)
{
	unsigned char res = 0;
#ifdef ENABLE_SDMMC_DMA
	int tout;
#endif

	DKFPRINTF(0x01, "dev=%p, data=%p, sector=%u, blkcount=%u\n", dev, data, sector, blkcount);

#ifdef ENABLE_SDMMC_DMA
	flg_dma_tx_cmp = 0;
	flg_sd_cmp = 0;

	res = BSP_SD_WriteBlocks_DMA((uint32_t *)data, sector, blkcount);

	tout = event_wait(&dma_tx_evt, 0, SD_TIMEOUT);
	if(tout < 0) {
		SYSERR_PRINT("event_wait dma_tx_evt timeout(res = %d)\n", res);
	}

	tout = get_kernel_time() + SD_TIMEOUT;
	while(tout > get_kernel_time()) {
		res = BSP_SD_GetCardState();
		if(res == SD_TRANSFER_OK) {
			break;
		}
	}
	if(res != SD_TRANSFER_OK) {
		SYSERR_PRINT("BSP_SD_GetCardState error(%d)\n", res);
	}
#else
	if((unsigned int)data & 0x3) {
		DKFPRINTF(0x10, "data=%p\n", data);
		int i, j;
		for(j=0; j<blkcount; j++) {
			unsigned char *sp = (unsigned char *)data + (j * BLOCKSIZE);
			unsigned char *dp = (unsigned char *)sdbuf;
			for(i=0; i<BLOCKSIZE; i++) {
				*dp = *sp;
				dp ++;
				sp ++;
			}
			disable_interrupt();
			res = BSP_SD_WriteBlocks(sdbuf, sector+j, 1, SD_TIMEOUT);
			enable_interrupt();

			while(BSP_SD_GetCardState() != SD_TRANSFER_OK) {
				;
			}
		}
	} else {
		disable_interrupt();
		res = BSP_SD_WriteBlocks((uint32_t *)data, sector, blkcount, SD_TIMEOUT);
		enable_interrupt();

		while(BSP_SD_GetCardState() != SD_TRANSFER_OK) {
			;
		}
	}
#endif

	if(res == SD_TRANSFER_OK) {
		return blkcount;
	} else {
		SYSERR_PRINT("Block write(data=%p, sector=%u, size=%d) error %u\n", data, sector, blkcount, res);
		return 0;
	}
}

static int sdmmc_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	HAL_SD_CardInfoTypeDef cardInfo;
	int rt = 0;

	DKFPRINTF(0x01, "dev=%p, com=%08x, arg=%08x, param=%p\n", dev, com, arg, param);

	BSP_SD_GetCardInfo(&cardInfo);

	DKPRINTF(0x02, "CardType     = %u\n", cardInfo.CardType);
	DKPRINTF(0x02, "CardVersion  = %u\n", cardInfo.CardVersion);
	DKPRINTF(0x02, "Class        = %u\n", cardInfo.Class);
	DKPRINTF(0x02, "RelCardAdd   = %u\n", cardInfo.RelCardAdd);
	DKPRINTF(0x02, "BlockNbr     = %u\n", cardInfo.BlockNbr);
	DKPRINTF(0x02, "BlockSize    = %u\n", cardInfo.BlockSize);
	DKPRINTF(0x02, "LogBlockNbr  = %u\n", cardInfo.LogBlockNbr);
	DKPRINTF(0x02, "LogBlockSize = %u\n", cardInfo.LogBlockSize);

	switch(com) {
	case IOCMD_SD_GET_SECTOR_COUNT:
		{
#if FF_MAX_SS != FF_MIN_SS
			rt = (int)cardInfo.BlockNbr;
#else
			rt = (int)cardInfo.LogBlockNbr;
#endif
		}
		break;

	case IOCMD_SD_GET_SECTOR_SIZE:
		{
#if FF_MAX_SS != FF_MIN_SS
			rt = (int)cardInfo.BlockSize;
#else
			rt = (int)cardInfo.LogBlockSize;
#endif
		}
		break;

	case IOCMD_SD_GET_BLOCK_SIZE:
		{
			rt = (int)cardInfo.BlockSize;
		}
		break;

	default:
		SYSERR_PRINT("Unknow command %08lX arg %08lX\n", com, arg);
		return 0;
	}

	return rt;
}

static int sdmmc_sync(struct st_device *dev)
{
	return 0;
}

static int sdmmc_suspend(struct st_device *dev)
{
	int rt = 0;

	return rt;
}

static int sdmmc_resume(struct st_device *dev)
{
	int rt = 0;

	return rt;
}


const struct st_device sdmmc_device = {
	.name		= DEF_DEV_NAME_SD,
	.explan		= "STM32F7xxx SD/MMC Storage",
	.register_dev	= sdmmc_register,
	.unregister_dev	= sdmmc_unregister,
	.open		= sdmmc_open,
	.close		= sdmmc_close,
	.block_read	= sdmmc_block_read,
	.block_write	= sdmmc_block_write,
	.ioctl		= sdmmc_ioctl,
	.sync		= sdmmc_sync,
	.suspend	= sdmmc_suspend,
	.resume		= sdmmc_resume,
};
