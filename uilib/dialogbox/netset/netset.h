/** @file
    @brief	時間設定ダイアログボックス

    @date	2018.01.27
    @author	Takashi SHUDO
*/

#ifndef NETSET_H
#define NETSET_H

#include "sysevent.h"

typedef int (* timeset_proc)(struct st_sysevent *event);

#ifdef GSC_COMP_ENABLE_FATFS
int load_network_setting(void);
void save_network_setting(void);
#endif
void prepare_netset(void);
void draw_netset(void);
int proc_netset(struct st_sysevent *event);
int do_netset(timeset_proc proc);
int open_netset_dialog(timeset_proc proc);

#endif // NETSET_H
