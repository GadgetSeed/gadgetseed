/** @file
    @brief	時計表示

    @date	2019.12.09
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "str.h"
#include "graphics.h"
#include "font.h"
#include "tprintf.h"
#include "datetime.h"
#include "ui_statictext.h"

#include "musicplay_view.h"
#include "clock_view.h"

static char str_datetime[DATETIME_STR_LEN];
static struct st_ui_statictext stext_clock = {
	.view_area = {
		.pos.x		= 0,
		.pos.y		= 0,
		.sur.width	= DEFFONT_WIDTH * (DATETIME_STR_LEN - 1),
		.sur.height	= DEFFONT_HEIGHR,
	},
	.fore_color	= &fore_color,
	.back_color	= &back_color,
	.font_name	= MPFONT,
	.hattr		= FONT_HATTR_LEFT,
	.vattr		= FONT_VATTR_TOP,
	.fillattr	= UI_STATICTEXT_FILLARRT_NOFILL,
	.text		= (uchar *)str_datetime,
};

static struct st_systime lsystime;

static void set_datetime_str(struct st_systime *systime)
{
	struct st_datetime datetime;

	systime_to_datetime(&datetime, systime);
	datetime_to_str(str_datetime, &datetime);
}

void init_clock_view(void)
{
	struct st_systime systime;

	get_systime(&systime);
	set_datetime_str(&systime);
}

extern const unsigned int fore_color;
extern const unsigned int back_color;

void draw_clock_view(void)
{
	draw_ui_statictext(&stext_clock);
}

void clock_proc(struct st_sysevent *event)
{
	struct st_systime systime;

	get_systime(&systime);

	if(lsystime.sec != systime.sec) {
		lsystime = systime;
		set_datetime_str(&systime);
		draw_clock_view();
	}
}
