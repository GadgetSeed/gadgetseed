//#define TEST
#ifdef TEST
#include <stdio.h>
#include <stdlib.h>
#endif

#include <math.h>

#include "fft.h"

void fft(FFTACC ar[], FFTACC ai[], int n, int iter, int flag)
{
	int i, it, j, j1, j2, k, xp, xp2;
	FFTACC arg, dr1, dr2, di1, di2, tr, ti, w, wr, wi;

	if(n < 2)
	{
#ifdef TEST
		fprintf(stderr, "Error : n < 2  in fft1()\n");
#endif
		return;
	}
	if(iter <= 0)
	{
		iter = 0;
		i = n;
		while((i /= 2) != 0)	iter++;
	}
	j = 1;
	for(i = 0; i < iter; i++)	j *= 2;
	if(n != j)
	{
#ifdef TEST
		fprintf(stderr, "Error : n != 2 ^ k  in fft1()\n");
#endif
		return;
	}
	w = (flag? M_PI: -M_PI) / (FFTACC)n;
	xp2 = n;
	for(it = 0; it < iter; it++)
	{
		xp = xp2;
		xp2 /= 2;
		w *= 2;
		for(k = 0, i = - xp; k < xp2; i++)
		{
			wr = FFTCOS(arg = w * k++);
			wi = FFTSIN(arg);
			for(j = xp; j <= n; j += xp)
			{
				j2 = (j1 = j + i) + xp2;
				tr = (dr1 = ar[j1]) - (dr2 = ar[j2]);
				ti = (di1 = ai[j1]) - (di2 = ai[j2]);
				ar[j1] = dr1 + dr2;
				ai[j1] = di1 + di2;
				ar[j2] = tr * wr - ti * wi;
				ai[j2] = ti * wr + tr * wi;
			}
		}
	}
	j = j1 = n / 2;
	j2 = n - 1;
	for(i = 1; i < j2; i++)
	{
		if(i < j)
		{
			w = ar[i];
			ar[i] = ar[j];
			ar[j] = w;
			w = ai[i];
			ai[i] = ai[j];
			ai[j] = w; 
		}
		k = j1;
		while(k <= j)
		{
			j -= k;
			k /= 2;
		}
		j += k;
	}
	if(flag == 0)	return;
	w = 1. / (FFTACC)n;
	for(i = 0; i < n; i++)
	{
		ar[i] *= w;
		ai[i] *= w;
	}
	return;
}

#ifdef TEST

static FFTACC ar[MAX_N] = { 0., 0., 0., 1., 1., 0., 0., 0.};
static FFTACC ai[MAX_N] = { 0., 0., 0., 0., 0., 0., 0., 0.};
static short pw[MAX_N];

void print_a(FFTACC *ar, FFTACC *ai, int n)
{
	int i;
	FFTACC *p, *q;

	printf("    ar              ai\n");
	for(i = 0, p = ar, q = ai; i < n; i++) {
		printf("%5d %8.3f %8.3f\n", i, *p++, *q++);
	}
}

void print_p(short *pw, int n)
{
	int i;

	printf("pw\n");
	for(i=0; i<n; i++) {
		printf("%5d %5d\n", i, *pw++);
	}
}

int main(int argc, char *argv[])
{
	int i;
	int n = MAX_N;

	init_window_table();

	for(i=0; i<MAX_N; i++) {
		//ar[i] = (i % 32);
		ar[i] = FFTSIN((FFTACC)i * M_PI / 18.0) * 10000;
		ai[i] = 0;
	}
	
	print_a(ar, ai, n);

	window_func(ar, n);
//	print_a(ar, ai, n);

	fft(ar, ai, n, 10, 0);
	print_a(ar, ai, n);

	calc_pw(ar, ai, pw, n);
	print_p(pw, n);

//	fft(ar, ai, n, 10, 1);
//	print_a(ar, ai, n);
}
#endif
