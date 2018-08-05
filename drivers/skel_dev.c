/** @file
    @brief	デバイスドライバのスケルトン

    @date	日時を記述
    @authoer	作成者名を記述
*/

#include "device.h"
#include "interrupt.h"

static int skel_addr;

void skel_inthdr(unsigned int intnum, void *sp)
{
}

static int skel_register(char *param)
{
	skel_addr = 0;

	register_interrpt(VECTNUM, skel_inthdr);

	return 0;
}

static int skel_unregister(void)
{
	return 0;
}

static int skel_open(void)
{
	return 0;
}

static int skel_close(void)
{
	return 0;
}

static int skel_read(void *buf, unsigned int count)
{
	return count;
}

static int skel_getc(unsigned char *data)
{
	return 1;
}

static int skel_write(const void *buf, unsigned int count)
{
	return count;
}

static int skel_putc(unsigned char data)
{
	return 1;
}

static int skel_ioctl(unsigned int com, unsigned int arg, void *param)
{
	return 0;
}

static int skel_seek(int offset, int whence)
{
	switch(whence) {
	case SEEK_SET:
		skel_addr = offset;
		break;

	case SEEK_CUR:
		skel_addr += offset;
		break;

	case SEEK_END:
		break;

	default:
		return -1;
	}

	return skel_addr;
}

const device skel_device = {
	.name		= "skel",
	.explan		= "Explan device",
	.register_dev	= skel_register,
	.unregister_dev	= skel_unregister,
	.open		= skel_open,
	.close		= skel_close,
	.read		= skel_read,
	.getc		= skel_getc,
	.write		= skel_write,
	.putc		= skel_putc,
	.ioctl		= skel_ioctl,
	.seek		= skel_seek,
};
