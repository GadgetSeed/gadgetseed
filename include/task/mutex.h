/** @file
    @brief	MUTEX制御

    @date	2011.03.20
    @author	Takashi SHUDO
*/

#ifndef MUTEX_H
#define MUTEX_H

#include "task.h"

struct st_mutex {
	struct st_queue	list;		///< MUTEXのキュー
	struct st_tcb	*lock_ps;	///< ロックしているタスク
	struct st_queue	wait_ps;	///< アンロック待ちタスクキュー
	const char	*name;		///< MUTEX名文字列
}; ///< MUTEX

void mutex_register_ISR(struct st_mutex *mutex, const char *name);
int mutex_unregister_ISR(struct st_mutex *mutex);

void mutex_lock_ISR(void *sp, struct st_mutex *mutex, unsigned int timeout);
void mutex_unlock_ISR(void *sp, struct st_mutex *mutex);

#endif // MUTEX_H
