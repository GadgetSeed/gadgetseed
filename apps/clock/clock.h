/** @file
    @brief	時計アプリケーション

    @date	2017.05.27
    @author	Takashi SHUDO
*/

#ifndef CLOCK_H
#define CLOCK_H

#define BACK_COLOR	RGB(50,50,50)
#define FORE_COLOR	RGB(220,220,220)
#define ACT_BACK_COLOR	RGB(150,150,150)
#define ACT_FORE_COLOR	RGB(250,250,250)
#define CUR_BACK_COLOR	RGB(20,20,20)
#define CUR_FORE_COLOR	RGB(10,210,210)

#define MODE_CLOCK	0
#define MODE_SETTING	1

extern int disp_mode;

void draw_clock_view(void);
void draw_clock(int flg_update);

#endif // CLOCK_H
