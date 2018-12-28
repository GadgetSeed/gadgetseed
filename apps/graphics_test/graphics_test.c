/** @file
    @brief	グラフィックステスト

    @date	2017.01.14
    @author	Takashi SHUDO
*/

#include "graphics.h"
#include "sysevent.h"
#include "key.h"
#include "font.h"
#include "tprintf.h"
#include "random.h"
#include "str.h"
#include "shell.h"
#include "task/syscall.h"
#include "sysconfig.h"

#define GFLG_LINE	0x00000001
#define GFLG_RECT	0x00000002
#define GFLG_CIRCLE	0x00000004
#define GFLG_VERTEX4	0x00000008
#define GFLG_ELLIPSE	0x00000010
#define GFLG_CHAR	0x00000020
#define GFLG_BITMAP	0x00000040
static unsigned int flg_draw = 0x7f;

static int log_on = 0;

static volatile int run_stat = 1;

void rand_color(void)
{
	int r, g, b;
	unsigned long color;

	r = genrand_int32() % 256;
	g = genrand_int32() % 256;
	b = genrand_int32() % 256;

	color = RGB(r, g, b);

	set_forecolor(color);
}

void draw_rand_line(void)
{
	struct st_rect rect;

	rect.left = (genrand_int32() % GSC_GRAPHICS_DISPLAY_WIDTH);
	rect.right = (genrand_int32() % GSC_GRAPHICS_DISPLAY_WIDTH);
	rect.top = (genrand_int32() % GSC_GRAPHICS_DISPLAY_HEIGHT);
	rect.bottom = (genrand_int32() % GSC_GRAPHICS_DISPLAY_HEIGHT);

	if(log_on != 0) {
		tprintf("line    %4d %4d  %4d %4d\n",
			rect.left, rect.top, rect.right, rect.bottom);
	}

	rand_color();
	draw_line(rect.left, rect.top, rect.right, rect.bottom);
}

void draw_rand_rect(int type)
{
	struct st_rect rect;
	short w, h, r = 0;

	rect.left = (genrand_int32() % GSC_GRAPHICS_DISPLAY_WIDTH);
	rect.right = (genrand_int32() % GSC_GRAPHICS_DISPLAY_WIDTH);
	rect.top = (genrand_int32() % GSC_GRAPHICS_DISPLAY_HEIGHT);
	rect.bottom = (genrand_int32() % GSC_GRAPHICS_DISPLAY_HEIGHT);

	correct_rect(&rect);

	w = rect.right - rect.left;
	h = rect.bottom - rect.top;
	if(w > h) {
		if((h/2) != 0) {
			r = genrand_int32() % (h/2);
		}
	} else {
		if((w/2) != 0) {
			r = genrand_int32() % (w/2);
		}
	}

	if(log_on != 0) {
		tprintf("rect    %4d %4d  %4d %4d (%d)\n",
			rect.left, rect.top, rect.right, rect.bottom, r);
	}

	rand_color();
	switch(type) {
	case 0:
		draw_rect(&rect);
		break;
	case 1:
		draw_fill_rect(&rect);
		break;
	case 2:
		draw_round_rect(&rect, r);
		break;
	case 3:
		draw_round_fill_rect(&rect, r);
		break;
	default:
		break;
	}
}

void draw_rand_circle(int fill)
{
	short x, y, r;

	x = (genrand_int32() % GSC_GRAPHICS_DISPLAY_WIDTH);
	y = (genrand_int32() % GSC_GRAPHICS_DISPLAY_HEIGHT);
	r = (genrand_int32() % GSC_GRAPHICS_DISPLAY_HEIGHT / 2);

	if(log_on != 0) {
		tprintf("circle  %4d %4d  %4d\n",
			x, y, r);
	}

	rand_color();
	if(fill != 0) {
		draw_fill_circle(x, y, r);
	} else {
		draw_circle(x, y, r);
	}
}

void draw_rand_ellipse(int fill)
{
	short x, y, rx, ry;

	x = (genrand_int32() % GSC_GRAPHICS_DISPLAY_WIDTH);
	y = (genrand_int32() % GSC_GRAPHICS_DISPLAY_HEIGHT);
	rx = (genrand_int32() % GSC_GRAPHICS_DISPLAY_WIDTH/2);
	ry = (genrand_int32() % GSC_GRAPHICS_DISPLAY_WIDTH/2);

	if(log_on != 0) {
		tprintf("ellipse %4d %4d  %4d %4d\n",
			x, y, rx, ry);
	}

	rand_color();
	if(fill != 0) {
		draw_fill_ellipse(x, y, rx, ry);
	} else {
		draw_ellipse(x, y, rx, ry);
	}
}

void draw_rand_vertex4(void)
{
	int i;
	short x[4], y[4];

	for(i=0; i<4; i++) {
		x[i] = (genrand_int32() % GSC_GRAPHICS_DISPLAY_WIDTH);
		y[i] = (genrand_int32() % GSC_GRAPHICS_DISPLAY_HEIGHT);
	}

	if(log_on != 0) {
		tprintf("vertex4 %4d %4d  %4d %4d  %4d %4d  %4d %4d\n",
				x[0], y[0],
				x[1], y[1],
				x[2], y[2],
				x[3], y[3]);
	}

	rand_color();
	draw_vertex4_region(x[0], y[0],
			    x[1], y[1],
			    x[2], y[2],
			    x[3], y[3]);
}

void draw_rand_char(void)
{
	short x, y;
	unsigned char c;

	x = (genrand_int32() % GSC_GRAPHICS_DISPLAY_WIDTH);
	y = (genrand_int32() % GSC_GRAPHICS_DISPLAY_HEIGHT);
	c = '0' + (genrand_int32() % 10);

	if(log_on != 0) {
		tprintf("char    %4d %4d  %c\n",
			x, y, c);
	}

	rand_color();
	draw_char(x, y, c);
}

extern struct st_bitmap gs_logo;

void draw_rand_bitmap(void)
{
	short x, y;
	int rate;

	x = (genrand_int32() % GSC_GRAPHICS_DISPLAY_WIDTH);
	y = (genrand_int32() % GSC_GRAPHICS_DISPLAY_HEIGHT);
	rate = (genrand_int32() % 16);

	if(log_on != 0) {
		tprintf("bitmap  %4d %4d  %c\n",
			x, y, rate);
	}

	rand_color();
	draw_enlarged_bitmap(x, y, &gs_logo, rate);
//	draw_bitmap(x, y, &gs_logo);
}


static int gtest_flag(int argc, uchar *argv[])
{
	if(argc < 2) {
		tprintf("%08X\n", flg_draw);
	} else {
		flg_draw = hstoi(argv[1]);
	}

	return 0;
}

const struct st_shell_command com_gtest_flag = {
	.name		= "flag",
	.command	= gtest_flag,
	.usage_str	= "<val>",
	.manual_str	= "Set graphics test execute flag"
};

static int gtest_log(int argc, uchar *argv[])
{
	if(argc < 2) {
		if(log_on != 0) {
			log_on = 0;
		} else {
			log_on = 1;
		}
	} else {
		log_on = hstoi(argv[1]);
	}

	return 0;
}

const struct st_shell_command com_gtest_log = {
	.name		= "log",
	.command	= gtest_log,
	.usage_str	= "[0:off | 1:on]",
	.manual_str	= "Set graphics test log print"
};

static int gtest_run(int argc, uchar *argv[])
{
	if(argc < 2) {
		if(run_stat != 0) {
			run_stat = 0;
		} else {
			run_stat = 1;
		}
	} else {
		run_stat = hstoi(argv[1]);
	}

	return 0;
}

const struct st_shell_command com_gtest_run = {
	.name		= "run",
	.command	= gtest_run,
	.usage_str	= "[0:off | 1:on]",
	.manual_str	= "Run or stop graphics test"
};

static int gtest_step(int argc, uchar *argv[])
{
	run_stat = 2;

	return 0;
}

const struct st_shell_command com_gtest_step = {
	.name		= "step",
	.command	= gtest_step,
	.attr		= CMDATTR_CONT,
	.manual_str	= "Set step execute"
};

const struct st_shell_command * const com_gtest_list[] = {
	&com_gtest_run,
	&com_gtest_step,
	&com_gtest_flag,
	&com_gtest_log,
	0
};

const struct st_shell_command com_gtest = {
	.name		= "gtest",
	.manual_str	= "Graphics test commands",
	.sublist	= com_gtest_list
};

static int test_no = 0;

void draw_test(void)
{
	int flg_ex = 0;

	if(flg_draw == 0) {
		return;
	}

	while(1) {
		switch(test_no) {
		case 0:
			if(flg_draw & GFLG_LINE) {
				draw_rand_line();
				flg_ex = 1;
			}
			break;
		case 1:
			if(flg_draw & GFLG_RECT) {
				draw_rand_rect(3);
				flg_ex = 1;
			}
			break;
		case 2:
			if(flg_draw & GFLG_CIRCLE) {
				draw_rand_circle(0);
				flg_ex = 1;
			}
			break;
		case 3:
			if(flg_draw & GFLG_VERTEX4) {
				draw_rand_vertex4();
				flg_ex = 1;
			}
			break;
		case 4:
			if(flg_draw & GFLG_ELLIPSE) {
				draw_rand_ellipse(1);
				flg_ex = 1;
			}
			break;
		case 5:
			if(flg_draw & GFLG_CHAR) {
				draw_rand_char();
				flg_ex = 1;
			}
			break;
		case 6:
			if(flg_draw & GFLG_BITMAP) {
				draw_rand_bitmap();
				flg_ex = 1;
			}
			break;
		default:
			break;
		}

		test_no ++;
		if(test_no > 6) {
			test_no = 0;
		}

		if(flg_ex != 0) {
			break;
		}
	}
}

int gt_task(char *arg)
{
#if 0
	struct event event;
	rect crect = { 100, 100, 300, 300 };

	set_clip_rect(&crect);
#endif
	add_shell_command((struct st_shell_command *)&com_gtest);

	while(1) {
#if 0
		if(get_event(&event, 50)) {
			switch(event.what) {
			case EVT_TOUCHSTART:
				draw_enlarged_bitmap(event.pos_x, event.pos_y, &gs_logo, 10);
				break;
			}
		}
#endif

		if(run_stat == 0) {
			continue;
		}

		draw_test();

		if(run_stat == 2) {
			run_stat = 0;
		}
	}

	return 0;
}

#define SIZEOFAPPTS	(1024*8)
static struct st_tcb tcb;
static unsigned int stack[SIZEOFAPPTS/sizeof(unsigned int)];

void startup_graphics_test(void)
{
	task_exec(gt_task, "graphs_test", TASK_PRIORITY_APP_HIGH, &tcb,
		     stack, SIZEOFAPPTS, 0);
}
