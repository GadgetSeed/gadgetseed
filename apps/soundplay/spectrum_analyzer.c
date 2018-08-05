/** @file
    @brief	スペクトラム・アナライザ

    @date	2017.07.09
    @auther	Takashi SHUDO
*/

#include "spectrum_analyzer.h"
#include "graphics.h"
#include "fft.h"
#include <math.h>

static FFTACC ar[SPA_SMP_NUM];
static FFTACC ai[SPA_SMP_NUM];
static FFTACC window_table[SPA_SMP_NUM];

void init_window_table(void)
{
	int i;

	for(i=0; i<SPA_SMP_NUM; i++) {
//		window_table[i] = 0.54 - (0.46 * cos(2 * M_PI * i/SPA_SMP_NUM));
		window_table[i] = 0.42 - 0.5 * cos(2 * M_PI * i/SPA_SMP_NUM) + 0.08 * cos(4 * M_PI * i/SPA_SMP_NUM);
	}
}

static void window_func(FFTACC *src, int n)
{
	int i;
	FFTACC *s=src;

	for(i=0; i<n; i++) {
		*s = *s * window_table[i];
		s ++;
	}
}

#define GAMMA	2.0

static void calc_pw(FFTACC *ar, FFTACC *ai, unsigned char *pw, int n)
{
	int i;
	FFTACC *par = ar;
	FFTACC *pai = ai;
	FFTACC dpw;
	long ipw;
	unsigned char *ppw = pw;

	for(i=0; i<n; i++) {
		dpw = (*par * *par) + (*pai * *pai);
		if(dpw != 0.0) {
			ipw = (20 * FFTLOG10(dpw));
			ipw = ((FFTACC)ipw) * ((((FFTACC)n) + (FFTACC)i*0.5)/n); // それっぽくなるように係数を掛け算
		} else {
			ipw = 0;
		}

		ipw = FFTPOW(1.0 * ipw/255, GAMMA) * 255;

		//ipw = (window_table[i*2] * 256); // Debug View Window
		//ipw = ar[i*2]/32; // Debug View AR

		// clip
		if(ipw > 255) {
			ipw = 255;
		}
		if(ipw < 0) {
			ipw = 0;
		}

		*ppw = ipw;

		par ++;
		pai ++;
		ppw ++;
	}
}

void proc_spectrum_analyse(unsigned char *spc, short *buf)
{
	int i;

	for(i=0; i<SPA_SMP_NUM * sizeof(short); i+=2) {
		ar[i/2] = buf[i];
		//ar[i/2] = 0.0;//!!! DEBUG
		ai[i/2] = 0.0;
	}

#ifdef VIEW_LOW_WAVE
	for(i=0; i<SPA_ANA_SMP; i++) {
		spc[i] = (buf[i] / 256) + 128;
	}
#else
	window_func(ar, SPA_SMP_NUM);
	fft(ar, ai, SPA_SMP_NUM, 0, 0);
	calc_pw(ar, ai, spc, SPA_ANA_SMP);
#endif
}
