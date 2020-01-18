/** @file
    @brief	10キー

    @date	2019.09.07
    @author	Takashi SHUDO
*/

#ifndef TENKEY_H
#define TENKEY_H

#include "sysevent.h"

#define BTN_ID_0	100
#define BTN_ID_1	101
#define BTN_ID_2	102
#define BTN_ID_3	103
#define BTN_ID_4	104
#define BTN_ID_5	105
#define BTN_ID_6	106
#define BTN_ID_7	107
#define BTN_ID_8	108
#define BTN_ID_9	109
#define BTN_ID_BACK	110
#define BTN_ID_FORWARD	112
#define BTN_ID_BACKSPACE	113
#define BTN_ID_DELETE	114

void set_mode_tenkey(int mode);
void draw_tenkey(void);
int proc_tenkey(struct st_button_event *obj_evt, struct st_sysevent *event);

#endif // TENKEY_H
