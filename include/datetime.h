/** @file
    @brief	日付時刻

    @date	2015.09.19
    @date	2007.05.01
    @author	Takashi SHUDO
*/

#ifndef DATETIME_H
#define DATETIME_H

#include "timer.h"

#ifndef GSC_TIMEZONE_STR
#define GSC_TIMEZONE_STR	"JST"	///< $gsc タイムゾーンを示す文字列
#endif
#ifndef GSC_DIFF_FROM_LOCAL_TIME_SEC
#define GSC_DIFF_FROM_LOCAL_TIME_SEC	(9*60*60)	///< $gsc UTCと日本時間(+9時間)との時差(秒)
#endif

typedef long long int	t_time;

#define DATE_STR_LEN	((unsigned int)sizeof("YYYY/MM/DD WWW"))
#define TIME_STR_LEN	((unsigned int)sizeof("HH/MM/SS"))
#define MSEC_STR_LEN	((unsigned int)sizeof(".mmm"))
#define DATETIME_STR_LEN	(DATE_STR_LEN + 1 + TIME_STR_LEN)
#define DATEMTIME_STR_LEN	(DATE_STR_LEN + 1 + TIME_STR_LEN + 1 + MSEC_STR_LEN)

struct st_datetime {
	short	year;	///< 年
	char	month;	///< 月
	char	day;	///< 日
	char	dayofweek;	///< 曜日 0:日曜日〜6:土曜日
	char	hour;	///< 時
	char	min;	///< 分
	char	sec;	///< 秒
	short	msec;	///< ミリ秒
};	///< 時刻構造体

struct st_systime {
	t_time	sec;	///< 秒
	int	usec;	///< マイクロ秒(0〜999999)
};	///< システム時間

extern struct st_systime system_time;
extern struct st_systime l_system_time;
extern timer_func sec_timer_func;

extern int read_rtc_time(struct st_datetime *datetime);
extern void datetime_to_str(char *str, struct st_datetime *datetime);
extern void datemtime_to_str(char *str, struct st_datetime *datetime);
extern void date_to_str(char *str, struct st_datetime *datetime);
extern void mtime_to_str(char *str, struct st_datetime *datetime);
extern void time_to_str(char *str, struct st_datetime *datetime);
extern int set_rtc(struct st_datetime *datetime);
extern int init_time(char *devname);
extern char date_to_dayofweek(short year, char month, char day);
extern int is_leap_year(int year);
extern int num_of_day_in_month(int year, int month);
extern void unixtime_to_datetime(struct st_datetime *datetime, struct st_systime *unixtime);
extern void systime_to_datetime(struct st_datetime * datetime, struct st_systime *stime);
extern t_time datetime_to_utc(struct st_datetime * datetime);
extern void datetime_to_systime(struct st_systime *systime, struct st_datetime *time);
extern void set_systime(struct st_systime *systime);
extern t_time get_systime_sec(void);
extern unsigned int fattime(void);
extern void get_systime(struct st_systime *systime);
extern void register_sec_timer_func(timer_func func);
extern void sync_systime_from_rtc(void);
extern void adjust_systime(void);
extern void sync_rtc_from_systime(void);

#endif // DATETIME_H
