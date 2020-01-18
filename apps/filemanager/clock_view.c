/** @file
    @brief	ファイルマネージャ時計表示

    @date	2017.12.24
    @author	Takashi SHUDO
*/

#include "key.h"
#include "graphics.h"
#include "font.h"
#include "tprintf.h"
#include "datetime.h"
#include "filemanager.h"
#include "task/syscall.h"

#include "clock_view.h"

static struct st_datetime now_datetime;

static unsigned char time_str[32];


void draw_clock(void)
{
	time_to_str((char *)time_str, &now_datetime);

	set_forecolor(FM_INFO_FORE_COLOR);
	set_backcolor(FM_INFO_BACK_COLOR);
	set_font_drawmode(FONT_FIXEDWIDTH);

	draw_str(GSC_GRAPHICS_DISPLAY_WIDTH - font_width(' ')*8, 0, time_str, 32);
}

struct st_tcb clock_tcb;

static void sec_timer(void *sp, unsigned long long systime)
{
	task_wakeup_id_ISR(sp, clock_tcb.id);
}

static int clock_task(void *arg)
{
	struct st_systime now_time;

	register_sec_timer_func(sec_timer);

	tprintf("Start Clock task\n");

	get_systime(&now_time);
	systime_to_datetime(&now_datetime, &now_time);
	draw_clock();

	while(1) {
		get_systime(&now_time);
		systime_to_datetime(&now_datetime, &now_time);
		draw_clock();
		task_pause();
	}

	return 0;
}

#define SIZEOFAPPTS	(1024*8)
static unsigned int stack[SIZEOFAPPTS/sizeof(unsigned int)] ATTR_STACK;

void startup_clock(void)
{
	task_exec(clock_task, "clock", TASK_PRIORITY_APP_HIGH, &clock_tcb,
		  stack, SIZEOFAPPTS, 0);
}
