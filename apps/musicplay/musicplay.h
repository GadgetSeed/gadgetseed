/** @file
    @brief	音楽再生アプリケーション

    @date	2017.05.01
    @auther	Takashi SHUDO
*/

#ifndef MUSICPLAY_H
#define MUSICPLAY_H

#define VOL_DEF		 30

#define MODE_PLAY	0
#define MODE_ALBUM_SEL	1
#define MODE_MUSIC_SEL	2
extern int disp_mode;

#define MUSICPLAY_STAT_STOP	0
#define MUSICPLAY_STAT_PLAY	1
#define MUSICPLAY_STAT_PAUSE	2
extern int musicplay_status;

extern int play_album_num;
extern int play_track_num;
extern int play_file_num;
extern int flg_shuffle;

void set_play_file_num(void);
void analyze_now_music(void);
void start_music_play(void);
void ff_music_play(void);
void fr_music_play(void);
void stop_music_play(void);
void pause_music_play(void);
void continue_music_play(void);

#endif // MUSICPLAY_H
