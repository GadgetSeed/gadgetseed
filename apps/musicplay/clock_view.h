/** @file
    @brief	時計表示

    @date	2019.12.09
    @auther	Takashi SHUDO
*/

#ifndef CLOCK_VIEW_H
#define CLOCK_VIEW_H

#include "sysevent.h"

void init_clock_view(void);
void draw_clock_view(void);
void clock_proc(struct st_sysevent *event);

#endif // CLOCK_VIEW_H
