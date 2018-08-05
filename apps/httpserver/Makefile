include $(PRJ_DIR)/common.mk

LIB	= httpserver.a

OBJS	= httpserver.o httpserver-netconn.o

CFLAGS += -I$(PRJ_DIR)/$(LWIP_DIR) -I$(PRJ_DIR)/$(LWIP_DIR)/lwip-2.0.3/src/include

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
