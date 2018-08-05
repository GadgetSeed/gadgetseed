/** @file
    @brief	MP3ファイル再生

    @date	2017.03.26
    @auther	Takashi SHUDO
*/

#include "tprintf.h"
#include "shell.h"
#include "file.h"
#include "str.h"
#include "ff.h"
#include "soundplay.h"
#include "id3tag.h"
#include "font.h"
#include "graphics.h"
#include "mad.h"
#include "mp3play.h"
#include "task/syscall.h"
#include "charcode.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


static struct mad_decoder mad_dec;
extern struct st_music_info music_info;
#define MP3BUF_MARGINE	8

static int flg_abort = 0;

static enum mad_flow input(void *data,
			   struct mad_stream *stream)
{
	int rtn = 0;
	int trtn = 0;
	int stsize = 0;
	int i;

	//tprintf(".");

	soundplay_mixer_proc();

	switch(next_soundplay_status) {
	case SOUND_PLAY:
		next_soundplay_status = -1;
		soundplay_status = SOUND_PLAY;
		break;

	case SOUND_STOP:
		next_soundplay_status = -1;
		flg_abort = 1;
		soundplay_status = SOUND_STOP;
		return MAD_FLOW_STOP;
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
		return MAD_FLOW_CONTINUE;
	}

	if(audio_frame_count == 0) {
		rtn = soundplay_readfile(file_buf, MPEG_FRAME_HEADER_SIZE);
		rtn = mpeg_frame_header_decode(&music_info, file_buf);
		if(rtn != 0) {
			DTPRINTF(0x01, "Frame Lost\n");
			XDUMP(0x02, file_buf, 16);
			//while(1);
		}
		DTPRINTF(0x01, "%4d : Frame Size : %d\n", audio_frame_count, music_info.frame_length);
		rtn = soundplay_readfile(&file_buf[MPEG_FRAME_HEADER_SIZE],
					 music_info.frame_length - MPEG_FRAME_HEADER_SIZE + MP3BUF_MARGINE);
		XDUMP(0x02, file_buf, 16/*music_info.frame_length*/);
		if(rtn <= 0) {
			soundplay_status = SOUND_STOP;
			return MAD_FLOW_STOP;
		}
		stsize = MPEG_FRAME_HEADER_SIZE + rtn;
		DTPRINTF(0x01, "%4d : Streem Size : %d\n", audio_frame_count, stsize);
		mad_stream_buffer(stream, file_buf, stsize);
	} else {
#ifdef STOP_TEST
		if(audio_frame_count > 20) {
			//while(1);
			soundplay_status = SOUND_STOP;
			return MAD_FLOW_STOP;
		}
#endif
		for(i=0; i<MP3BUF_MARGINE; i++) {
			file_buf[i] = file_buf[music_info.frame_length + i];
		}
		rtn = mpeg_frame_header_decode(&music_info, file_buf);
		if(rtn != 0) {
			DTPRINTF(0x01, "Frame Lost\n");
			XDUMP(0x02, file_buf, 16);
			soundplay_status = SOUND_STOP;
			return MAD_FLOW_STOP;
		}
		DTPRINTF(0x01, "%4d : Frame Size : %d\n", audio_frame_count, music_info.frame_length);
		rtn = soundplay_readfile(&file_buf[MP3BUF_MARGINE], music_info.frame_length);
		XDUMP(0x02, file_buf, 16/*music_info.frame_length*/);
		if(rtn <= 0) {
			soundplay_status = SOUND_STOP;
			return MAD_FLOW_STOP;
		}

		// 最終フレームチェック
		trtn = mpeg_frame_header_check(&file_buf[music_info.frame_length]);
		if(trtn != 0) {
			DTPRINTF(0x01, "Last Frame(%d).\n", audio_frame_count);
			stsize = rtn;
		} else {
			stsize = rtn + MP3BUF_MARGINE;
		}

		DTPRINTF(0x01, "%4d : Streem Size : %d\n", audio_frame_count, stsize);
		mad_stream_buffer(stream, file_buf, stsize);
	}

	return MAD_FLOW_CONTINUE;
}


static inline signed int scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

#define WORD_BUF
#ifdef WORD_BUF
static unsigned short audio_buf[1152*2];
#else
static char audio_buf[1152*4];
#endif

static enum mad_flow output(void *data,
			    struct mad_header const *header,
			    struct mad_pcm *pcm)
{
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;
	int i = 0;

	if(soundplay_status == SOUND_STOP) {
		flg_abort = 1;
		return MAD_FLOW_STOP;
	}

	nchannels = pcm->channels;
	nsamples  = pcm->length;
	left_ch   = pcm->samples[0];
#ifdef GSC_TARGET_SYSTEM_EMU
	right_ch  = pcm->samples[0];	// モノラル(左のみ)
#else
	right_ch  = pcm->samples[1];
#endif

	DTPRINTF(0x01, "CH:%d, LEN:%d\n", (int)nchannels, (int)nsamples);

	if(nchannels == 2) {
		for(i=0; i<nsamples; i++) {
#ifdef WORD_BUF
			audio_buf[i*2+0] = scale(*left_ch);
			audio_buf[i*2+1] = scale(*right_ch);
			left_ch ++;
			right_ch ++;
#ifdef GSC_TARGET_SYSTEM_EMU
			/*
			  何故かemu(Linux)だとMP3(libmad)のデコード結果の音程が低い
			  暫定的な対策(右チャンネルはノイジー)
			 */
			left_ch ++;
			right_ch ++;
#endif
#else
			signed int sample;
			sample = scale(*left_ch++);
			audio_buf[i*4 + 0] = ((sample >> 0) & 0xff);
			audio_buf[i*4 + 1] = ((sample >> 8) & 0xff);
			sample = scale(*right_ch++);
			audio_buf[i*4 + 2] = ((sample >> 0) & 0xff);
			audio_buf[i*4 + 3] = ((sample >> 8) & 0xff);
#endif
		}
	}

	if(audio_frame_count == 1) {
		soundplay_set_audiobuf_size(nsamples * 2 * 2 * 2);
		soundplay_set_smprate(music_info.sampling_rate);
	}

	soundplay_write_audiobuf((unsigned char *)audio_buf, nsamples * 2 * 2);

	if(audio_frame_count == 2) {
		soundplay_start_sound();
	}

	audio_play_time = calc_play_time(&music_info, audio_frame_count);
	disp_play_time(audio_play_time);

#if 0 // TODO
	if(next_audio_frame_count < 0) {
		audio_frame_count ++;
	} else {
		audio_frame_count = next_audio_frame_count;
		next_audio_frame_count = -1;;
	}
#else
	audio_frame_count ++;
#endif
	return MAD_FLOW_CONTINUE;
}

static enum mad_flow error(void *data,
			   struct mad_stream *stream,
			   struct mad_frame *frame)
{
	tprintf("Frame %d : decoding error 0x%04x (%s) at byte offset %lX\n",
		audio_frame_count,
		stream->error, mad_stream_errorstr(stream),
		(unsigned long)stream->this_frame);

	xdump(data, 16/*music_info.frame_length*/);

	return MAD_FLOW_CONTINUE;
}

static int mp3_play_proc(void)
{
	int rtn = 0;

	audio_frame_count = 0;
	rtn = mad_decoder_run(&mad_dec, MAD_DECODER_MODE_SYNC);
	DTPRINTF(0x01, "mad_decoder_run = %d\n", rtn);
	(void)rtn;
	mad_decoder_finish(&mad_dec);

	if(flg_abort != 0) {
		soundplay_stop_sound();
	} else {
		soundplay_end_sound();
	}

	return 1;
}

int mp3file_analyze(struct st_music_info *info, unsigned char *fname)
{
	int rtn = 0;

	rtn = soundplay_openfile(sj2utf8(fname));
	if(rtn < 0) {
		return -1;
	}

	rtn = id3tag_decode(info, soundplay_readfile, soundplay_seekfile);

	soundplay_closefile();

	return rtn;
}

static int mp3_analyze(int argc, uchar *argv[])
{
	int rtn = 0;

	if(argc < 2) {
		tprintf("Usage: mp3 <filename>\n");
		return 0;
	}

	rtn = mp3file_analyze(&music_info, (unsigned char *)argv[1]);

	if(rtn > 0) {
		disp_music_info(&music_info);
		soundplay_prepare_sound();
	}

	return rtn;
}

const struct st_shell_command com_mp3_analyze = {
	"mp3", 0, mp3_analyze, 0, "Analyze MP3 Data", 0
};

static int mp3_play(int argc, uchar *argv[])
{
	unsigned char *fname;
	int rtn = 0;

	if(argc < 2) {
		tprintf("Usage: mp3 <filename>\n");
		return 0;
	}

	if(soundplay_status != SOUND_STOP) {
		soundplay_stop_play();
	}

	flg_abort = 0;
	soundplay_init_time();

	fname = (unsigned char *)argv[1];
	sjisstr_to_utf8str(cname, fname, FF_MAX_LFN);
	rtn = soundplay_openfile(fname);
	if(rtn < 0) {
		tprintf("Cannot open \"%s\"\n", (char *)cname);
		return 0;
	}

	id3tag_decode(&music_info, soundplay_readfile, soundplay_seekfile);
	disp_music_info(&music_info);

	//set_draw_mode(GRP_DRAWMODE_NORMAL);
	//draw_str(3, 3, music_info.title);

	mad_decoder_init(&mad_dec, file_buf,
			 input, 0 /* header */, 0 /* filter */, output,
			 error, 0 /* message */);

	tprintf("Start Play \"%s\"\n", cname);

	soundplay_start_proc(mp3_play_proc);

	return 0;
}

const struct st_shell_command com_mp3_play = {
	"mp3", 0, mp3_play, 0, "Play MP3 Data", 0
};
