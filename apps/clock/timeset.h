/** @file
    @brief	時間設定インタフェース

    @date	2018.01.27
    @author	Takashi SHUDO
*/

#ifndef TIMESET_H
#define TIMESET_H

#include "sysevent.h"
#include "datetime.h"

void prepare_timeset(struct st_datetime *datetime);
void draw_timeset(void);
void proc_timeset(struct st_sysevent *event);

#endif // TIMESET_H
