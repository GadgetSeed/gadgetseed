/** @file
    @brief	仮想 AUDIO ドライバ

    @date	2012.11.02
    @authoer	Takashi SHUDO

    sudo apt-get install libasound2-dev
*/

#include <alsa/asoundlib.h>
#include <unistd.h>

#include "device.h"
#include "task/syscall.h"
#include "device/audio_ioctl.h"
#include "tkprintf.h"

//#define DEBUG
#ifdef DEBUG
#define VSPRINTF	tkprintf
#else
#define VSPRINTF(x, ...)
#endif

#define MAX_AUDIOBUF	(1152*8)
static int flg_play = 0;
static int volume = 50;
static int bufsize = MAX_AUDIOBUF;
static int sampling_rate = 44100;

static snd_pcm_t *hndl = NULL;
static snd_pcm_hw_params_t *hw_params;

static int set_vaudio_param(void)
{
	int ret;

	ret = snd_pcm_set_params(hndl,
				 SND_PCM_FORMAT_S16,
				 SND_PCM_ACCESS_RW_INTERLEAVED,
				 2	/* Channels */,
				 sampling_rate,
				 1,	/* resample */
				 100000	/* latency */
				 );
	if(ret != 0) {
		SYSERR_PRINT("Unable to set format\n");
		return ret;
	}

	return 0;
}

snd_mixer_t *handle;
snd_mixer_elem_t *elem;
long min, max;

static void setup_mixer(void)
{
	snd_mixer_selem_id_t *sid;
	const char *card = "default";

	snd_mixer_open(&handle, 0);
	snd_mixer_attach(handle, card);
	snd_mixer_selem_register(handle, NULL, NULL);
	snd_mixer_load(handle);

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, "Master");

	elem = snd_mixer_find_selem(handle, sid);

	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	VSPRINTF("MIXER Volume Range %ld - %ld\n", min, max);
}

static void close_mixer(void)
{
	snd_mixer_close(handle);
}

static void set_volume(int vol)
{
	int vol_val = vol * max / 100;
	int rtn = 0;

	VSPRINTF("VOLUME %d -> %d\n", vol, vol_val);

	/*
	  write()実行中に以下を実行すると固まる場合がある
	*/
	rtn = snd_mixer_selem_set_playback_volume_all(elem, vol_val);
	if(rtn !=0 ) {
		SYSERR_PRINT("snd_mixer_selem_set_playback_volume_all error %d\n", rtn);
	}
}

static int vaudio_register(struct st_device *dev, char *param)
{
	char *device = "default";
	int ret;

	VSPRINTF("vaudio_open\n");

	block_timer_interrupt();

	ret = snd_pcm_open(&hndl, device, SND_PCM_STREAM_PLAYBACK, 0);
	if(ret != 0) {
		SYSERR_PRINT("Unable to open PCM\n");
		return ret;
	}

	if((ret = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
		SYSERR_PRINT("cannot allocate hardware parameter structure (%s)\n",
			     snd_strerror(ret));
		return ret;
	}

	if((ret = snd_pcm_hw_params_any(hndl, hw_params)) < 0) {
		SYSERR_PRINT("cannot initialize hardware parameter structure (%s)\n",
			     snd_strerror(ret));
		return ret;
	}

	ret = set_vaudio_param();

	setup_mixer();

	unblock_timer_interrupt();

	return 0;
}

static int vaudio_unregister(struct st_device *dev)
{
	VSPRINTF("vaudio_close\n");

	close_mixer();

	snd_pcm_drain(hndl);

	if(hndl != NULL) {
		snd_pcm_close(hndl);
		hndl = NULL;
	}

	return 0;
}

static int vaudio_open(struct st_device *dev)
{
	return 0;
}

static int vaudio_close(struct st_device *dev)
{
	snd_pcm_drain(hndl);

	return 0;
}

void start_vtimer(unsigned long inttime);

static int vaudio_write(struct st_device *dev, const void *data, unsigned int size)
{
	int ret;

	VSPRINTF("vaudio_write %l\n", size);

	ret = snd_pcm_writei(hndl, (const void*)data, size/4);
	if(ret < 0) {
		if(snd_pcm_recover(hndl, ret, 0) < 0) {
			SYSERR_PRINT("Unable to recover Stream.");
			return ret;
		}
	}

#if 0
	static int va_count = 0;
	va_count ++;
	if((va_count % 16) == 0) {
		/*
		  これを入れると音声再生中でもeventとれる
		  但し ALSA unterrun 発生する場合あり
		*/
		task_sleep(10);
	}
#endif

	return ret;
}

static int vaudio_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	switch(com) {
	case IOCMD_AUDIO_SET_VOLUME:
		volume = arg;
		set_volume(volume);
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
			VSPRINTF("GET_BUFSIZE %d\n", bufsize);
		}
		break;

	case IOCMD_AUDIO_SET_BUFSIZE:
		if(arg <= MAX_AUDIOBUF) {
			bufsize = arg;
#if 0
			int ret = snd_pcm_hw_params_set_buffer_size(
					hndl, hw_params,
					bufsize);
			if(ret < 0) {
				SYSERR_PRINT("cannot set buffer size (%s)\n",
					     snd_strerror(ret));
			}
#endif
			VSPRINTF("SET_BUFSIZE %d\n", bufsize);
		}
		break;

	case IOCMD_AUDIO_PLAY_START:
		if(flg_play == 0) {
			flg_play = 1;
		}
		break;

	case IOCMD_AUDIO_PLAY_STOP:
		if(flg_play != 0) {
			flg_play = 0;
		}
		break;

	case IOCMD_AUDIO_GET_STATUS:
		return flg_play;
		break;

	case IOCMD_AUDIO_SET_SMPRATE:
		{
			if(sampling_rate != arg) {
				sampling_rate = arg;
				VSPRINTF("SET_SMPRATE %d\n", sampling_rate);
				return set_vaudio_param();
			}
		}
		break;

	case IOCMD_AUDIO_GET_SMPRATE:
		{
			long *p_sampling_rate = (long *)param;
			*p_sampling_rate = sampling_rate;
		}
		break;

#if 0
	case IOCMD_AUDIO_GET_BUFFER:
		{
			unsigned char **p_audio_buf = (unsigned char **)param;
			*p_audio_buf = audio_buf;
		}
		break;
#endif

	case IOCMD_AUDIO_WAIT_BUFFER:
		{
			VSPRINTF("WAIT_BUFFER\n");
			snd_pcm_drain(hndl);
		}
		break;

	default:
		SYSERR_PRINT("Unknow command %08X arg %08X\n", com, arg);
		break;
	}

	return 0;
}

static int vaudio_select(struct st_device *dev, unsigned int timeout)
{
	return 0;
}

const struct st_device vaudio_device = {
	.name		= DEF_DEV_NAME_AUDIO,
	.explan		= "EMU audio",
	.register_dev	= vaudio_register,
	.unregister_dev	= vaudio_unregister,
	.open		= vaudio_open,
	.close		= vaudio_close,
	.write		= vaudio_write,
	.ioctl		= vaudio_ioctl,
	.select		= vaudio_select,
};
