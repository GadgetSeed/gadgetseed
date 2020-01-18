/** @file
    @brief	Internet radio packet ストリーム

    @date	2018.09.19
    @auther	Takashi SHUDO
*/

#ifndef IR_STREAM_H
#define IR_STREAM_H

#include "str.h"

#define STREAM_FILENAME	"2:pipe0"

#define EVT_IRADIO_DISCONNECTED	101
#define EVT_IRADIO_CONNECTED	102
#define EVT_IRADIO_RECEIVE	103
#define EVT_IRADIO_ERROR	104
#define EVT_IRADIO_ABORT	105

typedef enum {
	DISCONNECTED,		// 未接続
	CONNECTING,		// 接続中
	RECEIVING,		// 受信中
	STREAM_STAT_MAX
} stream_stat;

void init_ir_stream(void);
void internet_radio_proc(void);

int connect_internet_radio(const uchar *url, const uchar *port, const uchar *path, const uchar *fname);
int reconnect_internet_radio(void);
int disconnect_internet_radio(void);

stream_stat internet_radio_status(void);
int internet_radio_stream_size(void);

#endif // IR_STREAM_H
