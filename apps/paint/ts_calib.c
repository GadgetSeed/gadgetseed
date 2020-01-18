/** @file
    @brief	タッチセンサキャリブレーション

    @date	2017.12.12
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "graphics.h"
#include "sysevent.h"
#include "task/syscall.h"
#include "device/ts_ioctl.h"

#include "ts_calib.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"

#define BACK_COLOR	RGB(20,20,20)
#define MARKER_COLOR	RGB(200,200,200)

#define MARK_SIZE	16
#define MARK_X_MARGINE	16
#define MARK_Y_MARGINE	16

static void draw_tp_marker(void)
{
	struct st_rect frect = { 0, 0, GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT };

	task_sleep(100);

	set_forecolor(BACK_COLOR);

	draw_fill_rect(&frect);

	set_forecolor(MARKER_COLOR);

	draw_h_line(MARK_X_MARGINE - MARK_SIZE/2, MARK_Y_MARGINE, MARK_SIZE);
	draw_v_line(MARK_X_MARGINE, MARK_Y_MARGINE - MARK_SIZE/2, MARK_SIZE);

	draw_h_line(GSC_GRAPHICS_DISPLAY_WIDTH-MARK_X_MARGINE*2 + MARK_X_MARGINE - MARK_SIZE/2, MARK_Y_MARGINE, MARK_SIZE);
	draw_v_line(GSC_GRAPHICS_DISPLAY_WIDTH-MARK_X_MARGINE*2 + MARK_X_MARGINE, MARK_Y_MARGINE - MARK_SIZE/2, MARK_SIZE);

	draw_h_line(MARK_X_MARGINE - MARK_SIZE/2, GSC_GRAPHICS_DISPLAY_HEIGHT-MARK_Y_MARGINE*2 + MARK_Y_MARGINE, MARK_SIZE);
	draw_v_line(MARK_X_MARGINE, GSC_GRAPHICS_DISPLAY_HEIGHT-MARK_Y_MARGINE*2 + MARK_Y_MARGINE - MARK_SIZE/2, MARK_SIZE);

	draw_h_line(GSC_GRAPHICS_DISPLAY_WIDTH-MARK_X_MARGINE*2 + MARK_X_MARGINE - MARK_SIZE/2,
		    GSC_GRAPHICS_DISPLAY_HEIGHT-MARK_Y_MARGINE*2 + MARK_Y_MARGINE, MARK_SIZE);
	draw_v_line(GSC_GRAPHICS_DISPLAY_WIDTH-MARK_X_MARGINE*2 + MARK_X_MARGINE,
		    GSC_GRAPHICS_DISPLAY_HEIGHT-MARK_Y_MARGINE*2 + MARK_Y_MARGINE - MARK_SIZE/2, MARK_SIZE);
}

static int flg_point[2][2];
static int mark_x[4];
static int mark_y[4];

int mark_pos(short x, short y, int vx, int vy)
{
	int flg_right = 0;
	int flg_bottom = 0;

	DTPRINTF(0x01, "X = %d, Y = %d, VX = %d, VY = %d\n", x, y, vx, vy);

	if(x < (GSC_GRAPHICS_DISPLAY_WIDTH / 2)) {
		mark_x[0] = vx;
	} else {
		mark_x[1] = vx;
		flg_right = 1;
	}

	if(y < (GSC_GRAPHICS_DISPLAY_HEIGHT / 2)) {
		mark_y[0] = vy;
	} else {
		mark_y[1] = vy;
		flg_bottom = 1;
	}

	flg_point[flg_right][flg_bottom] = 1;

	if((flg_right == 0) && (flg_bottom == 0)) {
		draw_circle(MARK_X_MARGINE, MARK_Y_MARGINE, MARK_SIZE/2);
	} else
	if((flg_right == 1) && (flg_bottom == 0)) {
		draw_circle(GSC_GRAPHICS_DISPLAY_WIDTH-MARK_X_MARGINE, MARK_Y_MARGINE, MARK_SIZE/2);
	} else
	if((flg_right == 0) && (flg_bottom == 1)) {
		draw_circle(MARK_X_MARGINE, GSC_GRAPHICS_DISPLAY_HEIGHT-MARK_Y_MARGINE, MARK_SIZE/2);
	} else
	if((flg_right == 1) && (flg_bottom == 1)) {
		draw_circle(GSC_GRAPHICS_DISPLAY_WIDTH-MARK_X_MARGINE, GSC_GRAPHICS_DISPLAY_HEIGHT-MARK_Y_MARGINE, MARK_SIZE/2);
	}

	if((flg_point[0][0] != 0) &&
	   (flg_point[1][0] != 0) &&
	   (flg_point[0][1] != 0) &&
	   (flg_point[1][1] != 0)) {
		struct st_ts_calib_data cd;
		struct st_device *tsd = open_device(DEF_DEV_NAME_TS);

		cd.left_pos = MARK_X_MARGINE;
		cd.left_val = mark_x[0];
		cd.right_pos = (GSC_GRAPHICS_DISPLAY_WIDTH - MARK_X_MARGINE);
		cd.right_val = mark_x[1];

		cd.top_pos = MARK_Y_MARGINE;
		cd.top_val = mark_y[0];
		cd.bottom_pos = (GSC_GRAPHICS_DISPLAY_HEIGHT - MARK_Y_MARGINE);
		cd.bottom_val = mark_y[1];

		ioctl_device(tsd, IOCMD_TS_SET_CALIB, 0, (void *)&cd);
		return 1;
	} else { 
		return 0;
	}
}

static int ts_calib_task(void *arg)
{
	draw_tp_marker();

	while(1) {
		struct st_sysevent event;

		if(get_event(&event, 50)) {
			switch(event.what) {
			case EVT_TOUCHSTART:
			//case EVT_TOUCHEND:
				{
					int *low_val = (int *)event.private_data;

					if(mark_pos((int)event.pos_x, (int)event.pos_y, low_val[0], low_val[1]) != 0) {
						return 1;
					}
				}
				break;

			default:
				break;
			}
		}
	}

	return 0;
}

#ifdef TS_CALIB_TASK
static struct st_tcb tcb;
#define SIZEOFSTACK	(1024*2)
static unsigned int stack[SIZEOFSTACK/sizeof(unsigned int)] ATTR_STACK;
#endif

void startup_ts_calib(void)
{
#ifdef TS_CALIB_TASK
	task_exec(ts_calib_task, "ts_calib", TASK_PRIORITY_APP_HIGH, &tcb,
		  stack, SIZEOFSTACK, 0);
#else
	while(1) {
		if(ts_calib_task(0) != 0) {
			break;
		}
	}
#endif
}
