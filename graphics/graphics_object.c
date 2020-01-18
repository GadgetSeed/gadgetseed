/** @file
    @brief	グラフィックス集合体描画

    @date	2017.05.04
    @author	Takashi SHUDO
*/

#include "graphics_object.h"
#include "graphics.h"
#include "font.h"
#include "tprintf.h"

#define DEFAULT_SC_NUM	0x10000
#define DEFAULT_SC_DEN	0x10000

static int sc_numerator = DEFAULT_SC_NUM;
static int sc_denominator = DEFAULT_SC_DEN;

static inline short SC(short val)
{
	return (((int)val) * sc_numerator)/sc_denominator;
}

/**
   @brief	描画するグラフィックオブジェクトの拡大率を設定する

   @param[in]	numerator	拡大率分子
   @param[in]	denominator	各倍率分母
*/
void set_graph_obj_scale(int numerator, int denominator)
{
	if((numerator == 0) || (denominator == 0)) {
		sc_numerator = DEFAULT_SC_NUM;
		sc_denominator = DEFAULT_SC_DEN;
	} else {
		sc_numerator = numerator;
		sc_denominator = denominator;
	}
}

static void set_box(short x, short y, struct st_box *box, const int arg[4])
{
	box->pos.x = SC(arg[0]) + x;
	box->pos.y = SC(arg[1]) + y;
	box->sur.width = SC(arg[2]);
	box->sur.height = SC(arg[3]);
}

/**
   @brief	グラフィックオブジェクトを描画する

   @param[in]	x	X座標
   @param[in]	y	Y座標
   @param[in]	gobj	グラフィックオブジェクト
*/
void draw_graph_object(short x, short y, const struct st_graph_object *gobj)
{
	const struct st_graph_object *go = gobj;

	//tprintf("g_obj %d, %d\n", x, y);

	while((go != 0) && (go->type != GO_TYPE_OBJECT_END)) {
		//tprintf("type : %d %d %d %d %d %d %d\n", go->type,
		//	go->arg[0], go->arg[1], go->arg[2], go->arg[3], go->arg[4], go->arg[5]);
		switch(go->type) {
		case GO_TYPE_MODE:
			set_draw_mode(go->arg[0]);
			break;

		case GO_TYPE_FORECOLOR:
			set_forecolor(go->arg[0]);
			break;

		case GO_TYPE_BACKCOLOR:
			set_backcolor(go->arg[0]);
			break;

		case GO_TYPE_FONT:
			(void)set_font_by_name((char *)(go->data));
			break;

		case GO_TYPE_TEXT:
			draw_str(SC(go->arg[0])+x, SC(go->arg[1])+y, (unsigned char *)(go->data), go->arg[2]);
			break;

		case GO_TYPE_TEXT_IN_BOX:
			{
				struct st_box box;
				set_box(x, y, &box, go->arg);
				draw_str_in_box(&box, go->arg[4], go->arg[5], (unsigned char *)(go->data));
			}
			break;

		case GO_TYPE_FILL_CIRCLE:
			{
				draw_fill_circle(SC(go->arg[0])+x, SC(go->arg[1])+y,
						 SC(go->arg[2]));
			}
			break;

		case GO_TYPE_BOX:
			{
				struct st_box box;
				set_box(x, y, &box, go->arg);
				draw_box(&box);
			}
			break;

		case GO_TYPE_ROUND_BOX:
			{
				struct st_box box;
				set_box(x, y, &box, go->arg);
				//tprintf("round_box %d %d %d %d %d\n",
				//	box.px, box.py, box.width, box.height, go->arg[4]);
				draw_round_box(&box, SC(go->arg[4]));
			}
			break;

		case GO_TYPE_ROUND_FILL_BOX:
			{
				struct st_box box;
				set_box(x, y, &box, go->arg);
				//tprintf("round_box %d %d %d %d %d\n",
				//	box.px, box.py, box.width, box.height, go->arg[4]);
				draw_round_fill_box(&box, SC(go->arg[4]));
			}
			break;

		case GO_TYPE_FILL_BOX:
			{
				struct st_box box;
				set_box(x, y, &box, go->arg);
				//tprintf("box %d %d %d %d\n",
				//	box.px, box.py, box.width, box.height);
				draw_fill_box(&box);
			}
			break;

		case GO_TYPE_VERTEX4:
			{
				draw_vertex4_region(SC(go->arg[0])+x, SC(go->arg[1])+y,
						    SC(go->arg[2])+x, SC(go->arg[3])+y,
						    SC(go->arg[4])+x, SC(go->arg[5])+y,
						    SC(go->arg[6])+x, SC(go->arg[7])+y);
			}
			break;

		case GO_TYPE_TRIANGLE:
			{
				draw_triangle_region(SC(go->arg[0])+x, SC(go->arg[1])+y,
						     SC(go->arg[2])+x, SC(go->arg[3])+y,
						     SC(go->arg[4])+x, SC(go->arg[5])+y);
			}
			break;

		case GO_TYPE_SECTOR:
			{
				draw_sector(SC(go->arg[0])+x, SC(go->arg[1])+y,
					    SC(go->arg[2]), SC(go->arg[3]),
					    go->arg[4]);
			}
			break;

		case GO_TYPE_BITMAP:
			{
				draw_bitmap(SC(go->arg[0])+x, SC(go->arg[1])+y,
					    (struct st_bitmap *)go->data);
			}
			break;

		case GO_TYPE_OBJECT:
			{
				int numerator = sc_numerator;
				int denominator = sc_denominator;

				set_graph_obj_scale(go->arg[2], go->arg[3]);
				draw_graph_object(SC(go->arg[0])+x, SC(go->arg[1])+y, (const struct st_graph_object *)(go->data));

				sc_numerator = numerator;
				sc_denominator = denominator;
			}
			break;

		default:
			break;
		}

		go++;
	}
}
