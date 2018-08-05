/** @file
    @brief	ユーザインタフェース - スイッチ

    @date	2018.02.04
    @auther	Takashi SHUDO
*/

#ifndef UI_SWITCH_H
#define UI_SWITCH_H

#include "sysevent.h"
#include "graphics.h"
#include "graphics_object.h"

#define UI_SWITCH_ST_INACTIVE	0
#define UI_SWITCH_ST_NORMAL	1
#define UI_SWITCH_ST_SELECT	2

#define UI_SWITCH_EVT_NULL	0
#define UI_SWITCH_EVT_OFF	1
#define UI_SWITCH_EVT_ON	2

#ifndef SWITCH_INACTIVE_COLOR
#define SWITCH_INACTIVE_COLOR	RGB(80, 80, 80)
#endif
#ifndef SWITCH_NORMAL_COLOR
#define SWITCH_NORMAL_COLOR	RGB(120, 120, 120)
#endif
#ifndef SWITCH_SELECT_COLOR
#define SWITCH_SELECT_COLOR	RGB(220, 220, 220)
#endif

#ifndef SWITCH_ON_COLOR
#define SWITCH_ON_COLOR		RGB(50,200,50)
#endif
#ifndef SWITCH_OFF_COLOR
#define SWITCH_OFF_COLOR	RGB(50,50,50)
#endif


struct st_switch_event {
	unsigned short id;	///< イベントが発生したスイッチID
	unsigned short what;	///< UIスイッチイベント種類
}; ///< UIスイッチイベント

struct st_ui_switch {
	int id;		///< スイッチID
	struct st_box text_area;
	struct st_box switch_area;
	struct st_box knob_area;
	char *font_name;
	unsigned char *name;
	int status;
	int value;
}; ///< UIスイッチ

void draw_ui_switch(struct st_ui_switch *ui_switch);
void draw_ui_switch_list(struct st_ui_switch **ui_switchs);
int proc_ui_switch(struct st_ui_switch *ui_switch, struct st_sysevent *event);
int proc_ui_switch_list(struct st_switch_event *ui_switch_event, struct st_ui_switch **ui_switchs, struct st_sysevent *event);

#endif // UI_SWITCH_H
