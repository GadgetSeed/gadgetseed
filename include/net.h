/** @file
    @brief	Network API

    @date	2018.05.01
    @author	Takashi SHUDO
*/

#ifndef	NET_H
#define	NET_H

void startup_network(void);
int link_up_netdev(void);
int link_down_netdev(void);
int net_status(void);

#define NET_DHCP_STAT_DISABLE	0	///< DHCP無効
#define NET_DHCP_STAT_ENABLE	1	///< DHCP有効
#define NET_DHCP_STAT_BOUND	2	///< DHCPアドレス取得

void enable_dhcp(void);
void disable_dhcp(void);
int dhcp_status(void);
void sntp_set_system_time(unsigned int sec, unsigned int usec);

#endif // NET_H
