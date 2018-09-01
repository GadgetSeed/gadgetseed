/** @file
    @brief	ネットワークタスク

    @date	2017.08.15
    @date	2013.01.02
    @author	Takashi SHUDO

    @page network ネットワーク

    GadgetSeedのTCP/IPプロトコルスタックは [LwIP](https://savannah.nongnu.org/projects/lwip/) を使用しています。

    TCP/IP を使用するには、以下のコンフィグ項目を有効にして下さい。

    * COMP_ENABLE_TCPIP

    TCP/IP を使用した場合、TCP/IP の処理のために専用のタスクが起動されます。TCP/IP 処理用のタスクはタスク名"network"です。

    @ref config_item で生成されるマクロで TCP/IP の以下の項目を設定することができます。

    | マクロ名				| 内容					|
    |-----------------------------------|:--------------------------------------|
    | GSC_TCPIP_DEFAULT_GATEWAY		| TCP/IPデフォルトゲートウェイアドレス	|
    | GSC_TCPIP_DEFAULT_IPADDR		| TCP/IPデフォルトIPアドレス		|
    | GSC_TCPIP_DEFAULT_NETMASK		| TCP/IPデフォルトネットマスク		|
    | GSC_TCPIP_DEFAULT_DNSSERVER	| DNS サーバアドレス			|
    | GSC_TCPIP_DEFAULT_DNSSERVER2	| DNS サーバアドレス2			|
*/

#include "net.h"
#include "lwip/init.h"
#include "lwip/debug.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/ip.h"
#include "lwip/udp.h"
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "netif/etharp.h"

#include "tprintf.h"
#include "device.h"
#include "task/syscall.h"

#include "devif.h"

//#define DEBUGKBITS 0x01
#include "dkprintf.h"


struct netif netif;
static ip_addr_t ipaddr, netmask, gateway, dnsserver[DNS_MAX_SERVERS];

static void init_netifs(void)
{
#ifdef GSC_TCPIP_DEFAULT_IPADDR	// $gsc TCP/IPデフォルトIPアドレス
	DKPRINTF(0x01, "IP Address      : %s\n", GSC_TCPIP_DEFAULT_IPADDR);
	ip4addr_aton(GSC_TCPIP_DEFAULT_IPADDR, &ipaddr);
#endif
#ifdef GSC_TCPIP_DEFAULT_NETMASK	// $gsc TCP/IPデフォルトネットマスク
	DKPRINTF(0x01, "Default Netmask : %s\n", GSC_TCPIP_DEFAULT_NETMASK);
	ip4addr_aton(GSC_TCPIP_DEFAULT_NETMASK, &netmask);
#endif
#ifdef GSC_TCPIP_DEFAULT_GATEWAY	// $gsc TCP/IPデフォルトゲートウェイアドレス
	DKPRINTF(0x01, "Default Gateway : %s\n", GSC_TCPIP_DEFAULT_GATEWAY);
	ip4addr_aton(GSC_TCPIP_DEFAULT_GATEWAY, &gateway);
#endif

	netif_set_default(netif_add(&netif, &ipaddr, &netmask, &gateway,
				    NULL,
				    devif_init,
				    tcpip_input
				    )
			  );

#ifdef GSC_TCPIP_DEFAULT_DNSSERVER	// $gsc DNSサーバアドレス
	DKPRINTF(0x01, "DNS Address     : %s\n", GSC_TCPIP_DEFAULT_DNSSERVER);
	ip4addr_aton(GSC_TCPIP_DEFAULT_DNSSERVER, &dnsserver[0]);
	dns_setserver(0, &dnsserver[0]);
#endif

#ifdef GSC_TCPIP_DEFAULT_DNSSERVER2	// $gsc DNSサーバアドレス2
	DKPRINTF(0x01, "DNS Address2    : %s\n", GSC_TCPIP_DEFAULT_DNSSERVER2);
	ip4addr_aton(GSC_TCPIP_DEFAULT_DNSSERVER2, &dnsserver[1]);
	dns_setserver(1, &dnsserver[1]);
#endif

	netif_set_up(&netif);
}

static void tcpip_init_done(void *arg)
{
	DKPRINTF(0x01, "### %s\n", __FUNCTION__);

	init_netifs();
}

int net_task(char *arg)
{
	DKPRINTF(0x01, "Network start\n");

	netif_init();

	tcpip_init(tcpip_init_done, 0);
	netif_set_link_up(&netif);

	DKPRINTF(0x01, "TCP/IP initialized.\n");

	return 0;
}

#define SIZEOFSTACK	(1024*4)
struct st_tcb net_tcb;
unsigned int net_stack[SIZEOFSTACK/sizeof(unsigned int)];

void startup_network(void)
{
	task_exec(net_task, "network", 0, &net_tcb,
		  net_stack, SIZEOFSTACK, 0);
}
