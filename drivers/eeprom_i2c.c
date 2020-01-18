/** @file
    @brief	 EEPROMドライバ(I2C接続)

    @date	2015.09.27
    @author	Takashi SHUDO

    @info

    24C1024等
    AKI-H8/3069用
*/

#include "device/i2c_ioctl.h"
#include "device.h"
#include "tkprintf.h"
#include "str.h"

#define USEBLOCKWRITE
#define DEV_ADDR0	0x50
#define DEV_ADDR1	0x51

#define EEPROMSIZE	((long)1024*1024/8)	// 24C1024(1Mbits = 128KBytes)
#define ONEDEVSIZE	(64*1024)	// 1デバイスのバイト数
#define BLOCKSIZE	256	// 24C1024

static struct st_device *i2c_dev;
static char i2c_devname[MAX_DEVNAMELRN];
static unsigned long eeprom_addr;

static int eeprom_register(struct st_device *dev, char *param)
{
	eeprom_addr = 0;

	if(param == 0) {
		SYSERR_PRINT("No I2C device name\n");
		return -1;
	} else {
		strncopy((uchar *)i2c_devname, (uchar *)param, MAX_DEVNAMELRN);
	}

	return 0;
}

static int eeprom_unregister(struct st_device *dev)
{
	return 0;
}

static int eeprom_open(struct st_device *dev)
{
	return 0;
}

static int eeprom_close(struct st_device *dev)
{
	return 0;
}

static int eeprom_read(struct st_device *dev, void *data, unsigned int size)
{
	int rtn = 0;
	unsigned char dev_addr;

	i2c_dev = open_device(i2c_devname);

	if(i2c_dev == 0) {
		SYSERR_PRINT("Cannot open device %s.\n", i2c_devname);
		return -1;
	}

	if(eeprom_addr < ONEDEVSIZE) {
		dev_addr = DEV_ADDR0;
	} else {
		dev_addr = DEV_ADDR1;
	}
	// [TODO] デバイスをまたいだアクセスは出来ない
	lock_device(i2c_dev, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_SLAVE_ADDR7, dev_addr, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_MEMADDRSIZE, I2C_MEM_ADDR_SIZE_16BIT, 0);
	rtn = read_device(i2c_dev, data, size);
	unlock_device(i2c_dev);

	close_device(i2c_dev);

	eeprom_addr += rtn;
	// サイズオーバーは無視

	return rtn;
}

static int eeprom_write(struct st_device *dev, const void *data, unsigned int size)
{
	int rtn = 0;
	unsigned char dev_addr;

	i2c_dev = open_device(i2c_devname);

	if(i2c_dev == 0) {
		SYSERR_PRINT("Cannot open device %s.\n", i2c_devname);
		return -1;
	}

	if(eeprom_addr < ONEDEVSIZE) {
		dev_addr = DEV_ADDR0;
	} else {
		dev_addr = DEV_ADDR1;
	}
	// [TODO] デバイスをまたいだアクセスは出来ない
	lock_device(i2c_dev, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_SLAVE_ADDR7, dev_addr, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_MEMADDRSIZE, I2C_MEM_ADDR_SIZE_16BIT, 0);
	rtn = write_device(i2c_dev, data, size);
	unlock_device(i2c_dev);

	close_device(i2c_dev);

	eeprom_addr += rtn;
	// サイズオーバーは無視

	return rtn;
}

static int eeprom_seek(struct st_device *dev, int offset, int whence)
{
	switch(whence) {
	case SEEK_SET:
		eeprom_addr = offset;
		break;
		
	case SEEK_CUR:
		eeprom_addr += offset;
		break;

	case SEEK_END:
		eeprom_addr = EEPROMSIZE - offset;
		break;
		
	default:
		return -1;
	}
	
	return eeprom_addr;
}

const struct st_device eeprom_device = {
	.name		= DEF_DEV_NAME_EEPRMOM,
	.explan		= "EEPROM(24C1024)",
	.register_dev	= eeprom_register,
	.unregister_dev	= eeprom_unregister,
	.open		= eeprom_open,
	.close		= eeprom_close,
	.read		= eeprom_read,
	.write		= eeprom_write,
	.seek		= eeprom_seek
};
