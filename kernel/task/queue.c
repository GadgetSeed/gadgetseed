/** @file
    @brief	キュー操作

    @date	2011.03.17
    @author	Takashi SHUDO
*/

#include "queue.h"

void init_queue(struct st_queue *queue)
{
	queue->next = queue;
	queue->prev = queue;
}

/* NOT API
    @brief	末尾にキューを追加する
*/
void add_queue(struct st_queue *queue, struct st_queue *entry)
{
	entry->prev = queue->prev;
	entry->next = queue;
	queue->prev->next = entry;
	queue->prev = entry;
}

/* NOT API
    @brief	先頭にキューを挿入する
*/
void insert_queue(struct st_queue *queue, struct st_queue *entry)
{
	entry->prev = queue;
	entry->next = queue->next;
	queue->next->prev = entry;
	queue->next = entry;
}

/* NOT API
    @brief	キューを削除する
*/
void del_queue(struct st_queue *entry)
{
	entry->prev->next = entry->next;
	entry->next->prev = entry->prev;

	init_queue(entry);
}

/* NOT API
    @brief	先頭キューを削除し、戻り値として返す
*/
struct st_queue *del_next_queue(struct st_queue *queue)
{
	struct st_queue *entry = 0;

	if(queue->next == queue) {
		return entry;
	}
	entry = queue->next;

	queue->next = entry->next;
	entry->next->prev = queue;

	return entry;
}

/* NOT API
    @brief	先頭キューを末尾に移動する
*/
void rotate_queue(struct st_queue *queue)
{
	if(queue->next != queue) {
		add_queue(queue, del_next_queue(queue));
	}
}

/* NOT API
   @brief	キューが空なら0を返す
*/
int check_queue(struct st_queue *queue)
{
	if(queue->next == queue) {
		return 0;
	}

	return 1;
}
