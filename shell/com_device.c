/** @file
    @brief	デバイスアクセスコマンド

    @date	2007.04.22
    @author	Takashi SHUDO

    @section dev_command devコマンド

    dev コマンドには以下のサブコマンドがあります。

    | サブコマンド	| 機能				| 詳細			|
    |:------------------|:------------------------------|:----------------------|
    | list		| @copybrief com_dev_list	| @ref com_dev_list	|
    | open		| @copybrief com_dev_open	| @ref com_dev_open	|
    | putc		| @copybrief com_dev_putc	| @ref com_dev_putc	|
    | getc		| @copybrief com_dev_getc	| @ref com_dev_getc	|
    | write		| @copybrief com_dev_write	| @ref com_dev_write	|
    | read		| @copybrief com_dev_read	| @ref com_dev_read	|
    | dump		| @copybrief com_dev_dump	| @ref com_dev_dump	|
    | ioctl		| @copybrief com_dev_ioctl	| @ref com_dev_ioctl	|
    | seek		| @copybrief com_dev_seek	| @ref com_dev_seek	|
    | blockread		| @copybrief com_dev_blockread	| @ref com_dev_blockread	|
    | capacity		| @copybrief com_dev_capacity	| @ref com_dev_capacity	|
    | suspend		| @copybrief com_dev_suspend	| @ref com_dev_suspend	|
    | resume		| @copybrief com_dev_resume	| @ref com_dev_resume	|
    | close		| @copybrief com_dev_close	| @ref com_dev_close	|
*/

#include "shell.h"
#include "lineedit.h"
#include "console.h"
#include "str.h"
#include "tprintf.h"

#define MAX_DEVWRITECNT	32

static int dev_list(int argc, uchar *argv[])
{
	int i;

	for(i=0; i<device_num(); i++) {
		tprintf("%9s : %s\n", device_name(i), device_explan(i));
	}

	return 0;
}

/**
   @brief	登録されているデバイスリスト表示
*/
static const struct st_shell_command com_dev_list = {
	.name		= "list",
	.command	= dev_list,
	.manual_str	= "Print device list"
};


static int dev_open(int argc, uchar *argv[]);

/**
   @brief	デバイスを開く
*/
static const struct st_shell_command com_dev_open = {
	.name		= "open",
	.command	= dev_open,
	.usage_str	= "<device_name>",
	.manual_str	= "Open(only) device"
};

static int dev_open(int argc, uchar *argv[])
{
	struct st_device *dev;

	if(argc < 2) {
		print_command_usage(&com_dev_open);
		return 0;
	}

	dev = open_device((char *)argv[1]);

	tprintf("return : 0x%p\n", dev);

	return 0;
}


static int dev_putc(int argc, uchar *argv[]);

/**
   @brief	デバイスに1バイトデータを書き込む
*/
static const struct st_shell_command com_dev_putc = {
	.name		= "putc",
	.command	= dev_putc,
	.usage_str	= "<device_name> [data ...]",
	.manual_str	= "1byte write data"
};

static int dev_putc(int argc, uchar *argv[])
{
	struct st_device *dev;
	int i;
	unsigned char data;
	int len = 0;
	int rt;

	if(argc < 3) {
		print_command_usage(&com_dev_putc);
		return 0;
	}

	dev = open_device((char *)argv[1]);

	if(dev == 0) {
		tprintf("Cannot open \"%s\".\n", argv[1]);
		return 0;
	}

	for(i=2; i<argc; i++) {
		data = (unsigned char)hdstou(argv[i]);
		rt = putc_device(dev, data);
		len ++;
	}

	tprintf("count : %d\n", len);

	tprintf("return : %d(0x%08X)\n", rt, rt);

	close_device(dev);

	return 0;
}


static int dev_getc(int argc, uchar *argv[]);

/**
   @brief	デバイスより1バイトデータを読み出す
*/
static const struct st_shell_command com_dev_getc = {
	.name		= "getc",
	.command	= dev_getc,
	.usage_str	= "<device_name>",
	.manual_str	= "1byte read data"
};

static int dev_getc(int argc, uchar *argv[])
{
	struct st_device *dev;
	unsigned char data;
	int rt;

	if(argc < 2) {
		print_command_usage(&com_dev_getc);
		return 0;
	}

	dev = open_device((char *)argv[1]);

	if(dev == 0) {
		tprintf("Cannot open \"%s\".\n", argv[1]);
		return 0;
	}

	rt = getc_device(dev, &data);

	tprintf("data : %02X\n", (int)data);

	tprintf("return : %d(0x%08X)\n", rt, rt);

	close_device(dev);

	return 0;
}


static int dev_write(int argc, uchar *argv[]);

/**
   @brief	デバイスに任意のサイズのデータを書き込む
*/
static const struct st_shell_command com_dev_write = {
	.name		= "write",
	.command	= dev_write,
	.usage_str	= "<device_name> [data ...]",
	.manual_str	= "Write data to device"
};

static int dev_write(int argc, uchar *argv[])
{
	struct st_device *dev;
	int i;
	unsigned char buf[MAX_DEVWRITECNT];
	unsigned char *bp = buf;
	unsigned int len = 0;
	int rt;

	if(argc < 3) {
		print_command_usage(&com_dev_write);
		return 0;
	}

	dev = open_device((char *)argv[1]);

	if(dev == 0) {
		tprintf("Cannot open \"%s\".\n", argv[1]);
		return 0;
	}

	for(i=2; i<argc; i++) {
		if(argv[i][0] == '\'') {
			(void)strncopy(bp, &argv[i][1], MAX_DEVWRITECNT);
			len += strleng(&argv[i][1]);
		} else {
			*bp = (unsigned char)hdstou(argv[i]);
			bp ++;
			len ++;
		}
	}

	tprintf("count : %d\n", len);

	rt = write_device(dev, buf, len);

	tprintf("return : %d(0x%08X)\n", rt, rt);

//	close_device(dev);

	return 0;
}

#define MAX_DEVREADCNT	512

static int dev_read(int argc, uchar *argv[]);

/**
   @brief	デバイスより任意のサイズのデータを読み出す
*/
static const struct st_shell_command com_dev_read = {
	.name		= "read",
	.command	= dev_read,
	.usage_str	= "<device_name> <size>",
	.manual_str	= "Read data from device"
};

static int dev_read(int argc, uchar *argv[])
{
	struct st_device *dev;
	int i;
	unsigned char buf[MAX_DEVREADCNT];
	unsigned int size = 0;
	int rt;

	if(argc < 3) {
		print_command_usage(&com_dev_read);
		return 0;
	}

	dev = open_device((char *)argv[1]);

	if(dev == 0) {
		tprintf("Cannot open \"%s\".\n", argv[1]);
		return 0;
	}

	if(argc > 2) {
		size = (unsigned int)dstoi((uchar *)argv[2]);
		if(size > MAX_DEVREADCNT) {
			size = MAX_DEVREADCNT;
		}
	}

	rt = read_device(dev, buf, size);

	for(i=0; i<rt; i++) {
		tprintf("%02x ", (int)buf[i]);
	}

	tprintf("\n");
	tprintf("return : %d(0x%08X)\n", rt, rt);

//	close_device(dev);

	return 0;
}


static int dev_dump(int argc, uchar *argv[]);

/**
   @brief	デバイスより任意のサイズのデータを読み出しダンプ表示する
*/
static const struct st_shell_command com_dev_dump = {
	.name		= "dump",
	.command	= dev_dump,
	.usage_str	= "<device_name> [start [size]]",
	.manual_str	= "Dump data from device"
};

static int dev_dump(int argc, uchar *argv[])
{
	struct st_device *dev;
	int i, j;
	unsigned char buf[MAX_DEVREADCNT];
	int start = 0;
	unsigned int size = MAX_DEVREADCNT;
	int rt = 0;

	if(argc < 3) {
		print_command_usage(&com_dev_dump);
		return 0;
	}

	dev = open_device((char *)argv[1]);

	if(dev == 0) {
		tprintf("Cannot open \"%s\".\n", argv[1]);
		return 0;
	}

	if(argc > 2) {
		start = hdstoi(argv[2]);
	}

	if(argc > 3) {
		size = (unsigned int)hdstoi(argv[3]);
		if(size > MAX_DEVREADCNT) {
			size = MAX_DEVREADCNT;
		}
	}

	seek_device(dev, start, SEEK_SET);
	rt += read_device(dev, buf, size);

	for(j=0; j<rt; j+=16) {
		uchar rd;

		tprintf("%08X : ", start+j);

		for(i=0; i<8; i++) {
			if((j+i) < rt) {
				tprintf("%02X ", (int)buf[j+i]);
			} else {
				tprintf("   ");
			}
		}
		tprintf(" ");
		for(i=8; i<16; i++) {
			if((j+i) < rt) {
				tprintf("%02X ", (int)buf[j+i]);
			} else {
				tprintf("   ");
			}
		}

		tprintf("  \"");

		for(i=0; i<16; i++) {
			if((j+i) < rt) {
				if(((' ' <= buf[j+i]) && (buf[j+i] <= 'Z'))
				   || (('a' <= buf[j+i]) && (buf[j+i] <= 'z'))) {
					cputc(buf[j+i]);
				} else {
					cputc('.');
				}
			} else {
				break;
			}
		}
		tprintf("\"\n");

		if(cgetcnw(&rd) == 0) {
			if(rd == ASCII_CTRL_C) {
				tprintf("Abort.\n");
				return 0;
			}
		}
	}

	tprintf("\n");
	tprintf("return : %d(0x%08X)\n", rt, rt);

//	close_device(dev);

	return 0;
}


static int dev_seek(int argc, uchar *argv[]);

/**
   @brief	デバイスのアクセス位置を設定する
*/
static const struct st_shell_command com_dev_seek = {
	.name		= "seek",
	.command	= dev_seek,
	.usage_str	= "<device_name> [offset [whence]]",
	.manual_str	= "Seek device address"
};

static int dev_seek(int argc, uchar *argv[])
{
	struct st_device *dev;
	int offset = 0;
	int whence = 0;
	int rt = 0;

	if(argc < 2) {
		print_command_usage(&com_dev_seek);
		return 0;
	}

	dev = open_device((char *)argv[1]);

	if(dev == 0) {
		tprintf("Cannot open \"%s\".\n", argv[1]);
		return 0;
	}

	if(argc > 2) {
		offset = hdstoi(argv[2]);
	}

	if(argc > 3) {
		whence = hdstoi(argv[3]);
	}

	rt = seek_device(dev, offset, whence);

	tprintf("return : %d(0x%08X)\n", rt, rt);

//	close_device(dev);

	return 0;
}


static int dev_blockread(int argc, uchar *argv[]);

/**
   @brief	デバイスよりブロック読み出しを行う
*/
static const struct st_shell_command com_dev_blockread = {
	.name		= "blockread",
	.command	= dev_blockread,
	.usage_str	= "<device_name> <sector> <count>",
	.manual_str	= "Block read device"
};

static int dev_blockread(int argc, uchar *argv[])
{
	struct st_device *dev;
	unsigned char buf[MAX_DEVREADCNT];
	unsigned int sector = 0;
	unsigned int count = 0;
	int rt = 0;
	int i;

	if(argc < 4) {
		print_command_usage(&com_dev_blockread);
		return 0;
	}

	dev = open_device((char *)argv[1]);

	if(dev == 0) {
		tprintf("Cannot open \"%s\".\n", argv[1]);
		return 0;
	}

	sector = hdstoi(argv[2]);
	count = hdstoi(argv[3]);

	for(i=0; i<count; i++) {
		rt = block_read_device(dev, buf, sector+i, 1);
		if(rt != 1) {
			break;
		} else {
			xadump(512*i, buf, 512);	// [TODO] 1セクタは512Bytes固定
		}
	}

	tprintf("return : %d(0x%08X)\n", rt, rt);

//	close_device(dev);

	return 0;
}


static int dev_capacity(int argc, uchar *argv[]);

/**
   @brief	デバイスの容量を取得する
*/
static const struct st_shell_command com_dev_capacity = {
	.name		= "capacity",
	.command	= dev_capacity,
	.usage_str	= "<device_name>",
	.manual_str	= "Print device capacity"
};

#include "device/sd_ioctl.h"

static int dev_capacity(int argc, uchar *argv[])
{
	struct st_device *dev;
	int rt = 0;

	if(argc < 2) {
		print_command_usage(&com_dev_capacity);
		return 0;
	}

	dev = open_device((char *)argv[1]);

	if(dev == 0) {
		tprintf("Cannot open \"%s\".\n", argv[1]);
		return 0;
	}

	rt = ioctl_device(dev, IOCMD_SD_GET_SECTOR_COUNT, 0, 0);
	if(rt == 0) {
		tprintf("IOCMD_SD_GET_SECTOR_COUNT return %d\n", rt);
	} else {
		tprintf("Sector count : %d(0x%08X)\n", rt, rt);
	}

	rt = ioctl_device(dev, IOCMD_SD_GET_SECTOR_SIZE, 0, 0);
	if(rt == 0) {
		tprintf("IOCMD_SD_GET_SECTOR_SIZE return %d\n", rt);
	} else {
		tprintf("Sector size  : %d(0x%08X)\n", rt, rt);
	}

	rt = ioctl_device(dev, IOCMD_SD_GET_BLOCK_SIZE, 0, 0);
	if(rt == 0) {
		tprintf("IOCMD_SD_GET_BLOCK_SIZE return %d\n", rt);
	} else {
		tprintf("Block size   : %d(0x%08X)\n", rt, rt);
	}

	//close_device(dev);

	return 0;
}


static int dev_ioctl(int argc, uchar *argv[]);

/**
   @brief	デバイスの制御(ioctl)を行う
*/
static const struct st_shell_command com_dev_ioctl = {
	.name		= "ioctl",
	.command	= dev_ioctl,
	.usage_str	= "<device_name> <command> [arg]",
	.manual_str	= "Ioctl device"
};

static int dev_ioctl(int argc, uchar *argv[])
{
	struct st_device *dev;
	unsigned int cmd = 0;
	unsigned int arg = 0;
	int rt = 0;

	if(argc < 2) {
		print_command_usage(&com_dev_ioctl);
		return 0;
	}

	dev = open_device((char *)argv[1]);

	if(dev == 0) {
		tprintf("Cannot open \"%s\".\n", argv[1]);
		return 0;
	}

	if(argc > 2) {
		cmd = hdstou(argv[2]);
	}

	if(argc > 3) {
		arg = hdstou(argv[3]);
	}

	rt = ioctl_device(dev, cmd, arg, 0);

	tprintf("return : %d(0x%08X)\n", rt, rt);

	//close_device(dev);

	return 0;
}


static int dev_suspend(int argc, uchar *argv[]);

/**
   @brief	デバイスをサスペンド状態にする(実験的なAPI)
*/
static const struct st_shell_command com_dev_suspend = {
	.name		= "suspend",
	.command	= dev_suspend,
	.usage_str	= "[device_name]",
	.manual_str	= "Suspend device"
};

static int dev_suspend(int argc, uchar *argv[])
{
	struct st_device *dev;
	int rt = 0;

	if(argc < 2) {
		suspend();
		return 0;
	}

	dev = open_device((char *)argv[1]);

	if(dev == 0) {
		tprintf("Cannot open \"%s\".\n", argv[1]);
		return 0;
	}

	rt = suspend_device(dev);

	tprintf("return : %d(0x%08X)\n", rt, rt);

//	close_device(dev);

	return 0;
}


static int dev_resume(int argc, uchar *argv[]);

/**
   @brief	デバイスをレジューム状態にする(実験的なAPI)
*/
static const struct st_shell_command com_dev_resume = {
	.name		= "resume",
	.command	= dev_resume,
	.usage_str	= "[device_name]",
	.manual_str	= "Resume device"
};

static int dev_resume(int argc, uchar *argv[])
{
	struct st_device *dev;
	int rt = 0;

	if(argc < 2) {
		resume();
		return 0;
	}

	dev = open_device((char *)argv[1]);

	if(dev == 0) {
		tprintf("Cannot open \"%s\".\n", argv[1]);
		return 0;
	}

	rt = resume_device(dev);

	tprintf("return : %d(0x%08X)\n", rt, rt);

//	close_device(dev);

	return 0;
}


static int dev_close(int argc, uchar *argv[]);

/**
   @brief	デバイスを閉じる
*/
static const struct st_shell_command com_dev_close = {
	.name		= "close",
	.command	= dev_close,
	.usage_str	= "<device_name>",
	.manual_str	= "Close device"
};

static int dev_close(int argc, uchar *argv[])
{
	struct st_device *dev;
	long rt = 0;

	if(argc < 2) {
		print_command_usage(&com_dev_close);
		return 0;
	}

	dev = open_device((char *)argv[1]);

	rt = close_device(dev);

	tprintf("return : %ld(0x%08lX)\n", rt, rt);

	return 0;
}


static const struct st_shell_command * const com_device_list[] = {
	&com_dev_list,
	&com_dev_open,
	&com_dev_putc,
	&com_dev_getc,
	&com_dev_write,
	&com_dev_read,
	&com_dev_dump,
	&com_dev_ioctl,
	&com_dev_seek,
	&com_dev_blockread,
//	&com_dev_blockwrite,	// @todo devコマンドにblockwrite追加
	&com_dev_capacity,
	&com_dev_suspend,
	&com_dev_resume,
	&com_dev_close,
	0
};

const struct st_shell_command com_dev = {
	.name		= "dev",	///< デバイスドライバ操作コマンド
	.manual_str	= "Device operation commands",
	.sublist	= com_device_list
}; ///< デバイスドライバ操作
