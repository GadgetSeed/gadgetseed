/** @file
    @brief	lwIP sys_archインターフェース

    @date	2017.08.16
    @date	2013.01.02
    @author	Takashi SHUDO
*/

#include "lwip/err.h"
#include "lwip/init.h"
#include "lwip/sys.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "timer.h"
#include "tkprintf.h"
#include "tprintf.h"
#include "task/syscall.h"
#include "task/event.h"
#include "task/mutex.h"
#include "lwipopts.h"

//#define DEBUGKBITS 0x04
#include "dkprintf.h"


/*
 * MUTEX
 */

#define MAX_NET_MUTEX	6
#define RESOURCE_NAME_LEN	12

struct sys_mutex {
	struct st_mutex mutex;
	char name[RESOURCE_NAME_LEN+1];
	int use;
};

static struct sys_mutex net_sys_mutex[MAX_NET_MUTEX];
static struct st_mutex lwip_mutex_mtx;

err_t sys_mutex_new(sys_mutex_t *mutex)
{
	int i;
	err_t errcode = ERR_MEM;

	DKFPRINTF(0x01, "(%p)\n", mutex);

	mutex_lock(&lwip_mutex_mtx, 0);

	for(i=0; i<MAX_NET_MUTEX; i++) {
		if(net_sys_mutex[i].use == 0) {
			net_sys_mutex[i].use = 1;
			tsnprintf(net_sys_mutex[i].name, RESOURCE_NAME_LEN, "lwip_mtx%d", i);
			mutex_register(&net_sys_mutex[i].mutex,
				       net_sys_mutex[i].name);
			*mutex = &net_sys_mutex[i];
			DKFPRINTF(0x01, "\"%s\"(%p)\n",
				  (*mutex)->mutex.name, mutex);
			errcode = ERR_OK;
			break;
		}
	}

	mutex_unlock(&lwip_mutex_mtx);

	if(i == MAX_NET_MUTEX) {
		SYSERR_PRINT("No left mutex\n");
	}

	return errcode;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
	DKFPRINTF(0x01, "\"%s\"(%p)\n", (*mutex)->mutex.name, mutex);

	mutex_lock(&((*mutex)->mutex), 0);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
	DKFPRINTF(0x01, "\"%s\"(%p)\n", (*mutex)->mutex.name, mutex);

	mutex_unlock(&((*mutex)->mutex));
}

void sys_mutex_free(sys_mutex_t *mutex)
{
	DKFPRINTF(0x01, "\"%s\"(%p)\n", (*mutex)->mutex.name, mutex);

	mutex_unregister(&(*mutex)->mutex);
	(*mutex)->use = 0;
}


/*
 * SEMAPHORE
 */

#define MAX_NET_SEM	4
#define SEM_QUEUE_COUNT	1

struct sys_sem {
	struct st_event event;
	char name[RESOURCE_NAME_LEN];
	int use;
	int valid;
	char msg[SEM_QUEUE_COUNT + 1];	// キュー出来るMBOXは最大1
};

static struct sys_sem net_sys_sem[MAX_NET_SEM];
static struct st_mutex lwip_semapho_mtx;

err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
	int i;
	err_t errcode = ERR_MEM;

	DKFPRINTF(0x02, "(%p) %d\n", sem, count);

	mutex_lock(&lwip_semapho_mtx, 0);

	if(count != 0) {
		SYSERR_PRINT("count = %d\n", count);
	}

	for(i=0; i<MAX_NET_SEM; i++) {
		if(net_sys_sem[i].use == 0) {
			net_sys_sem[i].use = 1;
			net_sys_sem[i].valid = 1;
			tsnprintf(net_sys_sem[i].name, RESOURCE_NAME_LEN, "lwip_sem%d", i);
			eventqueue_register(&net_sys_sem[i].event,
					    net_sys_sem[i].name, &(net_sys_sem[i].msg), 1, SEM_QUEUE_COUNT + 1);
			*sem = &net_sys_sem[i];
			DKFPRINTF(0x02, "\"%s\"(%08lx)\n",
				  (*sem)->event.name, sem);
			errcode = ERR_OK;
			break;
		}
	}

	mutex_unlock(&lwip_semapho_mtx);

	if(i == MAX_NET_SEM) {
		SYSERR_PRINT("No left semaphore\n");
	}

	return errcode;
}

void sys_sem_signal(sys_sem_t *sem)
{
	DKFPRINTF(0x02, "\"%s\"(%p)\n", (*sem)->event.name, sem);

	event_wakeup(&((*sem)->event), 0);
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeout)
{
	char tmp;
	int tout = 0;

	DKFPRINTF(0x02, "\"%s\"(%p) timeout=%u\n", (*sem)->event.name, sem, timeout);

	tout = event_wait(&((*sem)->event), &tmp, timeout);
	DKFPRINTF(0x04, "\"%s\"(%p) tout=%d\n", (*sem)->event.name, sem, tout);

	if(tout < 0) {
		tout = SYS_ARCH_TIMEOUT;
	}

	return tout;
}

void sys_sem_free(sys_sem_t *sem)
{
	DKFPRINTF(0x02, "\"%s\"(%p)\n", (*sem)->event.name, sem);

	eventqueue_unregister(&(*sem)->event);
	(*sem)->valid = 0;
	(*sem)->use = 0;
}

/*
  セマフォが有効な場合は1を返し、無効な場合は0を返します。インタを使用
  する場合、簡単な方法は、ポインタに！= NULLをチェックすることです。OS
  構造を直接使用する場合、これを実装する方がより複雑になる可能性があり
  ます。これは定義であってもよく、この場合、関数はプロトタイプ化されて
  いない。
*/
int sys_sem_valid(sys_sem_t *sem)
{
	DKFPRINTF(0x02, "\"%s\"(%p)\n", (*sem)->event.name, sem);

	if(((sem) != NULL) && (*(sem) != NULL)) {
		if((*sem)->valid != 0) {
			return 1;
		}
	}

	return 0;
}

/*
  sys_sem_valid()が0を返すようにセマフォを無効にします。

  注意：これは、セマフォの割り当てを解除することを意味するものではあり
  ません。

  sys_sem_free()は、この関数を呼び出す前に常に呼び出されます！これは定
  義であってもよく、この場合、関数はプロトタイプ化されていない。
*/
void sys_sem_set_invalid(sys_sem_t *sem)
{
	DKFPRINTF(0x02, "\"%s\"(%p)\n", (*sem)->event.name, sem);

	if(((sem) != NULL) && (*(sem) != NULL)) {
		(*sem)->valid = 0;
	}

	return;
}


/*
 * MBOX
 */

#define MAX_NET_MBOX 8
#define MBOX_QUEUE_COUNT	1

struct sys_mbox {
	struct st_event event;
	char name[RESOURCE_NAME_LEN];
	int use;
	int valid;
	void *msg[MBOX_QUEUE_COUNT + 1];	// キュー出来るMBOXは最大MBOX_QUEUE_COUNT
};

static struct sys_mbox net_sys_mbox[MAX_NET_MBOX];
static struct st_mutex lwip_mbox_mtx;

/*
  最大の「サイズ」要素のために空のメールボックスを作成します。 メール
  ボックスに格納される要素はポインタです。lwipopts.hにマクロ
  "_MBOX_SIZE"を定義するか、実装でこのパラメータを無視してデフォルトの
  サイズを使用する必要があります。メールボックスが作成されている場合は、
  ERR_OKが返されます。 他のエラーを返すと何がうまくいかなかったかのヒ
  ントが得られますが、アサーション以外は実際のエラー処理は実装されてい
  ません。
*/
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	int i;
	err_t errcode = ERR_MEM;

	DKFPRINTF(0x04, "(%p) %d\n", mbox, size);

	mutex_lock(&lwip_mbox_mtx, 0);

	for(i=0; i<MAX_NET_MBOX; i++) {
		if(net_sys_mbox[i].use == 0) {
			tsnprintf(net_sys_mbox[i].name, RESOURCE_NAME_LEN, "lwip_mbox%d", i);
			net_sys_mbox[i].use = 1;
			net_sys_mbox[i].valid = 1;
			eventqueue_register(&net_sys_mbox[i].event,
					    net_sys_mbox[i].name,
					    net_sys_mbox[i].msg, sizeof(void *), MBOX_QUEUE_COUNT + 1);
			*mbox = &net_sys_mbox[i];
			DKFPRINTF(0x04, "\"%s\"(%d)\n", (*mbox)->event.name, size);
			errcode = ERR_OK;
			break;
		}
	}

	mutex_unlock(&lwip_mbox_mtx);

	if(i == MAX_NET_MBOX) {
		SYSERR_PRINT("No left mbox\n");
	}

	return errcode;
}

/*
  "msg"をメールボックスに送信します。 この関数は "msg"が実際に投稿され
  るまでブロックする必要があります。
*/
void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
	void *arg;

	DKFPRINTF(0x04, "\"%s\"(%p) %p\n", (*mbox)->event.name, mbox, msg);

	arg = msg;
	event_wakeup(&((*mbox)->event), &arg);
}

/*
  "msg"をメールボックスに投稿してみてください。 これが満杯の場合は
  ERR_MEMを返し、それ以外の場合は「msg」が掲示されている場合はERR_OKを
  返します。
*/
err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
	void *arg;

	DKFPRINTF(0x04, "\"%s\"(%p) %p\n", (*mbox)->event.name, mbox, msg);

	if(event_check(&((*mbox)->event)) >= MBOX_QUEUE_COUNT) {
		DKFPRINTF(0x14, "%s full\n", (*mbox)->event.name);
		return ERR_MEM;
	}

	arg = msg;
	event_wakeup(&((*mbox)->event), &arg);

	return ERR_OK;
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
	return sys_mbox_trypost(mbox, msg);
}

/*
  メッセージがメールボックスに到着するまでスレッドをブロックしますが、
  スレッドを「タイムアウト」ミリ秒より長くブロックしません
  （sys_arch_sem_wait()関数に似ています）。 "timeout"が0の場合は、メッ
  セージが到着するまでスレッドをブロックする必要があります。 "msg"引数
  は、関数によって設定される結果パラメータです（つまり、 "* msg = ptr"
  を実行することによって）。 メッセージが破棄されるべきであることを示
  すために、「msg」パラメータはNULLとすることができる。

  戻り値は、sys_arch_sem_wait()関数と同じです。待機時間（ミリ秒）また
  はタイムアウトがあった場合のSYS_ARCH_TIMEOUT。

  同様の名前の関数sys_mbox_fetch()は、lwIPによって実装されていることに
  注意してください。
*/
u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout)
{
	int tout = -1;

	DKFPRINTF(0x04, "\"%s\"(%p) %p timeout=%u\n", (*mbox)->event.name, mbox, msg, timeout);

	if(msg == 0) {
		SYSERR_PRINT("msg = 0\n");
	}

	tout = event_wait(&((*mbox)->event), msg, timeout);
	DKFPRINTF(0x04, "\"%s\"(%p) tout=%d\n", (*mbox)->event.name, mbox, tout);

	if(tout < 0) {
		tout = SYS_ARCH_TIMEOUT;
	}

	DKFPRINTF(0x04, "\"%s\" rtn=%d\n", (*mbox)->event.name, tout);
	return tout;
}

/*
  これはsys_arch_mbox_fetchに似ていますが、メッセージがメールボックス
  に存在しない場合は、すぐにSYS_MBOX_EMPTYというコードで返されます。
  成功すると0が返されます。

  効率的な実装を可能にするために、これは通常の関数の代わりにsys_arch.h
  の関数型マクロとして定義することができます。 たとえば、素朴な実装は
  次のようになります。

  #define sys_arch_mbox_tryfetch(mbox, msg) \
  sys_arch_mbox_fetch(mbox、msg, 1)

  これは不必要な遅延を招く。
*/
u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
	int rtn = ERR_OK;

	DKFPRINTF(0x04, "\"%s\"(%p) %p\n", (*mbox)->event.name, mbox, msg);

	if(event_check(&((*mbox)->event)) == 0) {
		rtn = SYS_MBOX_EMPTY;
	} else {
		event_wait(&((*mbox)->event), msg, 0);
	}

	DKFPRINTF(0x04, "%s rtn = d\n", (*mbox)->name, rtn);
	return rtn;
}

/*
  メールボックスの割り当てを解除します。 メールボックスの割り当てが解
  除されたときにメールボックスにメッセージがまだ残っている場合は、lwIP
  のプログラミングエラーを示すもので、開発者に通知する必要があります。
*/
void sys_mbox_free(sys_mbox_t *mbox)
{
	DKFPRINTF(0x04, "\"%s\"(%p)\n", (*mbox)->event.name, mbox);

	eventqueue_unregister(&((*mbox)->event));
	(*mbox)->valid = 0;
	(*mbox)->use = 0;
}

/*
  メールボックスが有効な場合は1を返し、無効な場合は0を返します。ポイン
  タを使用する場合、簡単な方法は、ポインタに！= NULLをチェックすること
  です。OS構造を直接使用する場合、これを実装する方がより複雑になる可能
  性があります。これは定義であってもよく、この場合、関数はプロトタイプ
  化されていない。
*/
int sys_mbox_valid(sys_mbox_t *mbox)
{
	int valid = 0;

	DKFPRINTF(0x04, "(%p)\n", mbox);

	if(((mbox) != NULL) && (*(mbox) != NULL)) {
		if((*mbox)->valid != 0) {
			valid = 1;
		}
	}

	DKFPRINTF(0x04, "%s valid = %d\n", (*mbox)->name, valid);
	return valid;
}

/*
  sys_mbox_valid()が0を返すようにメールボックスを無効にします。

  注意：これは、メールボックスの割り当てを解除することを意味するもので
  はありません。

  sys_mbox_free()は、この関数を呼び出す前に常に呼び出されます！これは
  定義であってもよく、この場合、関数はプロトタイプ化されていない。
*/
void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
	DKFPRINTF(0x04, "(%p)\n", mbox);

	if(((mbox) != NULL) && (*(mbox) != NULL)) {
		(*mbox)->valid = 0;
	}

	return;
}

/*
  Protect/Unprotect
*/
static struct st_mutex lwip_sys_mtx;

sys_prot_t sys_arch_protect(void)
{
	DKFPRINTF(0x08, "\n");

//	disable_interrupt();
	mutex_lock(&lwip_sys_mtx, 0);

//	return 0;
	return (sys_prot_t)1;
}

void sys_arch_unprotect(sys_prot_t pval)
{
	DKFPRINTF(0x08, "\n");

//	enable_interrupt();
	mutex_unlock(&lwip_sys_mtx);
}

#define MAX_THREAD_NUM	4
#define SIZEOFTS	(1024*8)

static int lwip_thread_num = 0;
static struct st_tcb tcb[MAX_THREAD_NUM];
static unsigned int lwip_stack[MAX_THREAD_NUM][SIZEOFTS/sizeof(unsigned int)] ATTR_STACK;
static struct st_mutex lwip_thread_mtx;
static lwip_thread_fn lwip_thread_func[MAX_THREAD_NUM];

static int lwip_thread(void *arg)
{
	lwip_thread_func[lwip_thread_num](arg);

	return 0;
}

sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread,
			    void *arg, int stacksize, int prio)
{
	DKFPRINTF(0x01, "arg=%p stacksize=%d prio=%d\n", arg, stacksize, prio);

	mutex_lock(&lwip_thread_mtx, 0);

	if(lwip_thread_num >= MAX_THREAD_NUM) {
		SYSERR_PRINT("lwip_thread_num(%d) > MAX_THREAD_NUM(%d)\n",
			     lwip_thread_num, MAX_THREAD_NUM);
		goto end;
	}

	lwip_thread_func[lwip_thread_num] = thread;
	if(stacksize == 0) {
		stacksize = SIZEOFTS;
	}

	task_exec(lwip_thread,
		  (char *)name, prio,
		  &tcb[lwip_thread_num],
		  lwip_stack[lwip_thread_num],
		  stacksize, arg);

	lwip_thread_num ++;

end:
	mutex_unlock(&lwip_thread_mtx);

	return 0;
}

void sys_init(void)
{
	int i;

	DKFPRINTF(0x10, "\n");

	mutex_register(&lwip_mutex_mtx, "lwip_mutex");
	mutex_register(&lwip_semapho_mtx, "lwip_semapho");
	mutex_register(&lwip_mbox_mtx, "lwip_mbox");
	mutex_register(&lwip_thread_mtx, "lwip_thread");
	mutex_register(&lwip_sys_mtx, "lwip_sys");

	for(i=0; i<MAX_NET_MUTEX; i++) {
		net_sys_mutex[i].use = 0;
	}

	for(i=0; i<MAX_NET_SEM; i++) {
		net_sys_sem[i].use = 0;
	}

	for(i=0; i<MAX_NET_MBOX; i++) {
		net_sys_mbox[i].use = 0;
	}
}

u32_t sys_now(void)
{
	uint32_t ntime;

	DKFPRINTF(0x20, "");

	ntime = get_kernel_time();

	DKPRINTF(0x20, " %u \n", ntime);

	return ntime;
}

int lwip_errno = 0;

void lwip_set_errno(int err)
{
	lwip_errno = err;
}
