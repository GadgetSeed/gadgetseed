/** @file
    @brief	サウンドドライバ

    @date	2007.07.15
    @author	Takashi SHUDO

    @info

    下位ドライバに圧電ブザーデバイスドライバが必要

    使用方法

    ioctl(周波数(Hz), 再生時間(x1ms))
*/

#include "sysconfig.h"
#include "device.h"
#include "device/buzzer_ioctl.h"
#include "timer.h"
#include "tkprintf.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


#define TIMER_INTERVAL_TIME	GSC_KERNEL_TIMER_INTERVAL_MSEC

static struct st_device *buzzer_dev;
static int sound_cnt;

static void sound_timer(void *sp, unsigned long long time)
{
	if(sound_cnt > 0) {
		DKPRINTF(0x01, ".");
		sound_cnt -= TIMER_INTERVAL_TIME;
		if(sound_cnt <= 0) {
			ioctl_device(buzzer_dev, IOCMD_BUZZER_OFF, 0, 0);
			DKPRINTF(0x01, "SND OFF : %d\n", (int)get_kernel_time());
		}
	}
}

static int sound_register(struct st_device *dev, char *param)
{
	// param = ブザードライバ名

	buzzer_dev = open_device(param);

	if(buzzer_dev == 0) {
		SYSERR_PRINT("Cannot open device %s.\n", param);
		return -1;
	}

	if(register_timer_func(sound_timer, TIMER_INTERVAL_TIME)) {
		SYSERR_PRINT("Cannot register buzzer timer func.\n");
		return -1;
	}

	sound_cnt = 0;

	return 0;
}

static int sound_unregister(struct st_device *dev)
{
	unregister_timer_func(sound_timer);

	return 0;
}

static int sound_open(struct st_device *dev)
{
	return 0;
}

static int sound_close(struct st_device *dev)
{
	return 0;
}

static int sound_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	/*
	 * com : Hz
	 * arg : ms
	 */

	if(arg != 0) {
		sound_cnt = arg;
		DKPRINTF(0x01, "SND LEN : %d\n", (int)arg);
		DKPRINTF(0x01, "SND ON  : %d\n", (int)get_kernel_time());
		ioctl_device(buzzer_dev, IOCMD_BUZZER_ON, com, 0);
	} else {
		ioctl_device(buzzer_dev, IOCMD_BUZZER_OFF, 0, 0);
	}

	return 0;
}

const struct st_device sound_device = {
	.name		= "sound",
	.explan		= "Buzzer sound control",
	.register_dev	= sound_register,
	.unregister_dev	= sound_unregister,
	.open		= sound_open,
	.close		= sound_close,
	.ioctl		= sound_ioctl,
};
