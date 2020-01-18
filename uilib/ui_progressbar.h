/** @file
    @brief	ユーザインタフェース - プログレスバー

    @date	2019.03.03
    @auther	Takashi SHUDO
*/

#ifndef UI_PROGRESSBAR_H
#define UI_PROGRESSBAR_H

#include "graphics.h"
#include "graphics_object.h"

struct st_ui_progressbar {
	struct st_box view_area;	///< 表示エリア
	const struct st_graph_object *view;
	int max_value;
	int value;
}; ///< UIプログレスバー

void draw_ui_progressbar(struct st_ui_progressbar *progressbar);
void set_value_ui_progressbar(struct st_ui_progressbar *progressbar, int value);

#endif // UI_PROGRESSBAR_H
