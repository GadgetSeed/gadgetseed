/** @file
    @brief	STM32F4 I2Cドライバ

    @date	2015.08.10
    @author	Takashi SHUDO

    PB8 - I2C1_SCL
    PB9 - I2C1_SDA
*/

#include "sysconfig.h"
#include "device.h"
#include "interrupt.h"
#include "tkprintf.h"
#include "device/i2c_ioctl.h"
#include "task/event.h"
#include "task/mutex.h"
#include "task/syscall.h"

#include "stm32f4xx_hal.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


typedef struct st_i2c_data {
	struct st_i2c_config config;

	I2C_HandleTypeDef hi2c;

	unsigned short seek_addr;
} st_i2c_data;

static st_i2c_data i2c_prv_data[1];
static struct st_mutex i2c_mutex[1];

void HAL_I2C_MspInit(I2C_HandleTypeDef* hi2c)
{

	GPIO_InitTypeDef GPIO_InitStruct;
	if(hi2c->Instance==I2C1) {
		GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
		GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
		GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
		HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
		__HAL_RCC_I2C1_CLK_ENABLE();
	}
}


/*
 *
 */
extern const struct st_device i2c_device;

static void init_i2c(struct st_device *dev)
{
	st_i2c_data *i2c_data = (dev->private_data);
	I2C_HandleTypeDef *hi2c = &(i2c_data->hi2c);

	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();

	if(dev == &i2c_device) {
		hi2c->Instance = I2C1;
	}

	hi2c->Init.ClockSpeed = 100000;
	hi2c->Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c->Init.OwnAddress1      = 0;
	hi2c->Init.AddressingMode   = I2C_ADDRESSINGMODE_7BIT;
	hi2c->Init.DualAddressMode  = I2C_DUALADDRESS_DISABLE;
	hi2c->Init.OwnAddress2      = 0;
	hi2c->Init.GeneralCallMode  = I2C_GENERALCALL_DISABLE;
	hi2c->Init.NoStretchMode    = I2C_NOSTRETCH_DISABLE;
	if(HAL_I2C_Init(hi2c) != HAL_OK) {
		SYSERR_PRINT("I2C Initialize error(%s)\n", dev->name);
	}
}

static int i2c_register(struct st_device *dev, char *param)
{
	st_i2c_data *i2c_data = (dev->private_data);

	i2c_data->config.speed		= 100;
	i2c_data->config.address_mode	= I2C_ADDR_MODE_7BIT;
	i2c_data->config.slave_addr	= 0x00;
	i2c_data->config.mem_addr_size	= I2C_MEMADD_SIZE_8BIT;
	i2c_data->seek_addr		= 0;

	init_i2c(dev);

	return 0;
}

static int i2c_open(struct st_device *dev)
{
	return 0;
}

static int i2c_close(struct st_device *dev)
{
	return 0;
}

static int i2c_read(struct st_device *dev, void *data, unsigned int size)
{
	st_i2c_data *i2c_data = (dev->private_data);
	I2C_HandleTypeDef *hi2c = &(i2c_data->hi2c);
	unsigned short slave_addr = i2c_data->config.slave_addr;
	unsigned short i2c_memadd_size = I2C_MEMADD_SIZE_8BIT;
	HAL_StatusTypeDef rtn = 0;

	DKFPRINTF(0x01, "size = %d\n", size);

	switch(i2c_data->config.mem_addr_size) {
	case I2C_MEM_ADDR_SIZE_8BIT:
		i2c_memadd_size = I2C_MEMADD_SIZE_8BIT;
		break;
	case I2C_MEM_ADDR_SIZE_16BIT:
		i2c_memadd_size = I2C_MEMADD_SIZE_16BIT;
		break;
	default:
		break;
	}

	DKPRINTF(0x01, "I2C READ ADDRESS %08X\n", (unsigned int)i2c_data->seek_addr);

	rtn = HAL_I2C_Mem_Read(hi2c,
			       (slave_addr << 1),
			       i2c_data->seek_addr,
			       i2c_memadd_size,
			       data, size,
			       10/* tick */);

	if(rtn == HAL_OK) {
		i2c_data->seek_addr += size;
		return size;
	} else {
		return -1;
	}
}

static int i2c_write(struct st_device *dev, const void *data, unsigned int size)
{
	st_i2c_data *i2c_data = (dev->private_data);
	I2C_HandleTypeDef *hi2c = &(i2c_data->hi2c);
	unsigned short slave_addr = i2c_data->config.slave_addr;
	unsigned short i2c_memadd_size = I2C_MEMADD_SIZE_8BIT;
	HAL_StatusTypeDef rtn = 0;

	DKFPRINTF(0x01, "size = %d\n", size);

	switch(i2c_data->config.mem_addr_size) {
	case I2C_MEM_ADDR_SIZE_8BIT:
		i2c_memadd_size = I2C_MEMADD_SIZE_8BIT;
		break;
	case I2C_MEM_ADDR_SIZE_16BIT:
		i2c_memadd_size = I2C_MEMADD_SIZE_16BIT;
		break;
	default:
		break;
	}

	DKPRINTF(0x01, "I2C WRITE ADDRESS %08X\n", (unsigned int)i2c_data->seek_addr);

	rtn = HAL_I2C_Mem_Write(hi2c,
				(slave_addr << 1),
				i2c_data->seek_addr,
				i2c_memadd_size,
				(unsigned char *)data, size,
				10/* tick */);

	if(rtn == HAL_OK) {
		i2c_data->seek_addr += size;
		return size;
	} else {
		return -1;
	}
}

static int i2c_set_speed(I2C_TypeDef *i2cx, int speed)
{
	return 0;
}

static int i2c_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	st_i2c_data *i2c_data = (dev->private_data);
	I2C_TypeDef *i2cx = i2c_data->hi2c.Instance;
	int rtn = 0;

	switch(com) {
	case IOCMD_I2C_SPEED:
		i2c_data->config.speed = arg;
		rtn = i2c_set_speed(i2cx, arg);
		break;

	case IOCMD_I2C_SLAVE_ADDR7:
		{
			i2c_data->config.slave_addr = arg;
		}
		break;

	case IOCMD_I2C_MEMADDRSIZE:
		{
			DKPRINTF(0x01, "I2C IOCTL Memory Address Size %dbit\n",
				 (arg == I2C_MEM_ADDR_SIZE_16BIT) ? 16 : 8);
			i2c_data->config.mem_addr_size = arg;
		}
		break;

	case IOCMD_I2C_SETCONFIG:
		{
			struct st_i2c_config *conf = (struct st_i2c_config *)arg;

			i2c_data->config.speed = conf->speed;
			i2c_data->config.address_mode = conf->address_mode;
			i2c_data->config.slave_addr = conf->slave_addr;
			i2c_data->config.mem_addr_size = conf->mem_addr_size;
		}
		break;

	default:
		SYSERR_PRINT("Unknown ioctl(%08lX)\n", com);
		rtn = -1;
		break;
	}

	return rtn;
}

static int i2c_seek(struct st_device *dev, int offset, int whence)
{
	st_i2c_data *i2c_data = (dev->private_data);

	switch(whence) {
	case SEEK_SET:
		DKPRINTF(0x01, "I2C SEEK_SET Address %08X\n", offset);
		i2c_data->seek_addr = offset;
		break;
		
	case SEEK_CUR:
		DKPRINTF(0x01, "I2C SEEK_CUR Address %08X\n", offset);
		i2c_data->seek_addr += offset;
		break;

	case SEEK_END:
	default:
		SYSERR_PRINT("Unknown seek whence(%08lX)\n", whence);
		return -1;
	}

	return i2c_data->seek_addr;
}

static int i2c_suspend(struct st_device *dev)
{
	return 0;
}

static int i2c_resume(struct st_device *dev)
{
	init_i2c(dev);

	return 0;
}

const struct st_device i2c_device = {
	.name		= DEF_DEV_NAME_I2C,
	.explan		= "STM32F4 I2C1 Master",
	.private_data	= (void *)&i2c_prv_data[0],
	.register_dev	= i2c_register,
	.mutex		= &i2c_mutex[0],
	.open		= i2c_open,
	.close		= i2c_close,
	.read		= i2c_read,
	.write		= i2c_write,
	.ioctl		= i2c_ioctl,
	.seek		= i2c_seek,
	.suspend	= i2c_suspend,
	.resume		= i2c_resume,
};
