#ifndef __ARCH_SYS_ARCH_H__
#define __ARCH_SYS_ARCH_H__

typedef u32_t sys_prot_t;

struct sys_sem;
typedef struct sys_sem *sys_sem_t;

struct sys_mutex;
typedef struct sys_mutex *sys_mutex_t;

struct sys_mbox;
typedef struct sys_mbox *sys_mbox_t;

struct sys_thread;
typedef struct sys_thread * sys_thread_t;

#endif /* __ARCH_SYS_ARCH_H__ */
