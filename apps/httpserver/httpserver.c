/** @file
    @brief	HTTPサーバ

    @date	2017.08.16
    @auther	Takashi SHUDO
*/

#include "task/syscall.h"
#include "tprintf.h"
#include "httpserver-netconn.h"

static int httpserver_task(char *arg)
{
	http_server_netconn_init();

	return 0;
}


#define SIZEOFAPPTS	(1024*4)
static struct st_tcb tcb;
static unsigned int stack[SIZEOFAPPTS/sizeof(unsigned int)];

void startup_httpserver(void)
{
	tprintf("Startup HTTP Server\n");

	task_exec(httpserver_task, "httpserver", TASK_PRIORITY_APP_LOW, &tcb,
		  stack, SIZEOFAPPTS, 0);
}
