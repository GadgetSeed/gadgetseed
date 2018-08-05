/** @file
    @brief	Network API

    @date	2018.05.01
    @author	Takashi SHUDO
*/

#ifndef	NET_H
#define	NET_H

void startup_network(void);
int link_up_net(void);
int link_down_net(void);
int net_status(void);

#endif // NET_H
