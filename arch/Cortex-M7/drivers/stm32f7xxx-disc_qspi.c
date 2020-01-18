/** @file
    @brief	QSPI FLASH ROM ドライバ STM32F[769I|746G]-Discovery

    @date	2019.11.22
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

#ifdef GSC_TARGET_SYSTEM_STM32F769IDISCOVERY
#include "stm32f769i_discovery_qspi.h"
#elif defined(GSC_TARGET_SYSTEM_STM32F746GDISCOVERY)
#include "stm32746g_discovery_qspi.h"
#endif

//#define DEBUGKBITS 0x01
#include "dkprintf.h"

static QSPI_Info pInfo;
static struct st_mutex qspi_mutex;

static int qspi_register(struct st_device *dev, char *param)
{
	int rt = 0;
#if 0 // init_system2()で以下の処理実施
	unsigned char stat;

	stat = BSP_QSPI_Init();

	if(stat != HAL_OK) {
		SYSERR_PRINT("QSPI Initialize error(%d)\n", (int)stat);
		return -1;
	}
#endif
	rt = BSP_QSPI_GetInfo(&pInfo);

#ifdef GSC_TARGET_SYSTEM_STM32F746GDISCOVERY
	pInfo.EraseSectorSize = 0x2000;	// [TODO] pInfo.EraseSectorSizeは0x1000(4KB)であるがHALドライバは0x2000が1消去ブロックサイズ0x2000(8KB)になっている
	pInfo.EraseSectorsNumber = (pInfo.FlashSize / pInfo.EraseSectorSize);	// pInfo.EraseSectorsNumber;
#endif

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

	for(i=0; i<blkcount; i++) {
		qrt = BSP_QSPI_Read(dp, pInfo.ProgPageSize * (sector + i), pInfo.ProgPageSize);

		if(qrt == QSPI_OK) {
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

	for(i=0; i<blkcount; i++) {
		qrt = BSP_QSPI_Write(dp, pInfo.ProgPageSize * (sector + i), pInfo.ProgPageSize);

		if(qrt == QSPI_OK) {
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

#ifdef GSC_TARGET_SYSTEM_STM32F769IDISCOVERY
extern QSPI_HandleTypeDef QSPIHandle;

uint8_t BSP_QSPI_IndirectMode(void)
{ 
  QSPIHandle.Instance = QUADSPI;
  
  /* Call the DeInit function to reset the driver */
  if (HAL_QSPI_DeInit(&QSPIHandle) != HAL_OK)
  {
    return QSPI_ERROR;
  }
  
  /* System level initialization */
  BSP_QSPI_MspInit(&QSPIHandle, NULL);
  
  /* QSPI initialization */
  /* QSPI freq = SYSCLK /(1 + ClockPrescaler) = 216 MHz/(1+1) = 108 Mhz */
  QSPIHandle.Init.ClockPrescaler     = 1;   /* QSPI freq = 216 MHz/(1+1) = 108 Mhz */
  QSPIHandle.Init.FifoThreshold      = 16;
  QSPIHandle.Init.SampleShifting     = QSPI_SAMPLE_SHIFTING_HALFCYCLE; 
  QSPIHandle.Init.FlashSize          = POSITION_VAL(MX25L512_FLASH_SIZE) - 1;
  QSPIHandle.Init.ChipSelectHighTime = QSPI_CS_HIGH_TIME_4_CYCLE; /* Min 30ns for nonRead */
  QSPIHandle.Init.ClockMode          = QSPI_CLOCK_MODE_0;
  QSPIHandle.Init.FlashID            = QSPI_FLASH_ID_1;
  QSPIHandle.Init.DualFlash          = QSPI_DUALFLASH_DISABLE;
  
  if (HAL_QSPI_Init(&QSPIHandle) != HAL_OK)
  {
    return QSPI_ERROR;
  }

  return QSPI_OK;
}
#endif

static int qspi_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	int rt = 0;

	switch(com) {
	case IOCMD_SD_GET_SECTOR_COUNT:
	case IOCMD_SD_GET_SECTOR_SIZE:
		{
			rt = pInfo.ProgPageSize;
		}
		break;

	case IOCMD_SD_GET_BLOCK_SIZE:
		{
			rt = pInfo.EraseSectorSize;
		}
		break;

	case IOCMD_QSPI_GET_DEVICE_INFO:
		{
			struct st_qspi_info *qspi = param;

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
			rt = BSP_QSPI_Erase_Block(pInfo.EraseSectorSize * arg);
		}
		break;

	case IOCMD_QSPI_ERASE_CHIP:
		{
			rt = BSP_QSPI_Erase_Chip();
		}
		break;

	case IOCMD_QSPI_GET_STATUS:
		{
			rt = BSP_QSPI_GetStatus();
		}
		break;

	case IOCMD_QSPI_INDIRECT_MODE:
		{
#ifdef GSC_TARGET_SYSTEM_STM32F769IDISCOVERY
			rt = BSP_QSPI_IndirectMode();
#else
			rt = BSP_QSPI_Init();
#endif
		}
		break;

	case IOCMD_QSPI_MEMORYMAP_MODE:
		{
			rt = BSP_QSPI_EnableMemoryMappedMode();
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
	.explan		= "STM32F7xxx QSPI FLASH ROM",
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
