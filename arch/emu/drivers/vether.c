/** @file
    @brief	仮想 LANコントローラドライバ

    @date	2012.12.24
    @authoer	Takashi SHUDO
*/

/*
--- Setup example ---
brctl addbr br0
brctl addif br0 enx106f3fa6b61d
ifconfig br0 up
ifconfig enx106f3fa6b61d 0.0.0.0 up
--- END Setup ---

--- Cancel setting --
brctl delif br0 tap0
brctl delif br0 enx106f3fa6b61d
ifconfig br0 down
brctl delbr br0
--- END Cancel ---
*/
#include "sysconfig.h"

#include "device.h"
#include "device/ether_ioctl.h"
#include "interrupt.h"
#include "tkprintf.h"
#include "task/event.h"
#include "task/syscall.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include "dkprintf.h"

//#define DEBUG
#ifdef DEBUG
#define VEPRINTF	printf
#else
#define VEPRINTF(x, ...)
#endif

#define DEVTAP "/dev/net/tun"

#ifndef GSC_ETHERDEV_DEFAULT_MACADDRESS	// $gsc EtherデバイスデフォルトMACアドレス
static unsigned char macaddress[6] = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x01 };
#else
static const unsigned char macaddr[6] = {
	(GSC_ETHERDEV_DEFAULT_MACADDRESS >> 40) & 0xff,
	(GSC_ETHERDEV_DEFAULT_MACADDRESS >> 32) & 0xff,
	(GSC_ETHERDEV_DEFAULT_MACADDRESS >> 24) & 0xff,
	(GSC_ETHERDEV_DEFAULT_MACADDRESS >> 16) & 0xff,
	(GSC_ETHERDEV_DEFAULT_MACADDRESS >>  8) & 0xff,
	(GSC_ETHERDEV_DEFAULT_MACADDRESS >>  0) & 0xff
};
#endif

#define ETH_FRM_LEN	1536

static int fd;
static struct st_event select_evtque;
static unsigned char flg_open;

//#define ETH_SELECT_TIMEOUT	1

static void *task_vether(void *arg)
{
	fd_set fdset;
	int ret;
#ifdef ETH_SELECT_TIMEOUT
	struct timespec tv;
#endif

	while(1) {
		if(flg_open == 0) {
			sleep(1);
			continue;
		};

#ifdef ETH_SELECT_TIMEOUT
		tv.tv_sec = ETH_SELECT_TIMEOUT;
		tv.tv_nsec = 0;
#endif
		FD_ZERO(&fdset);
		FD_SET(fd, &fdset);

#ifdef ETH_SELECT_TIMEOUT
		ret = pselect(fd + 1, &fdset, NULL, NULL, &tv, NULL);
#else
		ret = pselect(fd + 1, &fdset, NULL, NULL, NULL, NULL);
#endif
		if(ret < 0) {
			VEPRINTF("%s select error %d\r\n", DEVTAP, ret);
			continue;
		} else if(ret == 0) {
			VEPRINTF("%s select timeout %d\r\n", DEVTAP, ret);
			continue;
		}

		VEPRINTF("vether receive %d\r\n", ret);
#if 0 // タイマ処理終了まで待つ場合
		while(is_in_interrupt()) {
			//printf("%s: in interrupt\n", __FUNCTION__);
			usleep(1);
		}
#endif
		lock_timer();
		event_wakeup_ISR(0, &select_evtque, 0);
		unlock_timer();
	}

	return 0;
}

static int vether_register(struct st_device *dev, char *param)
{
	pthread_t thread_id;
	int status;
	struct ifreq ifr;
	int ret;

	flg_open = 0;
	eventqueue_register(&select_evtque, "veth_sel", 0, 0, 0);

	fd = open(DEVTAP, O_RDWR);
	if(fd == -1) {
		SYSERR_PRINT("Cannot open %s\r\n", DEVTAP);
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
	ret = ioctl(fd, TUNSETIFF, (void *) &ifr);
	if(ret < 0) {
		SYSERR_PRINT("%s ioctl error %d\r\n", DEVTAP, ret);
		return -1;
	}

	block_timer_interrupt();
	status = pthread_create(&thread_id, NULL, task_vether, (void *)NULL);
	unblock_timer_interrupt();

	if(status != 0) {
		SYSERR_PRINT("pthread_create : %s",
			(char *)strerror(status));
	}

	return 0;
}

static int vether_unregister(struct st_device *dev)
{
	close(fd);

	return 0;
}

static int vether_open(struct st_device *dev)
{
	int ret;

	VEPRINTF("vether_open\r\n");

#if 1
	ret = system("ifconfig tap0 0.0.0.0 promisc up");
	ret = system("brctl addif br0 tap0");
	if(ret != 0) {
		printf("%s brctl addif error %d\n", DEVTAP, ret);
		printf("Bridge device \"br0\" is required.\n");
	}
#else
	ret = system("ifconfig tap0 inet 10.0.0.254");
	if(ret != 0) {
		SYSERR_PRINT("%s ifconfig error %d\r\n", DEVTAP, ret);
		return -1;
	}
#endif

	flg_open = 1;

	return 0;
}

static int vether_close(struct st_device *dev)
{
	int ret;

	flg_open = 0;

	ret = system("ifconfig tap0 down");
	if(ret != 0) {
		SYSERR_PRINT("%s ifconfig error %d\r\n", DEVTAP, ret);
		return -1;
	}

	return 0;
}


static int vether_read(struct st_device *dev, void *data, unsigned int size)
{
	int ret;

	VEPRINTF("vether_read(*data=%08lX, size=%ld)\r\n",
		 (unsigned long)data, size);

	ret = read(fd, data, ETH_FRM_LEN);
	if(ret == -1) {
		SYSERR_PRINT("read error %s\r\n", DEVTAP);
	}
	VEPRINTF("vether_read return(size)=%ld\r\n", ret);

	return ret;
}

static int vether_write(struct st_device *dev,const void *data, unsigned int size)
{
	int ret;

	VEPRINTF("vether_write(*data=%08lX, size=%ld)\r\n",
		 (unsigned long)data, size);

	ret = write(fd, data, size);
	if(ret == -1) {
		SYSERR_PRINT("write error %s\r\n", DEVTAP);
	}
	fsync(fd);

	return ret;
}

static int vether_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	switch(com) {
	case IOCMD_ETHER_GET_MACADDR:	// MACアドレス取得
	{
		unsigned char *mac = (unsigned char *)param;
		int i;

		for(i=0; i<6; i++) {
			mac[i] = macaddr[i];
		}

		DKPRINTF(0x01, "MAC Addr %02X:%02X:%02X:%02X:%02X:%02X\n",
			 (int)mac[0], (int)mac[1], (int)mac[2],
			 (int)mac[3], (int)mac[4], (int)mac[5]);
	}
	break;

	default:
		break;
	}

	return 0;
}

static int vether_select(struct st_device *dev, unsigned int timeout)
{
	int wt = 0;

	VEPRINTF("vether_select timeout=%ld\r\n", timeout);

	wt = event_wait(&select_evtque, 0, timeout);

	VEPRINTF("vether_select wait_event %ld\r\n", wt);

	if(timeout == 0) {
		return 1;
	} else {
		return wt;
	}
}

const struct st_device vether_device = {
	.name		= DEF_DEV_NAME_ETHER,
	.explan		= "EMU ETHER LAN Controller",
	.register_dev	= vether_register,
	.unregister_dev	= vether_unregister,
	.open		= vether_open,
	.close		= vether_close,
	.read		= vether_read,
	.write		= vether_write,
	.ioctl		= vether_ioctl,
	.select		= vether_select,
};
