/** @file
    @brief	ファイル/ラジオ切り替え

    @date	2019.01.03
    @auther	Takashi SHUDO
*/

#ifndef MODE_VIEW_H
#define MODE_VIEW_H

#include "sysevent.h"

typedef enum {
	SDCARD,		// SDカード
	RADIO		// Internaet radio
} enum_musicplay_mode;

extern enum_musicplay_mode musicplay_mode;

void init_mode_view(void);
void draw_mode_view(void);
void mode_proc(struct st_sysevent *event);

#endif // MODE_VIEW_H
