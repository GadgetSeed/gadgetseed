/** @file
    @brief	キュー操作

    @date	2017.11.25
    @author	Takashi SHUDO
*/

#ifndef QUEUE_OPRATION_H
#define QUEUE_OPRATION_H

#include "queue.h"

void init_queue(struct st_queue *queue);
void add_queue(struct st_queue *queue, struct st_queue *tcb);
void insert_queue(struct st_queue *queue, struct st_queue *tcb);
void del_queue(struct st_queue *queue);
struct st_queue *del_next_queue(struct st_queue *queue);
void rotate_queue(struct st_queue *queue);
int check_queue(struct st_queue *queue);

#endif // QUEUE_OPRATION_H
