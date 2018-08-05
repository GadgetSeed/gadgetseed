/** @file
    @brief	LED テスト

    @date	2017.01.07
    @author	Takashi SHUDO
*/

#include "task/syscall.h"
#include "tprintf.h"
#include "device.h"
#include "tkprintf.h"


static const char led_devname[] = DEF_DEV_NAME_LED;
static struct st_device *led_dev;
static unsigned char led_data[4] = { 0x00, 0x01, 0x02, 0x04 };

int hb_task(char *arg)
{
	led_dev = open_device((char *)led_devname);
	if(led_dev == 0) {
		SYSERR_PRINT("Cannot open \"%s\"\n", led_devname);
		return -1;
	}

	while(1) {
		int i;
		for(i=0; i<4; i++) {
			write_device(led_dev, &led_data[i], 1);
			task_sleep(250);
		}
	}

	return 0;
}

#define SIZEOFAPPTS	(1024*2)
static struct st_tcb tcb;
static unsigned int stack[SIZEOFAPPTS/sizeof(unsigned int)];

void startup_heartbeat(void)
{
	task_exec(hb_task, "heartbeat", 1, &tcb,
		  stack, SIZEOFAPPTS, 0);
}
