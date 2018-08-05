/** @file
    @brief	システムコールパラメータ定義

    @date	2017.11.25
    @author	Takashi SHUDO
*/

#ifndef SYSCALL_PARAM_H
#define SYSCALL_PARAM_H

#define	SYSCALL_DISPATCH	0
#define	SYSCALL_TASK_ADD	1
#define	SYSCALL_TASK_EXEC	2
#define	SYSCALL_TASK_EXIT	3
#define	SYSCALL_TASK_PAUSE	4
#define	SYSCALL_TASK_SLEEP	5
#define	SYSCALL_TASK_KILL	6
#define	SYSCALL_TASK_WAKEUP	7

#define	SYSCALL_EVTQUE_INIT	8
#define	SYSCALL_EVTQUE_WAIT	9
#define	SYSCALL_EVTQUE_PUSH	10
#define	SYSCALL_EVTQUE_CLEAR	11
#define	SYSCALL_EVTQUE_CHECK	12
#define	SYSCALL_EVTQUE_WAKEUP	13
#define	SYSCALL_EVTQUE_DISPOSE	14

#define	SYSCALL_MUTEX_INIT	15
#define	SYSCALL_MUTEX_LOCK	16
#define	SYSCALL_MUTEX_UNLOCK	17
#define	SYSCALL_MUTEX_DISPOSE	18

#define SYSCALL_SET_CONSOLE_IN	19
#define SYSCALL_SET_CONSOLE_OUT	20
#define SYSCALL_SET_ERROR_OUT	21

#define	SYSCALL_TIMEOUT_WAKEUP		22	// システムコールではない

#define	SYSCALL_PRINT_TASK_LIST		23
#define	SYSCALL_PRINT_TASK_QUEUE	24
#define	SYSCALL_PRINT_CALLTRACE		25
#define	SYSCALL_GET_TASKS_INFO		26

struct exec_task_param {
	task_func	func;
	char		*name;
	int		priority;
	struct st_tcb	*tcb;
	unsigned int	*stack;
	int		stack_size;
	char		*arg;
	int		ret;
}; ///< タスク追加、起動用システムコールパラメータ

struct evtque_param {
	struct st_event	*evtque;
	unsigned int	timeout;
	const char	*name;
	void		*arg;
	unsigned int	size;
	int		count;
	int		ret;
}; ///< イベント用システムコールパラメータ

struct mutex_param {
	struct st_mutex	*mutex;
	unsigned int	timeout;
	const char	*name;
	int		ret;
}; ///< MUTEX用システムコールパラメータ

struct st_task_info_param {
	struct st_task_info	*ti;
	int		count;
	int		ret;
}; ///< タスク情報取得用システムコールパラメータ

extern const char syscall_name[][16];
extern int last_syscall_type;

void sys_call(int type, void *param);

#endif // SYSCALL_PARAM_H
