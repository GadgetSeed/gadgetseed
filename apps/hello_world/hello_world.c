/** @file
    @brief	Hello, world!

    @date	2017.12.26
    @auther	Takashi SHUDO
*/

#include "task/syscall.h"
#include "tprintf.h"

int hello_world_task(char *arg)
{
	tprintf("Hello, world!\n");

	return 0;
}

#define SIZEOFAPPTS	(1024*2)
static struct st_tcb tcb;
static unsigned int stack[SIZEOFAPPTS/sizeof(unsigned int)];

void startup_hello_world(void)
{
	task_exec(hello_world_task, "hello_world", 1, &tcb,
		  stack, SIZEOFAPPTS, 0);
}
