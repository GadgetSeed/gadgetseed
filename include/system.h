/** @file
    @brief	システム固有初期化関連

    @date	2011.05.01
    @author	Takashi SHUDO
*/

#ifndef SYSTEM_H
#define SYSTEM_H

void init_sect(void);
void init_system(int *argc, char ***argv);
void init_system_drivers(void);
void init_system_process(void);
void reset_system(void);

#endif // SYSTEM_H
