/** @file
    @brief	QSPI FLASH ROM ドライバ STM32H747I-Discovery

    @date	2020.01.26
    @author	Takashi SHUDO

    @note	NOT TESTED

    QSPI FLASH ROMはメモリマップモードを使用した場合、BSP_QSPI_Write()、BSP_QSPI_Read()は使用できない
*/

#include "sysconfig.h"
#include "tkprintf.h"
#include "timer.h"
#include "device.h"
#include "str.h"
#include "interrupt.h"
#include "device/sd_ioctl.h"
#include "device/qspi_ioctl.h"
#include "task/mutex.h"
#include "task/syscall.h"

#include "stm32h747i_discovery_qspi.h"

//#define DEBUGKBITS 0x01
#include "dkprintf.h"

extern BSP_QSPI_Ctx_t     QSPI_Ctx[QSPI_INSTANCES_NUMBER];

static BSP_QSPI_Info_t pInfo;
static struct st_mutex qspi_mutex;
int flg_qspi_memmap = 0;

static int32_t init_qspi_stm32h747idisc(void)
{
 	BSP_QSPI_Init_t init;

	init.InterfaceMode = MT25TL01G_QPI_MODE;
	init.TransferRate = MT25TL01G_DTR_TRANSFER;
	init.DualFlashMode = MT25TL01G_DUALFLASH_ENABLE;

	return BSP_QSPI_Init(0, &init);
}

void init_qspi(void)
{
	uint8_t RetVal = 0;
	
	// QSPI-ROM
	if((RetVal = init_qspi_stm32h747idisc()) != BSP_ERROR_NONE) {
		SYSERR_PRINT("Failed to initialize the QSPI !! (Error %d)\n", RetVal);
	} else {
#ifdef GSC_DEV_QSPI_MEMORYMAP
		if((RetVal = BSP_QSPI_EnableMemoryMappedMode(0)) != BSP_ERROR_NONE) {
			SYSERR_PRINT("Failed to configure the QSPI !! (Error %d)\n", RetVal);
		} else {
			flg_qspi_memmap = 1;
			DKFPRINTF(0x01, "flg_qspi_memmap = %d\n", flg_qspi_memmap);
		}
#endif
	}
}

static int qspi_register(struct st_device *dev, char *param)
{
	int rt = 0;

	rt = BSP_QSPI_GetInfo(0, &pInfo);

	return rt;
}

static int qspi_unregister(struct st_device *dev)
{
	return 0;
}

static int qspi_open(struct st_device *dev)
{
	DKFPRINTF(0x01, "dev=%p\n", dev);

	return 0;
}

static int qspi_close(struct st_device *dev)
{
	DKFPRINTF(0x01, "dev=%p\n", dev);

	return 0;
}

static int qspi_block_read(struct st_device *dev, void *data, unsigned int sector, unsigned int blkcount)
{
	int i;
	uint8_t *dp = data;
	int rt = 0;
	unsigned char qrt = 0;

	DKFPRINTF(0x01, "sector = %u, blkcount = %u\n", sector, blkcount);

	for(i=0; i<blkcount; i++) {
		qrt = BSP_QSPI_Read(0, dp, pInfo.ProgPageSize * (sector + i), pInfo.ProgPageSize);

		if(qrt == BSP_ERROR_NONE) {
			dp += pInfo.ProgPageSize;
		} else {
			rt = -qrt;
			goto error;
		}
	}
	rt = i;

error:
	return rt;
}

static int qspi_block_write(struct st_device *dev, const void *data, unsigned int sector, unsigned int blkcount)
{
	int i;
	uint8_t *dp = (uint8_t *)data;
	int rt = 0;
	unsigned char qrt = 0;

	DKFPRINTF(0x01, "sector = %u, blkcount = %u\n", sector, blkcount);

	for(i=0; i<blkcount; i++) {
		qrt = BSP_QSPI_Write(0, dp, pInfo.ProgPageSize * (sector + i), pInfo.ProgPageSize);

		if(qrt == BSP_ERROR_NONE) {
			dp += pInfo.ProgPageSize;
		} else {
			rt = -qrt;
			goto error;
		}
	}
	rt = i;

error:

	return rt;
}

static int qspi_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	int rt = 0;

	DKFPRINTF(0x01, "com = %08X, arg = %08X\n", com, arg);

	switch(com) {
	case IOCMD_SD_GET_SECTOR_COUNT:
	case IOCMD_SD_GET_SECTOR_SIZE:
		{
			rt = pInfo.ProgPageSize;
			DKFPRINTF(0x01, "Program page size = %d\n", rt);
		}
		break;

	case IOCMD_SD_GET_BLOCK_SIZE:
		{
			rt = pInfo.EraseSectorSize;
			DKFPRINTF(0x01, "Erase sector size = %d\n", rt);
		}
		break;

	case IOCMD_QSPI_GET_DEVICE_INFO:
		{
			struct st_qspi_info *qspi = param;

			DKFPRINTF(0x01, "Device info\n");

			qspi->flash_size = pInfo.FlashSize;
			qspi->erase_sector_size = pInfo.EraseSectorSize;
			qspi->erase_sectors_number = pInfo.EraseSectorsNumber;
			qspi->prog_page_size = pInfo.ProgPageSize;
			qspi->prog_pages_number = pInfo.ProgPagesNumber;

			rt = 0;
		}
		break;

	case IOCMD_QSPI_ERASE_BLOCK:
		{
			DKFPRINTF(0x01, "Erase block = %u\n", arg);
			rt = BSP_QSPI_EraseBlock(0, pInfo.EraseSectorSize * arg, MT25TL01G_ERASE_4K);
		}
		break;

	case IOCMD_QSPI_ERASE_CHIP:
		{
			DKFPRINTF(0x01, "Erase chip\n");
			rt = BSP_QSPI_EraseChip(0);
		}
		break;

	case IOCMD_QSPI_GET_STATUS:
		{
			DKFPRINTF(0x01, "Get status\n");
			rt = BSP_QSPI_GetStatus(0);
		}
		break;

	case IOCMD_QSPI_INDIRECT_MODE:
		{
			DKFPRINTF(0x01, "Initialize INDIRECT_MODE\n");
			rt = BSP_QSPI_DeInit(0);
			if(rt != BSP_ERROR_NONE) {
				SYSERR_PRINT("QSPI deinit error(%d)\n", rt);
			}
			rt = init_qspi_stm32h747idisc();
			if(rt != BSP_ERROR_NONE) {
				SYSERR_PRINT("QSPI init error(%d)\n", rt);
			}
		}
//		if(0) //!!!
		{
			DKFPRINTF(0x01, "Indirect mode\n");
//			if(flg_qspi_memmap == 0) {
			if(QSPI_Ctx[0].IsInitialized == QSPI_ACCESS_INDIRECT) {
				DKFPRINTF(0x01, "Already indirect mode\n");
			} else {
				// 一度でもマップ済みのメモリにアクセスしないとマップモード解除でエラーになる?
#if 0
				int i;
				for(i=0; i<256; i++) {
					volatile unsigned char dummy = *((int *)(0x90000000+i));
					(void)dummy;
				}
#endif
				//rt = BSP_QSPI_GetStatus(0);
				//DKFPRINTF(0x01, "BSP_QSPI_GetStatus() = %d\n", rt);
				rt = BSP_QSPI_DisableMemoryMappedMode(0);
				if(rt != BSP_ERROR_NONE) {
					SYSERR_PRINT("Failed to disabele memory map the QSPI !! (Error %d)\n", rt);
				} else {
					flg_qspi_memmap = 0;
					DKFPRINTF(0x01, "flg_qspi_memmap = %d\n", flg_qspi_memmap);
					DKFPRINTF(0x01, "Change indirect mode success\n");
				}
			}
		}
		if(0) {
			DKFPRINTF(0x01, "Initialize\n");
			rt = BSP_QSPI_DeInit(0);
			if(rt != BSP_ERROR_NONE) {
				SYSERR_PRINT("QSPI deinit error(%d)\n", rt);
			}
			rt = init_qspi_stm32h747idisc();
			if(rt != BSP_ERROR_NONE) {
				SYSERR_PRINT("QSPI init error(%d)\n", rt);
			}
		}
		break;

	case IOCMD_QSPI_MEMORYMAP_MODE:
		{
			DKFPRINTF(0x01, "Initialize MEMORYMAP_MODE\n");
			rt = BSP_QSPI_DeInit(0);
			if(rt != BSP_ERROR_NONE) {
				SYSERR_PRINT("QSPI deinit error(%d)\n", rt);
			}
			rt = init_qspi_stm32h747idisc();
			if(rt != BSP_ERROR_NONE) {
				SYSERR_PRINT("QSPI init error(%d)\n", rt);
			}
		}
		{
			DKFPRINTF(0x01, "Memory map mode\n");
//			if(flg_qspi_memmap != 0) {
			if(QSPI_Ctx[0].IsInitialized == QSPI_ACCESS_MMP) {
				DKFPRINTF(0x01, "Already memory map mode\n");
			} else {
				//rt = BSP_QSPI_GetStatus(0);
				//DKFPRINTF(0x01, "BSP_QSPI_GetStatus() = %d\n", rt);
				rt = BSP_QSPI_EnableMemoryMappedMode(0);
				if(rt != BSP_ERROR_NONE) {
					SYSERR_PRINT("Failed to configure the QSPI !! (Error %d)\n", rt);
				} else {
					flg_qspi_memmap = 1;
					DKFPRINTF(0x01, "flg_qspi_memmap = %d\n", flg_qspi_memmap);
					DKFPRINTF(0x01, "Enable memory map mode success\n");
				}
			}
		}
		break;

	case IOCMD_QSPI_INIT:
		{
			DKFPRINTF(0x01, "Initialize\n");
			rt = BSP_QSPI_DeInit(0);
			if(rt != BSP_ERROR_NONE) {
				SYSERR_PRINT("QSPI deinit error(%d)\n", rt);
			}
			rt = init_qspi_stm32h747idisc();
			if(rt != BSP_ERROR_NONE) {
				SYSERR_PRINT("QSPI init error(%d)\n", rt);
			}
		}
		break;

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		return 0;
	}

	return rt;
}

static int qspi_suspend(struct st_device *dev)
{
	int rt = 0;

	return rt;
}

static int qspi_resume(struct st_device *dev)
{
	int rt = 0;

	return rt;
}


const struct st_device qspi_device = {
	.name		= DEF_DEV_NAME_QSPI,
	.explan		= "STM32H747I-Disc QSPI FLASH ROM",
	.mutex		= &qspi_mutex,
	.register_dev	= qspi_register,
	.unregister_dev	= qspi_unregister,
	.open		= qspi_open,
	.close		= qspi_close,
	.block_read	= qspi_block_read,
	.block_write	= qspi_block_write,
	.ioctl		= qspi_ioctl,
	.suspend	= qspi_suspend,
	.resume		= qspi_resume,
};
