/** @file
    @brief	メモリ管理

    @date	2008.03.18
    @author	Takashi SHUDO
*/

#ifndef MEMORY_H
#define MEMORY_H

extern void init_memory(void);
extern void display_memory_info(void);
extern void * alloc_memory(unsigned int size);
extern void free_memory(void *ptr);
extern unsigned long memory_size(void *ptr);
extern unsigned long heap_free_size(void);
extern unsigned long heap_total_size(void);

#endif // MEMORY_H
