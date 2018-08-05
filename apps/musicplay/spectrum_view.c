/** @file
    @brief	スペクトラム・アナライザ表示

    @date	2017.07.23
    @auther	Takashi SHUDO
*/

#include "graphics.h"
#include "musicplay_view.h"
#include "spectrum_view.h"

#define POS_X	0

#if GSC_GRAPHICS_DISPLAY_WIDTH >= 800
#define X_SCALE	256
#define Y_SCALE	128
#define POS_Y	200
#else
#define X_SCALE	128
#define Y_SCALE	32
#define POS_Y	144
#endif

#define Y_DEV	(256/Y_SCALE)
#define X_DEV	(SPA_ANA_SMP/X_SCALE)

static void draw_1ch_spectrum(short px, short py, unsigned char *pow)
{
	int i;

	for(i=0; i<SPA_ANA_SMP; i+=X_DEV) {
		int j;
		int pv = 0;

		for(j=0; j<X_DEV; j++) {
			if(pow != 0) {
				pv += pow[i+j];
			} else {
				pv = 0;
			}
		}
		pv = pv/X_DEV;

		set_forecolor(RGB(0,0,50));
		draw_v_line(px+(i/X_DEV), py, Y_SCALE - pv/Y_DEV - 1);

		set_forecolor(RGB(0,255,255));
		draw_point(px+(i/X_DEV), py + Y_SCALE - pv/Y_DEV - 1);

		set_forecolor(RGB(30,100,50));
		draw_v_line(px+(i/X_DEV), py + Y_SCALE - pv/Y_DEV, pv/Y_DEV);
	}
}

void draw_spectrum(struct st_audio_spectrum *asp)
{
	unsigned char *pow[2] = { 0, 0 };

	if(asp != 0) {
		pow[0] = &(asp->spectrum[0]);
		pow[1] = &(asp->spectrum[SPA_ANA_SMP]);
	}

	draw_1ch_spectrum(POS_X, POS_Y, pow[0]);
	draw_1ch_spectrum(POS_X+8+X_SCALE, POS_Y, pow[1]);
}
