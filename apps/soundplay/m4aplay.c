/** @file
    @brief	M4A(AAC)ファイル再生

    @date	2017.04.08
    @auther	Takashi SHUDO
*/

#include "tkprintf.h"
#include "tprintf.h"
#include "shell.h"
#include "file.h"
#include "str.h"
#include "ff.h"
#include "soundplay.h"
#include "soundio.h"
#include "soundfile.h"
#include "mp4tag.h"
#include "font.h"
#include "graphics.h"
#include "mad.h"
#include "neaacdec.h"
#include "sysevent.h"
#include "task/syscall.h"
#include "m4aplay.h"
#include "charcode.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


extern struct st_music_info music_info;

static unsigned int sample_size(int pos)
{
	unsigned int ssize = 0;

	if(pos < 0) {
		ssize = 0;
	} else if(pos > music_info.sample_count) {
		ssize = 0;
	} else {
		ssize = ((int)music_info.sample_size_data[pos*4 + 0] << 24) +
				((int)music_info.sample_size_data[pos*4 + 1] << 16) +
				((int)music_info.sample_size_data[pos*4 + 2] <<  8) +
				((int)music_info.sample_size_data[pos*4 + 3] <<  0);
	}

	return ssize;
}

void m4afile_seek(int pos)
{
	int j;
	int new_pos = music_info.frame_start;

	for(j=0; j<pos; j++) {
		unsigned int ssize = sample_size(j);
		new_pos += ssize;
	}

	audio_frame_count = pos;

	soundfile_seekset(new_pos);
}

static int m4a_play_proc(void)
{
	int rtn = 0;
	unsigned long samplerate;
	unsigned char channels;
	void *samplebuffer;
	NeAACDecFrameInfo hInfo;
	int buf_count = 0;
	static unsigned char dbuf[4] = { 0x12, 0x10, 0, 0 }; // "\022\020"

	unsigned long cap = NeAACDecGetCapabilities();
	// Check if decoder has the needed capabilities
	if(cap & FIXED_POINT_CAP) {
		DTPRINTF(0x01, " Fixed point version\n");
	} else {
		DTPRINTF(0x01, " Floating point version\n");
	}

	// Open the library
	NeAACDecHandle hAac = NeAACDecOpen();

	// Get the current config
	NeAACDecConfigurationPtr conf = NeAACDecGetCurrentConfiguration(hAac);

	//
	// If needed change some of the values in conf
	//

	// Set the new configuration
	NeAACDecSetConfiguration(hAac, conf);

	// Initialise the library using one of the initialization functions
	char err = NeAACDecInit2(hAac, dbuf, 2, &samplerate, &channels);
	if(err != 0) {
		//
		// Handle error
		//
		SYSERR_PRINT("NeAACDecInit2 Error %d\n", err);
	}

	// Loop until decoding finished
	for( ;audio_frame_count<music_info.sample_count; audio_frame_count++) {
		int rt;
		unsigned char event;

		soundio_mixer_proc();

		rt = read_fifo(&soundplay_event, &event, 1);
		if(rt == 0) {
			goto next;
		}

		switch(event) {
		case SOUND_EVENT_PLAY:
			soundplay_status = SOUND_STAT_PLAYING;
			break;

		case SOUND_EVENT_STOP:
			soundio_stop_sound();
			DTPRINTF(0x01, "m4aplay force stop\n");
			goto end;
			break;

		case SOUND_EVENT_PAUSE:
			soundplay_status = SOUND_STAT_READY;
			buf_count = 0;
			soundio_pause_sound();
			break;

		default:
			break;
		}
	next:

		if(soundplay_status == SOUND_STAT_READY) {
			task_sleep(10);
			audio_frame_count --;
			continue;
		}

		if(next_audio_frame_count >= 0) {
			m4afile_seek(next_audio_frame_count);
			next_audio_frame_count = -1;
		}

		//
		// Put next frame in buffer
		//
		unsigned long ssize = sample_size(audio_frame_count);
		rtn = soundfile_read(comp_audio_data, ssize);
		if(rtn != ssize) {
			eprintf("m4aplay read file error(ssize=%ld, rtn=%d)\n", ssize, rtn);
			soundio_end_sound();
			goto end;
		}
		DTPRINTF(0x01, "%6d %4ld ", audio_frame_count, ssize);
		XDUMP(0x02, comp_audio_data, 16);

		// Decode the frame in buffer
		samplebuffer = NeAACDecDecode(hAac, &hInfo, comp_audio_data, ssize);
		if((hInfo.error == 0) && (hInfo.samples > 0)) {
			//
			// do what you need to do with the decoded samples
			//
			DTPRINTF(0x01, "AAC Decode Success %ld\n", hInfo.samples);
			XDUMP(0x02, samplebuffer, 16);
			DTPRINTF(0x01, "%6d %4ld\n", audio_frame_count, ssize);
			soundio_write_audiobuf((unsigned char *)samplebuffer, hInfo.samples * 2);
			buf_count ++;
			if(buf_count == 1) {
				soundio_set_audiobuf_size(hInfo.samples * 2 * 2);
				soundio_set_smprate(music_info.sampling_rate);
			} else if(buf_count == 2) {
				soundio_start_sound();
			}
		} else if(hInfo.error != 0) {
			//
			// Some error occurred while decoding this frame
			//
			eprintf("AAC Decode Error %d\n", hInfo.error);
			task_sleep(10);
		}

		audio_play_time = calc_play_time(&music_info, audio_frame_count);
		disp_play_time(audio_play_time);
	};

	soundio_end_sound();
end:

	NeAACDecClose(hAac);

	mp4tag_dispose(&music_info);

	soundplay_status = SOUND_STAT_END;

	return 1;
}

int m4afile_open(uchar *fname)
{
	int rtn = 0;

	rtn = soundfile_open(fname);
	if(rtn < 0) {
		return -1;
	}

	audio_frame_count = 0;
	next_audio_frame_count = -1;

	rtn = mp4tag_decode(&music_info, soundfile_read, soundfile_seekcur, soundfile_tell);

	if(rtn > 0) {
		tprintf("M4A file stream start position : %d\n", music_info.frame_start);

		disp_music_info(&music_info);
		soundplay_status = SOUND_STAT_READY;
		soundio_prepared_sound();
		soundplay_start_proc(m4a_play_proc);
	}

	return rtn;
}

static int m4a_open(int argc, uchar *argv[])
{
	int rtn = 0;

	if(argc < 2) {
		tprintf("Usage: m4a <filename>\n");
		return 0;
	}

	rtn = m4afile_open((uchar *)argv[1]);

	return rtn;
}

const struct st_shell_command com_m4a_open = {
	"m4a", 0, m4a_open, 0, "Open MA4 File", 0
};
