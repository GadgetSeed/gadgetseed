/** @file
    @brief	時間設定ダイアログボックス

    @date	2018.01.27
    @author	Takashi SHUDO
*/

#ifndef TIMESET_H
#define TIMESET_H

#include "sysevent.h"
#include "datetime.h"

typedef int (* timeset_proc)(struct st_sysevent *event);

void prepare_timeset(struct st_datetime *datetime);
void draw_timeset(void);
int proc_timeset(struct st_sysevent *event);
int do_timeset(timeset_proc proc);
int open_timeset_dialog(timeset_proc proc);

#endif // TIMESET_H
