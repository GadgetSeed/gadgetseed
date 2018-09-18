/** @file
    @brief	日付時刻

    GadgetSeed の時計はカーネルタイマよりカウントされます。
    システム起動時に最初にRTCデバイスより同期を行います。
    RTCはUTC時刻が設定されます。

    @date	2015.09.19
    @date	2007.05.01
    @author	Takashi SHUDO

    @page date_time 日付時刻

    GadgetSeed は日付時刻を取得、設定する機能があります。\n
    GadgetSeed の日付時刻はカーネルタイマと同期しています。\n
    RTC(リアルタムクロック)を持つシステムはRTCより取得される日時と同期させるために時計の進行速度が調整されます。
    これらの機能は datetime.h で定義されています。


    ---
    @section system_time_struct システム時間構造体

    GadgetSeedの日付時刻はシステム時間を元に日付、時刻に変換する機能があります。\n
    システム時間は以下の構造体で定義されています。

    @ref st_systime @copybrief st_systime

    システム時間のフィールド sec は32ビット長のUNIX時間(1970/01/01 00:00:00 からの1秒カウンタ)です。\n
    システム時間のフィールド usec は32ビット長のナノ秒カウンタです。(0 - 999999)\n
    システム時間はロケーションに関わらず、UTC時刻を示します。


    ---
    @section datetime_struct 時刻構造体

    時刻情報は時刻を年、月、日、曜日、時、分、秒、ミリ秒で示すデータ構造です。\n
    時刻情報は以下の構造体で定義されています。

    @ref st_datetime @copybrief st_datetime


    ---
    @section datetime_api 日付時刻API

    @subsection get_time_api 時間取得API

    include ファイル : datetime.h

    | API名			| 機能				|
    |:--------------------------|:------------------------------|
    | get_systime()		| @copybrief get_systime	|
    | get_systime_sec()		| @copybrief get_systime_sec	|
    | fattime()			| @copybrief fattime		|

    @subsection time_convert_api システム時間、時刻情報変換API

    include ファイル : datetime.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | unixtime_to_datetime()	| @copybrief unixtime_to_datetime	|
    | systime_to_datetime()	| @copybrief systime_to_datetime	|
    | datetime_to_utc()		| @copybrief datetime_to_utc		|
    | datetime_to_systime()	| @copybrief datetime_to_systime	|

    @subsection time_strings_api 時刻情報文字列変換API

    include ファイル : datetime.h

    | API名			| 機能				|
    |:--------------------------|:------------------------------|
    | date_to_str()		| @copybrief date_to_str	|
    | time_to_str()		| @copybrief time_to_str	|
    | mtime_to_str()		| @copybrief mtime_to_str	|
    | datetime_to_str()		| @copybrief datetime_to_str	|
    | datemtime_to_str()	| @copybrief datemtime_to_str	|

    @subsection set_system_time_api システム時間設定API

    include ファイル : datetime.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | set_systime()		| @copybrief set_systime		|
    | sync_systime_from_rtc()	| @copybrief sync_systime_from_rtc	|
    | adjust_systime()		| @copybrief adjust_systime		|
    | sync_rtc_from_systime()	| @copybrief sync_rtc_from_systime	|

    @subsection set_rtc_api RTC制御API

    include ファイル : datetime.h

    | API名			| 機能			|
    |:--------------------------|:----------------------|
    | set_rtc()			| @copybrief set_rtc	|

    @subsection generic_time_api 汎用時間関数

    include ファイル : datetime.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | date_to_dayofweek()	| @copybrief date_to_dayofweek		|
    | is_leap_year()		| @copybrief is_leap_year		|
    | num_of_day_in_month()	| @copybrief num_of_day_in_month	|

    @subsection periodic_processing_api 周期処理

    include ファイル : datetime.h

    | API名			| 機能					|
    |:--------------------------|:--------------------------------------|
    | register_sec_timer_func()	| @copybrief register_sec_timer_func	|
*/

#include "sysconfig.h"
#include "datetime.h"
#include "timer.h"
#include "device.h"
#include "device/rtc_ioctl.h"
#include "tprintf.h"
#include "tkprintf.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"

#define DEF_SYSTIME_INC_COUNT	(1000 * GSC_KERNEL_TIMER_INTERVAL_MSEC) ///< システム時間カーネルタイマ間隔増加分

struct st_systime system_time;	/**< システム時間(UNIX時間、UTC)
				 * 1970/01/01 00:00:00 からの1秒カウン
				 * タ(2038年問題非対応)
				 */
struct st_systime l_system_time;	// 秒更新確認のための前回カーネルタイマ割り込み時のシステム時間

#ifdef GSC_DEV_ENABLE_RTC
static struct st_device *rtc_dev = 0;	///< RTCデバイス
#ifndef GSC_RTC_RESOLUTION_IS_NOT_SEC	// $gsc RTCの解像度は秒より精細
static struct st_systime last_rtcsystime;
#endif
#endif
static int systime_inc_count = DEF_SYSTIME_INC_COUNT;

enum tsync_status {
	TSYNC_NONE,
	TSYNC_SYNC,
	TSYNC_ADJUST
};	///< RTCから日時同期中フラグ

static volatile enum tsync_status tsync_status;

timer_func sec_timer_func = 0;	///< １秒間隔処理関数

#ifdef GSC_DEV_ENABLE_RTC	// $gsc リアルタイムクロックを有効にする
/**
   @brief	リアルタイムクロックより時刻を取得する

   @param[in]	time	実時間

   @return	!=0:エラー
*/
int read_rtc_time(struct st_datetime *time)
{
	if(rtc_dev == 0) {
		SYSERR_PRINT("NO RTC Device.\n");
		return -1;
	}

	if(ioctl_device(rtc_dev, IOCMD_RTC_GET, 0, (void *)time) != 0) {
		SYSERR_PRINT("Cannot read time from RTC.\n");
		return -1;
	}

	return 0;
}
#endif

#ifdef DEBUGKBITS
static void disp_systime(struct st_datetime *rtcdatetime)
{
	char str[DATEMTIME_STR_LEN+1];
	struct st_datetime sysdatetime;

	datemtime_to_str(str, rtcdatetime);
	DKPRINTF(0x01, "RTC : %s\n", str);

	systime_to_datetime(&sysdatetime, &system_time);
	datemtime_to_str(str, &sysdatetime);
	DKPRINTF(0x01, "SYS : %s\n", str);

	DKPRINTF(0x01, "UTC_COUNT : %ld.%06d\n", system_time.sec, system_time.usec);
	DKPRINTF(0x01, "Increment : %d\n", systime_inc_count);
}
#endif

#ifdef GSC_DEV_ENABLE_RTC
#ifdef GSC_RTC_DATETIME_SYNC_CYCLE
#define SYSTIME_ADJUST_MARGIN	(1000 * 100)	// システム時間補正しない誤差(100ms)

#define ABS(x) ((x) < 0 ? -(x) : (x))

static void calc_systime_inc_count(struct st_systime *rtcsystime)
{
	unsigned long long system_time_count;
	unsigned long long rtc_time_count;
	int rtc_sys_diff_count;	// RTCとシステム時間の差分
	int error_correction = 0;

	system_time_count = ((unsigned long long)(system_time.sec) * 1000000) + system_time.usec;
	rtc_time_count = ((unsigned long long)(rtcsystime->sec) * 1000000) + rtcsystime->usec;

	rtc_sys_diff_count = (int)(rtc_time_count - system_time_count);

	DKPRINTF(0x01, "RTC_COUNT : %lld\n", rtc_time_count);
	DKPRINTF(0x01, "SYS_COUNT : %lld\n", system_time_count);
	DKPRINTF(0x01, "RTC DIFF : %d\n", rtc_sys_diff_count);

	error_correction = rtc_sys_diff_count / GSC_RTC_DATETIME_SYNC_CYCLE;
	DKPRINTF(0x01, "Error correction : %d\n", error_correction);

	// 時間インクリメント値設定
	if(ABS(rtc_sys_diff_count) <= SYSTIME_ADJUST_MARGIN) {
		systime_inc_count = DEF_SYSTIME_INC_COUNT + (error_correction * GSC_KERNEL_TIMER_INTERVAL_MSEC);
	} else {
		// 誤差が大きい場合は補正を２倍
		DKPRINTF(0x01, "difference > %d\n", SYSTIME_ADJUST_MARGIN);
		systime_inc_count = DEF_SYSTIME_INC_COUNT + (error_correction * GSC_KERNEL_TIMER_INTERVAL_MSEC * 2);
	}
	DKPRINTF(0x01, "systime_inc_count : %d\n", systime_inc_count);
}
#endif
#endif

/**
   @brief	システムタイマ定期処理タイマ関数

   @param[in]	ktime	カーネル時間

   @return	!=0:エラー
*/
static void time_count_proc(void *sp, unsigned long long ktime)
{
#ifdef GSC_DEV_ENABLE_RTC
	int flg_just_sec = 0;
#endif

	UNUSED_VARIABLE(ktime);

	system_time.usec += systime_inc_count;
	if(system_time.usec >= (1000 * 1000)) {
		system_time.usec -= (1000 * 1000);
		system_time.sec ++;
#ifdef GSC_DEV_ENABLE_RTC
		flg_just_sec = 1;
#endif
	}
	//tkprintf("system_time = %12d.%06d\n", system_time.sec, system_time.usec);
	DKPRINTF(2, "system_time = %12d.%06d\n", system_time.sec, system_time.usec);

#ifdef GSC_DEV_ENABLE_RTC
	switch(tsync_status) {
	case TSYNC_NONE:
		break;

	case TSYNC_SYNC:
		if(rtc_dev == 0) {
			DKPRINTF(0x01, "RTC device not found\n");
			tsync_status = TSYNC_NONE;
		} else {
			/*
			 * 分解能が１秒のRTC（殆どがそう）の場合、秒カ
			 * ウンタが変化するまで時刻情報を取得し続ける
			 */
			struct st_datetime rtcdatetime;
			struct st_systime rtcsystime = { 0, 0 };
			DKPRINTF(0x02, "kernel time = %d\n", (int)ktime);
			if(read_rtc_time(&rtcdatetime) != 0) {
				SYSERR_PRINT("RTC access fail\n");
				tsync_status = TSYNC_NONE;
			} else {
#ifdef DEBUGKBITS
				char str[DATEMTIME_STR_LEN+1];
				datemtime_to_str(str, &rtcdatetime);
				DKPRINTF(0x02, "RTC time : %s\n", str);
#endif
				datetime_to_systime(&rtcsystime, &rtcdatetime);
#ifdef GSC_RTC_RESOLUTION_IS_NOT_SEC	// 
				set_systime(&rtcsystime);
				tsync_status = TSYNC_NONE;
#else // GSC_RTC_RESOLUTION_IS_NOT_SEC
				if(last_rtcsystime.sec == 0) {
					// 最初の時刻取得
					DKPRINTF(0x02, "RTC resolution is sec\n");
					last_rtcsystime.sec = rtcsystime.sec;
					last_rtcsystime.usec = rtcsystime.usec;
				} else {
					// 秒が変わるまで取得し続ける
					if(last_rtcsystime.sec != rtcsystime.sec) {
						set_systime(&rtcsystime);
						tsync_status = TSYNC_NONE;
					}
				}
#endif //  GSC_RTC_RESOLUTION_IS_NOT_SEC
#ifdef DEBUGKBITS
				if(tsync_status == TSYNC_NONE) {
					disp_systime(&rtcdatetime);
					DKPRINTF(0x01, "Time sync done.\n");
				}
#endif
			}
		}
		break;

#ifdef GSC_RTC_DATETIME_SYNC_CYCLE
	case TSYNC_ADJUST:
		if(rtc_dev == 0) {
			DKPRINTF(0x01, "RTC device not found\n");
			tsync_status = TSYNC_NONE;
		} else {
			/*
			  分解能が１秒のRTC（殆どがそう）の場合、秒カ
			  ウンタが変化するまで時刻情報を取得し続ける
			*/
			struct st_datetime rtcdatetime;
			struct st_systime rtcsystime = { 0, 0 };
			DKPRINTF(0x02, "kernel time = %d\n", (int)ktime);
			if(read_rtc_time(&rtcdatetime) != 0) {
				SYSERR_PRINT("RTC access fail\n");
				tsync_status = TSYNC_NONE;
			} else {
#ifdef DEBUGKBITS
				char str[DATEMTIME_STR_LEN+1];
				datemtime_to_str(str, &rtcdatetime);
				DKPRINTF(0x02, "RTC time : %s\n", str);
#endif
				datetime_to_systime(&rtcsystime, &rtcdatetime);
#ifdef GSC_RTC_RESOLUTION_IS_NOT_SEC	//
				calc_systime_inc_count(&rtcsystime);
				tsync_status = TSYNC_NONE;
#else // GSC_RTC_RESOLUTION_IS_NOT_SEC
				if(last_rtcsystime.sec == 0) {
					// 最初の時刻取得
					DKPRINTF(0x02, "RTC resolution is sec\n");
					last_rtcsystime.sec = rtcsystime.sec;
					last_rtcsystime.usec = rtcsystime.usec;
				} else {
					// 秒が変わるまで取得し続ける
					if(last_rtcsystime.sec != rtcsystime.sec) {
						calc_systime_inc_count(&rtcsystime);
						tsync_status = TSYNC_NONE;
					}
				}
#endif //  GSC_RTC_RESOLUTION_IS_NOT_SEC
#ifdef DEBUGKBITS
				if(tsync_status == TSYNC_NONE) {
					disp_systime(&rtcdatetime);
					DKPRINTF(0x01, "Time adjust done.\n");
				}
#endif
			}
		}
		break;
#endif

	default:
		SYSERR_PRINT("Invalid status %d\n", tsync_status);
		break;
	}
#endif // GSC_RTC_DATETIME_SYNC_CYCLE

#ifdef GSC_DEV_ENABLE_RTC
	if(flg_just_sec != 0) {
		if(sec_timer_func != 0) {
			sec_timer_func(sp, kernel_time_count);
		}
	}
#endif
}

static const char weekname[7][10] = {
	"Sunday", "Monday", "Tuesday", "Wednesday",
	"Thursday", "Friday", "Saturday"
}; ///< 曜日文字列

/**
   @brief	時間を日付文字列に変換する

   @param[out]	str	日付文字列ポインタ
   @param[in]	time	時間
*/
void date_to_str(char *str, struct st_datetime *time)
{
	(void)tsnprintf(str, DATE_STR_LEN, "%4d/%02d/%02d %3s",
			time->year,
			time->month,
			time->day,
			weekname[(int)time->dayofweek]);
}

/**
   @brief	時間を時間文字列に変換する

   @param[out]	str	時間文字列ポインタ
   @param[in]	time	時間
*/
void time_to_str(char *str, struct st_datetime *time)
{
	(void)tsnprintf(str, TIME_STR_LEN, "%02d:%02d:%02d",
			time->hour,
			time->min,
			time->sec);
}

/**
   @brief	時間をミリ秒時間文字列に変換する

   @param[out]	str	時間文字列ポインタ
   @param[in]	time	時間
*/
void mtime_to_str(char *str, struct st_datetime *time)
{
	time_to_str(str, time);
	(void)tsnprintf(&str[TIME_STR_LEN-1], MSEC_STR_LEN, ".%03d", time->msec);
}

/**
   @brief	時間を日付時間文字列に変換する

   @param[out]	str	日付時間文字列ポインタ
   @param[in]	time	時間
*/
void datetime_to_str(char *str, struct st_datetime *time)
{
	date_to_str(str, time);
	str[DATE_STR_LEN-1] = ' ';
	time_to_str(&str[DATE_STR_LEN], time);
}

/**
   @brief	時間を日付ミリ秒時間文字列に変換する

   @param[out]	str	日付時間文字列ポインタ
   @param[in]	time	時間
*/
void datemtime_to_str(char *str, struct st_datetime *time)
{
	date_to_str(str, time);
	str[DATE_STR_LEN-1] = ' ';
	mtime_to_str(&str[DATE_STR_LEN], time);
}

#ifdef GSC_DEV_ENABLE_RTC
/*
   @brief	時計をRTCと同期させる

   @return	!=0:エラー
*/
static int sync_time(void)
{
	DKPRINTF(0x01, "Time synchronization...\n");

	if(rtc_dev == 0) {
		SYSERR_PRINT("NO RTC Device.\n");
		return -1;
	}

#ifndef GSC_RTC_RESOLUTION_IS_NOT_SEC
	last_rtcsystime.sec = 0;
	last_rtcsystime.usec = 0;
#endif
	tsync_status = TSYNC_SYNC;

	while(tsync_status != TSYNC_NONE) {
#ifdef GSC_TARGET_SYSTEM_EMU
		usleep(1000);
#endif
	}

	DKPRINTF(0x01, "Time sync done.\n");

	return 0;
}

/**
   @brief	時間をリアルタイムクロックに設定する

   @param[in]	time	実時間

   @return	!=0:エラー
*/
int set_rtc(struct st_datetime *time)
{
	if(rtc_dev == 0) {
		SYSERR_PRINT("NO RTC Device\n");
		return -1;
	}

	if(ioctl_device(rtc_dev, IOCMD_RTC_SET, 0, (void *)time) != 0) {
		SYSERR_PRINT("Cannot set time to RTC.\n");
		return -1;
	}

	return 0;
}
#endif

/**
   @brief	時計を初期する

   時間をリアルタイムクロックより読み出し設定する

   @param[in]	devname	RTCデバイス名

   リアルタイムクロックが無いシステムでは"0"を設定する

   @return	!=0:エラー
*/
int init_time(char *devname)
{
	tsync_status = TSYNC_NONE;
	l_system_time.sec = 0;
	l_system_time.usec = 0;
	system_time.sec = 0;	// 1970/01/01 00:00:00
	system_time.usec = 0;

	if(register_timer_func(time_count_proc, GSC_KERNEL_TIMER_INTERVAL_MSEC)) {
		SYSERR_PRINT("Cannot register time func.\n");
		goto err;
	}

#ifdef GSC_DEV_ENABLE_RTC
	if(devname == 0) {
		return -1;
	}

	rtc_dev = open_device(devname);

	if(rtc_dev == 0) {
		SYSERR_PRINT("Cannot open RTC device.\n");
		goto err;
	}

	if(sync_time()) {
		goto err;
	}
#endif

//	time.dayofweek = date_to_dayofweek(time.year, time.month, time.day);

	return 0;

 err:
	return -1;
}

/*
  @brief	ユリウス日より日時を取得する

  @param[in]	jd	ユリウス日時
  @param[out]	year	西暦年
  @param[out]	month	月
  @param[out]	day	日
*/
static void jd2greg(int jd, short *year, char *month, char *day)
{
	int century;
	int y, m;

	jd += 32045;
	century = (4 * jd - 1)/ 146097L;
	jd -= 146097L * century / 4;
	y = (4 * jd - 1) / 1461;
	jd -= 1461L * y / 4;
	y += 100 * century;
	m = (5 * jd - 3) / 153 + 3;
	jd -= (153 * m - 457) / 5;
	if(m > 12) {
		m -= 12;
		y++;
	}
	*year = (y - 4800);
	*month = m;
	*day = jd;
}

/**
   @brief	西暦年、月、日より曜日を求める

   @param[out]	year	西暦年
   @param[out]	month	月
   @param[out]	day	日

   @return	曜日(0:日曜日、6:土曜日)
*/
char date_to_dayofweek(short year, char month, char day)
{
#if 0
	return (year + year/4 - year/100 + year/400
			+ (13 * month + 8)/5 + day) % 7;
#else
	static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
	year -= month < 3;
	return (year + year/4 - year/100 + year/400 + t[month-1] + day) % 7;
#endif
}

/**
   @brief	うるう年か調べる

   @param[in]	year	西暦年

   @return	0:うるう年ではない、1:うるう年
*/
int is_leap_year(int year)
{
	if(year < 4) {
		return 0;
	} else if(year % 400 == 0 || (year % 100 != 0 && year % 4 == 0)) {
		return 1;
	} else {
		return 0;
	}
}

/**
   @brief	うるう月か調べる

   @param[in]	year	西暦年
   @param[in]	month	月

   @return	日数
*/
int num_of_day_in_month(int year, int month)
{
	int rt = 0;

	const static int mofd[12] = {
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
	};

	if((month > 12) || (month < 1)) {
		return 0;
	}

	rt = mofd[month - 1];
	if(is_leap_year(year) != 0) {
		rt += 1;
	}

	return rt;
}

/**
   @brief	UNIX時間より時刻を求める

   @param[out]	datetime	時刻
   @param[in]	unixtime	UNIX時間
*/
void unixtime_to_datetime(struct st_datetime *datetime, struct st_systime *unixtime)
{
	t_time sec = unixtime->sec;
	t_time jday = (sec/((int)24*60*60)) + 2440588; // ユリウス日へ

	DKPRINTF(0x02, "jday = %d\n", jday);

	jd2greg(jday, &(datetime->year), &(datetime->month),
		&(datetime->day));

	datetime->dayofweek = date_to_dayofweek(datetime->year,
						datetime->month,
						datetime->day);

	datetime->hour = ((sec % (24L*60*60)) / (60*60));
	datetime->min = ((sec % (60*60)) / 60);
	datetime->sec = (sec % 60);
	datetime->msec = unixtime->usec / 1000;
}

/**
   @brief	システム時間よりローカル時刻を求める

   @param[out]	datetime	時刻(ローカル)
   @param[in]	time		システム時間
*/
void systime_to_datetime(struct st_datetime *datetime, struct st_systime *stime)
{
	struct st_systime localtime;

	localtime.sec = stime->sec + GSC_DIFF_FROM_LOCAL_TIME_SEC;
	localtime.usec = stime->usec;

	unixtime_to_datetime(datetime, &localtime);
}

/**
   @brief	ローカル西暦年月日、時間よりUTCを求める

   UTC 1970/01/01 00:00:00 が 0 になる
   JST 1970/01/01 09:00:00 が 0 になる

   @param[in]	datetime	西暦年月日、時間

   @return	UTC時間
*/
t_time datetime_to_utc(struct st_datetime *datetime)
{
	int year = datetime->year;
	int month = datetime->month;
	t_time utc;

	if((month -= 2) < 1) {
		year --;
		month += 12;
	}

	utc = (year/4 - year/100 + year/400
		+ 367L*month/12 + datetime->day) + year*365 - 719499L;

	DKPRINTF(0x02, "day = %d\n", utc);

	utc *= ((int)24*60*60);
	utc += ((int)(datetime->hour)*(60*60) + datetime->min*60 + datetime->sec);

	utc -= GSC_DIFF_FROM_LOCAL_TIME_SEC;

	return utc;
}

/**
   @brief	ローカル西暦年月日、時間よりシステム時間を求める

   1970/01/01 00:00:00 が 0 になる

   @param[out]	systime		システム時間
   @param[in]	datetime	西暦年月日、時間
*/
void datetime_to_systime(struct st_systime *systime, struct st_datetime *datetime)
{
	systime->sec = datetime_to_utc(datetime) + GSC_DIFF_FROM_LOCAL_TIME_SEC;
	systime->usec = datetime->msec * 1000;
}

/**
   @brief	UTC時刻からシステム時間を設定する

   @param[in]	systime	UTC時刻
*/
void set_systime(struct st_systime *systime)
{
	// [TODO] タイマ割り込みでも更新しているので排他的な処理が必要
	system_time.sec = systime->sec;
	system_time.usec = systime->usec;

#ifdef GSC_DEV_ENABLE_RTC
	sync_rtc_from_systime();
#endif

	(void)eprintf("Set RTC Time = %d.%03d\n", (int)systime->sec, (int)systime->usec/1000);
}

/**
   @brief	システム時間(秒)を取得する

   @return	現在UTC時刻
*/
t_time get_systime_sec(void)
{
	return (volatile int)system_time.sec;
}

/**
   @brief	FAT 現在実時間を取得する

   @return	FAT 現在実時間
*/
unsigned int fattime(void)
{
	struct st_systime systime;
	struct st_datetime time;

	get_systime(&systime);
	systime_to_datetime(&time, &systime);

	if(time.year < 1980) {
		time.year = 1980;
	}

	return		((unsigned int)(time.year-1980) << 25) +
			((unsigned int)(time.month) << 21) +
			((unsigned int)(time.day) << 16) +
			((unsigned int)(time.hour) << 11) +
			((unsigned int)(time.min) << 5) +
			((unsigned int)(time.sec) / 2);
}

/**
   @brief	システム時間を取得する

   @param[out]	システム時間

   @return	エラーコード
*/
void get_systime(struct st_systime *systime)
{
	int usec, usec2;

	systime->sec = system_time.sec;
	usec = system_time.usec;
	usec2 = system_time.usec;
	if(usec2 < usec) {
		systime->sec = system_time.sec;
	}
	systime->usec = system_time.usec;
}

/**
   @brief	時刻秒更新に同期した1秒周期処理を登録する

   @param[in]	func	1秒周期処理関数
*/
void register_sec_timer_func(timer_func func)
{
	sec_timer_func = func;
}

#ifdef GSC_DEV_ENABLE_RTC
/**
   @brief	RTC時刻からシステム時間を設定する
*/
void sync_systime_from_rtc(void)
{
	(void)sync_time();
}

#ifdef GSC_RTC_DATETIME_SYNC_CYCLE
/**
   @brief	RTC時刻からシステム時間の増加値を調整する
*/
void adjust_systime(void)
{
#ifndef GSC_RTC_RESOLUTION_IS_NOT_SEC
	last_rtcsystime.sec = 0;
	last_rtcsystime.usec = 0;
#endif
	tsync_status = TSYNC_ADJUST;
}
#endif

/**
   @brief	システム時間からRTC時刻を設定する
*/
void sync_rtc_from_systime(void)
{
	struct st_systime systime;
	struct st_datetime datetime;

	get_systime(&systime);
	unixtime_to_datetime(&datetime, &systime);
	(void)set_rtc(&datetime);
}
#endif
