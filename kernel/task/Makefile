include $(PRJ_DIR)/common.mk

CFLAGS	+= -iquote $(PRJ_DIR)/include -iquote $(PRJ_DIR)/include/task -DARCH=$(ARCH)

LIB	= task.a

OBJS	= task.o \
	  queue.o \
	  sleepqueue.o \
	  waitqueue.o \
	  event.o \
	  mutex.o \
	  syscall.o \
	  syscall_api.o

ifeq ($(KERNEL_ENABLE_CALLTRACE),YES)
	OBJS += calltrace.o
endif

.c.o:
	$(CC) $(CFLAGS) -c $<

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
