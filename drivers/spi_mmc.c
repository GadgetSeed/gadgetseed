/** @file
    @brief	MMC ドライバ

    SPIドライバ使用

    @date	2008.01.02
    @author	Takashi SHUDO
*/

#include "device.h"
#include "device/spi_ioctl.h"
#include "device/sd_ioctl.h"
#include "tkprintf.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


static struct st_device *mmc_spi;

/*-----------------------------------------------------------------------*/
/* MMC/SDSC/SDHC (in SPI mode) control module  (C)ChaN, 2007             */
/*-----------------------------------------------------------------------*/
/* Only rcvr_spi(), xmit_spi(), disk_timerproc() and some macros are     */
/* platform dependent.                                                   */
/*-----------------------------------------------------------------------*/


#include "ff.h"
#include "diskio.h"

/* MMC/SD command (in SPI) */
#define CMD0	(0x40+0)	/* GO_IDLE_STATE */
#define CMD1	(0x40+1)	/* SEND_OP_COND */
#define	ACMD41	(0xC0+41)	/* SEND_OP_COND (SDC) */
#define CMD8	(0x40+8)	/* SEND_IF_COND */
#define CMD9	(0x40+9)	/* SEND_CSD */
#define CMD10	(0x40+10)	/* SEND_CID */
#define CMD12	(0x40+12)	/* STOP_TRANSMISSION */
#define ACMD13	(0xC0+13)	/* SD_STATUS (SDC) */
#define CMD16	(0x40+16)	/* SET_BLOCKLEN */
#define CMD17	(0x40+17)	/* READ_SINGLE_BLOCK */
#define CMD18	(0x40+18)	/* READ_MULTIPLE_BLOCK */
#define CMD23	(0x40+23)	/* SET_BLOCK_COUNT */
#define	ACMD23	(0xC0+23)	/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24	(0x40+24)	/* WRITE_BLOCK */
#define CMD25	(0x40+25)	/* WRITE_MULTIPLE_BLOCK */
#define CMD41	(0x40+41)	/* SEND_OP_COND (ACMD) */
#define CMD55	(0x40+55)	/* APP_CMD */
#define CMD58	(0x40+58)	/* READ_OCR */


/* Control signals (Platform dependent) */

static void SELECT(void)
{
	ioctl_device(mmc_spi, IOCMD_SPI_CS0ASSERT, 0, 0);
}

static BYTE rcvr_spi(void);

static void DESELECT(void)
{
	ioctl_device(mmc_spi, IOCMD_SPI_CS0NEGATE, 0, 0);
	//ioctl_device(mmc_spi, IOCMD_SPI_FORCE_UNLOCK, 0, 0);
	rcvr_spi();
}

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

static volatile
DSTATUS Stat = STA_NOINIT;	/* Disk status */

static
BYTE CardType;			/* b0:MMC, b1:SDv1, b2:SDv2, b3:Block addressing */



/*-----------------------------------------------------------------------*/
/* Transmit a byte to MMC via SPI  (Platform dependent)                  */
/*-----------------------------------------------------------------------*/

static
void xmit_spi (BYTE dat)
{
	putc_device(mmc_spi, dat);
}



/*-----------------------------------------------------------------------*/
/* Receive a byte from MMC via SPI (Platform dependent)                  */
/*-----------------------------------------------------------------------*/

static
BYTE rcvr_spi (void)
{
	unsigned char rd;
	getc_device(mmc_spi, &rd);
	DKPRINTF(0x01, "rcvr_spi %02X\n", rd);

	return rd;
}



/*-----------------------------------------------------------------------*/
/* Wait for card ready                                                   */
/*-----------------------------------------------------------------------*/

#include "timer.h"

static
BYTE wait_ready (void)
{
	BYTE res;
	unsigned long tt = get_kernel_time() + 500;	// 500ms

	rcvr_spi();
	do {
		res = rcvr_spi();
	} while ((res != 0xFF) && (tt > get_kernel_time()));
	if(res != 0xFF) {
		SYSERR_PRINT("MMC wait ready timeout(%02X)\n", res);
	}
	return res;
}



/*-----------------------------------------------------------------------*/
/* Deselect the card and release SPI bus                                 */
/*-----------------------------------------------------------------------*/

static
void release_spi (void)
{
	DESELECT();
}



/*-----------------------------------------------------------------------*/
/* Power Control  (Platform dependent)                                   */
/*-----------------------------------------------------------------------*/
/* When the target system does not support socket power control, there   */
/* is nothing to do in these functions and chk_power always returns 1.   */

static
void power_on (void)
{
	DESELECT();

	wait_time(10);

	DKPRINTF(0x01, "MMC On\n");
}


static
void power_off (void)
{
	SELECT();				/* Wait for card ready */
	wait_ready();
	DESELECT();
	rcvr_spi();

	Stat |= STA_NOINIT;		/* Set STA_NOINIT */
	DKPRINTF(0x01, "MMC Off\n");
}



/*-----------------------------------------------------------------------*/
/* Receive a data packet from MMC                                        */
/*-----------------------------------------------------------------------*/

static
int rcvr_datablock (
	BYTE *buff,			/* Data buffer to store received data */
	UINT btr			/* Byte count (must be even number) */
)
{
	BYTE token;

//#define PKT_TOUT	100
#define PKT_TOUT	200
	unsigned long Timer1 = PKT_TOUT + get_kernel_time();
	unsigned long now_time = Timer1;

	DKPRINTF(0x01, "MMC read block buf=%08X size=%d\n", (int)buff, (int)btr);

	do {							/* Wait for data packet in timeout of 100ms */
		token = rcvr_spi();
	} while ((token == 0xFF) && (Timer1 > (now_time = get_kernel_time())));
	if(token != 0xFE) {
		SYSERR_PRINT("data read error(token = %02X)\n", token);
		SYSERR_PRINT("time = %ld\n", now_time - (Timer1 - PKT_TOUT));
		return 0;	/* If not valid data token, retutn with error */
	}

#ifdef NOTUSE_BLOCKREAD
	do {							/* Receive the data block into buffer */
		*buff++ = rcvr_spi();
		*buff++ = rcvr_spi();
	} while (btr -= 2);
#else
//	ioctl_device(mmc_spi, IOCMD_SPI_SPEED, 24000000UL);	// SCK 24MHz
//	ioctl_device(mmc_spi, IOCMD_SPI_SPEED, 12000000UL);	// SCK 12MHz
	read_device(mmc_spi, buff, btr);
#endif
	rcvr_spi();						/* Discard CRC */
	rcvr_spi();

	return 1;					/* Return with success */
}



/*-----------------------------------------------------------------------*/
/* Send a data packet to MMC                                             */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
static
int xmit_datablock (
	const BYTE *buff,	/* 512 byte data block to be transmitted */
	BYTE token			/* Data/Stop token */
)
{
	BYTE resp;
#ifdef NOTUSE_BLOCKREAD
	BYTE wc;
#endif

	if (wait_ready() != 0xFF) return 0;

	xmit_spi(token);					/* Xmit data token */
	if (token != 0xFD) {	/* Is data token */
#ifdef NOTUSE_BLOCKREAD
		wc = 0;
		do {							/* Xmit the 512 byte data block to MMC */
			xmit_spi(*buff++);
			xmit_spi(*buff++);
		} while (--wc);
#else
		write_device(mmc_spi, (unsigned char *)buff, 512);
#endif
		xmit_spi(0xFF);					/* CRC (Dummy) */
		xmit_spi(0xFF);
		resp = rcvr_spi();				/* Reveive data response */
		if ((resp & 0x1F) != 0x05) {		/* If not accepted, return with error */
			SYSERR_PRINT("xmit_datablock() error(%02X)\n", resp);
			return 0;
		}
	}

	return 1;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Send a command packet to MMC                                          */
/*-----------------------------------------------------------------------*/

static
BYTE send_cmd (
	BYTE cmd,		/* Command byte */
	DWORD arg		/* Argument */
)
{
	BYTE n, res;

	DKPRINTF(0x01, "MMC_cmd(%02X,%02X)\n", (int)cmd, (int)arg);

	if (cmd & 0x80) {	/* ACMD<n> is the command sequense of CMD55-CMD<n> */
		cmd &= 0x7F;
		res = send_cmd(CMD55, 0);
		if (res > 1) return res;
	}

	/* Select the card and wait for ready */
	DESELECT();
	SELECT();
	if (wait_ready() != 0xFF) return 0xFF;

	/* Send command packet */
	xmit_spi(cmd);						/* Start + Command index */
	xmit_spi((BYTE)(arg >> 24));		/* Argument[31..24] */
	xmit_spi((BYTE)(arg >> 16));		/* Argument[23..16] */
	xmit_spi((BYTE)(arg >> 8));			/* Argument[15..8] */
	xmit_spi((BYTE)arg);				/* Argument[7..0] */
	n = 0x01;							/* Dummy CRC + Stop */
	if (cmd == CMD0) n = 0x95;			/* Valid CRC for CMD0(0) */
	if (cmd == CMD8) n = 0x87;			/* Valid CRC for CMD8(0x1AA) */
	xmit_spi(n);

	/* Receive command response */
	if (cmd == CMD12) rcvr_spi();		/* Skip a stuff byte when stop reading */
	n = 10;								/* Wait for a valid response in timeout of 10 attempts */
	do
		res = rcvr_spi();
	while ((res & 0x80) && --n);

	return res;			/* Return with the response value */
}



/*--------------------------------------------------------------------------

   Public Functions

---------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------*/
/* Initialize Disk Drive                                                 */
/*-----------------------------------------------------------------------*/

static DSTATUS mmc_disk_initialize (
	BYTE drv		/* Physical drive nmuber (0) */
)
{
	BYTE n, ty, cmd, ocr[4];

	if (drv) return STA_NOINIT;			/* Supports only single drive */
	if (Stat & STA_NODISK) return Stat;	/* No card in the socket */

	power_on();							/* Force socket power on */
	ioctl_device(mmc_spi, IOCMD_SPI_SPEED, 400000UL, 0);	// SCK 400KHz

	for (n = 10; n; n--) rcvr_spi();	/* 80 dummy clocks */

	ty = 0;
	if (send_cmd(CMD0, 0) == 1) {			/* Enter Idle state */
		unsigned long Timer1 = 1000 + get_kernel_time();						/* Initialization timeout of 1000 msec */
		if (send_cmd(CMD8, 0x1AA) == 1) {	/* SDHC */
			for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();		/* Get trailing return value of R7 resp */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {				/* The card can work at vdd range of 2.7-3.6V */
				while ((Timer1 > get_kernel_time()) && send_cmd(ACMD41, 1UL << 30));	/* Wait for leaving idle state (ACMD41 with HCS bit) */
				if ((Timer1 > get_kernel_time()) && send_cmd(CMD58, 0) == 0) {		/* Check CCS bit in the OCR */
					for (n = 0; n < 4; n++) ocr[n] = rcvr_spi();
					ty = (ocr[0] & 0x40) ? 12 : 4;
				}
			}
		} else {							/* SDSC or MMC */
			if (send_cmd(ACMD41, 0) <= 1) 	{
				ty = 2; cmd = ACMD41;	/* SDSC */
			} else {
				ty = 1; cmd = CMD1;		/* MMC */
			}
			while ((Timer1 > get_kernel_time()) && send_cmd(cmd, 0));			/* Wait for leaving idle state */
			if (!(Timer1 > get_kernel_time()) || send_cmd(CMD16, 512) != 0)	/* Set R/W block length to 512 */
				ty = 0;
		}
	} else {
		//SYSERR_PRINT("MMC COMMAND(CMD0) error\n");
	}
	CardType = ty;
	release_spi();

	if (ty) {			/* Initialization succeded */
//		ioctl_device(mmc_spi, IOCMD_SPI_SPEED, 24000000UL, 0);	// SCK 24MHz
		ioctl_device(mmc_spi, IOCMD_SPI_SPEED, 12000000UL, 0);	// SCK 12MHz
		Stat &= ~STA_NOINIT;		/* Clear STA_NOINIT */
	} else {			/* Initialization failed */
		power_off();
		//SYSERR_PRINT("MMC Initialization failed %d\n", ty);
	}

	return Stat;
}


#if 0
/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/

static DSTATUS mmc_disk_status (
	BYTE drv			/* Drive number (0) */
)
{
	return (drv) ? STA_NODISK : Stat;
}
#endif


/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

static DRESULT mmc_disk_read (
	BYTE drv,			/* Physical drive nmuber (0) */
	BYTE *buff,			/* Pointer to the data buffer to store read data */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	if (!(CardType & 8)) sector *= 512;	/* Convert to byte address if needed */

	ioctl_device(mmc_spi, IOCMD_SPI_CS0ASSERT, 0, 0);

	if (count == 1) {	/* Single block read */
		if (send_cmd(CMD17, sector) == 0) {	/* READ_SINGLE_BLOCK */
			if(rcvr_datablock(buff, 512) != 0) {
				count = 0;
			} else {
				SYSERR_PRINT("MMC single block read error\n");
			}
		} else {
			SYSERR_PRINT("MMC CMD17 error\n");
		}
	}
	else {				/* Multiple block read */
		if (send_cmd(CMD18, sector) == 0) {	/* READ_MULTIPLE_BLOCK */
			do {
				if (!rcvr_datablock(buff, 512)) {
					SYSERR_PRINT("MMC multiple block read error\n");
					break;
				}
				buff += 512;
			} while (--count);
			send_cmd(CMD12, 0);				/* STOP_TRANSMISSION */
		}
	}
	release_spi();

	return count ? RES_ERROR : RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _READONLY == 0
static DRESULT mmc_disk_write (
	BYTE drv,			/* Physical drive nmuber (0) */
	const BYTE *buff,	/* Pointer to the data to be written */
	DWORD sector,		/* Start sector number (LBA) */
	BYTE count			/* Sector count (1..255) */
)
{
	if (drv || !count) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;
	if (Stat & STA_PROTECT) return RES_WRPRT;

	if (!(CardType & 8)) sector *= 512;	/* Convert to byte address if needed */

//	ioctl_device(mmc_spi, IOCMD_SPI_CSASSERT, 0);

	if (count == 1) {	/* Single block write */
		if ((send_cmd(CMD24, sector) == 0)	/* WRITE_BLOCK */
			&& xmit_datablock(buff, 0xFE))
			count = 0;
	}
	else {				/* Multiple block write */
		if (CardType & 6) send_cmd(ACMD23, count);
		if (send_cmd(CMD25, sector) == 0) {	/* WRITE_MULTIPLE_BLOCK */
			do {
				if (!xmit_datablock(buff, 0xFC)) break;
				buff += 512;
			} while (--count);
			if (!xmit_datablock(0, 0xFD))	/* STOP_TRAN token */
				count = 1;
		}
	}
	release_spi();

	return count ? RES_ERROR : RES_OK;
}
#endif /* _READONLY == 0 */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

static DRESULT mmc_disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	BYTE n, csd[16], *ptr = buff;
	WORD csize;


	if (drv) return RES_PARERR;
	if (Stat & STA_NOINIT) return RES_NOTRDY;

	res = RES_ERROR;

	ioctl_device(mmc_spi, IOCMD_SPI_CS0ASSERT, 0, 0);

	switch (ctrl) {
		case CTRL_SYNC :		/* Make sure that no pending write process */
			SELECT();
			if (wait_ready() == 0xFF)
				res = RES_OK;
			break;

		case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
				if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
					csize = csd[9] + ((WORD)csd[8] << 8) + 1;
					*(DWORD*)buff = (DWORD)csize << 10;
				} else {					/* SDC ver 1.XX or MMC */
					n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
					csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
					*(DWORD*)buff = (DWORD)csize << (n - 9);
				}
				res = RES_OK;
			}
			break;

		case GET_SECTOR_SIZE :	/* Get R/W sector size (WORD) */
			*(WORD*)buff = 512;
			res = RES_OK;
			break;

		case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
			if (CardType & 4) {			/* SDC ver 2.00 */
				if (send_cmd(ACMD13, 0) == 0) {		/* Read SD status */
					rcvr_spi();
					if (rcvr_datablock(csd, 16)) {				/* Read partial block */
						for (n = 64 - 16; n; n--) rcvr_spi();	/* Purge trailing data */
						*(DWORD*)buff = 16UL << (csd[10] >> 4);
						res = RES_OK;
					}
				}
			} else {					/* SDC ver 1.XX or MMC */
				if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
					if (CardType & 2) {			/* SDC ver 1.XX */
						*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
					} else {					/* MMC */
						*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
					}
					res = RES_OK;
				}
			}
			break;

		case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
			*ptr = CardType;
			res = RES_OK;
			break;

		case MMC_GET_CSD :		/* Receive CSD as a data block (16 bytes) */
			if (send_cmd(CMD9, 0) == 0		/* READ_CSD */
				&& rcvr_datablock(ptr, 16))
				res = RES_OK;
			break;

		case MMC_GET_CID :		/* Receive CID as a data block (16 bytes) */
			if (send_cmd(CMD10, 0) == 0		/* READ_CID */
				&& rcvr_datablock(ptr, 16))
				res = RES_OK;
			break;

		case MMC_GET_OCR :		/* Receive OCR as an R3 resp (4 bytes) */
			if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
				for (n = 4; n; n--) *ptr++ = rcvr_spi();
				res = RES_OK;
			}
			break;

		case MMC_GET_SDSTAT :	/* Receive SD statsu as a data block (64 bytes) */
			if (send_cmd(ACMD13, 0) == 0) {	/* SD_STATUS */
				rcvr_spi();
				if (rcvr_datablock(ptr, 64))
					res = RES_OK;
			}
			break;

		default:
			res = RES_PARERR;
	}

	release_spi();

	return res;
}

/*
  GadgetSeed ドライバ
*/

static int mmc_register(struct st_device *dev, char *param)
{
	mmc_spi = open_device(param);
	if(mmc_spi == 0) {
		SYSERR_PRINT("cannot open device \"%s\"\n", param);
		return -1;
	}

	ioctl_device(mmc_spi, IOCMD_SPI_SPEED, 400000UL, 0);	// SCK 400KHz
	Stat = STA_NOINIT;

	power_off();

	return 0;
}

static int mmc_unregister(struct st_device *dev)
{
	return 0;
}

static int mmc_open(struct st_device *dev)
{
	DSTATUS stat;

	stat = mmc_disk_initialize(0);

	if(stat) {
		//SYSERR_PRINT("MMC Initialize error(%d)\n", (int)stat);
		return -1;
	}

	return 0;
}

static int mmc_close(struct st_device *dev)
{
	Stat = STA_NOINIT;

	power_off();

	return 0;
}

#define S_SIZ 512

static int mmc_block_read(struct st_device *dev, void *data, unsigned int sector, unsigned int count)
{
	if(mmc_disk_read(0, data, sector, count) == RES_OK) {
		return count;
	} else {
		SYSERR_PRINT("MMC read error(data=%p,size=%d)\n", data, count);
		return 0;
	}
}

static int mmc_block_write(struct st_device *dev, const void *data, unsigned int sector, unsigned int count)
{
	if(mmc_disk_write(0, data, sector, count) == RES_OK) {
		return count;
	} else {
		SYSERR_PRINT("MMC write error(data=%p,size=%d)\n", data, count);
		return 0;
	}
}

static int mmc_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	int rt = 0;

	switch(com) {
	case IOCMD_SD_GET_SECTOR_COUNT:
		{
			unsigned int sectorcount = 0;
			mmc_disk_ioctl(0, GET_SECTOR_COUNT, (void *)&sectorcount);
			rt = (int)sectorcount;
		}
		break;

	case IOCMD_SD_GET_SECTOR_SIZE:
		{
			WORD sectorsize = 0;
			mmc_disk_ioctl(0, GET_SECTOR_SIZE, (void *)&sectorsize);
			rt = (int)sectorsize;
		}
		break;

	case IOCMD_SD_GET_BLOCK_SIZE:
		{
			unsigned int blocksize = 0;
			mmc_disk_ioctl(0, GET_BLOCK_SIZE, (void *)&blocksize);
			rt = (int)blocksize;
		}
		break;

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		return 0;
	}

	return rt;
}

static int mmc_sync(struct st_device *dev)
{
	mmc_disk_ioctl(0, CTRL_SYNC, 0);

	return 0;
}

static int mmc_suspend(struct st_device *dev)
{
	power_off();

	return 0;
}

static int mmc_resume(struct st_device *dev)
{
	return mmc_open(dev);	// 暫定
}

const struct st_device mmc_device = {
	.name		= DEF_DEV_NAME_SD,
	.explan		= "MMC/SD SPI mode Strage",
	.register_dev	= mmc_register,
	.unregister_dev	= mmc_unregister,
	.open		= mmc_open,
	.close		= mmc_close,
	.block_read	= mmc_block_read,
	.block_write	= mmc_block_write,
	.ioctl		= mmc_ioctl,
	.sync		= mmc_sync,
	.suspend	= mmc_suspend,
	.resume		= mmc_resume,
};
