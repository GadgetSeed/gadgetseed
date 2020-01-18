/** @file
    @brief	タスクコンテキストブロック

    @date	2011.10.19
    @author	Takashi SHUDO
*/

#ifndef TCB_H
#define TCB_H

#include "sysconfig.h"
#include "queue.h"
#include "asm.h"

#define	TASK_NAME_LEN	15	///< 最大タスク名長

#define PSTAT_DORMANT		0	///< タスク 休止状態
#define PSTAT_READY		1	///< タスク 実行可能状態
#define PSTAT_RUN		2	///< タスク 実行状態
#define PSTAT_TIMER_WAIT	3	///< タスク タイマ待ち状態
#define PSTAT_EVENT_WAIT	4	///< タスク イベント待ち状態
#define PSTAT_MUTEX_WAIT	5	///< タスク MUTEXロック解除待ち状態
#define PSTAT_REQUEST_WAIT	6	///< タスク 起床待ち状態

typedef int (* task_func)(void *arg);	///< タスク関数

struct tcb_queue {
	struct st_queue	queue;	///< キュー
	struct st_tcb *tcb;	///< タスクコンテキストブロックポインタ
}; ///< タスクキュー

struct st_tcb {
	struct tcb_queue queue;	///< タスクキュー(st_tcb をst_queueにキャストするために必ず最初に定義する)

	struct tcb_queue timer_list;	///< タイムアウト待ちキュー
	struct tcb_queue task_list;	///< 全タスクキュー

	int id;				///< タスクID
	char name[TASK_NAME_LEN + 1];	///< タスク名
	void *sp;			///< スタックポインタ
	void *stack_addr;		///< スタック先頭アドレス
	unsigned int stack_size;	///< スタックサイズ
	struct st_context ctx;		///< CPUコンテキスト(CPUアーキテクチャ依存)

	task_func main_func;		///< タスク関数
	void *arg;			///< タスク実行時引数文字列

	int priority;			///< タスクプライオリティ
	int next_priority;		///< スイッチ後のタスクプライオリティ
	unsigned int wup_time;		///< スリープタイムアウト時間
	int status;			///< タスク状態(PSTAT_*)

	struct st_device *stdin_dev;	///< タスク標準入力デバイス
	struct st_device *stdout_dev;	///< タスク標準出力デバイス
	struct st_device *error_dev;	///< タスクエラー出力デバイス

	unsigned int meas_time;		///< タスク実行時間計測開始システム時間(msec)
	unsigned int run_time;		///< タスク実行時間(msec)

	struct {
		int type;	///< システムコールタイプ
		void *param;	///< システムコール実行パラメータ
	} syscall;
}; ///< タスクコンテキスト

#endif // TCB_H
