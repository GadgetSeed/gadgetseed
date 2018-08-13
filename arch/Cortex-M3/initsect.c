/** @file
    @brief	セクション初期化

    @date	2007.07.14
    @author	Takashi SHUDO
*/

extern long *BSS_START;
extern long *BSS_END;
extern long *DATAROM_START;
extern long *DATARAM_START;
extern long *DATARAM_END;

void init_sect(void)
{
	long *s, *d;

	for(d=BSS_START; d<BSS_END; d++) {
		*d = 0;
	}

	for(s=DATAROM_START, d=DATARAM_START; d<DATARAM_END; s++, d++) {
		*d = *s;
	}
}
