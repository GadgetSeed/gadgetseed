/** @file
    @brief	仮想 RTC ドライバ

    @date	2009.10.28
    @author	Takashi SHUDO
*/

#include <time.h>
#include <sys/time.h>

#include "sysconfig.h"
#include "device.h"
#include "device/rtc_ioctl.h"
#include "datetime.h"
#include "tkprintf.h"

static int rtc_register(struct st_device *dev, char *param)
{
	return 0;
}

static int rtc_unregister(struct st_device *dev)
{
	return 0;
}

static int rtc_open(struct st_device *dev)
{
	return 0;
}

static int rtc_close(struct st_device *dev)
{
	return 0;
}

static int rtc_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	struct st_datetime *tp = (struct st_datetime *)param;

	switch(com) {
	case IOCMD_RTC_SET:
		return 0;
		break;

	case IOCMD_RTC_GET:
		{
			struct tm *tmptr = 0;
			struct timespec nowtime;

			clock_gettime(CLOCK_REALTIME, &nowtime);
			//tmptr = (struct tm *)localtime(&nowtime.tv_sec);
			tmptr = (struct tm *)gmtime(&nowtime.tv_sec);

			tp->year	= 1900 + tmptr->tm_year;
			tp->month	= tmptr->tm_mon + 1;
			tp->dayofweek	= tmptr->tm_wday;
			tp->day		= tmptr->tm_mday;
			tp->hour	= tmptr->tm_hour;
			tp->min		= tmptr->tm_min;
			tp->sec		= tmptr->tm_sec;
#ifdef GSC_RTC_RESOLUTION_IS_NOT_SEC
			tp->msec	= nowtime.tv_nsec/(1000 * 1000);
#else
			tp->msec	= 0;
#endif
		}
		return 0;
		break;

	default:
		SYSERR_PRINT("Unknown ioctl(%08X)\n", com);
		return -1;
	}

	return -1;
}

const struct st_device rtc_device = {
	.name		= DEF_DEV_NAME_RTC,
	.explan		= "EMU Real Time Clock",
	.register_dev	= rtc_register,
	.unregister_dev	= rtc_unregister,
	.open		= rtc_open,
	.close		= rtc_close,
	.ioctl		= rtc_ioctl,
};
