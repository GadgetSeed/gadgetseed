/** @file
    @brief	ログ記録バッファ

    @date	2018.11.16
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "device.h"
#include "device/logbuf_ioctl.h"
#include "str.h"
#include "tkprintf.h"

#ifndef GSC_MAX_LOGBUF_SIZE
#define GSC_MAX_LOGBUF_SIZE	(1024*8)	///< $gsc ログバッファサイズ
#endif

#define __ATTR_LOGBUFFER  __attribute__ ((section(".extram"))) __attribute__ ((aligned (4)))
static unsigned char log_buf[GSC_MAX_LOGBUF_SIZE] __ATTR_LOGBUFFER;
static struct st_device *out_dev = 0;
static int flg_buff_full = 0;
static unsigned char *log_wp;
static int flg_enable_log = 1;
static int flg_enable_out = 1;
static unsigned char *log_rp;

static int logout_register(struct st_device *dev, char *param)
{
	if(param != 0) {
		out_dev = open_device(param);
		if(out_dev == 0) {
			SYSERR_PRINT("Cannot open device \"%s\"\n", param);
		}
	}

	flg_buff_full = 0;
	log_wp = log_buf;
	log_rp = log_buf;

	return 0;
}

static void log_putc(unsigned char ch)
{
	if(flg_enable_log != 0) {
		*log_wp = ch;
		log_wp ++;
		if(log_wp >= &log_buf[GSC_MAX_LOGBUF_SIZE]) {
			log_wp = log_buf;
			if(flg_buff_full == 0) {
				flg_buff_full = 1;
			}
		}
	}
}

static int logout_putc(struct st_device *dev, unsigned char ch)
{
	log_putc(ch);

	if(flg_enable_out != 0) {
		if(out_dev != 0) {
			putc_device(out_dev, ch);
		}
	}

	return 1;
}

static int logbuf_register(struct st_device *dev, char *param)
{
	return 0;
}

static int logbuf_putc(struct st_device *dev, unsigned char ch)
{
	log_putc(ch);

	return 1;
}

static int logbuf_read(struct st_device *dev, void *buf, unsigned int count)
{
	int i;
	unsigned char *dp = buf;
	int rtn = 0;

	for(i=0; i<count; i++) {
		if(log_rp == log_wp) {
			break;
		}
		*dp = *log_rp;
		rtn ++;

		log_rp ++;
		if(log_rp >= &log_buf[GSC_MAX_LOGBUF_SIZE]) {
			log_rp = log_buf;
		}
		dp ++;
	}

	return rtn;
}

static int is_timestamp(unsigned char ch)
{
	if(ch == ' ') {
		return 1;
	}
	if(ch == '.') {
		return 1;
	}
	if(('0' <= ch) && (ch <= '9')) {
		return 1;
	}

	return 0;
}

static void set_log_pointer(unsigned char **sp, unsigned char **ep)
{
	if(flg_buff_full == 0) {
		*sp = log_buf;
		*ep = log_wp;
	} else {
		*sp = log_wp;
		if(*sp != log_buf) {
			*ep = log_wp - 1;
		} else {
			*ep = &log_buf[GSC_MAX_LOGBUF_SIZE-1];
		}
	}
}

static unsigned char * search_timestamp(unsigned long long tstamp)
{
	unsigned char *rp;
	unsigned char *sp;
	unsigned char *ep;
	int stat_ts = 0;
	int tcount = 0;
	char timestr[16];

	//tkprintf("%7u.%06u\n", (unsigned int)(tstamp/1000000), (unsigned int)(tstamp%1000000));

	set_log_pointer(&sp, &ep);

	rp = sp;
	while(rp != ep) {
		//kputs(rp, 1);
		switch(stat_ts) {
		case 0:
			if(*rp == '[') {
				stat_ts = 1;
				tcount = 0;
				//tkprintf("\"%8s\"\n", rp);
			}
			break;

		case 1:
			if(tcount > 13) {
				if(*rp == ']') {
					unsigned int sec;
					unsigned int usec;
					unsigned long long rtime = 0;

					timestr[tcount] = 0;
					//tkprintf("%d : %s\n", record_count, timestr);
					
					timestr[7] = 0;
					sec = dstou((uchar *)timestr);
					usec = dstou((uchar *)&timestr[8]);
					rtime = ((unsigned long long)sec * 1000000) + usec;
					//tkprintf("%llu\n", rtime);

					if(rtime > tstamp) {
						rp -= 15;
						//tkprintf("Find : %7u.%06u\n",
						//	 (unsigned int)(rtime/1000000),
						//	 (unsigned int)(rtime%1000000));
						goto find;
					}
				} else {
					//tkprintf("\"%8s\"\n", rp);
				}
				stat_ts = 0;
			} else {
				if(is_timestamp(*rp) != 0) {
					timestr[tcount] = *rp;
					tcount ++;
				} else {
					stat_ts = 0;
				}
			}
			break;
		}

		rp ++;
		if(rp == &log_buf[GSC_MAX_LOGBUF_SIZE-1]) {
			rp = log_buf;
		}
	}

find:
	//tkprintf("seek  = %p\n", rp);
	return rp;
}

static unsigned char * search_head_record(unsigned int recnum)
{
	unsigned char *rp;
	unsigned char *sp;
	unsigned char *ep;
	int stat_ts = 0;
	int tcount = 0;
	int reccount = 0;

	set_log_pointer(&sp, &ep);
	//tkprintf("start = %p\n", sp);
	//tkprintf("end   = %p\n", ep);

	rp = sp;
	while(rp != ep) {
		//kputs(rp, 1);
		switch(stat_ts) {
		case 0:
			if(*rp == '[') {
				stat_ts = 1;
				tcount = 0;
				//tkprintf("\"%8s\"\n", rp);
			}
			break;

		case 1:
			tcount ++;
			if(tcount == 15) {
				if(*rp == ']') {
					//tkprintf("\"%16s\"\n", rp-15);
					if(recnum == reccount) {
						//tkprintf("find  = %p\n", rp-15);
						rp -= 15;
						goto find;
					} else {
						reccount ++;
					}
				} else {
					//tkprintf("\"%8s\"\n", rp-15);
				}
				stat_ts = 0;
			} else {
				if(is_timestamp(*rp) == 0) {
					stat_ts = 0;
				}
			}
			break;
		}

		rp ++;
		if(rp == &log_buf[GSC_MAX_LOGBUF_SIZE-1]) {
			rp = log_buf;
		}
	}

find:
	//tkprintf("seek  = %p\n", rp);
	return rp;
}

static unsigned char * search_tail_record(unsigned int recnum)
{
	unsigned char *rp;
	unsigned char *sp;
	unsigned char *ep;
	int stat_ts = 0;
	int tcount = 0;
	int reccount = 0;

	set_log_pointer(&sp, &ep);
	//tkprintf("start = %p\n", sp);
	//tkprintf("end   = %p\n", ep);

	rp = ep;
	while(rp != sp) {
		//kputs(rp, 1);
		switch(stat_ts) {
		case 0:
			if(*rp == ']') {
				stat_ts = 1;
				tcount = 15;
				//tkprintf("\"%8s\"\n", rp);
			}
			break;

		case 1:
			tcount --;
			if(tcount == 0) {
				if(*rp == '[') {
					//tkprintf("\"%16s\"\n", rp);
					reccount ++;
					if(recnum == reccount) {
						//tkprintf("find  = %p\n", rp);
						goto find;
					}
				} else {
					//tkprintf("\"%8s\"\n", rp);
				}
				stat_ts = 0;
			} else {
				if(is_timestamp(*rp) == 0) {
					stat_ts = 0;
				}
			}
			break;
		}

		rp --;
		if(rp < log_buf) {
			rp = &log_buf[GSC_MAX_LOGBUF_SIZE-1];
		}
	}

find:
	//tkprintf("seek  = %p\n", rp);
	return rp;
}

static int get_log_info(struct st_loginfo *loginfo)
{
	unsigned long long start_time = 0xffffffffffffffff;
	unsigned long long last_time = 0;
	int record_count = 0;
	unsigned char *sp;
	unsigned char *ep;
	unsigned char *rp;
	char timestr[16];
	int stat_ts = 0;
	int tcount = 0;
	int logsize = 0;

	if(flg_buff_full != 0) {
		logsize = GSC_MAX_LOGBUF_SIZE - 1;
	} else {
		logsize = log_wp - log_buf;
	}

	set_log_pointer(&sp, &ep);
	//tkprintf("start = %p\n", sp);
	//tkprintf("end   = %p\n", ep);

	rp = sp;
	while(rp != ep) {
		//kputs(rp, 1);
		switch(stat_ts) {
		case 0:
			if(*rp == '[') {
				stat_ts = 1;
				tcount = 0;
				//tkprintf("\"%8s\"\n", rp);
			}
			break;

		case 1:
			if(tcount > 13) {
				if(*rp == ']') {
					unsigned int sec;
					unsigned int usec;
					unsigned long long rtime = 0;

					timestr[tcount] = 0;
					//tkprintf("%d : %s\n", record_count, timestr);

					timestr[7] = 0;
					sec = dstou((uchar *)timestr);
					usec = dstou((uchar *)&timestr[8]);
					rtime = ((unsigned long long)sec * 1000000) + usec;
					//tkprintf("%d : %llu\n", record_count, rtime);
					record_count ++;

					if(rtime < start_time) {
						start_time = rtime;
					}
					if(rtime > last_time) {
						last_time = rtime;
					}
				} else {
					//tkprintf("\"%8s\"\n", rp);
				}
				stat_ts = 0;
			} else {
				if(is_timestamp(*rp) != 0) {
					timestr[tcount] = *rp;
					tcount ++;
				} else {
					stat_ts = 0;
				}
			}
			break;
		}

		rp ++;
		if(rp == &log_buf[GSC_MAX_LOGBUF_SIZE-1]) {
			rp = log_buf;
		}
	}

	loginfo->maxsize = GSC_MAX_LOGBUF_SIZE - 1;
	loginfo->logsize = logsize;
	loginfo->record_count = record_count;
	if(start_time == 0xffffffffffffffff) {
		loginfo->start_time = 0;
	} else {
		loginfo->start_time = start_time;
	}
	loginfo->last_time = last_time;

	//tkprintf("Record : %d\n", record_count);
	//tkprintf("Start  : %7u.%06u\n", (unsigned int)(start_time/1000000), (unsigned int)(start_time%1000000));
	//tkprintf("Last   : %7u.%06u\n", (unsigned int)(last_time/1000000), (unsigned int)(last_time%1000000));

	return 0;
}

static int logbuf_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	switch(com) {
	case IOCMD_LOGBUF_GET_INFO:
	{
		struct st_loginfo *loginfo = param;

		get_log_info(loginfo);

		return 0;
	}
	break;

	case IOCMD_LOGBUF_SEEK_HEAD:
	{
		unsigned char *rp;

		rp = search_head_record(arg);
		log_rp = rp;

		return 0;
	}
	break;

	case IOCMD_LOGBUF_SEEK_TAIL:
	{
		unsigned char *rp;

		rp = search_tail_record(arg);
		log_rp = rp;

		return 0;
	}
	break;

	case IOCMD_LOGBUF_SEEK_TIME:
	{
		unsigned long long *stime;
		unsigned char *rp;

		stime = (unsigned long long *)param;
		rp = search_timestamp(*stime);
		log_rp = rp;

		return 0;
	}
	break;

	case IOCMD_LOGBUF_GET_LINESIZE:
	{
		unsigned char *rp = log_rp;
		int len = 0;
		int stat = 0;

		while(rp != log_wp) {
			switch(stat) {
			case 0:
				if(*rp == '\n') {
					stat = 1;
				}
				break;

			case 1:
				if(*rp == '[') {
					//tkprintf("len = %d\n", len);
					return len;
				} else {
					stat = 0;
				}
				break;
			}

			len ++;
			rp ++;
			if(rp >= &log_buf[GSC_MAX_LOGBUF_SIZE]) {
				rp = log_buf;
			}
		}

		return len;
	}
	break;

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		return -1;
	}

	return 0;
}

static 	int logbuf_seek(struct st_device *dev, int offset, int whence)
{
	if(offset > GSC_MAX_LOGBUF_SIZE) {
		return -1;
	}

	switch(whence) {
	case SEEK_SET:
		if(flg_buff_full == 0) {
			log_rp = log_buf + offset;
		} else {
			log_rp = log_wp + 1 + offset;
		}
		while(log_rp >= &log_buf[GSC_MAX_LOGBUF_SIZE]) {
			log_rp -= GSC_MAX_LOGBUF_SIZE;
		}
		break;

	case SEEK_CUR:
		break;

	case SEEK_END:
		break;

	default:
		return -1;
	}

	return 0;
}

const struct st_device logout_device = {
	.name		= DEF_DEV_NAME_LOGOUT,
	.explan		= "log output & buffer device",
	.register_dev	= logout_register,
	.putc		= logout_putc,
};

const struct st_device logbuf_device = {
	.name		= DEF_DEV_NAME_LOGBUF,
	.explan		= "log buffer device",
	.register_dev	= logbuf_register,
	.putc		= logbuf_putc,
	.read		= logbuf_read,
	.ioctl		= logbuf_ioctl,
	.seek		= logbuf_seek,
};
