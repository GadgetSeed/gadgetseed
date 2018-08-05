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

static int stream_start_pos;

static unsigned int sample_size(int pos)
{
	unsigned int ssize =
			((int)music_info.sample_size_data[pos*4 + 0] << 24) +
			((int)music_info.sample_size_data[pos*4 + 1] << 16) +
			((int)music_info.sample_size_data[pos*4 + 2] <<  8) +
			((int)music_info.sample_size_data[pos*4 + 3] <<  0);

	return ssize;
}

static int m4a_play_proc(void)
{
	int rtn = 0;
	unsigned long samplerate;
	unsigned char channels;
	void *samplebuffer;
	NeAACDecFrameInfo hInfo;
	int i;
	int flg_abort = 0;
	static unsigned char dbuf[4] = { 0x12, 0x10, 0, 0 }; // "\022\020"

	stream_start_pos = soundplay_tellfile();
	tprintf("M4A file stream start position : %d\n", stream_start_pos);

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
	char err = NeAACDecInit2(hAac, dbuf, 2, &samplerate,
				 &channels);
	if(err != 0) {
		//
		// Handle error
		//
		SYSERR_PRINT("NeAACDecInit2 Error %d\n", err);
	}

	// Loop until decoding finished
	for(i=0; i<music_info.sample_count; i++) {
		soundplay_mixer_proc();

		switch(next_soundplay_status) {
		case SOUND_PLAY:
			next_soundplay_status = -1;
			soundplay_status = SOUND_PLAY;
			break;

		case SOUND_STOP:
			next_soundplay_status = -1;
			DTPRINTF(0x01, "m4aplay force stop\n");
			flg_abort = 1;
			goto end;
			break;

		case SOUND_PAUSE:
			next_soundplay_status = -1;
			soundplay_status = SOUND_PAUSE;
			break;

		default:
			break;
		}

		if(soundplay_status == SOUND_PAUSE) {
			task_sleep(10);
			i --;
			continue;
		}

		//
		// Put next frame in buffer
		//
		unsigned long ssize = sample_size(i);
		rtn = soundplay_readfile(file_buf, ssize);
		if(rtn != ssize) {
			tprintf("m4aplay read file error(ssize=%ld, rtn=%d)\n", ssize, rtn);
			goto end;
		}
		DTPRINTF(0x01, "%6d %4ld ", i, ssize);
		XDUMP(0x02, file_buf, 16);

		// Decode the frame in buffer
		samplebuffer = NeAACDecDecode(hAac, &hInfo, file_buf, ssize);
		if((hInfo.error == 0) && (hInfo.samples > 0)) {
			//
			// do what you need to do with the decoded samples
			//
			DTPRINTF(0x01, "AAC Decode Success %ld\n", hInfo.samples);
			XDUMP(0x02, samplebuffer, 16);
			DTPRINTF(0x01, "%6d %4ld\n", i, ssize);
			if(i == 1) {
				soundplay_set_audiobuf_size(hInfo.samples * 2 * 2);
				soundplay_set_smprate(music_info.sampling_rate);
			}
			soundplay_write_audiobuf((unsigned char *)samplebuffer, hInfo.samples * 2);
			if(i == 2) {
				soundplay_start_sound();
			}
		} else if(hInfo.error != 0) {
			//
			// Some error occurred while decoding this frame
			//
			eprintf("AAC Decode Error %d\n", hInfo.error);
		}

		audio_play_time = calc_play_time(&music_info, audio_frame_count);
		disp_play_time(audio_play_time);

		if(next_audio_frame_count < 0) {
			audio_frame_count ++;
		} else {
			audio_frame_count = next_audio_frame_count;
			i = audio_frame_count - 1;
			{
				int j;
				long new_pos = stream_start_pos;
				for(j=0; j<audio_frame_count; j++) {
					unsigned long ssize = sample_size(j);
					new_pos += ssize;
				}
				soundplay_seeksetfile(new_pos);
			}
			next_audio_frame_count = -1;;
		}
	};
end:

	NeAACDecClose(hAac);

	mp4tag_dispose(&music_info);

	soundplay_status = SOUND_STOP;

	if(flg_abort != 0) {
		soundplay_stop_sound();
	} else {
		soundplay_end_sound();
	}

	return 1;
}

int m4afile_analyze(struct st_music_info *info, unsigned char *fname)
{
	int rtn = 0;

	rtn = soundplay_openfile(fname);
	if(rtn < 0) {
		return -1;
	}

	rtn = mp4tag_decode(info, soundplay_readfile, soundplay_seekfile);

	soundplay_closefile();

	return rtn;
}

static int m4a_analyze(int argc, uchar *argv[])
{
	int rtn = 0;

	if(argc < 2) {
		tprintf("Usage: m4a <filename>\n");
		return 0;
	}

	rtn = m4afile_analyze(&music_info, (unsigned char *)argv[1]);

	if(rtn > 0) {
		disp_music_info(&music_info);
		soundplay_prepare_sound();
	}

	return rtn;
}

const struct st_shell_command com_m4a_analyze = {
	"m4a", 0, m4a_analyze, 0, "Analyze MA4 Data", 0
};

static int m4a_play(int argc, uchar *argv[])
{
	unsigned char *fname;
	int rtn = 0;

	if(argc < 2) {
		tprintf("Usage: m4a <filename>\n");
		return 0;
	}

	if(soundplay_status != SOUND_STOP) {
		soundplay_stop_play();
	}

	soundplay_init_time();

	fname = (unsigned char *)argv[1];
	sjisstr_to_utf8str(cname, fname, FF_MAX_LFN);
	rtn = soundplay_openfile(fname);
	if(rtn < 0) {
		tprintf("Cannot open \"%s\"\n", cname);
		return 0;
	}

	mp4tag_decode(&music_info, soundplay_readfile, soundplay_seekfile);
	disp_music_info(&music_info);

	//set_draw_mode(GRP_DRAWMODE_NORMAL);
	//draw_str(3, 3, music_info.title);

	tprintf("Start Play \"%s\"\n", cname);

	soundplay_start_proc(m4a_play_proc);

	return 0;
}

const struct st_shell_command com_m4a_play = {
	"m4a", 0, m4a_play, 0, "Play M4A(AAC) Data", 0
};
