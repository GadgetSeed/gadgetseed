/** @file
    @brief	タスクキュー操作

    GadgetSeed のタスクキュー操作

    @date	2011.03.17
    @author	Takashi SHUDO
*/

#ifndef QUEUE_H
#define QUEUE_H

struct st_queue {
	struct st_queue *next;
	struct st_queue *prev;
}; ///< キュー構造

#endif // QUEUE_H
