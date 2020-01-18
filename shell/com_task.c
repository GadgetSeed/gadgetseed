/** @file
    @brief	タスク関連コマンド

    @date	2012.01.22
    @author	Takashi SHUDO

    @section task_command taskコマンド

    task コマンドには以下のサブコマンドがあります。

    | サブコマンド	| 機能				| 詳細			|
    |:------------------|:------------------------------|:----------------------|
    | top		| @copybrief com_task_top	| @ref com_task_top	|
    | list		| @copybrief com_task_list	| @ref com_task_list	|
    | queue		| @copybrief com_task_queue	| @ref com_task_queue	|
    | kill		| @copybrief com_task_kill	| @ref com_task_kill	|
    | priority		| @copybrief com_task_priority	| @ref com_task_priority|
    | calltrace		| @copybrief com_task_calltrace	| @ref com_task_calltrace	|
*/

#include "sysconfig.h"
#include "shell.h"
#include "str.h"
#include "tkprintf.h"
#include "tprintf.h"
#include "task/task.h"
#include "task/syscall.h"
#include "task/calltrace.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


#ifndef GSC_KERNEL_MESSAGEOUT_MEMORY_SIZE
#define GSC_KERNEL_MESSAGEOUT_MEMORY_SIZE	(1024 * 3)	///< $gsc カーネルメッセージ出力メモリサイズ
#endif
static unsigned char kmess_buff[GSC_KERNEL_MESSAGEOUT_MEMORY_SIZE];

#ifndef GSC_MAX_TASK_INFO_NUM
#define GSC_MAX_TASK_INFO_NUM	10	///< $gsc topコマンドで表示可能な最大タスク数
#endif

static struct st_task_info task_info_list[GSC_MAX_TASK_INFO_NUM];
static unsigned int last_run_time[GSC_MAX_TASK_INFO_NUM];

static int task_top(int argc, uchar *argv[])
{
	int i, count;
	int flg_scroll = 0;
	int flg_start = 0;
	struct st_task_info *ti;
	unsigned long sum_time;

	if(argv[1][0] == 's') {
		flg_scroll = 1;
	}

	count = task_get_tasks_info(task_info_list, GSC_MAX_TASK_INFO_NUM);

	do {
		ti = task_info_list;
		for(i=0; i<count; i++) {
			last_run_time[i] = ti->run_time;
			ti ++;
		}

		if(cwait(1000) >= 0) {
			unsigned char rd;
			if(cgetcnw(&rd) != 0) {
				return 0;
			}
		}

		count = task_get_tasks_info(task_info_list, GSC_MAX_TASK_INFO_NUM);

		sum_time = 0;
		ti = task_info_list;
		for(i=0; i<count; i++) {
			sum_time += (ti->run_time - last_run_time[i]);
			ti ++;
		}

		ti = task_info_list;
		if(flg_scroll == 0) {
			if(flg_start == 0) {
				tprintf("\033[2J"); // 画面クリア
				flg_start = 1;
			}
			tprintf("\033[0;0H");	// カーソルを0,0へ
		}
		tprintf("PID TASK-NAME       PRI RUN-TIME(us)   %%CPU\n");
		for(i=0; i<count; i++) {
			unsigned long run_time = ti->run_time - last_run_time[i];
			unsigned long long percent = 0;
			if(sum_time != 0) {
				percent = ((unsigned long long)run_time * 10000) / sum_time;
			}
			tprintf("%3d %15s %3d   %10ld %3ld.%02ld\n",
				ti->id, ti->name, ti->priority, run_time,
				(unsigned long)percent/100, (unsigned long)percent%100);
			ti ++;
		}
		//tprintf("%10ld\n", sum_time); // DEBUG
	} while(1);

	return 0;
}

/**
   @brief      実行中タスクのCPU使用率を表示する
*/
static const struct st_shell_command com_task_top = {
	.name		= "top",
	.command	= task_top,
	.usage_str	= "top [\"s\"(scroll)]",
	.manual_str	= "Print task load"
};


static int task_list(int argc, uchar *argv[])
{
	unsigned int mess_len;

	set_kernel_message_out_mem(kmess_buff, GSC_KERNEL_MESSAGEOUT_MEMORY_SIZE);
	print_task_list();
	mess_len = set_kernel_message_out_mem(0, 0);
	cputs(kmess_buff, mess_len);

	return 0;
}

/**
   @brief	実行中タスクの状態を表示する
*/
static const struct st_shell_command com_task_list = {
	.name		= "list",
	.command	= task_list,
	.manual_str	= "Print task list"
};


static int task_queue(int argc, uchar *argv[])
{
	unsigned int mess_len;

	set_kernel_message_out_mem(kmess_buff, GSC_KERNEL_MESSAGEOUT_MEMORY_SIZE);
	print_task_queue();
	mess_len = set_kernel_message_out_mem(0, 0);
	cputs(kmess_buff, mess_len);

	return 0;
}

/**
   @brief	各タスクキューに登録されているタスクを表示する
*/
static const struct st_shell_command com_task_queue = {
	.name		= "queue",
	.command	= task_queue,
	.manual_str	= "Print task queue"
};


static int command_task_kill(int argc, uchar *argv[]);

/**
   @brief	任意のタスクを終了させる
*/
static const struct st_shell_command com_task_kill = {
	.name		= "kill",
	.command	= command_task_kill,
	.usage_str	= "<PID>",
	.manual_str	= "Kill task"
};

static int command_task_kill(int argc, uchar *argv[])
{
	if(argc < 2) {
		print_command_usage(&com_task_kill);
		return 0;
	}

	task_kill(dstoi(argv[1]));

	return 0;
}


static int command_task_priority(int argc, uchar *argv[]);

/**
   @brief	タスクの優先度を設定する
*/
static const struct st_shell_command com_task_priority = {
	.name		= "priority",
	.command	= command_task_priority,
	.usage_str	= "<PID> <PRIORITY>",
	.manual_str	= "Set task priority"
};

static int command_task_priority(int argc, uchar *argv[])
{
	if(argc < 3) {
		print_command_usage(&com_task_priority);
		return 0;
	}

	task_priority(dstoi(argv[1]), dstoi(argv[2]));

	return 0;
}



#ifdef GSC_KERNEL_ENABLE_CALLTRACE
static int command_call_trace(int argc, uchar *argv[])
{
	unsigned int mess_len;

	set_kernel_message_out_mem(kmess_buff, GSC_KERNEL_MESSAGEOUT_MEMORY_SIZE);
	print_call_trace();
	mess_len = set_kernel_message_out_mem(0, 0);
	cputs(kmess_buff, mess_len);

	return 0;
}

/**
   @brief	タスクAPIの実行ログを表示する

   このコマンドを有効にするには GSC_KERNEL_ENABLE_CALLTRACE をコンフィグレーションで定義する必要があります
*/
static const struct st_shell_command com_task_calltrace = {
	.name		= "calltrace",
	.command	= command_call_trace,
	.manual_str	= "Print calltrace"
};
#endif


static const struct st_shell_command * const com_task_com_list[] = {
	&com_task_top,
	&com_task_list,
	&com_task_queue,
	&com_task_kill,
	&com_task_priority,
#ifdef GSC_KERNEL_ENABLE_CALLTRACE
	&com_task_calltrace,
#endif
	0
};

const struct st_shell_command com_task = {
	.name		= "task",
	.manual_str	= "Task operation commands",
	.sublist	= com_task_com_list
}; ///< タスク状態取得
