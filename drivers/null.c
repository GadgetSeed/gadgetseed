/** @file
    @brief	nullデバイス

    @date	2011.01.29
    @author	Takashi SHUDO
*/

#include "device.h"

static int null_register(struct st_device *dev, char *param)
{
	return 0;
}

static int null_putc(struct st_device *dev, unsigned char ch)
{
	return 1;
}

const struct st_device null_device = {
	.name		= "null",
	.explan		= "null device",
	.register_dev	= null_register,
	.putc		= null_putc,
};
