include $(PRJ_DIR)/common.mk

LIB	= net.a

LWIP	= $(PRJ_DIR)/$(LWIP_DIR)/lwip-$(LWIP_VER)

CFLAGS	+= -I. -I$(PRJ_DIR)/$(LWIP_DIR) -I$(LWIP)/src/include -I$(LWIP)/src/include/ipv4

OBJS	= \
	nettask.o \
	sys_arch.o \
	devif.o

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

distclean:
	make clean

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
