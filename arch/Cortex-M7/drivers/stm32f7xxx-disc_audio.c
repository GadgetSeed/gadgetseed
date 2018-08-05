/** @file
    @brief	STM32F769I-Discovery Audio Driver

    @date	2017.02.11
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "device.h"
#include "interrupt.h"
#include "task/event.h"
#include "task/syscall.h"
#include "task/mutex.h"
#include "device/audio_ioctl.h"
#include "device/ts_ioctl.h"
#include "timer.h"
#include "tkprintf.h"
#include "tprintf.h"
#include "sysevent.h"

//#define DEBUGKBITS 0x01
#include "dkprintf.h"


#if defined(GSC_TARGET_SYSTEM_STM32F746GDISCOVERY)	// $gsc ターゲットシステムは32F746GDISCOVERY
#include "stm32746g_discovery_audio.h"
#elif defined(GSC_TARGET_SYSTEM_STM32F769IDISCOVERY)	// $gsc ターゲットシステムは32F769IDISCOVERY
#include "stm32f769i_discovery_audio.h"
#endif

static struct st_device *ts_dev = 0;

#define MAX_AUDIOBUF	(1152*2*2*2)
static unsigned char audio_buf[MAX_AUDIOBUF];
static int flg_buf_half = 0;

static int flg_play = 0;
static int flg_audio_start = 0;

static int volume = 30;
static int bufsize = MAX_AUDIOBUF;
static int flg_bs_chg = 0;	// バッファサイズが変わった?フラグ
static int sampling_rate = I2S_AUDIOFREQ_48K;
static struct st_mutex audio_mutex;
static struct st_event tx_evq;

extern SAI_HandleTypeDef haudio_out_sai;

void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
	if(flg_play != 0) {
		flg_buf_half = 2;
	}
}

void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
	if(flg_play != 0) {
		flg_buf_half = 1;
	}
}

void BSP_AUDIO_OUT_Error_CallBack(void)
{
	SYSERR_PRINT("AUDIO OUT Error\n");
}

static void inthdr_audio(unsigned int intnum, void *sp)
{
	HAL_DMA_IRQHandler(haudio_out_sai.hdmatx);

	event_wakeup_ISR(sp, &tx_evq, 0);
}

static int audio_register(struct st_device *dev, char *param)
{
	eventqueue_register_ISR(&tx_evq, "audio_tx", 0, 0, 0);

	register_interrupt(IRQ2VECT(AUDIO_OUT_SAIx_DMAx_IRQ), inthdr_audio);

	ts_dev = open_device(DEF_DEV_NAME_TS);
	if(ts_dev != 0) {
		tprintf("Touch sensor found\n");
	}

	return 0;
}

static void lock_ts(void)
{
	if(ts_dev != 0) {
		lock_device(ts_dev, 0);
	}
}

static void unlock_ts(void)
{
	if(ts_dev != 0) {
		unlock_device(ts_dev);
	}
}

static int audio_init(int vol, int smprate)
{
	lock_ts();
	if(BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, vol, smprate) != AUDIO_OK) {
		unlock_ts();
		SYSERR_PRINT("AUDIO Init Error.\n");
		return -1;
	} else {
		BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
		HAL_NVIC_SetPriority(AUDIO_OUT_SAIx_DMAx_IRQ, 0, 0);
		unlock_ts();
	}

	return 0;
}

static int audio_open(struct st_device *dev)
{
	audio_init(volume, sampling_rate);

	return 0;
}

static int audio_close(struct st_device *dev)
{
	lock_ts();
	BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
	BSP_AUDIO_OUT_DeInit();
	unlock_ts();

	return 0;
}

static int audio_write(struct st_device *dev, const void *data, unsigned int size)
{
	long i;
	const unsigned char *sp = data;
	unsigned char *dp = &audio_buf[0];;
	long len = size;

	if(flg_play == 0) {
		DKPRINTF(0x02, "S");
		if(len > bufsize) {
			len = bufsize;
		}
	} else {
		if(len > (bufsize/2)) {
			len = (bufsize/2);
		}
	}

	if(flg_buf_half != 1) {
		DKPRINTF(0x02, "F");
		dp = &audio_buf[bufsize/2];
	} else {
		DKPRINTF(0x02, "H");
		dp = &audio_buf[0];
	}

	for(i=0; i<len; i++) {
		if(dp >= &audio_buf[MAX_AUDIOBUF]) {
			SYSERR_PRINT("Write buffer over %d %ld\n", bufsize, size);
			break;
		}
		*dp = *sp;
		dp ++;
		sp ++;
	}

	// 音データの最後のはずなので、残りは"0"で埋める(暫定)
	if(len < (bufsize/2)) {
		while(dp < &audio_buf[bufsize]) {
			*dp = 0;
			dp ++;
		}
	}

	flg_buf_half = 0;

	return len;
}

static int audio_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	int rt = 0;

	switch(com) {
	case IOCMD_AUDIO_SET_VOLUME:
		DKFPRINTF(0x01, "IOCMD_AUDIO_SET_VOLUME %ld\n", arg);
		volume = arg;
		if(flg_play) {
			lock_ts();
			if(BSP_AUDIO_OUT_SetVolume(volume) == AUDIO_OK) {
				rt = 0;
			} else {
				rt = -1;
			}
			unlock_ts();
		}
		break;

	case IOCMD_AUDIO_GET_VOLUME:
		{
			long *p_volume = (long *)param;
			*p_volume = volume;
		}
		break;

	case IOCMD_AUDIO_GET_BUFSIZE:
		{
			long *p_bufsize = (long *)param;
			*p_bufsize = bufsize;
		}
		break;

	case IOCMD_AUDIO_SET_BUFSIZE:
		if(arg <= MAX_AUDIOBUF) {
			if(bufsize != arg) {
				flg_bs_chg = 1;
				bufsize = arg;
				//eprintf("bufsize = %d\n", bufsize);
			}
		}
		break;

	case IOCMD_AUDIO_PLAY_START:
		if(flg_play == 0) {
			flg_play = 1;
			//audio_init(volume, sampling_rate);
			if(flg_audio_start == 0) {
				//eprintf("bufsize = %d\n", bufsize);
				lock_ts();
				BSP_AUDIO_OUT_Play((uint16_t*)audio_buf, bufsize);
				unlock_ts();
				flg_audio_start = 1;
			} else {
				lock_ts();
				if(flg_bs_chg == 0) {
					BSP_AUDIO_OUT_Resume();
				} else {
					BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
					BSP_AUDIO_OUT_Play((uint16_t*)audio_buf, bufsize);
					BSP_AUDIO_OUT_Resume();
					flg_bs_chg = 0;
				}
				BSP_AUDIO_OUT_SetVolume(volume);
				unlock_ts();
			}
		}
		break;

	case IOCMD_AUDIO_PLAY_STOP:
		if(flg_play != 0) {
			int i;
			lock_ts();
			//BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
			BSP_AUDIO_OUT_Pause();
			unlock_ts();
			flg_play = 0;
			flg_buf_half = 0;
			for(i=0; i<MAX_AUDIOBUF; i++) {
				audio_buf[i] = 0;
			}
			event_wakeup(&tx_evq, 0);
			break;
		}
		break;

	case IOCMD_AUDIO_GET_STATUS:
		return flg_play;
		break;

	case IOCMD_AUDIO_SET_SMPRATE:
		{
			if(sampling_rate != arg) {
				sampling_rate = arg;
				lock_ts();
				BSP_AUDIO_OUT_SetFrequency(sampling_rate);
				unlock_ts();
			}
		}
		break;

	case IOCMD_AUDIO_GET_SMPRATE:
		{
			long *p_sampling_rate = (long *)param;
			*p_sampling_rate = sampling_rate;
		}
		break;

	case IOCMD_AUDIO_GET_BUFFER:
		{
			unsigned char **p_audio_buf = (unsigned char **)param;
			*p_audio_buf = audio_buf;
		}
		break;

	case IOCMD_AUDIO_WAIT_BUFFER:
		{
			unsigned char **p_audio_buf = (unsigned char **)param;
			event_wait(&tx_evq, 0, 0);
			if(flg_buf_half != 1) {
				DKPRINTF(0x02, "F");
				*p_audio_buf = &audio_buf[bufsize/2];
			} else {
				DKPRINTF(0x02, "H");
				*p_audio_buf = &audio_buf[0];
			}
			flg_buf_half = 0;
		}
		break;

	case IOCMD_AUDIO_SET_MUTE:
		{
			lock_ts();
			if(arg == 0) {
				BSP_AUDIO_OUT_SetMute(AUDIO_MUTE_OFF);
			} else {
				BSP_AUDIO_OUT_SetMute(AUDIO_MUTE_ON);
			}
			unlock_ts();
		}
		break;

	default:
		SYSERR_PRINT("Unknow command %08lX arg %08lX\n", com, arg);
		break;
	}

	return rt;
}

static int audio_select(struct st_device *dev, unsigned int timeout)
{
	if(flg_play == 0) {
		return (long)timeout;
	} else {
		return event_wait(&tx_evq, 0, timeout);
	}
}

const struct st_device audio_device = {
	.name		= DEF_DEV_NAME_AUDIO,
	.explan		= "STM32F769I-Disc Audio Out",
	.register_dev	= audio_register,
	.mutex		= &audio_mutex,
	.open		= audio_open,
	.close		= audio_close,
	.write		= audio_write,
	.ioctl		= audio_ioctl,
	.select		= audio_select,
};
