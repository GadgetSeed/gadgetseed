/** @file
    @brief	疑似乱数発生

    @date	2008.01.09
    @author	Takashi SHUDO
*/

#ifndef RANDOM_H
#define RANDOM_H

extern void init_genrand(unsigned long s);
extern unsigned long genrand_int32(void);

#endif // RANDOM_H
