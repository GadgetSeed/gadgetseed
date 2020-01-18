/** @file
    @brief	musicplay/internetradio各種設定

    @date	2019.12.07
    @auther	Takashi SHUDO
*/

#ifndef CONFIG_VIEW_H
#define CONFIG_VIEW_H

#include "sysevent.h"

void init_config_view(void);
void draw_config_view(void);
void config_proc(struct st_sysevent *event);

#endif // CONFIG_VIEW_H
