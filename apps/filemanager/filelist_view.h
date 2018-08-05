/** @file
    @brief	ファイルリスト表示

    @date	2017.11.18
    @author	Takashi SHUDO
*/

#ifndef FILELIST_VIEW_H
#define FILELIST_VIEW_H

#include "graphics.h"

#include "ui_selectlist.h"

#include "filelist_data.h"

#define MAX_PATH_LEN	255
#define MAX_DIR_DEPTH	16

#define FLV_EVT_NONE		0
#define FLV_EVT_DIR_SEL		1
#define FLV_EVT_MOVE_DIR	2
#define FLV_EVT_FILE_FIRST_SEL	3
#define FLV_EVT_FILE_SECCOND_SEL	4

struct st_filelist_data {
	char file_print_form[32];
	char dir_print_form[8];
	struct st_fileinfo **fileinfo_index;
	int dir_depth;
	int select_num[MAX_DIR_DEPTH];
};

struct st_filelist_view {
	char *font_name;
	struct st_box info_area;
	struct st_box list_area;
	const struct st_graph_object *info_view;
	const struct st_graph_object *list_view;
	uchar path[MAX_PATH_LEN+1];
	struct st_ui_selectlist selectlist;
	struct st_ui_scrollbar scrollbar;
	struct st_fileinfo fileinfo_list;
	struct st_filelist_data filelist_data;
};

void draw_file_item(struct st_ui_selectlist *selectlist, struct st_box *box, int item_num, int flg_select);
void draw_filelist_view(struct st_filelist_view *filelist_view);
int filelist_view_proc(struct st_filelist_view *filelist_view, struct st_sysevent *event);
void init_filelist_view(struct st_filelist_view *filelist_view);
void updir_filelist_view(struct st_filelist_view *filelist_view);
void downdir_filelist_view(struct st_filelist_view *filelist_view);
char *get_filelist_select_fname(struct st_filelist_view *filelist_view, char *fname, int len);

#endif // FILELIST_VIEW_H
