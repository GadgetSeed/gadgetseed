/** @file
    @brief	コマンドシェルタスク

    @date	2015.09.06
    @author	Takashi SHUDO

    @page command_shell コマンドシェル

    GadgetSeedは主にシステム及びアプリケーションのデバッグを目的としたコマンドシェルがあります。

    コマンドシェルを使用するには、以下のコンフィグ項目を有効にして下さい。

    * COMP_ENABLE_SHELL


    デフォルトで使用できるコマンドは以下があります。

    | コマンド名	| 機能				| 詳細			|
    |:------------------|:------------------------------|:----------------------|
    | help		| @copybrief com_help		| @ref help_command 参照|
    | sys		| @copybrief com_sys		| @ref sys_command 参照	|
    | mem		| @copybrief com_mem		| @ref mem_command 参照	|
    | dev		| @copybrief com_dev		| @ref dev_command 参照	|
    | task		| @copybrief com_task		| @ref task_command 参照|
    | graph		| @copybrief com_graphics	| @ref graph_command 参照|
    | i2c		| @copybrief com_i2c		| @ref i2c_command 参照	|
    | file		| @copybrief com_file		| @ref file_command 参照|
    | net		| @copybrief com_net		| @ref net_command 参照	|
    | log		| @copybrief com_log		| @ref log_command 参照	|

    ユーザ独自のコマンドを追加することができます。
    詳細は @ref add_shell_command を参照して下さい。
*/

#include "sysconfig.h"
#include "task/syscall.h"
#include "shell.h"
#include "tprintf.h"
#include "tkprintf.h"
#include "str.h"

#define SIZEOFSHELLST	(1024*8)

extern const struct st_shell_command com_help;
extern const struct st_shell_command com_sys;
extern const struct st_shell_command com_mem;
extern const struct st_shell_command com_dev;
extern const struct st_shell_command com_task;
#ifdef GSC_COMP_ENABLE_GRAPHICS
extern const struct st_shell_command com_graphics;
#endif
#ifdef GSC_DEV_ENABLE_I2C
extern const struct st_shell_command com_i2c;
#endif
#ifdef GSC_COMP_ENABLE_FATFS
extern const struct st_shell_command com_file;
#endif
#ifdef GSC_COMP_ENABLE_TCPIP
extern const struct st_shell_command com_net;
#endif
#ifdef GSC_KERNEL_MESSAGEOUT_LOG
extern const struct st_shell_command com_log;
#endif

struct st_shell gs_shell;
static const uchar prompt[] = ": ";
int shell_com_count;

#ifdef GSC_SHELL_USER_COMMAND_NUM	///< $gsc shellユーザコマンド登録可能数
static const struct st_shell_command * const icom_list[] = {
#else
const struct st_shell_command * const com_list[] = {
#endif
	&com_help,
	&com_sys,
	&com_mem,
	&com_dev,
	&com_task,
#ifdef GSC_COMP_ENABLE_GRAPHICS
	&com_graphics,
#endif
#ifdef GSC_DEV_ENABLE_I2C
	&com_i2c,
#endif
#ifdef GSC_COMP_ENABLE_FATFS
	&com_file,
#endif
#ifdef GSC_COMP_ENABLE_TCPIP
	&com_net,
#endif
#ifdef GSC_KERNEL_MESSAGEOUT_LOG
	&com_log,
#endif
	0
};

#ifdef GSC_SHELL_USER_COMMAND_NUM
#define MAX_COMMAND_NUM	((sizeof(icom_list)/sizeof(struct st_shell_command *))+GSC_SHELL_USER_COMMAND_NUM)
struct st_shell_command *com_list[MAX_COMMAND_NUM];

/**
   @brief	shell にユーザコマンドを追加する

   @param[in]	command	ユーザコマンド

   @return	0:成功、!=0:エラー

   @info	本関数を使用するにはマクロ GSC_SHELL_USER_COMMAND_NUM に、追加したいコマンド数を定義する必要があります
*/
int add_shell_command(struct st_shell_command *command)
{
	int i = 0;

	for(i=0; i<MAX_COMMAND_NUM; i++) {
		if(com_list[i] == 0) {
			com_list[i] = (struct st_shell_command *)command;
			tprintf("Add user shell command \"%s\"\n", command->name);
			return 0;
		}
	}

	SYSERR_PRINT("Cannot add user shell command \"%s\"\n", command->name);

	return -1;
}
#else
#define MAX_COMMAND_NUM	((sizeof(com_list)/sizeof(struct st_shell_command *))-1)
#endif

static int shell_task(char *arg)
{
#ifdef GSC_SHELL_USER_COMMAND_NUM
	int i = 0;

	while(icom_list[i] != 0) {
		com_list[i] = (struct st_shell_command *)icom_list[i];
		i ++;
	}
	shell_com_count = i;
#else
	shell_com_count = MAX_COMMAND_NUM;
#endif
	init_shell(&gs_shell, (struct st_shell_command * const *)com_list, prompt);

	print_prompt(&gs_shell);

	while(1) {
		unsigned char rd;

		if(cgetc(&rd)) {
			task_shell(&gs_shell, rd);
		}
	}

	return 0;
}

static struct st_tcb shell_tcb;
static unsigned int shell_stack[SIZEOFSHELLST/sizeof(unsigned int)] ATTR_STACK;

void startup_shell(void)
{
#ifdef STARTUP_APP
	task_add(shell_task, "shell", TASK_PRIORITY_SHELL, &shell_tcb,
		 shell_stack, SIZEOFSHELLST, 0);
#else
	task_exec(shell_task, "shell", TASK_PRIORITY_SHELL, &shell_tcb,
		  shell_stack, SIZEOFSHELLST, 0);
#endif
}

int exec_command(uchar *str)
{
	uchar cmd[GSC_SHELL_MAX_LINE_COLUMS + 1];

	(void)strncopy(cmd, str, GSC_SHELL_MAX_LINE_COLUMS);

	return exec_shell_command(&gs_shell, cmd);
}
