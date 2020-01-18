/** @file
    @brief	AUDIOデバイス操作

    @date	2017.02.12
    @auther	Takashi SHUDO
*/

#include "soundio.h"
#include "soundfile.h"
#include "soundplay.h"
#include "spectrum_analyzer.h"

#include "tprintf.h"
#include "tkprintf.h"
#include "device.h"
#include "sysevent.h"
#include "device/audio_ioctl.h"

#define GSLOG_PREFIX	"SIO: "
#include "log.h"

#define SPIO_TIMEOUT	500

static const char audio_devname[] = "audio";
static struct st_device *audio_dev;
static int smp_rate = 48000;
static int now_volume = 20;
static int next_volume = 20;
int audio_buf_size = MAX_FILEBUF;

void init_soundio(void)
{
	audio_dev = open_device((char *)audio_devname);
	if(audio_dev == 0) {
		tprintf("Cannot open \"%s\"\n", audio_devname);
		return;
	}

	ioctl_device(audio_dev, IOCMD_AUDIO_GET_BUFSIZE, 0, (void *)&audio_buf_size);
	if(audio_buf_size > MAX_FILEBUF) {
		tprintf("Error AUDIO Buffer size = %d\n", audio_buf_size);
		return;
	}
	tprintf("AUDIO Buffer Size : %d\n", audio_buf_size);

	init_window_table();
}

int soundio_get_volume(void)
{
	return now_volume;
}

void soundio_set_volume(int vol)
{
	next_volume = vol;
}

void soundio_mixer_proc(void)
{
	if(now_volume != next_volume) {
		now_volume = next_volume;
		if(lock_device(audio_dev, SPIO_TIMEOUT) < 0) {
			GSLOG(0, "audio ctrl lock timeout\n");
		}
		ioctl_device(audio_dev, IOCMD_AUDIO_SET_VOLUME, now_volume, 0);
		unlock_device(audio_dev);
		create_event(EVT_SOUND_VOLUME, now_volume, 0);
	}
}

void soundio_prepared_sound(void)
{
	GSLOG(0, "[soundio] PREPARED\n");

	create_event(EVT_SOUND_PREPARED, 0, (void *)&music_info);
}

void soundio_start_sound(void)
{
	GSLOG(0, "[soundio] START\n");

	ioctl_device(audio_dev, IOCMD_AUDIO_PLAY_START, 0, 0);

	create_event(EVT_SOUND_START, 0, (void *)&music_info);
}

void soundio_end_sound(void)
{
	GSLOG(0, "[soundio] END\n");

	ioctl_device(audio_dev, IOCMD_AUDIO_PLAY_STOP, 0, 0);

	create_event(EVT_SOUND_END, 0, 0);
}

void soundio_stop_sound(void)
{
	GSLOG(0, "[soundio] STOP\n");

	ioctl_device(audio_dev, IOCMD_AUDIO_PLAY_STOP, 0, 0);

	create_event(EVT_SOUND_STOP, 0, 0);
}

void soundio_pause_sound(void)
{
	GSLOG(0, "[soundio] PAUSE\n");

	ioctl_device(audio_dev, IOCMD_AUDIO_PLAY_STOP, 0, 0);

	create_event(EVT_SOUND_PAUSE, 0, (void *)&music_info);
}

int soundio_set_smprate(int rate)
{
	int rtn = 0;

	smp_rate = rate;

	rtn =  ioctl_device(audio_dev, IOCMD_AUDIO_SET_SMPRATE, smp_rate, 0);

	return rtn;
}

int soundio_set_audiobuf_size(int size)
{
	return ioctl_device(audio_dev, IOCMD_AUDIO_SET_BUFSIZE, size, 0);
}

static struct st_audio_spectrum asp;

int soundio_write_audiobuf(unsigned char *buf, int size)
{
	int rtn;

	rtn = select_device(audio_dev, SPIO_TIMEOUT);
	if(rtn < 0) {
		tkprintf("Audio Decode & Play time too long(%d).\n", rtn);
	}

	if(lock_device(audio_dev, SPIO_TIMEOUT) == 0) {
		tkprintf("audio buf write lock timeout\n");
	}

	rtn = write_device(audio_dev, buf, size);
	unlock_device(audio_dev);

	proc_spectrum_analyze(&(asp.spectrum[0]), (short *)buf);
	proc_spectrum_analyze(&(asp.spectrum[SPA_ANA_SMP]), (short *)(buf+2));
	asp.frame_num = audio_frame_count;

	create_event(EVT_SOUND_ANALYZE, 0, (void *)&asp);

	return rtn;
}

void soundio_wait_audiobuf(unsigned char **p_buf)
{
	unsigned char *p_abuf;

	ioctl_device(audio_dev, IOCMD_AUDIO_WAIT_BUFFER, 0, (void *)&p_abuf);

	*p_buf = p_abuf;
}
