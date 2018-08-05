/** @file
    @brief	LwIP デバイスインタフェース

    @date	2013.03.03
    @author	Takashi SHUDO
*/
/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "devif.h"

#include "lwip/debug.h"

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/ip.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"

#include "netif/etharp.h"

#include "device.h"
#include "tprintf.h"
#include "tkprintf.h"
#include "device/ether_ioctl.h"
#include "str.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


#define DEVNAME	"eth"
#define STACKSIZE	(1024*8)

#define IFNAME0 'e'
#define IFNAME1 't'

struct devif {
	struct eth_addr *ethaddr;
	/* Add whatever per-interface state that is needed here. */
	struct st_device *dev;
};

/* Forward declarations. */
static void devif_input(struct netif *netif);

static void devif_thread(void *data);

/*---------------------------------------------------------------------------*/
static int low_level_init(struct netif *netif)
{
	struct devif *devif;
	unsigned char my_macaddr[6];
	long rt;
	int i;

	DKPRINTF(0x01, "##D %s\n", __FUNCTION__);

	devif = (struct devif *)netif->state;

	/* Do whatever else is needed to initialize interface. */

	devif->dev = open_device(DEVNAME);
	DKPRINTF(0x01, "%s: fd %08lx\n", __FUNCTION__, (unsigned long)devif->dev);
	if(devif->dev == 0) {
		SYSERR_PRINT("%s: cannot open " DEVNAME "\n", __FUNCTION__);
		return -1;
	}

	rt = ioctl_device(devif->dev, IOCMD_ETHER_GET_MACADDR, 0, (void *)&my_macaddr);
	if(rt != 0) {
		SYSERR_PRINT("Cannot get MAC address \"%s\".\n", DEVNAME);
	}
	ioctl_device(devif->dev, IOCMD_ETHER_CLEAR_BUF, 0, 0);
	// 受信バッファクリア

	for(i=0; i<NETIF_MAX_HWADDR_LEN; i++) {
		netif->hwaddr[i] = my_macaddr[i];
	}

	tprintf("MAC Address : %02x %02x %02x %02x %02x %02x\n",
		netif->hwaddr[0],
		netif->hwaddr[1],
		netif->hwaddr[2],
		netif->hwaddr[3],
		netif->hwaddr[4],
		netif->hwaddr[5]);

	sys_thread_new("devif_thread", devif_thread, netif,
		       STACKSIZE, DEFAULT_THREAD_PRIO);

	return 0;
}
/*---------------------------------------------------------------------------*/
/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 */
/*---------------------------------------------------------------------------*/
static char obuf[1514];

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	struct pbuf *q;
	char *bufptr;
	struct devif *devif;

	DKPRINTF(0x01, "##D %s\n", __FUNCTION__);

	devif = (struct devif *)netif->state;
	/* initiate transfer(); */

	bufptr = &obuf[0];

	for(q = p; q != NULL; q = q->next) {
		/* Send the data from the pbuf to the interface, one pbuf at a
		   time. The size of the data in each pbuf is kept in the ->len
		   variable. */
		/* send data from(q->payload, q->len); */
		memorycopy(bufptr, q->payload, q->len);
		bufptr += q->len;
	}

	/* signal that packet should be sent(); */
	if(write_device(devif->dev, (unsigned char *)obuf, p->tot_len) == -1) {
		SYSERR_PRINT("devif: write error\n");
	}
	return ERR_OK;
}

/*---------------------------------------------------------------------------*/
/*
 * low_level_input():
 *
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 */
/*---------------------------------------------------------------------------*/
static char ibuf[1514];

static struct pbuf * low_level_input(struct devif *devif)
{
	struct pbuf *p, *q;
	long len;
	char *bufptr;

	DKPRINTF(0x01, "##D %s\n", __FUNCTION__);

	/* Obtain the size of the packet and put it into the "len"
	   variable. */
	len = read_device(devif->dev, (unsigned char *)ibuf, sizeof(ibuf));
	DKPRINTF(0x01, "len = %ld\n", len);

#ifdef DEBUG
#if 1
#define NET_SHORT(x)	((((x) & 0x00ff)<<8) | (((x) & 0xff00)>>8))
#else
#define NET_SHORT(x)	x
#endif

	if(len != 0) {
		struct ETHER_HEADER *ethhd = (struct ETHER_HEADER *)ibuf;
		int i;

		eprintf("DST : ");
		for(i=0; i<sizeof(struct MAC_ADDR); i++) {
			eprintf("%02X ", ethhd->dst_mac.addr[i]);
		}
		eprintf("\n");

		eprintf("SRC : ");
		for(i=0; i<sizeof(struct MAC_ADDR); i++) {
			eprintf("%02X ", ethhd->src_mac.addr[i]);
		}
		eprintf("\n");

		eprintf("PROTOCOL : %04X\n", NET_SHORT(ethhd->protocol));
	}
#endif

	/* We allocate a pbuf chain of pbufs from the pool. */
	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

	if(p != NULL) {
		/* We iterate over the pbuf chain until we have read the entire
		   packet into the pbuf. */
		bufptr = &ibuf[0];
		for(q = p; q != NULL; q = q->next) {
			/* Read enough bytes to fill this pbuf in the chain. The
			   available data in the pbuf is given by the q->len
			   variable. */
			/* read data into(q->payload, q->len); */
			memorycopy(q->payload, bufptr, q->len);
			bufptr += q->len;
		}
		/* acknowledge that packet has been read(); */
	} else {
		/* drop packet(); */
	}

	return p;
}
/*---------------------------------------------------------------------------*/
static struct netif *netif;
static struct devif *devif;
static struct devif lwip_dev;

static void devif_thread(void *arg)
{
	netif = (struct netif *)arg;
	devif = (struct devif *)netif->state;

	DKPRINTF(0x01, "##D %s start\n", __FUNCTION__);

	while(1) {
		int ret;
		/* Wait for a packet to arrive. */
		DKPRINTF(0x01, "##D %s select in\n", __FUNCTION__);
		ret = select_device(devif->dev, 0);
		DKPRINTF(0x01, "##D %s select out\n", __FUNCTION__);

		if(ret >= 0) {
			/* Handle incoming packet. */
			devif_input(netif);
		} else if(ret == -1) {
			SYSERR_PRINT("devif_thread: select error\n");
		}
	}
}
/*---------------------------------------------------------------------------*/
/*
 * devif_input():
 *
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface.
 *
 */
/*---------------------------------------------------------------------------*/
static void devif_input(struct netif *netif)
{
	struct devif *devif;
	struct eth_hdr *ethhdr;
	struct pbuf *p;

	DKPRINTF(0x01, "##D %s\n", __FUNCTION__);

	devif = (struct devif *)netif->state;

	p = low_level_input(devif);

	if(p == NULL) {
		SYSERR_PRINT("devif_input: low_level_input returned NULL\n");
		return;
	}
	ethhdr = (struct eth_hdr *)p->payload;

	switch(htons(ethhdr->type)) {
		/* IP or ARP packet? */
	case ETHTYPE_IP:
	case ETHTYPE_ARP:
#if PPPOE_SUPPORT
		/* PPPoE packet? */
	case ETHTYPE_PPPOEDISC:
	case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
		/* full packet send to tcpip_thread to process */
		if (netif->input(p, netif) != ERR_OK) {
			LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
			pbuf_free(p);
			p = NULL;
		}
		break;
	default:
		pbuf_free(p);
		break;
	}
}

/*---------------------------------------------------------------------------*/
/*
 * devif_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */
/*---------------------------------------------------------------------------*/
err_t devif_init(struct netif *netif)
{
	struct devif *devif;
	err_t rtn = ERR_OK;

	DKPRINTF(0x01, "##D %s\n", __FUNCTION__);

	devif = &lwip_dev;
	if(!devif) {
		SYSERR_PRINT("malloc error %p\n", devif);
		return ERR_MEM;
	}
	netif->state = devif;
	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
	netif->output = etharp_output;
	netif->linkoutput = low_level_output;
	netif->mtu = 1500;
	/* hardware address length */
	netif->hwaddr_len = 6;

	devif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP;

	if(low_level_init(netif) != 0) {
		rtn = ERR_IF;
	}

	return rtn;
}
/*---------------------------------------------------------------------------*/

#include "net.h"

int link_up_net(void)
{
	struct devif *devif;

	devif = (struct devif *)netif->state;

	ioctl_device(devif->dev, IOCMD_ETHER_LINK_UP, 0, 0);

	return 0;
}

int link_down_net(void)
{
	struct devif *devif;

	devif = (struct devif *)netif->state;

	ioctl_device(devif->dev, IOCMD_ETHER_LINK_DOWN, 0, 0);

	return 0;
}

int net_status(void)
{
	struct devif *devif;

	devif = (struct devif *)netif->state;

	return ioctl_device(devif->dev, IOCMD_ETHER_GET_LINK_STATUS, 0, 0);
}
