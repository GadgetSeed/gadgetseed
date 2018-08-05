/** @file
    @brief	コマンドヒストリ

    @date	2007.03.18
    @author	Takashi SHUDO
*/

#ifndef HISTORY_H
#define HISTORY_H

#include "sysconfig.h"
#include "lineedit.h"

#ifndef GSC_SHELL_MAX_COM_HIS
#define	GSC_SHELL_MAX_COM_HIS		8	///< $gsc shellコマンドヒストリの数
#endif

struct st_history {
	uchar	his_buf[GSC_SHELL_MAX_COM_HIS+1][GSC_SHELL_MAX_LINE_COLUMS+1];	///< ヒストリバッファ
	short	pos;		///< 現在表示編集中のヒストリ
	short	num;		///< 記録されているヒストリ数
};	///< コマンドヒストリ

void init_history(struct st_history *his);
void save_history(struct st_history *his, uchar *com);
void foward_history(struct st_history *his, struct st_lineedit *le);
void back_history(struct st_history *his, struct st_lineedit *le);

#endif // HISTORY_H
