/** @file
    @brief	RX-8025ドライバ

    @date	2015.07.20
    @author	Takashi SHUDO

    @info

    リアルタイムクロック(RX-8025)ドライバ

    下位にI2Cドライバを使用する。
*/

#include "device.h"
#include "device/i2c_ioctl.h"
#include "device/rtc_ioctl.h"
#include "datetime.h"
#include "tkprintf.h"
#include "system.h"

#define DEV_ADDR	0x32

static const char def_v_dev[] = DEF_DEV_NAME_I2C;
static struct st_device *i2c_dev;

static int rx8025_register(struct st_device *dev, char *param)
{
	char *dev_name = (char *)def_v_dev;

	if(param != 0) {
		dev_name = param;
	}

	i2c_dev = open_device(dev_name);
	if(i2c_dev == 0) {
		SYSERR_PRINT("Cannot open device \"%s\"\n", dev_name);
		return -1;
	}

	return 0;
}

#define BCD2BIN(x)	(((((x) & 0xf0)>>4) * 10) + ((x) & 0x0f))
#define BIN2BCD(x)	(((((x)/10) % 10)<<4) + ((x) % 10))

static int rtc_set(struct st_datetime *tp)
{
	int len = 0;
	unsigned char data[7];

	data[0]	= BIN2BCD(tp->sec);
	data[1]	= BIN2BCD(tp->min);
	data[2]	= BIN2BCD(tp->hour);
	data[3]	= BIN2BCD(tp->dayofweek);
	data[4]	= BIN2BCD(tp->day);
	data[5]	= BIN2BCD(tp->month);
	if(tp->year >= 2000) {
		data[6] = BIN2BCD(tp->year - 2000);
	} else {
		data[6] = BIN2BCD(tp->year - 1900);
	}

	lock_device(i2c_dev, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_SLAVE_ADDR7, DEV_ADDR, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_MEMADDRSIZE, I2C_MEM_ADDR_SIZE_8BIT, 0);
	seek_device(i2c_dev, 0, SEEK_SET);
	len = write_device(i2c_dev, data, 7);
	unlock_device(i2c_dev);
	if(len != 7) {
		return -1;
	}

	return 0;
}

static int rtc_get(struct st_datetime *tp)
{
	int len = 0;
	unsigned char data[7];

	lock_device(i2c_dev, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_SLAVE_ADDR7, DEV_ADDR, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_MEMADDRSIZE, I2C_MEM_ADDR_SIZE_8BIT, 0);
	seek_device(i2c_dev, 0, SEEK_SET);
	len = read_device(i2c_dev, data, 7);
	unlock_device(i2c_dev);
	if(len != 7) {
		return -1;
	}

	tp->year	= 2000 + BCD2BIN(data[6]);
	tp->month	= BCD2BIN(data[5] & 0x1f);
	tp->day		= BCD2BIN(data[4] & 0x3f);
	tp->dayofweek	= BCD2BIN(data[3] & 0x07);
	tp->hour	= BCD2BIN(data[2] & 0x3f);
	tp->min		= BCD2BIN(data[1] & 0x7f);
	tp->sec		= BCD2BIN(data[0] & 0x7f);
	tp->msec	= 0;

	return 0;
}

static int rx8025_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	switch(com) {
	case IOCMD_RTC_SET:
		{
			struct st_datetime *tp = (struct st_datetime *)param;

			return rtc_set(tp);
		}
		break;

	case IOCMD_RTC_GET:
		{
			struct st_datetime *tp = (struct st_datetime *)param;

			return rtc_get(tp);
		}
		break;

	default:
		SYSERR_PRINT("Unknown ioctl(%08X)\n", com);
		return -1;
	}

	return -1;
}

const struct st_device rx8025_device = {
	.name		= DEF_DEV_NAME_RTC,
	.explan		= "RX-8025 RTC(I2C)",
	.register_dev	= rx8025_register,
	.ioctl		= rx8025_ioctl,
};
