/** @file
    @brief	MUTEX操作

    @date	2011.03.20
    @author	Takashi SHUDO
*/

#include "tkprintf.h"
#include "timer.h"
#include "mutex.h"
#include "calltrace.h"

#include "queue_opration.h"
#include "mutex_opration.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


struct st_mutex mutex_queue_list;		///< MUTEXキュー

void init_mutex(void)
{
	DKFPRINTF(0x01, "done\n");

	init_queue(&mutex_queue_list.list);
}

/**
   @brief	MUTEXを登録する

   システムのキューに登録される

   @param[in]	mutex	MUTEX
   @param[in]	name	MUTEX名
*/
void mutex_register_ISR(struct st_mutex *mutex, const char *name)
{
	DKFPRINTF(0x01, "mutex = %p, name = %s\n", mutex, name);

	mutex->lock_ps = 0;
	mutex->name = name;

	init_queue(&mutex->wait_ps);

	add_queue(&mutex_queue_list.list, (struct st_queue *)mutex);
}

/**
   @brief	MUTEXを登録解除する

   システムのキューより削除される

   @param[in]	mutex	MUTEX
*/
int mutex_unregister_ISR(struct st_mutex *mutex)
{
	DKFPRINTF(0x01, "mutex = %p\n", mutex);

	/*
	  本当は待ちキューがあれば破棄失敗
	*/

	del_queue((struct st_queue *)mutex);

	return 0;
}

/* NOT API
   @brief	MUTEXをロックする

   既にロックされていた場合、自タスクを実行キューから外し
   mutex_wait() をコールし MUTEX 待ちキューに登録する

   @param[in]	mutex	MUTEX
   @param[in]	tcb	MUTEXをロックするタスクのTCB

   @return

   0: ロック成功
   1: ロック解除待ち
*/
int _mutex_lock(struct st_mutex *mutex, struct st_tcb *tcb)
{
	int rtc = 0;
	DKFPRINTF(0x02, "%s %d\n", mutex->name, tcb->id);

	if(mutex->lock_ps == 0) {
		// ロックされていないMUTEX
		mutex->lock_ps = tcb;
		DKFPRINTF(0x01, "(%p:%s) Lock PID=%d \"%s\"\n",
			  mutex, mutex->name, tcb->id, tcb->name);
		rtc = 0;
	} else if(mutex->lock_ps == tcb) {
		// 自タスクで既にロック済みだったMUTEX
#ifdef CHECK_MULTI_LOCK_MUTEX
#ifdef __x86_64__
		SYSERR_PRINT("(%p:%s) Already Locked PID=%d \"%s\"\n",
			     mutex, mutex->name, tcb->id, tcb->name);
#else
		unsigned long long stime = get_system_utime();
		SYSERR_PRINT("(%p:%s) Already Locked PID=%d \"%s\" Lock PID=%d \"%s\" time = %08ld.%03ld\n",
			     mutex, mutex->name,
			     mutex->lock_ps->id, mutex->lock_ps->name,
			     tcb->id, tcb->name,
			     stime / 1000, stime % 1000);
#endif
		print_queues();
		print_calltrace();
#endif
		rtc = -1;
	} else {
		// 既にロック済みのMUTEX
		DKFPRINTF(0x01, "(%p:%s) Wait Unlock PID=%d \"%s\"\n",
			  mutex, mutex->name, tcb->id, tcb->name);
		rtc = 1;
	}

	//print_queues(); // DEBUG

	return rtc;
}

/* NOT API
    @brief	MUTEXを待つ
*/
void _mutex_wait(struct st_mutex *mutex, struct st_tcb *tcb)
{
	add_queue(&mutex->wait_ps, (struct st_queue *)tcb);
}

/* NOT API
    @brief	MUTEXをアンロックする

    自身がロックしたMUTEXでなければ失敗
    待ちタスクがあった場合、次の待ちタスクが実行される

    @return

    0: ロックされていない、または tcb がロックした MUTEX ではない
    またはアンロックしたけど待ちタスクがいない

    !=0: 待ちタスク tcb
*/
struct st_tcb * _mutex_unlock(struct st_mutex *mutex, struct st_tcb *tcb)
{
	struct st_tcb *wup_tcb = 0;

	DKFPRINTF(0x02, "MUTEX UNLOCK %s %d\n", mutex->name, tcb->id);

	if(mutex->lock_ps == 0) {
		// ロックされていないMUTEX
#ifdef CHECK_MULTI_LOCK_MUTEX
		SYSERR_PRINT("(%p:%s) Not locked PID=%d \"%s\"\n",
			 mutex, mutex->name, tcb->id, tcb->name);
		print_queues();
		print_calltrace();
#endif
		return (struct st_tcb *)0;
	} else if(mutex->lock_ps != tcb) {
		// 他のタスクにロックされていたMUTEX
		SYSERR_PRINT("(%p:%s) Cannot Unlock PID=%d \"%s\"\n",
			     mutex, mutex->name,  tcb->id, tcb->name);
		SYSERR_PRINT("MUTEX Lock PID=%d \"%s\"\n",
			     mutex->lock_ps->id, mutex->lock_ps->name);
		wup_tcb = (struct st_tcb *)0;
	} else {
		// 自タスクでロックしていたMUTEX
		struct st_tcb *l_ps = mutex->lock_ps;
		DKFPRINTF(0x01, "(%p:%s) Unlock PID=%d \"%s\"\n",
			  mutex, mutex->name, tcb->id, tcb->name);
		mutex->lock_ps = 0;
		DKFPRINTF(0x02, "MUTEX UNLOCK SUCCESS %s %d\n", mutex->name, tcb->id);

		if(check_queue(&mutex->wait_ps) == 0) {
			// ロック待ちのタスクが無かった
			DKFPRINTF(0x01, "(%p:%s) No Wait Process PID=%d \"%s\"\n",
				 mutex, mutex->name, tcb->id, tcb->name);
			wup_tcb = (struct st_tcb *)0;
			goto end;
		}

		// ロック待ちのタスクが有る
		// 次のロック待ちタスクを待ちキューから取り出し
		wup_tcb = (struct st_tcb *)del_next_queue(&mutex->wait_ps);
		if(l_ps == wup_tcb) {
			SYSERR_PRINT("Next queue is some queue ? %d %d\n", l_ps->id, wup_tcb->id);
		}
		// 今ロックしているタスクに設定
		mutex->lock_ps = wup_tcb;
		DKPRINTF(0x02, "MUTEX UNLOCK next process %s %d\n", mutex->name, mutex->lock_ps->id);
		DKPRINTF(0x01, "(%p:%s) Unlocked Wakeup PID=%d \"%s\"\n",
			 mutex, mutex->name, wup_tcb->id, wup_tcb->name);
	}

end:
	//print_queues(); // DEBUG
	// いまロックしているタスクを返す
	return wup_tcb;
}
