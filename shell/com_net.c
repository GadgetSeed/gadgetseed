/** @file
    @brief	ネット関連コマンド

    @date	2011.05.08
    @author	Takashi SHUDO

    @section net_command netコマンド

    net コマンドには以下のサブコマンドがあります。

    | サブコマンド	| 機能				| 詳細			|
    |:------------------|:------------------------------|:----------------------|
    | arp		| @copybrief com_net_arp	| @ref com_net_arp	|
    | ip		| @copybrief com_net_ip		| @ref com_net_ip	|
    | dhcp		| @copybrief com_net_dhcp	| @ref com_net_dhcp	|
    | up		| @copybrief com_net_up		| @ref com_net_up	|
    | dowm		| @copybrief com_net_down	| @ref com_net_down	|
*/

#include "shell.h"
#include "device.h"
#include "str.h"
#include "tprintf.h"
#include "net.h"
#include "device/ether_ioctl.h"

#include "lwip/etharp.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/netifapi.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


extern struct netif netif;


static int arp(int argc, uchar *argv[]);

/**
   @brief	ARPテーブルを表示する
*/
static const struct st_shell_command com_net_arp = {
	.name		= "arp",
	.command	= arp,
	.usage_str	= "",
	.manual_str	= "Display ARP tables",
};

static int arp(int argc, uchar *argv[])
{
	int i;

	tprintf("Address         HW Address\n");
	tprintf("--------------- -----------------\n");
	for(i=0; i<ARP_TABLE_SIZE; i++) {
		ip4_addr_t *ip;
		struct netif *nif;
		struct eth_addr *ethaddr;

		if(etharp_get_entry(i, &ip, &nif, &ethaddr) != 0) {
			tprintf("%15s", ip4addr_ntoa(ip));
			tprintf(" %02X:%02X:%02X:%02X:%02X:%02X\n",
				(int)ethaddr->addr[0],
				(int)ethaddr->addr[1],
				(int)ethaddr->addr[2],
				(int)ethaddr->addr[3],
				(int)ethaddr->addr[4],
				(int)ethaddr->addr[5]);
		}
	}

	return 0;
}


static int ip(int argc, uchar *argv[]);

/**
   @brief	IPアドレス、ネットマスク、デフォルトゲートウェイを設定する
*/
static const struct st_shell_command com_net_ip = {
	.name		= "ip",
	.command	= ip,
	.usage_str	= "[ip_address] [<mask> <mask_address>] [<gw> <gw_address>]",
	.manual_str	= "Set network address",
};

static int ip(int argc, uchar *argv[])
{
	const ip4_addr_t *addr;
	ip4_addr_t ipaddr = {.addr = 0}, netmask = {.addr = 0}, gateway = {.addr = 0};

	tprintf("MAC Address : %02X:%02X:%02X:%02X:%02X:%02X\n",
		netif.hwaddr[0],
		netif.hwaddr[1],
		netif.hwaddr[2],
		netif.hwaddr[3],
		netif.hwaddr[4],
		netif.hwaddr[5]);

	if(argc == 1) {
		int stat;

		stat = net_status();
		if(stat & IORTN_BIT_ETHER_LINK_UP) {
			tprintf("Link UP,");
		} else {
			tprintf("Link Down,");
		}
		if(stat & IORTN_BIT_ETHER_100M) {
			tprintf(" 100baseT");
		} else {
			tprintf(" 10baseT");
		}
		if(stat & IORTN_BIT_ETHER_FULLDUPLEX) {
			tprintf("/Full");
		} else {
			tprintf("/Half");
		}
		tprintf("\n");

		addr = netif_ip4_addr(&netif);
		tprintf("IP Address  : %s\n", ip4addr_ntoa(addr));
		addr = netif_ip4_netmask(&netif);
		tprintf("Netmask     : %s\n", ip4addr_ntoa(addr));
		addr = netif_ip4_gw(&netif);
		tprintf("Gateway     : %s\n", ip4addr_ntoa(addr));

		return 0;
	} else if(argc == 2) {
		if(ip4addr_aton((char *)argv[1], &ipaddr) == 0) {
			goto error;
		}
	} else if((argc == 4) || (argc == 6)) {
		int i, opt = 1;

		if(ip4addr_aton((char *)argv[1], &ipaddr) == 0) {
			goto error;
		}

		if(argc == 6) {
			opt = 2;
		}

		for(i=0; i<opt; i++) {
			if(strcomp((uchar *)"mask", argv[2+i*2]) == 0) {
				if(ip4addr_aton((char *)argv[3+i*2], &netmask) == 0) {
					goto error;
				}
			} else if(strcomp((uchar *)"gw", argv[2+i*2]) == 0) {
				if(ip4addr_aton((char *)argv[3+i*2], &gateway) == 0) {
					goto error;
				}
			} else {
				goto error;
			}
		}
	}

	// Set IP, MASK, GW
	if(ipaddr.addr != 0) {
		tprintf("IP = %s\n", ip4addr_ntoa(&ipaddr));
	} else {
		addr = netif_ip4_addr(&netif);
		ipaddr = *addr;
	}
	if(netmask.addr != 0) {
		tprintf("MASK = %s\n", ip4addr_ntoa(&netmask));
	} else {
		addr = netif_ip4_addr(&netif);
		netmask = *addr;
	}
	if(gateway.addr != 0) {
		tprintf("GW = %s\n", ip4addr_ntoa(&gateway));
	} else {
		addr = netif_ip4_addr(&netif);
		gateway = *addr;
	}


	netif_set_addr(&netif, &ipaddr, &netmask, &gateway);

	return 0;

error:
	print_command_usage(&com_net_ip);

	return 0;
}

static int dhcp(int argc, uchar *argv[]);

/**
   @brief	DHCPによりIPアドレス、ネットマスク、デフォルトゲートウェイを設定する
*/
static const struct st_shell_command com_net_dhcp = {
	.name		= "dhcp",
	.command	= dhcp,
	.usage_str	= "[ip_address]",
	.manual_str	= "Start DHCP client",
};

static int dhcp(int argc, uchar *argv[])
{
	err_t rt = ERR_OK;
	dhcp_start(&netif);
	if(rt == ERR_OK) {
		tprintf("DHCP OK\n");
	} else {
		tprintf("DHCP Error\n");
	}

	return 0;
}


static int up(int argc, uchar *argv[]);

/**
   @brief	ネットワークインタフェースをリンクアップする
*/
static const struct st_shell_command com_net_up = {
	.name		= "up",
	.command	= up,
	.manual_str	= "Link up network",
};

static int up(int argc, uchar *argv[])
{
	link_up_net();

	return 0;
}


static int down(int argc, uchar *argv[]);

/**
   @brief	ネットワークインタフェースをリンクダウンする
*/
static const struct st_shell_command com_net_down = {
	.name		= "down",
	.command	= down,
	.manual_str	= "Link down network",
};

static int down(int argc, uchar *argv[])
{
	link_down_net();

	return 0;
}


static const struct st_shell_command * const com_net_list[] = {
	&com_net_arp,
	&com_net_ip,
	&com_net_dhcp,
	&com_net_up,
	&com_net_down,
	0
};

const struct st_shell_command com_net = {
	.name		= "net",
	.manual_str	= "Network operation commands",
	.sublist	= com_net_list
}; ///< ネットワーク情報取得
