/** @file
    @brief	BME280 温湿度・気圧センサ

    @date	2018.01.22
    @authoer	Takashi SHUDO
*/

#include "device.h"
#include "str.h"
#include "device/i2c_ioctl.h"
#include "device/envsnsr_ioctl.h"

//#define DEBUGKBITS 0x01
#include "dkprintf.h"


static struct st_device *i2c_dev;
static char dev_name[MAX_DEVNAMELRN];

typedef int		BME280_S32_t;
typedef unsigned int	BME280_U32_t;

static unsigned short	dig_T1;	// Reg 0x88, 0x89
static signed short	dig_T2;	// Reg 0x8A, 0x8B
static signed short	dig_T3;	// Reg 0x8C, 0x8D
static unsigned short	dig_P1;	// Reg 0x8E, 0x8F
static signed short	dig_P2;	// Reg 0x90, 0x91
static signed short	dig_P3;	// Reg 0x92, 0x93
static signed short	dig_P4;	// Reg 0x94, 0x95
static signed short	dig_P5;	// Reg 0x96, 0x97
static signed short	dig_P6;	// Reg 0x98, 0x99
static signed short	dig_P7;	// Reg 0x9A, 0x9B
static signed short	dig_P8;	// Reg 0x9C, 0x9D
static signed short	dig_P9;	// Reg 0x9E, 0x9F
static unsigned char	dig_H1;	// Reg 0xA1
static signed short	dig_H2;	// Reg 0xE1, 0xE2
static unsigned char	dig_H3;	// Reg 0xE3
static signed short	dig_H4;	// Reg 0xE5, 0xE5[3:0]
static signed short	dig_H5;	// Reg 0xE5[7:4], 0xE6
static signed char	dig_H6;	// Reg 0xE7

// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.
// t_fine carries fine temperature as global value
BME280_S32_t t_fine;
BME280_S32_t BME280_compensate_T_int32(BME280_S32_t adc_T)
{
	BME280_S32_t var1, var2, T;
	var1 = ((((adc_T>>3) - ((BME280_S32_t)dig_T1<<1))) * ((BME280_S32_t)dig_T2)) >> 11;
	var2 = (((((adc_T>>4) - ((BME280_S32_t)dig_T1)) * ((adc_T>>4) - ((BME280_S32_t)dig_T1))) >> 12) *
		((BME280_S32_t)dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;
	return T;
}

// Returns pressure in Pa as unsigned 32 bit integer. Output value of “96386” equals 96386 Pa = 963.86 hPa
BME280_U32_t BME280_compensate_P_int32(BME280_S32_t adc_P)
{
	BME280_S32_t var1, var2;
	BME280_U32_t p;
	var1 = (((BME280_S32_t)t_fine)>>1) - (BME280_S32_t)64000;
	var2 = (((var1>>2) * (var1>>2)) >> 11 ) * ((BME280_S32_t)dig_P6);
	var2 = var2 + ((var1*((BME280_S32_t)dig_P5))<<1);
	var2 = (var2>>2)+(((BME280_S32_t)dig_P4)<<16);
	var1 = (((dig_P3 * (((var1>>2) * (var1>>2)) >> 13 )) >> 3) + ((((BME280_S32_t)dig_P2) * var1)>>1))>>18;
	var1 =((((32768+var1))*((BME280_S32_t)dig_P1))>>15);
	if (var1 == 0)
	{
		return 0; // avoid exception caused by division by zero
	}
	p = (((BME280_U32_t)(((BME280_S32_t)1048576)-adc_P)-(var2>>12)))*3125;
	if (p < 0x80000000)
	{
		p = (p << 1) / ((BME280_U32_t)var1);
	}
	else
	{
		p = (p / (BME280_U32_t)var1) * 2;
	}
	var1 = (((BME280_S32_t)dig_P9) * ((BME280_S32_t)(((p>>3) * (p>>3))>>13)))>>12;
	var2 = (((BME280_S32_t)(p>>2)) * ((BME280_S32_t)dig_P8))>>13;
	p = (BME280_U32_t)((BME280_S32_t)p + ((var1 + var2 + dig_P7) >> 4));
	return p;
}

// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits).
// Output value of “47445” represents 47445/1024 = 46.333 %RH
BME280_U32_t bme280_compensate_H_int32(BME280_S32_t adc_H)
{
	BME280_S32_t v_x1_u32r;
	v_x1_u32r = (t_fine - ((BME280_S32_t)76800));
	v_x1_u32r = (((((adc_H << 14) - (((BME280_S32_t)dig_H4) << 20) - (((BME280_S32_t)dig_H5) * v_x1_u32r)) +
		       ((BME280_S32_t)16384)) >> 15) * (((((((v_x1_u32r * ((BME280_S32_t)dig_H6)) >> 10) * (((v_x1_u32r *
			((BME280_S32_t)dig_H3)) >> 11) + ((BME280_S32_t)32768))) >> 10) + ((BME280_S32_t)2097152)) *
			((BME280_S32_t)dig_H2) + 8192) >> 14));
	v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((BME280_S32_t)dig_H1)) >> 4));
	v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);
	v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);
	return (BME280_U32_t)(v_x1_u32r>>12);
}

#define DEV_ADDR	0x76

static void init_bme280(void)
{
	unsigned char osrs_t = 1;             // Temperature oversampling x 1
	unsigned char osrs_p = 1;             // Pressure oversampling x 1
	unsigned char osrs_h = 1;             // Humidity oversampling x 1
	unsigned char mode = 3;               // Normal mode
	unsigned char t_sb = 5;               // Tstandby 1000ms
	unsigned char filter = 0;             // Filter off 
	unsigned char spi3w_en = 0;           // 3-wire SPI Disable

	unsigned char ctrl_meas_reg = (osrs_t << 5) | (osrs_p << 2) | mode;
	unsigned char config_reg    = (t_sb << 5) | (filter << 2) | spi3w_en;
	unsigned char ctrl_hum_reg  = osrs_h;

	lock_device(i2c_dev, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_SLAVE_ADDR7, DEV_ADDR, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_MEMADDRSIZE, I2C_MEM_ADDR_SIZE_8BIT, 0);
	seek_device(i2c_dev, 0xf2, SEEK_SET);
	putc_device(i2c_dev, ctrl_hum_reg);
	seek_device(i2c_dev, 0xf4, SEEK_SET);
	putc_device(i2c_dev, ctrl_meas_reg);
	seek_device(i2c_dev, 0xf5, SEEK_SET);
	putc_device(i2c_dev, config_reg);
	unlock_device(i2c_dev);
}

static void calib_bme280(void)
{
	unsigned char reg[32];

	lock_device(i2c_dev, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_SLAVE_ADDR7, DEV_ADDR, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_MEMADDRSIZE, I2C_MEM_ADDR_SIZE_8BIT, 0);

	seek_device(i2c_dev, 0x88, SEEK_SET);
	read_device(i2c_dev, reg, 24);

	seek_device(i2c_dev, 0xA1, SEEK_SET);
	read_device(i2c_dev, &reg[24], 1);

	seek_device(i2c_dev, 0xE1, SEEK_SET);
	read_device(i2c_dev, &reg[25], 7);

	unlock_device(i2c_dev);

	KXDUMP(0x01, reg, 32);

	dig_T1 = (((unsigned short)reg[1])<<8) + reg[0];
	DKPRINTF(0x01, "T1 : %d\n", dig_T1);
	dig_T2 = (((signed short)reg[3])<<8) + reg[2];
	DKPRINTF(0x01, "T2 : %d\n", dig_T2);
	dig_T3 = (((signed short)reg[5])<<8) + reg[4];
	DKPRINTF(0x01, "T3 : %d\n", dig_T3);

	dig_P1 = (((unsigned short)reg[7])<<8) + reg[6];
	dig_P2 = ((signed short)reg[9]<<8) + reg[8];
	dig_P3 = (((signed short)reg[11])<<8) + reg[10];
	dig_P4 = (((signed short)reg[13])<<8) + reg[12];
	dig_P5 = (((signed short)reg[15])<<8) + reg[14];
	dig_P6 = (((signed short)reg[17])<<8) + reg[16];
	dig_P7 = (((signed short)reg[19])<<8) + reg[18];
	dig_P8 = (((signed short)reg[21])<<8) + reg[20];
	dig_P9 = (((signed short)reg[23])<<8) + reg[22];
	dig_H1 = reg[24];
	dig_H2 = (((signed short)reg[26])<<8) + reg[25];
	dig_H3 = reg[27];
	dig_H4 = (((signed short)reg[28])<<4) + (0x0F & reg[29]);
	dig_H5 = (((signed short)reg[30])<<4) + ((reg[29]>>4) & 0x0F);
	dig_H6 = reg[31];
}

static int bme280_register(struct st_device *dev, char *param)
{
	strncopy((uchar *)dev_name, (uchar *)param, MAX_DEVNAMELRN);

	i2c_dev = open_device(dev_name);
	if(i2c_dev == 0) {
		SYSERR_PRINT("Cannot open device \"%s\"\n", dev_name);
		return -1;
	}

	init_bme280();
	calib_bme280();

	close_device(i2c_dev);

	return 0;
}

unsigned int hum_raw;
unsigned int temp_raw;
unsigned int pres_raw;

static void get_raw_data(void)
{
	unsigned char reg[8];

	lock_device(i2c_dev, 0);

	ioctl_device(i2c_dev, IOCMD_I2C_SLAVE_ADDR7, DEV_ADDR, 0);
	ioctl_device(i2c_dev, IOCMD_I2C_MEMADDRSIZE, I2C_MEM_ADDR_SIZE_8BIT, 0);

	seek_device(i2c_dev, 0xf7, SEEK_SET);
	read_device(i2c_dev, reg, 8);

	unlock_device(i2c_dev);

	pres_raw = (((unsigned int)reg[0]) << 12) | (((unsigned int)reg[1]) << 4) | (((unsigned int)reg[2]) >> 4);
	temp_raw = (((unsigned int)reg[3]) << 12) | (((unsigned int)reg[4]) << 4) | (((unsigned int)reg[5]) >> 4);
	hum_raw = (((unsigned int)reg[6]) << 8) | reg[7];

	DKFPRINTF(0x01, "PRES_RAW : %d(%x)\n", pres_raw, pres_raw);
	DKFPRINTF(0x01, "TEMP_RAW : %d(%x)\n", temp_raw, temp_raw);
	DKFPRINTF(0x01, "HUM_RAW  : %d(%x)\n", hum_raw, hum_raw);
}

static int bme280_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	switch(com) {
	case IOCMD_ENVSNSR_GET_TEMP:
		{
			get_raw_data();
			int rtn = BME280_compensate_T_int32(temp_raw);
			DKPRINTF(0x01, "ENVSNSR_GET_TEMP %d\n", rtn);
			return rtn;
		}

	case IOCMD_ENVSNSR_GET_HUM:
		{
			get_raw_data();
			unsigned int rtn = bme280_compensate_H_int32(hum_raw);
			DKPRINTF(0x01, "ENVSNSR_GET_HUM %d\n", rtn);
			return rtn;
		}

	case IOCMD_ENVSNSR_GET_PRESS:
		{
			get_raw_data();
			unsigned int rtn = BME280_compensate_P_int32(pres_raw);
			DKPRINTF(0x01, "ENVSNSR_GET_PRESS %d\n", rtn);
			return rtn;
		}

	case IOCMD_ENVSNSR_GET_THP:
		{
			get_raw_data();
			int *thp = param;
			thp[0] = BME280_compensate_T_int32(temp_raw);
			thp[1] = bme280_compensate_H_int32(hum_raw);
			thp[2] = BME280_compensate_P_int32(pres_raw);
			DKPRINTF(0x01, "ENVSNSR_GET_THP T:%d H:%d P:%d\n", thp[0], thp[1], thp[2]);
			return 0;
		}

	default:
		SYSERR_PRINT("Unknown ioctl(%08X)\n", com);
		return -1;
	}

	return 0;
}

const struct st_device bme280_device = {
	.name		= DEF_DEV_NAME_ENVSNSR,
	.explan		= "BME280 Temp. sensor",
	.register_dev	= bme280_register,
	.ioctl		= bme280_ioctl,
};
