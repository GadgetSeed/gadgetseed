/** @file
    @brief	イベント待ちタスクキューの操作

    @date	2011.03.20
    @author	Takashi SHUDO
*/

#ifndef EVENT_H
#define EVENT_H

#include "queue.h"
#include "fifo.h"
#include "task.h"

struct st_event {
	struct st_queue list;		///< イベントキューのキュー
	struct st_queue proc_head;	///< イベント待ちタスクキュー
	struct st_fifo event;		///< イベントデータバッファ
	unsigned int size;		///< 1イベントのサイズ
	const char *name;		///< イベント名文字列
}; ///< イベント

void eventqueue_register_ISR(struct st_event *evtque, const char *name, void *args, unsigned int arg_size, int arg_count);
void eventqueue_unregister_ISR(struct st_event *evtque);

void event_wait_ISR(void *sp, struct st_event *evtque, void *arg, unsigned int timeout);
int event_check_ISR(void *sp, struct st_event *evtque);
void event_clear_ISR(void *sp, struct st_event *evtque);
void event_push_ISR(void *sp, struct st_event *evtque, void *arg);
void event_set_ISR(void *sp, struct st_event *evtque);
void event_wakeup_ISR(void *sp, struct st_event *evtque, void *arg);

#endif // EVENT_H
