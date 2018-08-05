include $(PRJ_DIR)/common.mk

LIB	= arch.a

OBJS	+= \
	interrupt.o \
	initsect.o \
	initsystem.o \
	dispatch.o \
	emu_task.o

SRCS	= \
	interrupt.c \
	initsect.c \
	initsystem.c \
	dispatch.c \
	emu_task.c

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
