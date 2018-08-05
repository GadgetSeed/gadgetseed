/** @file
    @brief	システムイベント

    @date	2017.11.25
    @date	2007.05.05
    @author	Takashi SHUDO
*/

#ifndef SYSEVENT_H
#define SYSEVENT_H

struct st_sysevent {
	unsigned short what;		///< イベント種類
	unsigned short arg;		///< イベント引数
	unsigned short pos_x;		///< イベント発生X座標
	unsigned short pos_y;		///< イベント発生Y座標
	unsigned long long when;	///< イベントが発生したカーネル時間
	void *private_data;
};	///< システムイベント

/*
  イベント定義
*/
// event.what
#define EVT_NULL		0	///< イベント無し

#define EVT_KEYDOWN		1	///< キーを押した
#define EVT_KEYUP		2	///< キーを離した
#define EVT_KEYDOWN_REPEAT	3	///< キーを押した(リピート)

#define EVT_TOUCHSTART		4	///< (画面に)タッチした
#define EVT_TOUCHMOVE		5	///< (画面に)タッチしたまま動かした
#define EVT_TOUCHEND		6	///< (画面に)タッチした状態から離した

#define EVT_POWEROFF		16	///< 電源Off

#define EVT_SOUND_PREPARE	47
#define EVT_SOUND_START		48
#define EVT_SOUND_PAUSE		49
#define EVT_SOUND_CONTINUE	50
#define EVT_SOUND_END		51
#define EVT_SOUND_STOP		52
#define EVT_SOUND_STATUS	53
#define EVT_SOUND_VOLUME	54
#define EVT_SOUND_ANALYZE	55

extern void init_event(void);
extern int set_event(struct st_sysevent *event);
extern int create_event(unsigned short what, unsigned short arg, void *private_data);
extern int get_event(struct st_sysevent *event, unsigned int timeout);
extern void push_event_interrupt(void *sp, struct st_sysevent *event);
extern void set_event_interrupt(void *sp);

#endif /* SYSEVENT_H */
