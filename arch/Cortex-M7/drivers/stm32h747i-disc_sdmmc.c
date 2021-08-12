/** @file
    @brief	SD MMC ドライバ STM32H747I-Discovery

    @date	2020.01.19
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

#include "stm32h7xx_hal_sd.h"
#include "stm32h747i_discovery_sd.h"
//#define ENABLE_SDMMC_DMA	/// DMA転送を使用する(HAL:FW.H7.1.6.0では動作できない)
#define SDMMC_IRQn	SDMMC1_IRQn

#define SD_TIMEOUT (3*1000)

#define DEBUGKBITS 0x02
#include "dkprintf.h"


#ifdef ENABLE_SDMMC_DMA
extern SD_HandleTypeDef hsd_sdmmc[SD_INSTANCES_NBR];

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

void BSP_SD_AbortCallback(uint32_t Instance)
{
	SYSERR_PRINT("Abort\n");
}

static int flg_dma_rx_cmp = 0;
static int flg_dma_tx_cmp = 0;
static int flg_sd_cmp = 0;

void BSP_SD_ReadCpltCallback(uint32_t Instance)
{
	DKFPRINTF(0x02, "\n");

	flg_dma_rx_cmp = 1;
}

void BSP_SD_WriteCpltCallback(uint32_t Instance)
{
	DKFPRINTF(0x02, "\n");

	flg_dma_tx_cmp = 1;
}


static void inthdr_sd(unsigned int intnum, void *sp)
{
	DKFPRINTF(0x02, "intnum=%d\n", intnum);

	HAL_SD_IRQHandler(&hsd_sdmmc[0]);

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

	stat = BSP_SD_Init(0);

	if(stat != HAL_OK) {
		SYSERR_PRINT("SD/MMC Initialize error(%d)\n", (int)stat);
		return -1;
	}

#ifdef ENABLE_SDMMC_DMA
	//BSP_SD_ITConfig();

	HAL_NVIC_SetPriority(SDMMC_IRQn, 0, 0);	// 割り込みプライオリティは最低(0)
	register_interrupt(IRQ2VECT(SDMMC_IRQn), inthdr_sd);
#endif //ENABLE_SDMMC_DMA

	return 0;
}

static int sdmmc_unregister(struct st_device *dev)
{
#ifdef ENABLE_SDMMC_DMA
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
			res = BSP_SD_ReadBlocks_IT(0, (uint32_t *)sdbuf, sector+i, 1);
			if(res != BSP_ERROR_NONE) {
				SYSERR_PRINT("BSP_SD_ReadBlocks_DMA(%d)\n", res);
				goto rd_error;
			}

			tout = event_wait(&dma_rx_evt, 0, SD_TIMEOUT);
			if(tout < 0) {
				SYSERR_PRINT("event_wait dma_rx_evt timeout(res = %d)\n", res);
			}

			res = BSP_SD_GetCardState(0);
			if(res == SD_TRANSFER_OK) {
				alignedAddr = (uint32_t)sdbuf & ~0x1F;
				SCB_InvalidateDCache_by_Addr((uint32_t *)alignedAddr, BLOCKSIZE + ((uint32_t)sdbuf - alignedAddr));
				memorycopy(data, (const void *)sdbuf, BLOCKSIZE);
				data += BLOCKSIZE;
				res = BSP_ERROR_NONE;
			} else {
				SYSERR_PRINT("BSP_SD_GetCardState error(%d)\n", res);
				res = -1;
				break;
			}
		}
	} else {
		flg_dma_rx_cmp = 0;
		flg_sd_cmp = 0;

		res = BSP_SD_ReadBlocks_IT(0, (uint32_t *)data, sector, blkcount);
		if(res != BSP_ERROR_NONE) {
			SYSERR_PRINT("BSP_SD_ReadBlocks_DMA(%d)\n", res);
			goto rd_error;
		}

		tout = event_wait(&dma_rx_evt, 0, SD_TIMEOUT);
		if(tout < 0) {
			SYSERR_PRINT("event_wait dma_rx_evt timeout(res = %d)\n", res);
		}

		res = BSP_SD_GetCardState(0);
		if(res == SD_TRANSFER_OK) {
			alignedAddr = (uint32_t)data & ~0x1F;
			SCB_InvalidateDCache_by_Addr((uint32_t *)alignedAddr, blkcount * BLOCKSIZE + ((uint32_t)data - alignedAddr));
			res = BSP_ERROR_NONE;
		} else {
			SYSERR_PRINT("BSP_SD_GetCardState error(%d)\n", res);
			res = -1;
		}
	}
rd_error:
#else
	disable_interrupt();	/// @todo 割込を禁止しないと SDMMC_FLAG_RXOVERR エラーとなる。解析する。
	res = BSP_SD_ReadBlocks(0, (uint32_t *)data, sector, blkcount);
	enable_interrupt();
#endif

	if(res == BSP_ERROR_NONE) {
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

	res = BSP_SD_WriteBlocks_IT(0, (uint32_t *)data, sector, blkcount);

	tout = event_wait(&dma_tx_evt, 0, SD_TIMEOUT);
	if(tout < 0) {
		SYSERR_PRINT("event_wait dma_tx_evt timeout(res = %d)\n", res);
	}

	tout = get_kernel_time() + SD_TIMEOUT;
	while(tout > get_kernel_time()) {
		res = BSP_SD_GetCardState(0);
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
			res = BSP_SD_WriteBlocks(0, sdbuf, sector+j, 1);
			enable_interrupt();

			while(BSP_SD_GetCardState(0) != SD_TRANSFER_OK) {
				;
			}
		}
	} else {
		disable_interrupt();
		res = BSP_SD_WriteBlocks(0, (uint32_t *)data, sector, blkcount);
		enable_interrupt();

		while(BSP_SD_GetCardState(0) != SD_TRANSFER_OK) {
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

	BSP_SD_GetCardInfo(0, &cardInfo);

	DKPRINTF(0x02, "CardType     = %u\n", (unsigned int)cardInfo.CardType);
	DKPRINTF(0x02, "CardVersion  = %u\n", (unsigned int)cardInfo.CardVersion);
	DKPRINTF(0x02, "Class        = %u\n", (unsigned int)cardInfo.Class);
	DKPRINTF(0x02, "RelCardAdd   = %u\n", (unsigned int)cardInfo.RelCardAdd);
	DKPRINTF(0x02, "BlockNbr     = %u\n", (unsigned int)cardInfo.BlockNbr);
	DKPRINTF(0x02, "BlockSize    = %u\n", (unsigned int)cardInfo.BlockSize);
	DKPRINTF(0x02, "LogBlockNbr  = %u\n", (unsigned int)cardInfo.LogBlockNbr);
	DKPRINTF(0x02, "LogBlockSize = %u\n", (unsigned int)cardInfo.LogBlockSize);

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
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
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
	.explan		= "STM32H747I SD/MMC Storage",
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
