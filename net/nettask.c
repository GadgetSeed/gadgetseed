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

#include "sysconfig.h"

#include "net.h"
#include "log.h"
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
#include "datetime.h"
#include "device.h"
#include "task/syscall.h"
#include "device/ether_ioctl.h"

#include "devif.h"

//#define DEBUGKBITS 0x01
#include "dkprintf.h"

#define NETWORK_STATCHECK_INTARVAL	500


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
#endif
	dns_setserver(0, &dnsserver[0]);

#ifdef GSC_TCPIP_DEFAULT_DNSSERVER2	// $gsc DNSサーバアドレス2
	DKPRINTF(0x01, "DNS Address2    : %s\n", GSC_TCPIP_DEFAULT_DNSSERVER2);
	ip4addr_aton(GSC_TCPIP_DEFAULT_DNSSERVER2, &dnsserver[1]);
#endif
	dns_setserver(1, &dnsserver[1]);

	netif_set_up(&netif);
}

static void tcpip_init_done(void *arg)
{
	DKPRINTF(0x01, "### %s\n", __FUNCTION__);

	init_netifs();
}

static void start_dhcp(void)
{
	err_t rt = ERR_OK;

	dhcp_start(&netif);

	if(rt == ERR_OK) {
		gslog(0, "DHCP Start\n");
	} else {
		gslog(0, "DHCP Start Error\n");
	}
}

static void stop_dhcp(void)
{
	dhcp_release_and_stop(&netif);

	gslog(0, "DHCP Stop\n");
}

static int flg_link_stat = 0;
static int flg_enable_dhcp = 0;
static int flg_dhcp_stat = 0;

void enable_dhcp(void)
{
	int stat = 0;

	flg_enable_dhcp = 1;
	stat = net_status();
	if(stat & IORTN_BIT_ETHER_LINK_UP) {
		start_dhcp();
	}

	gslog(0, "DHCP Enable\n");
}

void disable_dhcp(void)
{
	flg_enable_dhcp = 0;

	stop_dhcp();

	gslog(0, "DHCP Disable\n");
}

int dhcp_status(void)
{
	int rtn = NET_DHCP_STAT_DISABLE;

	if(flg_enable_dhcp != 0) {
		if(flg_dhcp_stat != 0) {
			rtn = NET_DHCP_STAT_BOUND;
		} else {
			rtn = NET_DHCP_STAT_ENABLE;
		}
	}

	return rtn;
}

#ifdef GSC_TCPIP_ENABLE_SNTP
#include "lwip/apps/sntp.h"

void sntp_set_system_time(unsigned int sec, unsigned int usec)
{
	struct st_systime systime;
	struct st_datetime datetime;
	char str[DATEMTIME_STR_LEN];

	gslog(2, "SNTP sec = %u, usec = %u\n", sec, usec);

	systime.sec = sec;
	systime.usec = usec;
	set_systime(&systime);

	unixtime_to_datetime(&datetime, &systime);
	datemtime_to_str(str, &datetime);
	gslog(0, "Received NTP datetime : %s\n", str);
}

static void init_sntp(void)
{
	sntp_setoperatingmode(SNTP_OPMODE_POLL);

#ifdef GSC_TCPIP_DEFAULT_NTP_SERVERNAME	// $gsc デフォルトのNTPサーバ名
	sntp_setservername(0, GSC_TCPIP_DEFAULT_NTP_SERVERNAME);
#  ifdef GSC_TCPIP_ENABLE_START_SNTPINIT	// $gsc システム起動時にSNTPを有効にする
	sntp_init();
#  endif
#endif
}
#endif

int net_task(void *arg)
{
	DKPRINTF(0x01, "Network start\n");

	netif_init();

	tcpip_init(tcpip_init_done, 0);

	DKPRINTF(0x01, "TCP/IP initialized.\n");

#ifdef GSC_TCPIP_ENABLE_DHCP	// $gsc DHCPを有効にする
	flg_enable_dhcp = 1;
#endif // GSC_TCPIP_ENABLE_DHCP

#ifdef GSC_TCPIP_ENABLE_SNTP	// $gsc SNTPを有効にする
	init_sntp();
#endif

	while(1) {
		int stat;
		stat = net_status();

		if(flg_link_stat != stat) {
			gslog(0, "Link %s, %sMb/s, %s\n",
			      (stat & IORTN_BIT_ETHER_LINK_UP) ? "Up" : "Down",
			      (stat & IORTN_BIT_ETHER_100M) ? "100" : "10",
			      (stat & IORTN_BIT_ETHER_FULLDUPLEX) ? "Full" : "Half");

			if(stat & IORTN_BIT_ETHER_LINK_UP) {
				netif_set_link_up(&netif);
			} else {
				netif_set_link_down(&netif);
			}

			if(stat & IORTN_BIT_ETHER_LINK_UP) {
				if(flg_enable_dhcp != 0) {
					start_dhcp();
				}
			}

			flg_link_stat = stat;
		}

		stat = dhcp_supplied_address(&netif);
		if(flg_dhcp_stat != stat) {
			if(stat != 0) {
				gslog(0, "DHCP Bound\n");
			}
			flg_dhcp_stat = stat;
		}

		task_sleep(NETWORK_STATCHECK_INTARVAL);
	}

	return 0;
}

#define SIZEOFSTACK	(1024*4)
struct st_tcb net_tcb;
unsigned int net_stack[SIZEOFSTACK/sizeof(unsigned int)] ATTR_STACK;

void startup_network(void)
{
	task_exec(net_task, "network", TASK_PRIORITY_NETWORK, &net_tcb,
		  net_stack, SIZEOFSTACK, 0);
}
