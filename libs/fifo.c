/** @file
    @brief	汎用リングバッファ

    @date	2017.09.07
    @date	2002.03.24
    @author	Takashi SHUDO
*/

#include "fifo.h"

/**
   @brief	fifoを初期化する

   @param[in] fp	初期化するfifo
   @param[in] buf	fifoとなるバッファポインタ
   @param[in] size	バッファのバイト数

   @note	バッファ内に保存できるデータは size-1 となる
*/
void init_fifo(struct st_fifo *fp, unsigned char *buf, unsigned int size)
{
	fp->buf = buf;
	fp->size = size;
	fp->wp = buf;
	fp->rp = buf;
}

/**
   @brief fifoにデータを書き込む

   @param[in]	fp	データが書き込まれるfifo
   @param[in]	data	書き込むデータ
   @param[in]	length	書き込むデータの長さ

   @return	書き込みサイズ
*/
int write_fifo(struct st_fifo *fp, unsigned char *data, unsigned int length)
{
	unsigned int i;

	if(fifo_free_size(fp) < length) {
		return 0;
	}

	for(i=0; i<length; i++) {
		unsigned char *np;

		np = fp->wp + 1;
		if(np >= &fp->buf[fp->size]) {
			np = fp->buf;
		}

		if(np != fp->rp) {
			*fp->wp = *data;
			data ++;
			fp->wp = np;
		} else {
			return (int)i;	// バッファフル
		}
	}

	return (int)i;
}

/**
   @brief fifoからデータを読み出す

   @param[in]	fp	データが読み出されるfifo
   @param[out]	data	読み出されるデータのポインタ
   @param[in]	length	読み出されるデータの長さ

   @return	読み出しサイズ
*/
int read_fifo(struct st_fifo *fp, unsigned char *data, unsigned int length)
{
	unsigned int i;

	for(i=0; i<length; i++) {
		unsigned char *np;

		if(fp->rp != fp->wp) {
			*data = *fp->rp;
			data ++;

			np = fp->rp + 1;
			if(np >= &fp->buf[fp->size]) {
				np = fp->buf;
			}
			fp->rp = np;
		} else {
			return (int)i;	// バッファエンプティ
		}
	}

	return (int)i;
}

/**
   @brief fifoに書き込まれているデータを全て消去する

   @param[in]	fp	データのサイズを読み出すfifo
*/
void clear_fifo(struct st_fifo *fp)
{
	fp->wp = fp->buf;
	fp->rp = fp->buf;
}

/**
   @brief fifoに書き込まれているデータのサイズを返す

   @param[in]	fp	データのサイズを読み出すfifo

   @return	fifoに書き込まれているデータサイズ
*/
unsigned int fifo_size(struct st_fifo *fp)
{
	if(fp->wp >= fp->rp) {
		return (unsigned int)(fp->wp - fp->rp);
	} else {
		return (unsigned int)(fp->size - (fp->rp - fp->wp));
	}
}

/**
   @brief fifoに書き込み可能なデータのサイズを返す

   @param[in]	fp	データのサイズを読み出すfifo

   @return	fifoに書き込み可能なデータサイズ
*/
unsigned int fifo_free_size(struct st_fifo *fp)
{
	if(fp->wp >= fp->rp) {
		return (unsigned int)(fp->size - (fp->wp - fp->rp) - 1);
	} else {
		return (unsigned int)((fp->rp - fp->wp) - 1);
	}
}

/**
   @brief fifoからデータを捨てる

   @param[in]	fp	データが捨てられるfifo
   @param[in]	length	捨てるデータの長さ

   @return	捨てたサイズ
*/
int drop_fifo(struct st_fifo *fp, unsigned int length)
{
	unsigned int i;

	for(i=0; i<length; i++) {
		unsigned char *np;

		if(fp->rp != fp->wp) {
			np = fp->rp + 1;
			if(np >= &fp->buf[fp->size]) {
				np = fp->buf;
			}
			fp->rp = np;
		} else {
			return (int)i;	// バッファエンプティ
		}
	}

	return (int)i;
}
