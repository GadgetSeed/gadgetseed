/** @file
    @brief	コマンドヒストリ

    @date	2007.03.18
    @author	Takashi SHUDO
*/

#include "history.h"
#include "str.h"

/*
  ヒストリーのクリア
*/
void init_history(struct st_history *his)
{
	int j;

	for(j=0; j<(GSC_SHELL_MAX_COM_HIS+1); j++) {
		(void)memoryset(his->his_buf[j], 0, GSC_SHELL_MAX_LINE_COLUMS+1);
	}
	
	his->pos = 0;
	his->num = 0;
}

/*
  ヒストリーに記録
*/
void save_history(struct st_history *his, uchar *com)
{
	int i;

	if(strleng(com) != 0) {
		(void)strncopy(his->his_buf[0], com, GSC_SHELL_MAX_LINE_COLUMS);
	
		for(i=GSC_SHELL_MAX_COM_HIS; i>0; i--) {
			(void)strncopy(his->his_buf[i], his->his_buf[i-1],
				 GSC_SHELL_MAX_LINE_COLUMS);
		}
		if(his->num <= GSC_SHELL_MAX_COM_HIS) {
			his->num ++;
		}
		his->pos = 0;
	}
}

/*
  次のヒストリーを表示
*/
void foward_history(struct st_history *his, struct st_lineedit *le)
{
	if(his->pos > 0) {
		his->pos --;
		set_str_lineedit(le, his->his_buf[his->pos]);
	}
}

/*
  前のヒストリーを表示
*/
void back_history(struct st_history *his, struct st_lineedit *le)
{
	if((his->pos < GSC_SHELL_MAX_COM_HIS) &&
	   (his->pos < his->num)) {
		if(his->pos == 0) {
			(void)strncopy(his->his_buf[0], le->buf, GSC_SHELL_MAX_LINE_COLUMS);
		}
		
		his->pos ++;

		set_str_lineedit(le, his->his_buf[his->pos]);
	}
}
