/** @file
    @brief	セクション初期化

    @date	2007.07.14
    @author	Takashi SHUDO
*/

extern int *BSS_START;
extern int *BSS_END;
extern int *DATAROM_START;
extern int *DATARAM_START;
extern int *DATARAM_END;

void init_sect(void)
{
	int *s, *d;

	for(d=BSS_START; d<BSS_END; d++) {
		*d = 0;
	}

	for(s=DATAROM_START, d=DATARAM_START; d<DATARAM_END; s++, d++) {
		*d = *s;
	}
}
