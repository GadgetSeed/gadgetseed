/** @file
    @brief	音楽情報データ

    @date	2017.03.30
    @auther	Takashi SHUDO
*/

#include "music_info.h"
#include "memory.h"
#include "tprintf.h"
#include "sysevent.h"
#include "str.h"
#include "charcode.h"

void init_music_info(struct st_music_info *info)
{
	info->title[0] = 0;
	info->artist[0] = 0;
	info->album[0] = 0;
	info->format = MUSIC_FMT_UNKNOWN;
	info->track = 0;
	info->last_track = 0;
	info->bit_rate = 0;
	info->sampling_rate = 44100;
	info->mpeg_padding = 0;
	info->channel = 2;
	info->frame_size = 0;
	info->sample_count = 0;
	info->time_length = 0;
	if(info->sample_size_data != 0) {
		free_memory(info->sample_size_data);
		info->sample_size_data = 0;
	}
	info->flg_have_artwork = 0;
}

void disp_music_info(struct st_music_info *info)
{
	tprintf("Title  : %s\n", (char *)(info->title));
	tprintf("Artist : %s\n", (char *)(info->artist));
	tprintf("Album  : %s\n", (char *)(info->album));
	tprintf("Track  : %d/%d\n", (int)info->track, (int)info->last_track);
	tprintf("BitRate: %d\n", info->bit_rate);
	tprintf("Rate   : %d\(Hz)\n", info->sampling_rate);
	tprintf("Frame  : %d\n", info->frame_size);
	tprintf("Count  : %d\n", info->sample_count);
	tprintf("Time   : %d(ms) %2d:%02d:%02d.%03d\n", info->time_length,
		(info->time_length / 1000 / 60 / 60),	// H
		(info->time_length / 1000 / 60) % 60,	// M
		(info->time_length / 1000) % 60,	// S
		(info->time_length % 1000));	// ms
}

unsigned int calc_play_time(struct st_music_info *info, unsigned int frame_count)
{
	return (unsigned int)(((unsigned long long)frame_count * info->frame_size * 1000) / info->sampling_rate);
}

void disp_play_time(unsigned int play_time)
{
	static unsigned int lptime = 0;
	static unsigned int nptime = 0;

	nptime = play_time / 1000;

#if 0
	tprintf("\r%3d:%02d.%03d",
		(play_time / 1000 / 60),// M
		(play_time / 1000) % 60,// S
		(play_time % 1000));	// ms
#endif

	if(lptime != nptime) {
		create_event(EVT_SOUND_STATUS, 0, (void *)&nptime);
		lptime = nptime;
	}
}
