include $(PRJ_DIR)/common.mk

LIB	= kernel.a

OBJS	= init.o \
	  device.o \
	  timer.o \
	  datetime.o \
	  sysevent.o \
	  console.o \
	  tkprintf.o

ifneq ($(ARCH),emu)
	OBJS += syscalls.o
endif

ifeq ($(MEMORY_ENABLE_HEAP_MEMORY),YES)
	OBJS += memory.o
endif

.c.o:
	$(CC) $(CFLAGS) -c $<

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND) memtest

memtest: memory.c
	gcc -DTEST -I$(PRJ_DIR)/include -o memtest memory.c

include $(PWD)/rules.mk

-include $(DEPEND)
