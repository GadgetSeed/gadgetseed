/** @file
    @brief	QSPI FLASH ROMCコマンド

    @date	2019.11.22
    @author	Takashi SHUDO

    @section qspi_command qspiコマンド

    qspi コマンドには以下のサブコマンドがあります。

    | サブコマンド	| 機能				| 詳細				|
    |:------------------|:------------------------------|:------------------------------|
    | info		| @copybrief com_qspi_info	| @ref com_qspi_info		|
    | eraseblock	| @copybrief com_qspi_eraseblock| @ref com_qspi_eraseblock	|
    | erasechip		| @copybrief com_qspi_erasechip	| @ref com_qspi_erasechip	|
    | status		| @copybrief com_qspi_status	| @ref com_qspi_status		|
    | indirect		| @copybrief com_qspi_indirect	| @ref com_qspi_indirect	|
    | memorymap		| @copybrief com_qspi_memorymap	| @ref com_qspi_memoryumap	|
*/

#include "shell.h"
#include "tprintf.h"
#include "device.h"
#include "device/qspi_ioctl.h"


/*
  info
*/
static int info(int argc, uchar *argv[]);

/**
   @brief	QSPI FLASH ROMデバイスの情報を取得する
*/
static const struct st_shell_command com_qspi_info = {
	.name		= "info",
	.command	= info,
	.usage_str	= "",
	.manual_str	= "Print QSPI FLASH ROM Info",
};

static int info(int argc, uchar *argv[])
{
	int rtn = 0;
	struct st_device *qspi_dev;
	struct st_qspi_info qspi_info;

	qspi_dev = open_device(DEF_DEV_NAME_QSPI);
	if(qspi_dev == 0) {
		tprintf("Cannot open device \"%s\"\n", DEF_DEV_NAME_QSPI);
		return -1;
	}

	rtn = ioctl_device(qspi_dev, IOCMD_QSPI_GET_DEVICE_INFO, 0, (void *)&qspi_info);
	if(rtn == 0) {
		tprintf("Flash size           : %d\n", qspi_info.flash_size);
		tprintf("Erase sector size    : %d\n", qspi_info.erase_sector_size);
		tprintf("Erase sectors number : %d\n", qspi_info.erase_sectors_number);
		tprintf("Program page size    : %d\n", qspi_info.prog_page_size);
		tprintf("Program pages number : %d\n", qspi_info.prog_pages_number);
	} else {
		tprintf("Device \"%s\" ioctl IOCMD_QSPI_GET_DEVICE_INFO error(%d)\n", DEF_DEV_NAME_QSPI, rtn);
	}

	close_device(qspi_dev);

	return rtn;
}

/*
  eraseblock
*/
static int eraseblock(int argc, uchar *argv[]);

/**
   @brief	QSPI FLASH ROMデバイスの任意のアドレスのブロックを消去する
*/
static const struct st_shell_command com_qspi_eraseblock = {
	.name		= "eraseblock",
	.command	= eraseblock,
	.usage_str	= "<block_addr>",
	.manual_str	= "QSPI FLASH ROM Erase 1 block",
};

static int eraseblock(int argc, uchar *argv[])
{
	int rtn = 0;
	struct st_device *qspi_dev;
	unsigned int addr;

	if(argc < 2) {
		print_command_usage(&com_qspi_eraseblock);
		return 0;
	}

	qspi_dev = open_device(DEF_DEV_NAME_QSPI);
	if(qspi_dev == 0) {
		tprintf("Cannot open device \"%s\"\n", DEF_DEV_NAME_QSPI);
		return -1;
	}

	addr = hdstoi(argv[1]);
	rtn = ioctl_device(qspi_dev, IOCMD_QSPI_ERASE_BLOCK, addr, 0);

	tprintf("Result : %02X\n", rtn);

	close_device(qspi_dev);

	return rtn;
}

/*
  status
*/
static int status(int argc, uchar *argv[]);

/**
   @brief	QSPI FLASH ROMデバイスの状態を取得
*/
static const struct st_shell_command com_qspi_status = {
	.name		= "status",
	.command	= status,
	.usage_str	= "",
	.manual_str	= "QSPI FLASH ROM Status",
};

static int status(int argc, uchar *argv[])
{
	int rtn = 0;
	struct st_device *qspi_dev;

	qspi_dev = open_device(DEF_DEV_NAME_QSPI);
	if(qspi_dev == 0) {
		tprintf("Cannot open device \"%s\"\n", DEF_DEV_NAME_QSPI);
		return -1;
	}

	rtn = ioctl_device(qspi_dev, IOCMD_QSPI_GET_STATUS, 0, 0);

	tprintf("Status : %02X\n", rtn);

	close_device(qspi_dev);

	return rtn;
}

/*
  indirect
*/
static int indirect(int argc, uchar *argv[]);

/**
   @brief	QSPI FLASH ROMデバイスを関節アクセスモードにする
*/
static const struct st_shell_command com_qspi_indirect = {
	.name		= "indirect",
	.command	= indirect,
	.usage_str	= "",
	.manual_str	= "Change Indirect mode QSPI FLASH ROM driver",
};

static int indirect(int argc, uchar *argv[])
{
	int rtn = 0;
	struct st_device *qspi_dev;

	qspi_dev = open_device(DEF_DEV_NAME_QSPI);
	if(qspi_dev == 0) {
		tprintf("Cannot open device \"%s\"\n", DEF_DEV_NAME_QSPI);
		return -1;
	}

	rtn = ioctl_device(qspi_dev, IOCMD_QSPI_INDIRECT_MODE, 0, 0);

	tprintf("Result : %02x\n", rtn);

	close_device(qspi_dev);

	return rtn;
}

/*
  memorymap
*/
static int memorymap(int argc, uchar *argv[]);

/**
   @brief	QSPI FLASH ROMデバイスもメモリにマップする
*/
static const struct st_shell_command com_qspi_memoryumap = {
	.name		= "memorymap",
	.command	= memorymap,
	.usage_str	= "",
	.manual_str	= "Change Memory-mapped mode QSPI FLASH ROM driver",
};

static int memorymap(int argc, uchar *argv[])
{
	int rtn = 0;
	struct st_device *qspi_dev;

	qspi_dev = open_device(DEF_DEV_NAME_QSPI);
	if(qspi_dev == 0) {
		tprintf("Cannot open device \"%s\"\n", DEF_DEV_NAME_QSPI);
		return -1;
	}

	rtn = ioctl_device(qspi_dev, IOCMD_QSPI_MEMORYMAP_MODE, 0, 0);

	tprintf("Result : %02X\n", rtn);

	close_device(qspi_dev);

	return rtn;
}


static const struct st_shell_command * const com_qspi_list[] = {
	&com_qspi_info,
	&com_qspi_eraseblock,
	&com_qspi_status,
	&com_qspi_indirect,
	&com_qspi_memoryumap,
	0
};

const struct st_shell_command com_qspi = {
	.name		= "qspi",
	.manual_str	= "QSPI FLASH ROM operation commands",
	.sublist	= com_qspi_list
}; ///< QSPI FLASH ROMデバイス情報取得、制御
