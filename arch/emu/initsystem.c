/** @file
    @brief	仮想 GadgetSeed システム初期化

    @date	2009.11.06
    @author	Takashi SHUDO
*/

#include <stdio.h>

#include "sysconfig.h"
#include "system.h"
#include "device.h"
#include "random.h"
#include "datetime.h"
#include "graphics.h"
#include "console.h"
#include "device/rtc_ioctl.h"
#include "device/sd_ioctl.h"
#include "device/video_ioctl.h"

#ifdef GSC_COMP_ENABLE_FATFS
#include "storage.h"
#include "file.h"
#endif

extern int open_lcdwindow(int *argc, char ***argv);

extern const struct st_device logout_device;
extern const struct st_device logbuf_device;

extern const struct st_device GSC_KERNEL_ERROUT_DEVICE;
extern const struct st_device vconsole_device;
extern const struct st_device rtc_device;
extern const struct st_device vlcd_device;
extern const struct st_device framebuf_device;
//extern const struct st_device fbconsole_device;
extern const struct st_device grconsole_device;
extern const struct st_device eventcon_device;
extern const struct st_device sound_device;
extern const struct st_device veeprom_device;
extern const struct st_device vqspi_device;
extern const struct st_device vmmc_device;
extern const struct st_device vaudio_device;
extern const struct st_device vether_device;

extern const struct st_device powerkey_device;
extern const struct st_device gbkey_device;
extern const struct st_device led_device;
extern const struct st_device null_device;

// デバイスドライバリスト
static const struct st_device * const dev_list[] = {
	&vconsole_device,
#ifdef GSC_DEV_ENABLE_RTC
	&rtc_device,
#endif
#ifdef GSC_COMP_ENABLE_GRAPHICS
	&vlcd_device,
#endif
#ifdef GSC_DEV_ENABLE_I2CEEPROM
	&veeprom_device,
#endif
#ifdef GSC_DEV_ENABLE_STORAGE
	&vmmc_device,
#endif
#ifdef GSC_DEV_ENABLE_QSPI
	&vqspi_device,
#endif
#ifdef GSC_DEV_ENABLE_AUDIO	// $gsc オーディオデバイスを有効にする
	&vaudio_device,
#endif
#ifdef GSC_DEV_ENABLE_ETHER	// $gsc Etherデバイスを有効にする
	&vether_device,
#endif
	0
};

#ifdef GSC_COMP_ENABLE_FATFS
// ドライブリスト
static const char * const storade_devices[] = {
	DEF_DEV_NAME_SD,
	0
};
#endif

extern void set_mmc_filename(char *fname);

void init_system(int *argcp, char ***argvp)
{
#ifdef GSC_COMP_ENABLE_FATFS
	int argc = *argcp;
	char **argv = *argvp;

	if(argc > 1) {
		printf("MMC file : %s\n", argv[1]);
		set_mmc_filename(argv[1]);
	}
#endif

#ifdef GSC_COMP_ENABLE_GRAPHICS
	open_lcdwindow(argcp, argvp);
#endif
}

void init_system2(void)
{
}

static void register_devices(const struct st_device * const list[])
{
	struct st_device **dev = (struct st_device **)list;

	while(*dev) {
		register_device(*dev, 0);
		dev ++;
	}
}

void init_system_drivers(void)
{
	// デバイスドライバ初期化
	register_devices(dev_list);
	register_console_out_dev(&vconsole_device);
	register_console_in_dev(&vconsole_device);
#ifdef GSC_KERNEL_MESSAGEOUT_LOG
	register_error_out_dev(&logout_device);
	register_log_out_dev(&logbuf_device);
#else
	register_error_out_dev(&GSC_KERNEL_ERROUT_DEVICE);
#endif

	// RTC初期化
#ifdef GSC_DEV_ENABLE_RTC
	init_time(DEF_DEV_NAME_RTC);
#else
	init_time(0);
#endif

	// グラフィックドライバ初期化
#ifdef GSC_COMP_ENABLE_GRAPHICS	// $gsc グラフィック描画を有効にする
	init_graphics(DEF_DEV_NAME_VIDEO);
	extern struct st_bitmap gs_logo;
	draw_enlarged_bitmap(0, 0, &gs_logo, 4);
#ifdef GSC_DEV_ENABLE_GRCONSOLE	// $gsc グラフィックデバイス用コンソール出力を有効にする
	register_device(&grconsole_device, 0);
#endif
#endif

	// 乱数初期化
#ifdef GSC_LIB_ENABLE_RANDOM	// $gsc 乱数APIを有効にする
	init_random(get_systime_sec());
#endif

#ifdef GSC_DEV_ENABLE_NULL
	// NULLデバイス初期化
	register_device(&null_device, 0);
#endif
}

void init_system_process(void)
{
#ifdef GSC_COMP_ENABLE_FATFS
	// ファイルシステム初期化
	init_storage();
	register_storage_device(storade_devices);
	init_file();
#endif
}

void init_syscalltbl(void)
{
}

void reset_system(void)
{
	// 未実装
}
