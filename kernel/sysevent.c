/** @file
    @brief	システムイベント

    @date	2007.05.05
    @author	Takashi SHUDO

    @page system_event システムイベント

    GadgetSeed のシステムイベントは FIFO 構造のバッファにより管理されます。

    標準的なモデルのユーザアプリケーションは get_event() よりシステム
    イベントを取得し、取得したシステムイベントに応じた処理を実行します。

    キーボード等のデバイスドライバは、システムイベントとして
    set_event() で検知したイベントを登録します。

    割り込みハンドラからシステムイベントを登録する場合、 set_event_interrupt() を使用して下さい。

    システムイベントは最大 @ref GSC_KERNEL_MAX_SYSTEMEVENT_COUNT までバッファすることが出来ます。

    システムイベントには時間的寿命があります。

    登録されたシステムイベントは、取得されるまで
    @ref GSC_KERNEL_SYSTEMEVENT_LIFE_TIME (ms)経過すると寿命切れイベントと
    して取得されます。


    ---
    @section system_event_struct システムイベントデータ

    システムイベントデータ構造体は @ref st_sysevent で定義されています。


    ---
    @section system_event_api システムイベントAPI

    include ファイル : sysevent.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | set_event()		| @copybrief set_event			|
    | push_event_interrupt()	| @copybrief push_event_interrupt	|
    | set_event_interrupt()	| @copybrief set_event_interrupt	|
    | create_event()		| @copybrief create_event		|
    | get_event()		| @copybrief get_event			|


    ---
    @section system_event_define 予約済みシステムイベント定義

    システムイベントデータ構造体のメンバ what は、発生したイベントの種類を示します。
    what は以下のマクロが定義済みです。

    include ファイル : sysevent.h

    | イベント名(マクロ定義)	| イベント説明			|
    |:--------------------------|:------------------------------|
    | @ref EVT_NULL		| @copybrief EVT_NULL		|
    | @ref EVT_KEYDOWN		| @copybrief EVT_KEYDOWN	|
    | @ref EVT_KEYUP		| @copybrief EVT_KEYUP		|
    | @ref EVT_KEYDOWN_REPEAT	| @copybrief EVT_KEYDOWN_REPEAT	|
    | @ref EVT_TOUCHSTART	| @copybrief EVT_TOUCHSTART	|
    | @ref EVT_TOUCHMOVE	| @copybrief EVT_TOUCHMOVE	|
    | @ref EVT_TOUCHEND		| @copybrief EVT_TOUCHEND	|
*/

#include "sysconfig.h"
#include "tkprintf.h"
#include "sysevent.h"
#include "timer.h"
#include "task/syscall.h"
#include "task/task.h"
#include "task/event.h"

//#define DEBUGKBITS 0x01
#include "dkprintf.h"

#ifndef GSC_KERNEL_MAX_SYSTEMEVENT_COUNT
#define	GSC_KERNEL_MAX_SYSTEMEVENT_COUNT	8	///< $gsc システムイベントの最大バッファ数
#endif

#ifndef GSC_KERNEL_SYSTEMEVENT_LIFE_TIME
#define GSC_KERNEL_SYSTEMEVENT_LIFE_TIME	3000	///< $gsc システムイベントの寿命(msec)
#endif

#ifndef GSC_KEY_REPEAT_START_TIME
#define GSC_KEY_REPEAT_START_TIME	500	///< $gsc キーリピート開始までの時間(msec)
#endif

#ifndef GSC_KEY_REPEAT_INTERVAL_TIME
#define GSC_KEY_REPEAT_INTERVAL_TIME	50	///< $gsc キーリピート間隔時間(msec)
#endif

static struct st_event evtque;

static struct st_sysevent event_buf[GSC_KERNEL_MAX_SYSTEMEVENT_COUNT+1];	///< イベントバッファ
static unsigned short last_key;	///< 最後に押されたキー
static unsigned int key_repeat_start_time;	///< キーリピート開始までの時間
static unsigned int key_repeat_start_count;	///< キーリピート開始カウンタ
static unsigned int key_repeat_flg;		///< キーリピート開始フラグ
static unsigned int key_repeat_int_time;	///< キーリピート間隔時間
static unsigned long long key_repeat_int_count;	///< キーリピート時間カウンタ

/**
   @brief	全てのシステムイベントバッファのイベントを初期化(削除)する
*/
void init_event(void)
{
	eventqueue_register_ISR(&evtque, "sysevent", (void *)event_buf, (unsigned int)sizeof(struct st_sysevent), GSC_KERNEL_MAX_SYSTEMEVENT_COUNT+1);

	last_key = 0;
	key_repeat_start_time = GSC_KEY_REPEAT_START_TIME;
	key_repeat_int_time = GSC_KEY_REPEAT_INTERVAL_TIME;

	key_repeat_flg = 0;

	key_repeat_start_count = 0;
	key_repeat_int_count = 0;
}

/**
   @brief	システムイベントを登録する

   @param event 登録するシステムイベント

   @return !=0:エラー
*/
int set_event(struct st_sysevent *event)
{
	int rtn = 0;

	DKFPRINTF(0x01, "what = %d arg = %d\n", event->what, event->arg);

	event->when = get_kernel_time();
	event_wakeup(&evtque, event);

	return rtn;
}

/**
   @brief	割り込みハンドラからシステムイベントを登録する

   @param sp	スタックポインタ
   @param event 登録するシステムイベント
*/
void push_event_interrupt(void *sp, struct st_sysevent *event)
{
	DKFPRINTF(0x01, "what = %d arg = %d\n", event->what, event->arg);

	event->when = get_kernel_time();
	event_push_ISR(sp, &evtque, event);
}

/**
   @brief	割り込みハンドラからシステムイベント待ちタスクを起床する

   @param sp	スタックポインタ
*/
void set_event_interrupt(void *sp)
{
	event_set_ISR(sp, &evtque);
}

/**
   @brief	システムイベントを作成し、追加する

   @param what	システムイベントの種類
   @param arg	システムイベントの引数

   @return	 =0:成功
		!=0:エラー
*/
int create_event(unsigned short what, unsigned short arg, void *private_data)
{
	struct st_sysevent event;

	event.what = what;
	event.arg = arg;
	event.private_data = private_data;

	return set_event(&event);
}

static int do_keyrepeat(struct st_sysevent *event, int flg_add)
{
	if(key_repeat_flg) {
		// キーリピート間隔
		unsigned long long nt = get_kernel_time();
		if(nt >= (key_repeat_int_count + key_repeat_int_time)) {
			if(flg_add != 0) {
				struct st_sysevent event;
				key_repeat_int_count = nt;
				event.what = EVT_KEYDOWN_REPEAT;
				event.arg = last_key;
				set_event(&event);
			} else {
				key_repeat_int_count = nt;
				event->what = EVT_KEYDOWN_REPEAT;
				event->arg = last_key;
			}
			return 1;	// イベント有り
		}
	} else {
		if(last_key != 0) {
			// キーリピートイベント追加処理
			unsigned long long nt = get_kernel_time();
			if(nt >= (key_repeat_start_count + key_repeat_start_time)) {
				// キーリピート開始
				key_repeat_int_count = nt;
				key_repeat_flg = 1;
			}
		}
	}

	return 0;
}

/**
   @brief	システムイベントを待つ

   @param event		取得したシステムイベント
   @param timeout	システムイベント取得タイムアウト時間
   
   @note		キーリピートを取得するには timeout は50以下に設定する必要がある

   @return	 0:取得イベントなし
		 1:イベント取得成功
		 -1:寿命切れイベントの取得
*/
int get_event(struct st_sysevent *event, unsigned int timeout)
{
	int rtn = 0;

	rtn = event_wait(&evtque, event, timeout);

	if(rtn >= 0) {
		if((event->when + GSC_KERNEL_SYSTEMEVENT_LIFE_TIME) > get_kernel_time()) {
			DKPRINTF(0x01, "get_event what = %d arg = %d\n", event->what, event->arg);
			if(event->what == EVT_KEYDOWN) {
				last_key = event->arg;
				key_repeat_start_count = get_kernel_time();
			} else if(event->what == EVT_KEYUP) {
				last_key = 0;
				key_repeat_flg = 0;
			} else {
				do_keyrepeat(event, 1);
			}
			return 1;	// イベント有り
		} else {
			return -1;	// 寿命切れイベント
		}
	} else {
		if(do_keyrepeat(event, 0) != 0) {
			return 1;
		} else {
			goto no_event;
		}
	}

no_event:
	event->what = EVT_NULL;
	event->when = get_kernel_time();

	return 0;	// イベント無し
}
