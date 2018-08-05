#ifndef FFT_H
#define FFT_H

#ifdef FFT_DOUBLE
#define FFTACC	double
#define FFTSIN	sin
#define FFTCOS	cos
#define FFTPOW	pow
#define FFTLOG10	log10
#else
#define FFTACC	float
#define FFTSIN	sinf
#define FFTCOS	cosf
#define FFTPOW	powf
#define FFTLOG10	log10f
#endif

void fft(FFTACC ar[], FFTACC ai[], int n, int iter, int flag);

#endif // FFT_H
