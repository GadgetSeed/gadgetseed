/** @file
    @brief	ADT7410 ドライバ(I2C I/F)

    @date	2014.12.14
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "device.h"
#include "interrupt.h"
#include "tkprintf.h"
#include "str.h"
#include "device/i2c_ioctl.h"
#include "device/envsnsr_ioctl.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


#define REG_TEMP_MSB	0x00
#define REG_TEMP_LSB	0x01
#define REG_STATUS	0x02
#define REG_CONFIG	0x03

#define DEV_ADDR	0x48

static struct st_device *i2c_dev;
static char dev_name[MAX_DEVNAMELRN];

static int adt7410_register(struct st_device *dev, char *param)
{
	strncopy((uchar *)dev_name, (uchar *)param, MAX_DEVNAMELRN);

	i2c_dev = open_device(dev_name);
	if(i2c_dev == 0) {
		SYSERR_PRINT("Cannot open device \"%s\"\n", dev_name);
		return -1;
	}

	close_device(i2c_dev);

	return 0;
}

static int get_temp(void)
{
	unsigned short val = 0;
	unsigned char data[2];
	int temp;
	int th, tl;

	lock_device(i2c_dev, 0);

	ioctl_device(i2c_dev, IOCMD_I2C_SLAVE_ADDR7, DEV_ADDR, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_MEMADDRSIZE, I2C_MEM_ADDR_SIZE_8BIT, 0);
	seek_device(i2c_dev, 0, SEEK_SET);
	read_device(i2c_dev, data, 2);

	unlock_device(i2c_dev);

	val = ((((unsigned short)data[0] << 8) + data[1]) >> 3);

	temp = (int)val;
	if(val & (0x8000 >> 3)) {	// 符号判定
		temp = temp - 8192;	// 負数のとき
	}

	th = temp/16;
	tl = (val & 15) * 10 / 16;

	return (th * 100) + (tl * 10);
}

static int adt7410_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	switch(com) {
	case IOCMD_ENVSNSR_GET_TEMP:
		{
			int rtn = get_temp();
			DKPRINTF(0x01, "ENVSNSR_GET_TEMP %d\n", rtn);
			return rtn;
		}

	case IOCMD_ENVSNSR_GET_THP:
		{
			int *thp = param;
			thp[0] = get_temp();
			thp[1] = 0;
			thp[2] = 0;
			DKPRINTF(0x01, "ENVSNSR_GET_THP T:%d H:%d P:%d\n", thp[0], thp[1], thp[2]);
			return 0;
		}

	default:
		SYSERR_PRINT("Unknown ioctl(%08X)\n", com);
		return -1;
	}

	return 0;
}

const struct st_device adt7410_device = {
	.name		= DEF_DEV_NAME_ENVSNSR,
	.explan		= "ATD7410 I2C Temperature Sensor",
	.register_dev	= adt7410_register,
	.ioctl		= adt7410_ioctl,
};
