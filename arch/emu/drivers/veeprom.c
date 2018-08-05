/** @file
    @brief	仮想 EEPROM ドライバ

    @date	2009.10.30
    @author	Takashi SHUDO
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define EEPROMFILE "EEPROM.DAT"

static int eeprom_fd;

#include "device.h"
#include "timer.h"

#define EEPROMSIZE	((long)1024*1024/8)	// 24C1024(1Mbits = 128KBytes)
#define BLOCKSIZE	256	// 24C1024

static unsigned char eeprom_data[EEPROMSIZE];
static unsigned long eeprom_addr;

static int eeprom_register(struct st_device *dev, char *param)
{
	eeprom_addr = 0;

	eeprom_fd = open(EEPROMFILE, O_RDWR);
	if(eeprom_fd < 0) {
		int rt;

		fprintf(stderr, "Cannot open \"%s\".\r\n", EEPROMFILE);
		fprintf(stderr, "Create \"%s\".\r\n", EEPROMFILE);
		eeprom_fd = open(EEPROMFILE, O_RDWR | O_CREAT, S_IRWXU);
		if(eeprom_fd < 0) {
			fprintf(stderr, "Cannot create \"%s\".\r\n",
				EEPROMFILE);
		}

		memset(eeprom_data, 0, EEPROMSIZE);

		rt = write(eeprom_fd, eeprom_data, EEPROMSIZE);
		if(rt != EEPROMSIZE) {
			fprintf(stderr, "NG.\r\n");
		} else {
			fprintf(stderr, "OK.\r\n");
		}
	}

	if(eeprom_fd) {
		int rt;
		lseek(eeprom_fd, 0, SEEK_SET);
		rt = read(eeprom_fd, eeprom_data, EEPROMSIZE);
		fprintf(stderr, "Read \"%s\" ", EEPROMFILE);
		if(rt != EEPROMSIZE) {
			fprintf(stderr, "NG.\r\n");
		} else {
			fprintf(stderr, "OK.\r\n");
		}
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

#if 0
static long eeprom_read(struct st_device *dev, unsigned char *data, long size)
{
	unsigned int i;

	for(i=0; i<size; i++) {
		*data = eeprom_data[eeprom_addr];
		data ++;
		eeprom_addr ++;
	}

	return size;
}
#endif

static int eeprom_getc(struct st_device *dev, unsigned char *data)
{
	*data = eeprom_data[eeprom_addr];

	eeprom_addr ++;

	return 1;
}

#if 0
static long eeprom_write(struct st_device *dev, unsigned char *data, long size)
{
	unsigned int i;

	for(i=0; i<size; i++) {
		eeprom_data[eeprom_addr] = *data;
		data ++;
		eeprom_addr ++;
	}

	return size;
}
#endif

static int eeprom_putc(struct st_device *dev, unsigned char data)
{
	int rt;

	fprintf(stderr, "EEPROM putc %02X\r\n", (int)data);
	eeprom_data[eeprom_addr] = data;
	lseek(eeprom_fd, eeprom_addr, SEEK_SET);
	rt = write(eeprom_fd, &data, 1);
	eeprom_addr += rt;

	return rt;
}

static int eeprom_ioctl(struct st_device *dev, long com, long arg)
{
	return 0;
}

static long eeprom_seek(struct st_device *dev, long offset, int whence)
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

const device veeprom_device = {
	.name		= DEF_DEV_NAME_EEPRMOM,
	.explan		= "EMU EEPROM",
	.register_dev	= eeprom_register,
	.unregister_dev	= eeprom_unregister,
	.open		= eeprom_open,
	.close		= eeprom_close,
#if 0
	.read		= eeprom_read,
	.write		= eeprom_write,
#endif
	.getc		= eeprom_getc,
	.putc		= eeprom_putc,
	.ioctl		= eeprom_ioctl,
	.seek		= eeprom_seek
};
