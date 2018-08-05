/** @file
    @brief	I2Cコマンド

    @date	2015.07.20
    @author	Takashi SHUDO

    @section i2c_command i2cコマンド

    i2c コマンドには以下のサブコマンドがあります。

    | サブコマンド	| 機能				| 詳細			|
    |:------------------|:------------------------------|:----------------------|
    | detect		| @copybrief com_i2c_detect	| @ref com_i2c_detect	|
    | dump		| @copybrief com_i2c_dump	| @ref com_i2c_dump	|
    | get		| @copybrief com_i2c_get	| @ref com_i2c_get	|
    | put		| @copybrief com_i2c_put	| @ref com_i2c_put	|
*/

#include "asm.h"
#include "shell.h"
#include "lineedit.h"
#include "console.h"
#include "timer.h"
#include "str.h"
#include "tprintf.h"
#include "device.h"
#include "device/i2c_ioctl.h"

#define MAX_I2C_DATA	256

struct st_device *i2c_dev;
static const char def_dev_name[] = DEF_DEV_NAME_I2C;
static char *dev_name = (char *)def_dev_name;
static unsigned char reg_data[MAX_I2C_DATA];

/*
  I2Cデバイス検出
*/
static int check_devaddr(unsigned int addr)
{
	int rtn = 0;
	unsigned char data = 0;

	lock_device(i2c_dev, 0);

	ioctl_device(i2c_dev, IOCMD_I2C_MEMADDRSIZE, I2C_MEM_ADDR_SIZE_8BIT, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_SLAVE_ADDR7, addr, 0);

	if(read_device(i2c_dev, &data, 1) >= 0) {
		rtn = 1;	// デバイス応答
	}

	unlock_device(i2c_dev);
	
	return rtn;
}


static int detect(int argc, uchar *argv[]);

/**
   @brief	I2Cデバイスを検出する
*/
static const struct st_shell_command com_i2c_detect = {
	.name		= "detect",
	.command	= detect,
	.usage_str	= "[i2c_device_name]"
};

static int detect(int argc, uchar *argv[])
{
	unsigned int i;

	if(argc > 1) {
		dev_name = (char *)argv[1];
	}

	tprintf("[%s]\n", dev_name);

	i2c_dev = open_device(dev_name);
	if(i2c_dev == 0) {
		tprintf("Cannot open device \"%s\"\n", dev_name);
		return -1;
	}

	tprintf("    ");
	for(i=0; i<0x10; i++) {
		tprintf("  %1X", i);
	}

#if 0 // デバイスアドレス 0 は試さない
	tprintf("\n00 : --");
	for(i=1; i<0x80; i++) {
#else 
	for(i=0; i<0x80; i++) {
#endif
		if((i % 0x10) == 0) {
			tprintf("\n%02X :", i);
		}

		if(check_devaddr(i) > 0) {
			tprintf(" %02X", i);
		} else {
			tprintf(" --");
		}
	}

	tprintf("\n");

	close_device(i2c_dev);

	return 0;
}


static int dump(int argc, uchar *argv[]);

/**
   @brief	I2Cデバイスのレジスタをダンプ表示する
*/
static const struct st_shell_command com_i2c_dump = {
	.name		= "dump",
	.command	= dump,
	.usage_str	= "<device_address>"
};

static int dump(int argc, uchar *argv[])
{
	int rtn = 0;
	unsigned int dev_addr;
	unsigned int size = MAX_I2C_DATA;
	unsigned int i, j;

	if(argc < 2) {
		print_command_usage(&com_i2c_dump);
		return 0;
	}

	dev_addr = hstou(argv[1]);

	if(argc > 2) {
		size = dstou(argv[2]);
		if(size > MAX_I2C_DATA) {
			size = MAX_I2C_DATA;
		}
	}

	i2c_dev = open_device(dev_name);
	if(i2c_dev == 0) {
		tprintf("Cannot open device \"%s\"\n", dev_name);
		return -1;
	}

	lock_device(i2c_dev, 0);

	ioctl_device(i2c_dev, IOCMD_I2C_MEMADDRSIZE, I2C_MEM_ADDR_SIZE_8BIT, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_SLAVE_ADDR7, dev_addr, 0);
	seek_device(i2c_dev, SEEK_SET, 0);
	rtn = read_device(i2c_dev, reg_data, size);

	unlock_device(i2c_dev);

	close_device(i2c_dev);

	for(j=0; j<size; j+=16) {
		tprintf("%02X : ", j);
		for(i=0; i<8; i++) {
			tprintf("%02X ", (int)reg_data[j+i]);
		}
		tprintf(" ");
		for(i=8; i<16; i++) {
			tprintf("%02X ", (int)reg_data[j+i]);
		}
		tprintf("\n");
	}

	return rtn;
}



static int get(int argc, uchar *argv[]);

/**
   @brief	I2Cデバイスのレジスタを1バイト読み出し表示する
*/
static const struct st_shell_command com_i2c_get = {
	.name		= "get",
	.command	= get,
	.usage_str	= "<device_addr> <reg_addr> [size]"
};

static int get(int argc, uchar *argv[])
{
	int rtn = 0;
	unsigned int dev_addr;
	unsigned int size = 1;
	int reg_addr;
	unsigned int i;

	if(argc < 3) {
		print_command_usage(&com_i2c_get);
		return 0;
	}

	dev_addr = hstou(argv[1]);
	reg_addr = hstoi(argv[2]);

	if(argc > 3) {
		size = dstou(argv[3]);
		if(size > MAX_I2C_DATA) {
			size = MAX_I2C_DATA;
		}
	}

	i2c_dev = open_device(dev_name);
	if(i2c_dev == 0) {
		tprintf("Cannot open device \"%s\"\n", dev_name);
		return -1;
	}

	lock_device(i2c_dev, 0);

	ioctl_device(i2c_dev, IOCMD_I2C_MEMADDRSIZE, I2C_MEM_ADDR_SIZE_8BIT, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_SLAVE_ADDR7, dev_addr, 0);
	seek_device(i2c_dev, reg_addr, SEEK_SET);
	rtn = read_device(i2c_dev, reg_data, size);

	unlock_device(i2c_dev);

	close_device(i2c_dev);

	tprintf("DEV-ADDR %02X - %02X :", dev_addr, reg_addr);
	for(i=0; i<size; i++) {
		tprintf(" %02X", (int)reg_data[i]);
	}
	tprintf("\n");

	return rtn;
}


static int put(int argc, uchar *argv[]);

/**
   @brief	I2Cデバイスのレジスタに1バイト書き込む
*/
static const struct st_shell_command com_i2c_put = {
	.name		= "put",
	.command	= put,
	.usage_str	= "<device_addr> <reg_addr> <val>"
};

static int put(int argc, uchar *argv[])
{
	int rtn = 0;
	unsigned int dev_addr;
	int reg_addr;
	unsigned char data;

	if(argc < 3) {
		print_command_usage(&com_i2c_put);
		return 0;
	}

	dev_addr = hstou(argv[1]);
	reg_addr = hstou(argv[2]);
	data = hstou(argv[3]);

	i2c_dev = open_device(dev_name);
	if(i2c_dev == 0) {
		tprintf("Cannot open device \"%s\"\n", dev_name);
		return -1;
	}

	lock_device(i2c_dev, 0);

	ioctl_device(i2c_dev, IOCMD_I2C_MEMADDRSIZE, I2C_MEM_ADDR_SIZE_8BIT, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_SLAVE_ADDR7, dev_addr, 0);
	seek_device(i2c_dev, reg_addr, SEEK_SET);
	rtn = putc_device(i2c_dev, data);

	unlock_device(i2c_dev);

	close_device(i2c_dev);

	return rtn;
}


static const struct st_shell_command * const com_i2c_list[] = {
	&com_i2c_detect,
	&com_i2c_dump,
	&com_i2c_get,
	&com_i2c_put,
	0
};

const struct st_shell_command com_i2c = {
	.name		= "i2c",
	.manual_str	= "I2C operation commands",
	.sublist	= com_i2c_list
}; ///< I2Cデバイス情報取得、制御
