/** @file
    @brief	startup

    @date	2013.02.13
    @author	Takashi SHUDO
*/

extern void init_cpu(void);
extern void init_sect(void);
extern void main(void);

void start(void)
{
	init_sect();
	init_cpu();
	main();
}
