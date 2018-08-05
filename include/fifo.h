/** @file
    @brief	汎用リングバッファ

    @date	2017.09.07
    @date	2002.03.24
    @author	Takashi SHUDO
*/

#ifndef	FIFO_H
#define	FIFO_H

struct st_fifo {
	unsigned char	*buf;	///< バッファのポインタ
	unsigned int	size;	///< バッファのサイズ
	unsigned char	*wp;	///< 書き込みデータのポインタ
	unsigned char	*rp;	///< 読み込みデータのポインタ
};	///< 汎用FIFO

extern void init_fifo(struct st_fifo *fp, unsigned char *buf, unsigned int size);
extern int write_fifo(struct st_fifo *fp, unsigned char *data, unsigned int length);
extern int read_fifo(struct st_fifo *fp, unsigned char *data, unsigned int length);
extern void clear_fifo(struct st_fifo *fp);
extern unsigned int fifo_size(struct st_fifo *fp);
extern unsigned int fifo_free_size(struct st_fifo *fp);
extern int drop_fifo(struct st_fifo *fp, unsigned int length);

#endif	// FIFO_H
