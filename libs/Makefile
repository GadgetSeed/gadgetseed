include $(PRJ_DIR)/common.mk

LIB	= libs.a

OBJS	= fifo.o \
	  str.o \
	  tprintf.o \
	  vtprintf.o \
	  crc.o \
	  charcode.o

ifeq ($(LIB_ENABLE_RANDOM),YES)
ifeq ($(LIB_ENABLE_MT19937AR),YES)
	CFLAGS += -I$(PRJ_DIR)/$(EXTLIBS_DIR)/mt19937ar
endif
	OBJS += random.o
endif

.c.o:
	$(CC) $(CFLAGS) -c $<

all:	$(LIB)

$(LIB): $(OBJS) $(MAKECONFIGSYS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

.PHONY: distclean
distclean:
	make clean

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
