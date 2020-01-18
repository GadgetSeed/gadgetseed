include $(PRJ_DIR)/common.mk

LIB	= internetradio.a

OBJS	= internetradio.o shoutcast.o ir_socket.o ir_stream.o m3u.o pls.o

CFLAGS += -I$(PRJ_DIR)/$(UILIB_DIR) -I$(PRJ_DIR)/$(LWIP_DIR) -I$(PRJ_DIR)/$(LWIP_DIR)/lwip-$(LWIP_VER)/src/include -I$(PRJ_DIR)/apps/soundplay

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
