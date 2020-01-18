/** @file
    @brief	Internet radio socket

    @date	2018.10.27
    @auther	Takashi SHUDO
*/

#ifndef IR_SOCKET_H
#define IR_SOCKET_H

#include "str.h"

#define MAX_URL_STR_LEN	64

void perse_internet_radio_url(const uchar *url_path, uchar *url, uchar *port, uchar *path);
int open_internet_radio(const uchar *url_path, const uchar *port);
void close_internet_radio(void);
int write_intrnet_radio(char *data, unsigned int len);
int read_intrnet_radio(char *data, unsigned int len);

#endif // IR_SOCKET_H
