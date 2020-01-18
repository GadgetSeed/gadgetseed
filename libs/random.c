/** @file
    @brief	疑似乱数発生

    @date	2019.10.14
    @author	Takashi SHUDO
*/

#include "random.h"

#ifdef GSC_LIB_ENABLE_MT19937AR
#include "mt19937ar.h"
#else
#include <stdlib.h>
#endif

void init_random(unsigned int seed)
{
#ifdef GSC_LIB_ENABLE_MT19937AR
	init_genrand(seed);
#else
	srand(seed);
#endif
}

unsigned int gen_random(void)
{
	unsigned int rt = 0;

#ifdef GSC_LIB_ENABLE_MT19937AR
	rt = genrand_int32();
#else
	rt = rand();
#endif

	return rt;
}
