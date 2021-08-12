/** @file
    @brief	イベント待ちタスクキューの操作

    @date	2011.03.20
    @author	Takashi SHUDO
*/

#include "event_opration.h"
#include "queue_opration.h"

//#define DEBUGKBITS 0x01
#include "dkprintf.h"


struct st_event event_queue_list;	// イベントキューのキュー

void init_eventqueue(void)
{
	DKFPRINTF(0x01, "done\n");

	init_queue(&event_queue_list.list);
}


/**
   @brief	イベントキューを登録する

   @note	システムのキューに登録される

   @param[in]	evtque		イベントキューポインタ
   @param[in]	name		イベントキュー名文字列ポインタ
   @param[in]	args		イベントキュー引数バッファポインタ
   @param[in]	arg_size	1イベント引数のサイズ
   @param[in]	arg_count	キューするイベント数
*/
void eventqueue_register_ISR(struct st_event *evtque, const char *name, void *args, unsigned int arg_size, int arg_count)
{
	DKFPRINTF(0x01, "\"%s\", args = %p, size = %d, count = %d\n",
		  name, args, arg_size, arg_count);

	init_fifo(&evtque->event, args, arg_size * arg_count);

	evtque->name = name;
	evtque->size = arg_size;

	init_queue(&evtque->proc_head);

	add_queue(&event_queue_list.list, (struct st_queue *)evtque);
}

/**
   @brief	イベントキューを登録解除する

   @note	システムのキューより削除される

   @param[in]	evtque		イベントキューポインタ
*/
void eventqueue_unregister_ISR(struct st_event *evtque)
{
	DKFPRINTF(0x01, "evtque = %p\n", evtque);

	/*
	 * 本当は待ちキューがあれば復帰する
	 * 今は、待ちキューは、迷子のキューになってしまう
	 */

	del_queue((struct st_queue *)evtque);
}

/**
 * @brief	タスクをイベント待ちキューに登録する
 */
void _eventqueue_wait(struct st_event *evtque, struct st_tcb *tcb)
{
	DKFPRINTF(0x01, "Wait_Event PID=%d \"%s\"\n", tcb->id, tcb->name);

	add_queue(&evtque->proc_head, (struct st_queue *)tcb);

	return;
}

/**
 * @brief	イベント待ちタスクを返す
 */
struct st_tcb *_eventqueue_wakeup(struct st_event *evtque)
{
	struct st_tcb *wup_tcb = 0;

	DKFPRINTF(0x01, "name = \"%s\"\n", evtque->name);

	// イベント待ちキューがある？
	if(check_queue(&evtque->proc_head) == 0) {
		// 待ちキュー無し
		return (struct st_tcb *)0;
	}

	/*
	  起床するタスク
	  タイムアウト時間の早い順に起床してしまう。
	  待ち登録順の方が良いか？
	*/
	wup_tcb = (struct st_tcb *)del_next_queue(&evtque->proc_head);

	DKPRINTF(0x02, "Event wakeup PID=%d \"%s\"\n", wup_tcb->id, wup_tcb->name);
	DKPRINTF(0x02, "  wup = %u\n", wup_tcb->wup_time);

	return wup_tcb;
}
