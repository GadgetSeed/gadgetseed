/** @file
    @brief	musicplay/internetradio各種構成設定

    @date	2019.12.21
    @auther	Takashi SHUDO
*/

#ifndef SETTINGS_VIEW_H
#define SETTINGS_VIEW_H

#include "sysevent.h"

extern int flg_setting;

void init_settings_view(void);
void draw_settings_view(void);
int settings_proc(struct st_sysevent *event);
int open_settings_dialog(void);

#endif // SETTINGS_VIEW_H
