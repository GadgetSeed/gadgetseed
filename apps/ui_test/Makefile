include $(PRJ_DIR)/common.mk

LIB	= ui_test.a

OBJS	= ui_test.o

CFLAGS += -I$(PRJ_DIR)/$(UILIB_DIR)
CFLAGS += -I$(PRJ_DIR)/$(LWIP_DIR) -I$(PRJ_DIR)/$(LWIP_DIR)/lwip-$(LWIP_VER)/src/include

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
