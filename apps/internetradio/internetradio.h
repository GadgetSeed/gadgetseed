/** @file
    @brief	Internet radio アプリケーション

    @date	2018.09.02
    @auther	Takashi SHUDO
*/

#ifndef INTERNETRADIO_H
#define INTERNETRADIO_H

void set_ir_event(unsigned char event);
int open_internetradio(uchar *url);
int close_internetradio(void);

#endif // INTERNETRADIO_H
